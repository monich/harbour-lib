/*
 * Copyright (C) 2024 Slava Monich <slava@monich.com>
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

#ifndef HARBOUR_WAKEUP_TIMER_H
#define HARBOUR_WAKEUP_TIMER_H

#include <QtCore/QObject>

// This is a drop-in replacement for a single-shot QTimer or QML Timer,
// which also works in the background, e.g. when the screen is off and
// the system is likely to be suspended most of the time.
//
// Its accuracy isn't great but is far better than the plain Qt/QML
// timer with a long interval (tens of seconds or longer) running in
// the background, when a 5 min interval may take hours to trigger.
//
// To use this class, add PKGCONFIG += keepalive to your .pro file

class HarbourWakeupTimer :
    public QObject
{
    Q_OBJECT
    Q_PROPERTY(int interval READ getInterval WRITE setInterval NOTIFY intervalChanged)
    Q_PROPERTY(bool running READ isRunning WRITE setRunning NOTIFY runningChanged)

public:
    HarbourWakeupTimer(QObject* aParent = Q_NULLPTR);
    ~HarbourWakeupTimer();

    // milliseconds
    int getInterval() const;
    void setInterval(int);

    bool isRunning() const;
    void setRunning(bool);

    Q_INVOKABLE void restart();
    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();

Q_SIGNALS:
    void intervalChanged();
    void runningChanged();
    void triggered();

private:
    class Private;
    Private* iPrivate;
};

#endif // HARBOUR_WAKEUP_TIMER_H
