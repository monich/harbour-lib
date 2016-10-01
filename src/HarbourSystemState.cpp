/*
 * Copyright (C) 2015-2016 Jolla Ltd.
 * Contact: Slava Monich <slava.monich@jolla.com>
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   - Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   - Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   - Neither the name of Jolla Ltd nor the names of its contributors
 *     may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "HarbourSystemState.h"
#include "HarbourDebug.h"

#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>

#define MCE_SERVICE "com.nokia.mce"

const QString HarbourSystemState::MCE_DISPLAY_ON("on");
const QString HarbourSystemState::MCE_DISPLAY_DIM("dimmed");
const QString HarbourSystemState::MCE_DISPLAY_OFF("off");
const QString HarbourSystemState::MCE_TK_LOCKED("locked");
const QString HarbourSystemState::MCE_TK_UNLOCKED("unlocked");

// ==========================================================================
// HarbourSystemState::Private
// ==========================================================================
class HarbourSystemState::Private : public QObject
{
    Q_OBJECT

public:
    static QWeakPointer<HarbourSystemState> sSharedInstance;
    HarbourSystemState* iParent;
    QString iDisplayStatus;
    QString iLockMode;

    Private(HarbourSystemState* aParent);

private:
    void setupProperty(QString aQueryMethod, QString aSignal,
        const char* aQuerySlot, const char* aSignalSlot);
    void setDisplayStatus(QString aStatus);
    void setLockMode(QString aStatus);

public Q_SLOTS:
    void onDisplayStatusChanged(QString);
    void onDisplayStatusQueryDone(QDBusPendingCallWatcher*);
    void onLockModeChanged(QString);
    void onLockModeQueryDone(QDBusPendingCallWatcher*);
};

QWeakPointer<HarbourSystemState> HarbourSystemState::Private::sSharedInstance;

HarbourSystemState::Private::Private(
    HarbourSystemState* aParent) :
    QObject(aParent),
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

void
HarbourSystemState::Private::setupProperty(
    QString aQueryMethod,
    QString aSignal,
    const char* aQuerySlot,
    const char* aSignalSlot)
{
    QDBusConnection systemBus(QDBusConnection::systemBus());
    connect(new QDBusPendingCallWatcher(systemBus.asyncCall(
        QDBusMessage::createMethodCall(MCE_SERVICE, "/com/nokia/mce/request",
        "com.nokia.mce.request", aQueryMethod)), this),
        SIGNAL(finished(QDBusPendingCallWatcher*)), this, aQuerySlot);
    systemBus.connect(MCE_SERVICE, "/com/nokia/mce/signal",
        "com.nokia.mce.signal", aSignal, this, aSignalSlot);
}

void
HarbourSystemState::Private::setDisplayStatus(
    QString aStatus)
{
    if (iDisplayStatus != aStatus) {
        iDisplayStatus = aStatus;
        Q_EMIT iParent->displayStatusChanged();
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

void
HarbourSystemState::Private::setLockMode(
    QString aMode)
{
    if (iLockMode != aMode) {
        iLockMode = aMode;
        Q_EMIT iParent->lockModeChanged();
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
    return iPrivate->iDisplayStatus == MCE_DISPLAY_OFF;
}

bool
HarbourSystemState::locked() const
{
    return iPrivate->iLockMode == MCE_TK_LOCKED;
}

#include "HarbourSystemState.moc"
