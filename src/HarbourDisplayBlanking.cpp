/*
 * Copyright (C) 2015-2024 Slava Monich <slava@monich.com>
 * Copyright (C) 2015-2019 Jolla Ltd.
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer
 *     in the documentation and/or other materials provided with the
 *     distribution.
 *
 *  3. Neither the names of the copyright holders nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation
 * are those of the authors and should not be interpreted as representing
 * any official policies, either expressed or implied.
 */

#include "HarbourDisplayBlanking.h"
#include "HarbourDebug.h"
#include "HarbourMce.h"

#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>

#include <QTimer>

// ==========================================================================
// HarbourDisplayBlanking::Private
// ==========================================================================

class HarbourDisplayBlanking::Private : public HarbourMce {
    Q_OBJECT

public:
    enum {
        MIN_REPEAT_INTERVAL_SEC = 1,
        DEFAULT_REPEAT_INTERVAL_SEC = 50
    };

    static const QString BLANK_ACTIVE;
    static const QString BLANK_INACTIVE;
    static QWeakPointer<HarbourDisplayBlanking> sSharedInstance;

    bool iPaused;
    bool iPauseAllowed;
    bool iPauseRequested;
    bool iPauseActive;
    int iRequestIntervalSec;
    QTimer* iRepeatTimer;

    Private(HarbourDisplayBlanking*);

    void setPauseRequested(bool);
    void setRequestInterval(int);

private:
    HarbourDisplayBlanking* parentObject() const;
    void updateDisplayBlankingPause(QString);
    void updateDisplayBlankingAllowed(bool);
    void checkPause();
    void requestPause();
    void cancelPause();

public Q_SLOTS:
    void onDisplayBlankingPauseChanged(QString);
    void onDisplayBlankingPauseAllowedChanged(bool);
    void onDisplayBlankingPauseQueryDone(QDBusPendingCallWatcher*);
    void onDisplayBlankingPauseAllowedQueryDone(QDBusPendingCallWatcher*);
    void onRepeatBlankingRequest();
};

const QString HarbourDisplayBlanking::Private::BLANK_ACTIVE("active");
const QString HarbourDisplayBlanking::Private::BLANK_INACTIVE("inactive");
QWeakPointer<HarbourDisplayBlanking> HarbourDisplayBlanking::Private::sSharedInstance;

HarbourDisplayBlanking::Private::Private(
    HarbourDisplayBlanking* aParent) :
    HarbourMce(aParent),
    iPaused(false),
    iPauseAllowed(false),
    iPauseRequested(false),
    iPauseActive(false),
    iRequestIntervalSec(DEFAULT_REPEAT_INTERVAL_SEC),
    iRepeatTimer(NULL)
{
    HDEBUG("created");
    setupProperty("get_display_blanking_pause", "display_blanking_pause_ind",
        SLOT(onDisplayBlankingPauseQueryDone(QDBusPendingCallWatcher*)),
        SLOT(onDisplayBlankingPauseChanged(QString)));
    setupProperty("get_display_blanking_pause_allowed", "display_blanking_pause_allowed_ind",
        SLOT(onDisplayBlankingPauseAllowedQueryDone(QDBusPendingCallWatcher*)),
        SLOT(onDisplayBlankingPauseAllowedChanged(bool)));
}

HarbourDisplayBlanking*
HarbourDisplayBlanking::Private::parentObject() const
{
    return qobject_cast<HarbourDisplayBlanking*>(parent());
}

void
HarbourDisplayBlanking::Private::updateDisplayBlankingPause(
    QString aState)
{
    const bool paused = (aState == BLANK_ACTIVE);
    if (iPaused != paused) {
        iPaused = paused;
        Q_EMIT parentObject()->pausedChanged();
    }
}

void
HarbourDisplayBlanking::Private::updateDisplayBlankingAllowed(
    bool aAllowed)
{
    if (iPauseAllowed != aAllowed) {
        iPauseAllowed = aAllowed;
        Q_EMIT parentObject()->pauseAllowedChanged();
        checkPause();
    }
}

void
HarbourDisplayBlanking::Private::onDisplayBlankingPauseChanged(
    QString aState)
{
    HDEBUG(aState);
    updateDisplayBlankingPause(aState);
}

void
HarbourDisplayBlanking::Private::onDisplayBlankingPauseAllowedChanged(
    bool aAllowed)
{
    HDEBUG(aAllowed);
    updateDisplayBlankingAllowed(aAllowed);
}

