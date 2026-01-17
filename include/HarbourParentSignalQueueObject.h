/*
 * Copyright (C) 2026 Slava Monich <slava@monich.com>
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

#ifndef HARBOUR_PARENT_SIGNAL_QUEUE_OBJECT_H
#define HARBOUR_PARENT_SIGNAL_QUEUE_OBJECT_H

#include <QtCore/QObject>

// Common signal queueing pattern as a template.
//
// Note that the actual signals are emitted by the parent object.
// See HarbourSelectionListModel as an example of how it's supposed to be used.

template <typename Parent, typename Enum, Enum Count>
class HarbourParentSignalQueueObject :
    public QObject
{
public:
    typedef void (Parent::*SignalEmitter)();
    typedef uint SignalMask;

    HarbourParentSignalQueueObject(
        Parent* aParent,
        const SignalEmitter* aEmitters) :
        QObject(aParent),
        iEmitters(aEmitters),
        iQueuedSignals(0),
        iFirstQueuedSignal(Count)
    {}

    bool
    signalQueued(
        Enum aSignal) const
    {
        return (iQueuedSignals & (SignalMask(1) << aSignal)) != 0;
    }

    void
    queueSignal(
        Enum aSignal)
    {
        if (aSignal >= 0 && aSignal < Count) {
            const SignalMask signalBit = (SignalMask(1) << aSignal);
            if (iQueuedSignals) {
                iQueuedSignals |= signalBit;
                if (iFirstQueuedSignal > aSignal) {
                    iFirstQueuedSignal = aSignal;
                }
            } else {
                iQueuedSignals = signalBit;
                iFirstQueuedSignal = aSignal;
            }
        }
    }

    void
    emitQueuedSignals()
    {
        if (iQueuedSignals) {
            Parent* obj = qobject_cast<Parent*>(parent());
            // Reset first queued signal before emitting the signals.
            // Signal handlers may emit more signals.
            uint i = iFirstQueuedSignal;
            iFirstQueuedSignal = Count;
            for (; i < Count && iQueuedSignals; i++) {
                const SignalMask signalBit = (SignalMask(1) << i);
                if (iQueuedSignals & signalBit) {
                    iQueuedSignals &= ~signalBit;
                    Q_EMIT (obj->*(iEmitters[i]))();
                }
            }
        }
    }

    bool
    emitQueuedSignal(
        Enum aSignal)
    {
        const SignalMask signalBit = (SignalMask(1) << aSignal);
        if (iQueuedSignals & signalBit) {
            iQueuedSignals &= ~signalBit;
            Q_EMIT (qobject_cast<Parent*>(parent())->*(iEmitters[aSignal]))();
            return true;
        } else {
            return false;
        }
    }

private:
    const SignalEmitter* iEmitters;
    SignalMask iQueuedSignals;
    Enum iFirstQueuedSignal;
};

#endif // HARBOUR_PARENT_SIGNAL_QUEUE_OBJECT_H
