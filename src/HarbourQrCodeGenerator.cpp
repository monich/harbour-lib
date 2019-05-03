/*
 * Copyright (C) 2019 Jolla Ltd.
 * Copyright (C) 2019 Slava Monich <slava.monich@jolla.com>
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

#include "HarbourQrCodeGenerator.h"

#include "HarbourBase32.h"
#include "HarbourTask.h"
#include "HarbourDebug.h"

#include <QThreadPool>

#include "qrencode.h"

// ==========================================================================
// HarbourQrCodeGenerator::Task
// ==========================================================================

class HarbourQrCodeGenerator::Task : public HarbourTask {
    Q_OBJECT
public:
    Task(QThreadPool* aPool, QString aText);
    void performTask() Q_DECL_OVERRIDE;
public:
    QString iText;
    QString iQrCode;
};

HarbourQrCodeGenerator::Task::Task(QThreadPool* aPool, QString aText) :
    HarbourTask(aPool),
    iText(aText)
{
}

void HarbourQrCodeGenerator::Task::performTask()
{
    QByteArray bytes(generate(iText));
    if (!bytes.isEmpty()) {
        iQrCode = HarbourBase32::toBase32(bytes);
    }
}

// ==========================================================================
// HarbourQrCodeGenerator::Private
// ==========================================================================

class HarbourQrCodeGenerator::Private : public QObject {
    Q_OBJECT

public:
    Private(HarbourQrCodeGenerator* aParent);
    ~Private();

    HarbourQrCodeGenerator* parentObject() const;
    void setText(QString aValue);

public Q_SLOTS:
    void onTaskDone();

public:
    QThreadPool* iThreadPool;
    Task* iTask;
    QString iText;
    QString iQrCode;
};

HarbourQrCodeGenerator::Private::Private(HarbourQrCodeGenerator* aParent) :
    QObject(aParent),
    iThreadPool(new QThreadPool(this)),
    iTask(Q_NULLPTR)
{
    // Serialize the tasks:
    iThreadPool->setMaxThreadCount(1);
}

HarbourQrCodeGenerator::Private::~Private()
{
    iThreadPool->waitForDone();
}

inline HarbourQrCodeGenerator* HarbourQrCodeGenerator::Private::parentObject() const
{
    return qobject_cast<HarbourQrCodeGenerator*>(parent());
}

void HarbourQrCodeGenerator::Private::setText(QString aText)
{
    if (iText != aText) {
        iText = aText;
        HarbourQrCodeGenerator* obj = parentObject();
        const bool wasRunning = (iTask != Q_NULLPTR);
        if (iTask) iTask->release(this);
        iTask = new Task(iThreadPool, aText);
        iTask->submit(this, SLOT(onTaskDone()));
        Q_EMIT obj->textChanged();
        if (!wasRunning) {
            Q_EMIT obj->runningChanged();
        }
    }
}

void HarbourQrCodeGenerator::Private::onTaskDone()
{
    if (sender() == iTask) {
        HarbourQrCodeGenerator* obj = parentObject();
        const bool qrCodeChanged = (iQrCode != iTask->iQrCode);
        iQrCode = iTask->iQrCode;
        iTask->release();
        iTask = NULL;
        if (qrCodeChanged) {
            Q_EMIT obj->qrcodeChanged();
        }
        Q_EMIT obj->runningChanged();
    }
}

// ==========================================================================
// HarbourQrCodeGenerator
// ==========================================================================

HarbourQrCodeGenerator::HarbourQrCodeGenerator(QObject* aParent) :
    QObject(aParent),
    iPrivate(new Private(this))
{
}

QString HarbourQrCodeGenerator::text() const
{
    return iPrivate->iText;
}

void HarbourQrCodeGenerator::setText(QString aValue)
{
    iPrivate->setText(aValue);
}

QString HarbourQrCodeGenerator::qrcode() const
{
    return iPrivate->iQrCode;
}

bool HarbourQrCodeGenerator::running() const
{
    return iPrivate->iTask != Q_NULLPTR;
}

QByteArray HarbourQrCodeGenerator::generate(QString aText)
{
    QByteArray in(aText.toUtf8()), out;
    QRcode* code = QRcode_encodeString(in.constData(), 0, QR_ECLEVEL_M, QR_MODE_8, true);
    if (code) {
        const int bytesPerRow = (code->width + 7) / 8;
        if (bytesPerRow > 0) {
            out.reserve(bytesPerRow * code->width);
            for (int y = 0; y < code->width; y++) {
                const unsigned char* row = code->data + (code->width * y);
                char c = (row[0] & 1);
                int x = 1;
                for (; x < code->width; x++) {
                    if (!(x % 8)) {
                        out.append(&c, 1);
                        c = row[x] & 1;
                    } else {
                        c = (c << 1) | (row[x] & 1);
                    }
                }
                const int rem = x % 8;
                if (rem) {
                    // Most significant bit first
                    c <<= (8 - rem);
                }
                out.append(&c, 1);
            }
        }
        QRcode_free(code);
    }
    return out;
}

#include "HarbourQrCodeGenerator.moc"
