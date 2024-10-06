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

#include "HarbourWakeupTimer.h"

#include "HarbourDebug.h"

#include <QtCore/QBasicTimer>
#include <QtCore/QDateTime>

// To use this class, add
// PKGCONFIG += keepalive
// to your .pro file

#include <backgroundactivity.h>

// Debug trace tweaks
#define PRINTABLE_TIME(t) \
    qPrintable((t).toString(QStringLiteral("hh:mm:ss.zzz")))
#define PRINTABLE_ACTIVITY_STATE(a) \
    qPrintable(activityStateString(a))
#if HARBOUR_DEBUG
static
QString
activityStateString(
    BackgroundActivity* iActivity)
{
    switch (iActivity->state()) {
    case BackgroundActivity::Waiting: return QStringLiteral("Waiting");
    case BackgroundActivity::Stopped: return QStringLiteral("Stopped");
    case BackgroundActivity::Running: return QStringLiteral("Running");
    }
    return QString::number(iActivity->state());
}
#  define HDEBUG_(x) HDEBUG(((void*)parentObject()) << x)
#else
#  define HDEBUG_(expr) HDEBUG(expr)
#endif // HARBOUR_DEBUG

// ==========================================================================
// HarbourWakeupTimer::Private
// ==========================================================================

class HarbourWakeupTimer::Private :
    public QObject
{
    Q_OBJECT

public:
    Private(HarbourWakeupTimer*);
    ~Private();

    HarbourWakeupTimer* parentObject() const;
    void setInterval(int);
    void setRunning(bool);
    void stop();
    void start();
    void restart();
    void startWait();
    void startTimer();
    void scheduleWakeup(int);

protected:
    void timerEvent(QTimerEvent*) Q_DECL_OVERRIDE;

private Q_SLOTS:
    void onActivityStateChanged();

public:
    BackgroundActivity* iActivity;
    QDateTime iExpirationTime;
    QBasicTimer iTimer;
    int iInterval; // milliseconds
    bool iRunning;
};

HarbourWakeupTimer::Private::Private(
    HarbourWakeupTimer* aTimer) :
    QObject(aTimer),
    iActivity(Q_NULLPTR),
    iInterval(1000),
    iRunning(false)
{
}

HarbourWakeupTimer::Private::~Private()
{
    delete iActivity;
}

inline
HarbourWakeupTimer*
HarbourWakeupTimer::Private::parentObject() const
{
    return qobject_cast<HarbourWakeupTimer*>(parent());
}

void
HarbourWakeupTimer::Private::setInterval(
    int aMilliseconds)
{
    if (iInterval != aMilliseconds) {
        iInterval = aMilliseconds;
        if (iRunning) {
            start();
        }
        Q_EMIT parentObject()->intervalChanged();
    }
}

void
HarbourWakeupTimer::Private::setRunning(
    bool aRunning)
{
    if (aRunning && !iRunning) {
        start();
        Q_EMIT parentObject()->runningChanged();
    } else if (!aRunning && iRunning) {
        stop();
        Q_EMIT parentObject()->runningChanged();
    }
}

void
HarbourWakeupTimer::Private::stop()
{
    iRunning = false;
    iActivity->stop();
    iTimer.stop();
}

void
HarbourWakeupTimer::Private::start()
{
    const QDateTime now(QDateTime::currentDateTime());

    iRunning = true;
    iExpirationTime = now.addMSecs(iInterval);
    HDEBUG_("timer" << iInterval << "ms starts" << PRINTABLE_TIME(now) <<
        "expires" << PRINTABLE_TIME(iExpirationTime));
    startWait();
}

void
HarbourWakeupTimer::Private::restart()
{
    const bool wasRunning = iRunning;

    start();
    if (!wasRunning) {
        Q_EMIT parentObject()->runningChanged();
    }
}

void
HarbourWakeupTimer::Private::startWait()
{
    const QDateTime now(QDateTime::currentDateTime());
    const int secondsLeft = (int) (now.msecsTo(iExpirationTime)/1000);

    HASSERT(iRunning);

    // Create activity on demand
    if (!iActivity) {
        iActivity = new BackgroundActivity(this);
        connect(iActivity, SIGNAL(stateChanged()),
            this, SLOT(onActivityStateChanged()));
    }

    if (secondsLeft < 2) {
        // The last step of the wait - run the timer for the remaining
        // part of the interval (if anything is left) while the system
        // is prevented from suspending.
        if (iActivity->state() != BackgroundActivity::Running) {
            // Make sure that we wake up ASAP if the system suddenly
            // gets syspended.
            scheduleWakeup(1);
        }
    } else {
        scheduleWakeup(secondsLeft);
    }

    // In any case, start (or restart) the timer. There is a chance that
    // the system doesn't get suspended, and the timer expires on time.
    // If the system does get suspended, the timer will be restared when
    // we receive the wakeup call.
    startTimer();
}

