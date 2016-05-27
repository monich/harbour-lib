/*
 * Copyright (C) 2016 Jolla Ltd.
 * Contact: Slava Monich <slava.monich@jolla.com>
 *
 * You may use this file under the terms of BSD license as follows:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *   3. Neither the name of Jolla Ltd nor the names of its contributors may
 *      be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "HarbourSigChildHandler.h"
#include "HarbourDebug.h"

#include <QSocketNotifier>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>

// Based on "Calling Qt Functions From Unix Signal Handlers" article
// http://doc.qt.io/qt-4.8/unix-signals.html

class HarbourSigChildHandler::Private : public QSocketNotifier {
    Q_OBJECT
public:
    static Private* get();

private:
    Private(int aFd[]);
    ~Private();

    static void handler(int, siginfo_t* aInfo, void*);

Q_SIGNALS:
    void processDied(int aPid, int aStatus);

private Q_SLOTS:
    void handleSigChild();

private:
    static Private* gInstance;
    int iFd[2];
};

HarbourSigChildHandler::Private* HarbourSigChildHandler::Private::gInstance = NULL;

HarbourSigChildHandler::Private::Private(int aFd[]) :
    QSocketNotifier(aFd[1], QSocketNotifier::Read)
{
    iFd[0] = aFd[0];
    iFd[1] = aFd[1];
    connect(this, SIGNAL(activated(int)), SLOT(handleSigChild()));
}

// This object is actually never deleted because there's no easy way
// to properly synchronize deletion with the incoming signal.
HarbourSigChildHandler::Private::~Private()
{
    close(iFd[0]);
    close(iFd[1]);
}

HarbourSigChildHandler::Private* HarbourSigChildHandler::Private::get()
{
    if (!gInstance) {
        int fd[2];
        if (socketpair(AF_UNIX, SOCK_STREAM, 0, fd) == 0) {
            struct sigaction act;
            memset (&act, 0, sizeof(act));
            act.sa_sigaction = Private::handler;
            act.sa_flags = SA_SIGINFO;
            HDEBUG("Installing SIGCHLD handler");
            gInstance = new Private(fd);
            sigaction(SIGCHLD, &act, NULL);
        } else {
            HWARN("Failed to create socket pair");
        }
    }
    return gInstance;
}

void HarbourSigChildHandler::Private::handler(int, siginfo_t* aInfo, void*)
{
    int status = -1;
    waitpid(aInfo->si_pid, &status, 0);
    HASSERT(gInstance);
    if (gInstance) {
        int data[2];
        data[0] = aInfo->si_pid;
        data[1] = status;
        write(gInstance->iFd[0], data, sizeof(data));
    }
}

void HarbourSigChildHandler::Private::handleSigChild()
{
    int data[2];
    if (read(iFd[1], data, sizeof(data)) == sizeof(data)) {
        HDEBUG("Child" << data[0] << "died, status" << data[1]);
        Q_EMIT processDied(data[0], data[1]);
    } else {
        HWARN("Error handling SIGCHLD");
    }
}

HarbourSigChildHandler::HarbourSigChildHandler(Private* aPrivate, QObject* aParent) :
    QObject(aParent), iPrivate(aPrivate)
{
    connect(iPrivate, SIGNAL(processDied(int,int)), SIGNAL(processDied(int,int)));
}

HarbourSigChildHandler::~HarbourSigChildHandler()
{
}

HarbourSigChildHandler* HarbourSigChildHandler::install(QObject* aParent)
{
    Private* priv = Private::get();
    return priv ? new HarbourSigChildHandler(priv, aParent) : NULL;
}

#include "HarbourSigChildHandler.moc"
