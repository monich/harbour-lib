/*
 * Copyright (C) 2019-2021 Jolla Ltd.
 * Copyright (C) 2019-2021 Slava Monich <slava.monich@jolla.com>
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

#include "HarbourAztecCodeGenerator.h"

#include "HarbourBase32.h"
#include "HarbourTask.h"
#include "HarbourDebug.h"

#include <QThreadPool>

#include "aztec_encode.h"   // Requires https://github.com/monich/libaztec

// ==========================================================================
// HarbourAztecCodeGenerator::Task
// ==========================================================================

class HarbourAztecCodeGenerator::Task : public HarbourTask {
    Q_OBJECT
public:
    Task(QThreadPool* aPool, QString aText, int aEcLevel);
    void performTask() Q_DECL_OVERRIDE;
public:
    QString iText;
    QString iCode;
    int iEcLevel;
};

HarbourAztecCodeGenerator::Task::Task(QThreadPool* aPool, QString aText,
    int aEcLevel) :
    HarbourTask(aPool),
    iText(aText),
    iEcLevel(aEcLevel)
{
}

void HarbourAztecCodeGenerator::Task::performTask()
{
    QByteArray bytes(generate(iText, iEcLevel));
    if (!bytes.isEmpty()) {
        iCode = HarbourBase32::toBase32(bytes);
    }
}

// ==========================================================================
// HarbourAztecCodeGenerator::Private
// ==========================================================================

class HarbourAztecCodeGenerator::Private : public QObject {
    Q_OBJECT

public:
    Private(HarbourAztecCodeGenerator* aParent);
    ~Private();

    HarbourAztecCodeGenerator* parentObject() const;
    void setText(QString aValue);
    void setEcLevel(int aValue);
    void regenerate();

    static int realEcLevel(int aEcLevel);

public Q_SLOTS:
    void onTaskDone();

public:
    QThreadPool* iThreadPool;
    Task* iTask;
    int iEcLevel;
    QString iText;
    QString iCode;
};

HarbourAztecCodeGenerator::Private::Private(HarbourAztecCodeGenerator* aParent) :
    QObject(aParent),
    iThreadPool(new QThreadPool(this)),
    iTask(Q_NULLPTR),
    iEcLevel(ECLevelDefault)
{
    // Serialize the tasks:
    iThreadPool->setMaxThreadCount(1);
}

HarbourAztecCodeGenerator::Private::~Private()
{
    iThreadPool->waitForDone();
}

inline HarbourAztecCodeGenerator* HarbourAztecCodeGenerator::Private::parentObject() const
{
    return qobject_cast<HarbourAztecCodeGenerator*>(parent());
}

int HarbourAztecCodeGenerator::Private::realEcLevel(int aEcLevel)
{
    return (aEcLevel == ECLevelDefault) ? AZTEC_CORRECTION_DEFAULT :
        (aEcLevel < ECLevelLowest) ?  ECLevelLowest :
        (aEcLevel > ECLevelHighest) ?  ECLevelHighest :
        aEcLevel;
}

void HarbourAztecCodeGenerator::Private::setText(QString aText)
{
    if (iText != aText) {
        iText = aText;
        regenerate();
        Q_EMIT parentObject()->textChanged();
    }
}

void HarbourAztecCodeGenerator::Private::setEcLevel(int aValue)
{
    const int level = (aValue == ECLevelDefault) ? ECLevelDefault : realEcLevel(aValue);
    if (iEcLevel != level) {
        const int prevRealLevel = realEcLevel(iEcLevel);
        iEcLevel = level;
        HDEBUG(level);
        if (realEcLevel(level) != prevRealLevel) {
            regenerate();
        }
        Q_EMIT parentObject()->ecLevelChanged();
    }
}

void HarbourAztecCodeGenerator::Private::regenerate()
{
    const bool wasRunning = (iTask != Q_NULLPTR);
    if (iTask) iTask->release(this);
    iTask = new Task(iThreadPool, iText, iEcLevel);
    iTask->submit(this, SLOT(onTaskDone()));
    if (!wasRunning) {
        Q_EMIT parentObject()->runningChanged();
    }
}

void HarbourAztecCodeGenerator::Private::onTaskDone()
{
    if (sender() == iTask) {
        HarbourAztecCodeGenerator* obj = parentObject();
        const bool changed = (iCode != iTask->iCode);
        iCode = iTask->iCode;
        iTask->release();
        iTask = NULL;
        if (changed) {
            Q_EMIT obj->codeChanged();
        }
        Q_EMIT obj->runningChanged();
    }
}

// ==========================================================================
// HarbourAztecCodeGenerator
// ==========================================================================

HarbourAztecCodeGenerator::HarbourAztecCodeGenerator(QObject* aParent) :
    QObject(aParent),
    iPrivate(new Private(this))
{
}

// Callback for qmlRegisterSingletonType<HarbourAztecCodeGenerator>
QObject* HarbourAztecCodeGenerator::createSingleton(QQmlEngine*, QJSEngine*)
{
    return new HarbourAztecCodeGenerator();
}

QString HarbourAztecCodeGenerator::text() const
{
    return iPrivate->iText;
}

void HarbourAztecCodeGenerator::setText(QString aValue)
{
    iPrivate->setText(aValue);
}

int HarbourAztecCodeGenerator::ecLevel() const
{
    return iPrivate->iEcLevel;
}

void HarbourAztecCodeGenerator::setEcLevel(int aValue)
{
    iPrivate->setEcLevel(aValue);
}

QString HarbourAztecCodeGenerator::code() const
{
    return iPrivate->iCode;
}

bool HarbourAztecCodeGenerator::running() const
{
    return iPrivate->iTask != Q_NULLPTR;
}

QByteArray HarbourAztecCodeGenerator::generate(QString aText, int aEcLevel)
{
    HDEBUG(aText);
    QByteArray in(aText.toUtf8()), out;
    AztecSymbol* aztec = aztec_encode_inv(in.constData(), in.size(),
        Private::realEcLevel(aEcLevel));
    if (aztec) {
        const int bytesPerRow = (aztec->size + 7) / 8;
        out.reserve(bytesPerRow * aztec->size);
        for (guint y = 0; y < aztec->size; y++) {
            out.append((char*)aztec->rows[y], bytesPerRow);
        }
        aztec_symbol_free(aztec);
    }
    return out;
}

#include "HarbourAztecCodeGenerator.moc"
