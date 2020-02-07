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

#ifndef HARBOUR_SYSTEM_TIME_H
#define HARBOUR_SYSTEM_TIME_H

#include <QObject>
#include <QSharedPointer>

class QQmlEngine;
class QJSEngine;

class HarbourSystemTime: public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(HarbourSystemTime)
    Q_PROPERTY(QString notification READ emptyString NOTIFY notify)

public:
    explicit HarbourSystemTime(QObject* aParent = NULL);
    ~HarbourSystemTime();

    // Dummy getter for notification property which can be used to
    // force re-evaluation of a JavaScript expression. As its name
    // suggests, always returns empty string.
    static QString emptyString();

    // Callback for qmlRegisterSingletonType<HarbourSystemTime>
    static QObject* createSingleton(QQmlEngine* aEngine, QJSEngine* aScript);

    // Shared instance for use by native code
    static QSharedPointer<HarbourSystemTime> sharedInstance();

Q_SIGNALS:
    // Recipient must be ready to receive multiple notifications for
    // a single time or timezone change.
    void preNotify(); // For Date.timeZoneUpdated()
    void notify();    // For everything else

private:
    class Private;
    Private* iPrivate;
};

#endif // HARBOUR_SYSTEM_TIME_H