void
HarbourDisplayBlanking::Private::onDisplayBlankingPauseQueryDone(
    QDBusPendingCallWatcher* aWatcher)
{
    QDBusPendingReply<QString> reply(*aWatcher);
    if (reply.isValid() && !reply.isError()) {
        HDEBUG(reply);
        updateDisplayBlankingPause(reply.value());
    } else {
        HWARN(reply.error());
    }
    aWatcher->deleteLater();
}

void
HarbourDisplayBlanking::Private::onDisplayBlankingPauseAllowedQueryDone(
    QDBusPendingCallWatcher* aWatcher)
{
    QDBusPendingReply<bool> reply(*aWatcher);
    if (reply.isValid() && !reply.isError()) {
        HDEBUG(reply);
        updateDisplayBlankingAllowed(reply.value());
    } else {
        HWARN(reply.error());
        updateDisplayBlankingAllowed(true);
    }
    aWatcher->deleteLater();
}

void
HarbourDisplayBlanking::Private::setPauseRequested(
    bool aValue)
{
    if (iPauseRequested != aValue) {
        iPauseRequested = aValue;
        HDEBUG(aValue);
        Q_EMIT parentObject()->pauseRequestedChanged();
        checkPause();
    }
}

void
HarbourDisplayBlanking::Private::setRequestInterval(
    int aValue)
{
    const int interval = qMax(int(MIN_REPEAT_INTERVAL_SEC), aValue);
    if (iRequestIntervalSec != interval) {
        iRequestIntervalSec = interval;
        HDEBUG(interval);
        if (iRepeatTimer) {
            iRepeatTimer->setInterval(interval * 1000);
            if (iRepeatTimer->isActive()) {
                iRepeatTimer->start();
            }
        }
        Q_EMIT parentObject()->requestIntervalChanged();
    }
}

void
HarbourDisplayBlanking::Private::checkPause()
{
    const bool active = iPauseRequested && iPauseAllowed;
    if (iPauseActive != active) {
        iPauseActive = active;
        HDEBUG(active);
        if (iPauseActive) {
            if (!iRepeatTimer) {
                iRepeatTimer = new QTimer(this);
                iRepeatTimer->setInterval(iRequestIntervalSec * 1000);
                iRepeatTimer->setSingleShot(false);
                connect(iRepeatTimer, SIGNAL(timeout()),
                    SLOT(onRepeatBlankingRequest()));
            }
            iRepeatTimer->start();
            requestPause();
        } else {
            iRepeatTimer->stop();
            cancelPause();
        }
    }
}

void
HarbourDisplayBlanking::Private::onRepeatBlankingRequest()
{
    HDEBUG("requesting");
    requestPause();
}

void
HarbourDisplayBlanking::Private::requestPause()
{
    request("req_display_blanking_pause");
}

void
HarbourDisplayBlanking::Private::cancelPause()
{
    request("req_display_cancel_blanking_pause");
}

// ==========================================================================
// HarbourDisplayBlanking
// ==========================================================================

HarbourDisplayBlanking::HarbourDisplayBlanking(
    QObject* aParent) :
    QObject(aParent),
    iPrivate(new Private(this))
{
    HDEBUG("created");
}

HarbourDisplayBlanking::~HarbourDisplayBlanking()
{
    HDEBUG("destroyed");
}

// Callback for qmlRegisterSingletonType<HarbourDisplayBlanking>
QObject*
HarbourDisplayBlanking::createSingleton(
    QQmlEngine*,
    QJSEngine*)
{
    return new HarbourDisplayBlanking();
}

QSharedPointer<HarbourDisplayBlanking>
HarbourDisplayBlanking::sharedInstance()
{
    QSharedPointer<HarbourDisplayBlanking> instance = Private::sSharedInstance;
    if (instance.isNull()) {
        // QObject::deleteLater protects against trouble in case if the
        // recipient of the signal drops the last shared reference.
        instance = QSharedPointer<HarbourDisplayBlanking>
            (new HarbourDisplayBlanking, &QObject::deleteLater);
        Private::sSharedInstance = instance;
    }
    return instance;
}

bool
HarbourDisplayBlanking::paused() const
{
    return iPrivate->iPaused;
}

bool
HarbourDisplayBlanking::pauseAllowed() const
{
    return iPrivate->iPauseAllowed;
}

bool
HarbourDisplayBlanking::pauseRequested() const
{
    return iPrivate->iPauseRequested;
}

void
HarbourDisplayBlanking::setPauseRequested(
    bool aValue)
{
    iPrivate->setPauseRequested(aValue);
}

int
HarbourDisplayBlanking::requestInterval() const
{
    return iPrivate->iRequestIntervalSec;
}

void
HarbourDisplayBlanking::setRequestInterval(
    int aValue)
{
    return iPrivate->setRequestInterval(aValue);

}

#include "HarbourDisplayBlanking.moc"
