/*
 * Copyright (C) 2018-2026 Slava Monich <slava@monich.com>
 * Copyright (C) 2018-2019 Jolla Ltd.
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

#include "HarbourTask.h"
#include "HarbourDebug.h"

#include <QtCore/QAtomicInteger>
#include <QtCore/QCoreApplication>
#include <QtCore/QPointer>
#include <QtCore/QThreadPool>

// ==========================================================================
// HarbourTask::Private
// ==========================================================================

class HarbourTask::Private
{
public:
    Private(QThreadPool*);

public:
    QThreadPool* iPool;
    QPointer<QObject> iTarget;
    // These flags are set by the worker thread:
    QAtomicInteger<bool> iStarted;
    QAtomicInteger<bool> iFinished;
    // These are set by the main thread (and checked by the worker):
    QAtomicInteger<bool> iAboutToQuit;
    QAtomicInteger<bool> iReleased;
    // And these are manipulated only by the main thread:
    bool iSubmitted;
    bool iDone;
};

HarbourTask::Private::Private(
    QThreadPool* aPool) :
    iPool(aPool),
    iStarted(false),
    iFinished(false),
    iAboutToQuit(false),
    iReleased(false),
    iSubmitted(false),
    iDone(false)
{}

// ==========================================================================
// HarbourTask
// ==========================================================================

HarbourTask::HarbourTask(
    QThreadPool* aPool) :
    QObject(aPool),
    iPrivate(new Private(aPool))
{
    setAutoDelete(false);
    connect(qApp, SIGNAL(aboutToQuit()), SLOT(onAboutToQuit()));
    connect(this, SIGNAL(runFinished()), SLOT(onRunFinished()),
        Qt::QueuedConnection);
}

HarbourTask::~HarbourTask()
{
    // The target can be destroyed before done() signal is delivered to the
    // main thread and the target has a chance to release the task.
    HASSERT(iPrivate->iReleased || !iPrivate->iTarget);
    HASSERT(!iPrivate->iSubmitted || iPrivate->iFinished);
    delete iPrivate;
}

bool
HarbourTask::isStarted() const
{
    return iPrivate->iStarted;
}

bool
HarbourTask::isCanceled() const
{
    return iPrivate->iReleased || iPrivate->iAboutToQuit;
}

void
HarbourTask::submit()
{
    HASSERT(!iPrivate->iSubmitted);
    HASSERT(iPrivate->iPool);
    if (iPrivate->iPool && !iPrivate->iSubmitted) {
        iPrivate->iSubmitted = true;
        iPrivate->iPool->start(this);
    }
}

void
HarbourTask::submit(
    QObject* aTarget,
    const char* aSlot)
{
    HASSERT(!iPrivate->iTarget);
    iPrivate->iTarget = aTarget;
    aTarget->connect(this, SIGNAL(done()), aSlot);
    submit();
}

void
HarbourTask::release()
{
    if (iPrivate->iTarget) {
        disconnect(iPrivate->iTarget.data());
        iPrivate->iTarget.clear();
    }
    released();
}

void
HarbourTask::released()
{
    iPrivate->iReleased = true;
    if (!iPrivate->iSubmitted || iPrivate->iDone) {
        delete this;
    }
}

void
HarbourTask::run()
{
    HASSERT(!iPrivate->iStarted);
    iPrivate->iStarted = true;
    if (!isCanceled()) {
        performTask();
    }
    iPrivate->iFinished = true;
    Q_EMIT runFinished();
}

void
HarbourTask::onRunFinished()
{
    // Invoked on the main thread
    HASSERT(!iPrivate->iDone);
    if (!iPrivate->iReleased) {
        Q_EMIT done();
    }
    iPrivate->iDone = true;
    if (iPrivate->iReleased) {
        delete this;
    }
}

void
HarbourTask::onAboutToQuit()
{
    HDEBUG("OK");
    iPrivate->iAboutToQuit = true;
}
