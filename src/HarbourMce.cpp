/*
 * Copyright (C) 2018 Jolla Ltd.
 * Copyright (C) 2018 Slava Monich <slava.monich@jolla.com>
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

#include "HarbourMce.h"

#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>

const QString HarbourMce::MCE_SERVICE("com.nokia.mce");
const QString HarbourMce::MCE_REQUEST_PATH("/com/nokia/mce/request");
const QString HarbourMce::MCE_REQUEST_INTERFACE("com.nokia.mce.request");
const QString HarbourMce::MCE_SIGNAL_PATH("/com/nokia/mce/signal");
const QString HarbourMce::MCE_SIGNAL_INTERFACE("com.nokia.mce.signal");

HarbourMce::HarbourMce(QObject* aParent) :
    QObject(aParent)
{
}

QDBusConnection
HarbourMce::bus()
{
    return QDBusConnection::systemBus();
}

QDBusPendingCall
HarbourMce::request(
    QString aMethod)
{
    return bus().asyncCall(QDBusMessage::createMethodCall(MCE_SERVICE,
        MCE_REQUEST_PATH, MCE_REQUEST_INTERFACE, aMethod));
}

void
HarbourMce::setupProperty(
    QString aQueryMethod,
    QString aSignal,
    const char* aQuerySlot,
    const char* aSignalSlot)
{
    bus().connect(MCE_SERVICE, MCE_SIGNAL_PATH, MCE_SIGNAL_INTERFACE,
        aSignal, this, aSignalSlot);
    connect(new QDBusPendingCallWatcher(request(aQueryMethod), this),
        SIGNAL(finished(QDBusPendingCallWatcher*)), this, aQuerySlot);
}
