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

#include "HarbourSystemState.h"
#include "HarbourDebug.h"
#include "HarbourMce.h"

#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusPendingCallWatcher>
#include <QtDBus/QDBusPendingReply>

const QString HarbourSystemState::MCE_DISPLAY_ON("on");
const QString HarbourSystemState::MCE_DISPLAY_DIM("dimmed");
const QString HarbourSystemState::MCE_DISPLAY_OFF("off");
const QString HarbourSystemState::MCE_TK_LOCKED("locked");
const QString HarbourSystemState::MCE_TK_UNLOCKED("unlocked");

// ==========================================================================
// HarbourSystemState::Private
// ==========================================================================
class HarbourSystemState::Private :
    public HarbourMce
{
    Q_OBJECT

public:
    static QWeakPointer<HarbourSystemState> sSharedInstance;
    HarbourSystemState* iParent;
    QString iDisplayStatus;
    QString iLockMode;

    Private(HarbourSystemState*);

    bool displayOff() const;
    bool locked() const;

private:
    void setDisplayStatus(const QString&);
    void setLockMode(const QString&);

public Q_SLOTS:
    void onDisplayStatusChanged(QString);
    void onDisplayStatusQueryDone(QDBusPendingCallWatcher*);
    void onLockModeChanged(QString);
    void onLockModeQueryDone(QDBusPendingCallWatcher*);
};

QWeakPointer<HarbourSystemState> HarbourSystemState::Private::sSharedInstance;

HarbourSystemState::Private::Private(
    HarbourSystemState* aParent) :
    HarbourMce(aParent),
    iParent(aParent)
{
    HDEBUG("created");
    setupProperty("get_display_status", "display_status_ind",
        SLOT(onDisplayStatusQueryDone(QDBusPendingCallWatcher*)),
        SLOT(onDisplayStatusChanged(QString)));
    setupProperty("get_tklock_mode", "tklock_mode_ind",
        SLOT(onLockModeQueryDone(QDBusPendingCallWatcher*)),
        SLOT(onLockModeChanged(QString)));
}

bool
HarbourSystemState::Private::displayOff() const
{
    return iDisplayStatus == MCE_DISPLAY_OFF;
}

void
HarbourSystemState::Private::setDisplayStatus(
    const QString& aStatus)
{
    if (iDisplayStatus != aStatus) {
        const bool displayWasOff = displayOff();
        iDisplayStatus = aStatus;
        const bool displayIsOff = displayOff();
        Q_EMIT iParent->displayStatusChanged();
        if (displayWasOff != displayIsOff) {
            Q_EMIT iParent->displayOffChanged();
        }
    }
}

void
HarbourSystemState::Private::onDisplayStatusChanged(
    QString aStatus)
{
    HDEBUG(aStatus);
    setDisplayStatus(aStatus);
}

void
HarbourSystemState::Private::onDisplayStatusQueryDone(
    QDBusPendingCallWatcher* aWatcher)
{
    QDBusPendingReply<QString> reply(*aWatcher);
    HDEBUG(reply);
    if (reply.isValid() && !reply.isError()) {
        setDisplayStatus(reply.value());
    }
    aWatcher->deleteLater();
}

bool
HarbourSystemState::Private::locked() const
{
    return iLockMode == MCE_TK_LOCKED;
}

void
HarbourSystemState::Private::setLockMode(
    const QString& aMode)
{
    if (iLockMode != aMode) {
        const bool wasLocked = locked();
        iLockMode = aMode;
        const bool isLocked = locked();
        Q_EMIT iParent->lockModeChanged();
        if (wasLocked != isLocked) {
            Q_EMIT iParent->lockedChanged();
        }
    }
}

void
HarbourSystemState::Private::onLockModeChanged(
    QString aMode)
{
    HDEBUG(aMode);
    setLockMode(aMode);
}

void
HarbourSystemState::Private::onLockModeQueryDone(
    QDBusPendingCallWatcher* aWatcher)
{
    QDBusPendingReply<QString> reply(*aWatcher);
    HDEBUG(reply);
    if (reply.isValid() && !reply.isError()) {
        setLockMode(reply.value());
    }
    aWatcher->deleteLater();
}

// ==========================================================================
// HarbourSystemState
// ==========================================================================

HarbourSystemState::HarbourSystemState(
    QObject* aParent) :
    QObject(aParent),
    iPrivate(new Private(this))
{
    HDEBUG("created");
}

HarbourSystemState::~HarbourSystemState()
{
    HDEBUG("destroyed");
}

// Callback for qmlRegisterSingletonType<HarbourSystemState>
QObject*
HarbourSystemState::createSingleton(
    QQmlEngine*,
    QJSEngine*)
{
    return new HarbourSystemState();
}

QSharedPointer<HarbourSystemState>
HarbourSystemState::sharedInstance()
{
    QSharedPointer<HarbourSystemState> instance = Private::sSharedInstance;
    if (instance.isNull()) {
        // QObject::deleteLater protects against trouble in case if the
        // recipient of the signal drops the last shared reference.
        instance = QSharedPointer<HarbourSystemState>(new HarbourSystemState,
            &QObject::deleteLater);
        Private::sSharedInstance = instance;
    }
    return instance;
}

QString
HarbourSystemState::displayStatus() const
{
    return iPrivate->iDisplayStatus;
}

QString
HarbourSystemState::lockMode() const
{
    return iPrivate->iLockMode;
}

bool
HarbourSystemState::displayOff() const
{
    return iPrivate->displayOff();
}

bool
HarbourSystemState::locked() const
{
    return iPrivate->locked();
}

#include "HarbourSystemState.moc"
