/*
 * Copyright (C) 2020 Jolla Ltd.
 * Copyright (C) 2020 Slava Monich <slava@monich.com>
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer
 *      in the documentation and/or other materials provided with the
 *      distribution.
 *   3. Neither the names of the copyright holders nor the names of its
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
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

#include "HarbourSystemTime.h"
#include "HarbourDebug.h"

#include <QDBusConnection>

#include <gutil_timenotify.h>

// ==========================================================================
// HarbourSystemTime::Private
// ==========================================================================

class HarbourSystemTime::Private : public QObject
{
    Q_OBJECT
public:
    Private(HarbourSystemTime* aSystemTime);
    ~Private();

    static void timeNotify(GUtilTimeNotify*, void*);

public Q_SLOTS:
    void onDBusNotify();
    void notify();

public:
    GUtilTimeNotify* iNotify;
    gulong iNotifyId;
};

HarbourSystemTime::Private::Private(HarbourSystemTime* aParent) :
    QObject(aParent),
    iNotify(gutil_time_notify_new()),
    iNotifyId(gutil_time_notify_add_handler(iNotify, timeNotify, this))
{
    QDBusConnection::systemBus().connect("com.nokia.time", "/com/nokia/time",
        "com.nokia.time", "settings_changed", this, SLOT(onDBusNotify()));
}

HarbourSystemTime::Private::~Private()
{
    gutil_time_notify_remove_handler(iNotify, iNotifyId);
    gutil_time_notify_unref(iNotify);
}

void HarbourSystemTime::Private::timeNotify(GUtilTimeNotify*, void* aSelf)
{
    HDEBUG("System time changed");
    QMetaObject::invokeMethod((QObject*)aSelf, "notify");
}

void HarbourSystemTime::Private::onDBusNotify()
{
    HDEBUG("timed settings changed");
    notify();
}

void HarbourSystemTime::Private::notify()
{
    HarbourSystemTime* obj = qobject_cast<HarbourSystemTime*>(parent());
    Q_EMIT obj->preNotify();    // For Date.timeZoneUpdated()
    Q_EMIT obj->notify();       // For everything else
}

// ==========================================================================
// HarbourSystemTime
// ==========================================================================

HarbourSystemTime::HarbourSystemTime(QObject* aParent) :
    QObject(aParent),
    iPrivate(new Private(this))
{
    HDEBUG("created");
}

HarbourSystemTime::~HarbourSystemTime()
{
    HDEBUG("deleted");
    delete iPrivate;
}

// Callback for qmlRegisterSingletonType<HarbourSystemTime>
QObject* HarbourSystemTime::createSingleton(QQmlEngine*, QJSEngine*)
{
    return new HarbourSystemTime();
}

// Getter for notification property which can be used to force
// re-evaluation of a JavaScript expression. Always returns an
// empty string.
QString HarbourSystemTime::emptyString()
{
    return QString();
}

QSharedPointer<HarbourSystemTime> HarbourSystemTime::sharedInstance()
{
    static QWeakPointer<HarbourSystemTime> sSharedInstance;
    QSharedPointer<HarbourSystemTime> instance = sSharedInstance;
    if (instance.isNull()) {
        // QObject::deleteLater protects against trouble in case if the
        // recipient of the signal drops the last shared reference.
        sSharedInstance = instance = QSharedPointer<HarbourSystemTime>
            (new HarbourSystemTime, &QObject::deleteLater);
    }
    return instance;
}

#include "HarbourSystemTime.moc"