void
HarbourWakeupTimer::Private::scheduleWakeup(
    int aSeconds)
{
    switch (iActivity->state()) {
    case BackgroundActivity::Running:
        HDEBUG_("stopping current activity");
        iActivity->stop();
        break;
    case BackgroundActivity::Waiting:
        HDEBUG_("stopping scheduled wakeup");
        iActivity->stop();
        break;
    case BackgroundActivity::Stopped:
        break;
    }

    const int minWait = qMax(aSeconds - 1, 1);
    const int maxWait = qMax(aSeconds, 1);

    HDEBUG_("wakeup in" << minWait << ".." << maxWait << "sec");
    iActivity->wait(minWait, maxWait);
}

void
HarbourWakeupTimer::Private::startTimer()
{
    const QDateTime now(QDateTime::currentDateTime());
    const int msToExpiration = (int) now.msecsTo(iExpirationTime);
    const int timeout = qMax(msToExpiration, 0);

    HASSERT(iRunning);
    HDEBUG_("remaining time" << timeout << "ms");
    iTimer.start(timeout, Qt::PreciseTimer, this);
}

void
HarbourWakeupTimer::Private::onActivityStateChanged()
{
    HDEBUG_("=>" << PRINTABLE_ACTIVITY_STATE(iActivity) << "at" <<
        PRINTABLE_TIME(QDateTime::currentDateTime()));
    if (iRunning) {
        switch (iActivity->state()) {
        case BackgroundActivity::Running:
            // Re-evaluate the state (most likely, restart the timer to
            // complete the wait exactly on time, while the system is
            // prevented from suspending).
            startWait();
            break;
        case BackgroundActivity::Waiting:
        case BackgroundActivity::Stopped:
            // Nothing to do
            break;
        }
    } else {
        // Make sure that no wakeups remain scheduled without the need
        iActivity->stop();
    }
}

void
HarbourWakeupTimer::Private::timerEvent(
    QTimerEvent* aEvent)
{
    if (aEvent->timerId() == iTimer.timerId()) {
        const QDateTime now(QDateTime::currentDateTime());

        HASSERT(iRunning);
        if (now >= iExpirationTime) {
            HDEBUG_("expired" << PRINTABLE_TIME(iExpirationTime) <<
                "triggered at" << PRINTABLE_TIME(now) << "(+" <<
                iExpirationTime.msecsTo(now) << "ms)");
            iRunning = false;
            stop();
            Q_EMIT parentObject()->triggered();
        } else {
            // It's very unlikely that the timer will ever fire too early.
            // But just in case, re-evaluate the state.
            startWait();
        }
    } else {
        QObject::timerEvent(aEvent);
    }
}

// ==========================================================================
// HarbourWakeupTimer
// ==========================================================================

HarbourWakeupTimer::HarbourWakeupTimer(
    QObject* aParent) :
    QObject(aParent),
    iPrivate(new Private(this))
{}

HarbourWakeupTimer::~HarbourWakeupTimer()
{
    delete iPrivate;
}

int
HarbourWakeupTimer::getInterval() const
{
    return iPrivate->iInterval;
}

void
HarbourWakeupTimer::setInterval(
    int aMilliseconds)
{
    iPrivate->setInterval(qMax(aMilliseconds, 0));
}

bool
HarbourWakeupTimer::isRunning() const
{
    return iPrivate->iRunning;
}

void
HarbourWakeupTimer::setRunning(
    bool aRunning)
{
    return iPrivate->setRunning(aRunning);
}

void
HarbourWakeupTimer::restart()
{
    iPrivate->restart();
}

void
HarbourWakeupTimer::start()
{
    iPrivate->setRunning(true);
}

void
HarbourWakeupTimer::stop()
{
    iPrivate->setRunning(false);
}

#include "HarbourWakeupTimer.moc"
