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
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in
 *      the documentation and/or other materials provided with the
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

#include "HarbourTask.h"
#include "HarbourDebug.h"

#include <QCoreApplication>
#include <QThreadPool>

// ==========================================================================
// HarbourTask::Private
// ==========================================================================

class HarbourTask::Private {
public:
    Private(QThreadPool* aPool);

public:
    QThreadPool* iPool;
    QObject* iTarget;
    bool iAboutToQuit;
    bool iSubmitted;
    bool iStarted;
    bool iReleased;
    bool iDone;
};

HarbourTask::Private::Private(QThreadPool* aPool) :
    iPool(aPool),
    iTarget(NULL),
    iAboutToQuit(false),
    iSubmitted(false),
    iStarted(false),
    iReleased(false),
    iDone(false)
{
}

// ==========================================================================
// HarbourTask
// ==========================================================================

HarbourTask::HarbourTask(QThreadPool* aPool, QThread* aTargetThread) :
    QObject(aTargetThread ? NULL : aPool), // Cannot move objects with a parent
    iPrivate(new Private(aPool))
{
    if (aTargetThread) moveToThread(aTargetThread);
    setAutoDelete(false);
    connect(qApp, SIGNAL(aboutToQuit()), SLOT(onAboutToQuit()));
    connect(this, SIGNAL(runFinished()), SLOT(onRunFinished()),
        Qt::QueuedConnection);
}

HarbourTask::~HarbourTask()
{
    HASSERT(iPrivate->iReleased);
    if (iPrivate->iSubmitted) wait();
    delete iPrivate;
}

bool HarbourTask::isStarted() const
{
    return iPrivate->iStarted;
}

bool HarbourTask::isCanceled() const
{
    return iPrivate->iReleased || iPrivate->iAboutToQuit;
}

void HarbourTask::submit()
{
    HASSERT(!iPrivate->iSubmitted);
    HASSERT(iPrivate->iPool);
    if (iPrivate->iPool && !iPrivate->iSubmitted) {
        iPrivate->iSubmitted = true;
        iPrivate->iPool->start(this);
    }
}

void HarbourTask::submit(QObject* aTarget, const char* aSlot)
{
    HASSERT(!iPrivate->iTarget);
    iPrivate->iTarget = aTarget;
    connect(aTarget, SIGNAL(destroyed(QObject*)), SLOT(onTargetDestroyed(QObject*)));
    aTarget->connect(this, SIGNAL(done()), aSlot);
    submit();
}

void HarbourTask::release(QObject* aHandler)
{
    aHandler->disconnect(this);
    disconnect(aHandler);
    released();
}

void HarbourTask::release()
{
    if (iPrivate->iTarget) {
        iPrivate->iTarget->disconnect(this);
        disconnect(iPrivate->iTarget);
    }
    released();
}

void HarbourTask::released()
{
    iPrivate->iReleased = true;
    if (!iPrivate->iSubmitted || iPrivate->iDone) {
        delete this;
    }
}

void HarbourTask::run()
{
    HASSERT(!iPrivate->iStarted);
    iPrivate->iStarted = true;
    performTask();
    Q_EMIT runFinished();
}

void HarbourTask::onRunFinished()
{
    HASSERT(!iPrivate->iDone);
    if (!iPrivate->iReleased) {
        Q_EMIT done();
    }
    iPrivate->iDone = true;
    if (iPrivate->iReleased) {
        delete this;
    }
}

void HarbourTask::onAboutToQuit()
{
    HDEBUG("OK");
    iPrivate->iAboutToQuit = true;
}

void HarbourTask::onTargetDestroyed(QObject* aObject)
{
    HDEBUG(aObject);
    HASSERT(iPrivate->iTarget == aObject);
    if (iPrivate->iTarget == aObject) {
        iPrivate->iTarget = NULL;
    }
}
