/*
 * Copyright (C) 2019-2024 Slava Monich <slava@monich.com>
 * Copyright (C) 2019 Jolla Ltd.
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

#include "HarbourClipboard.h"
#include "HarbourDebug.h"

#include <QtGui/QClipboard>
#include <QtGui/QGuiApplication>

// ==========================================================================
// HarbourClipboard::Private
// ==========================================================================

class HarbourClipboard::Private :
    public QObject
{
    Q_OBJECT

public:
    Private(HarbourClipboard*);

    HarbourClipboard* parentObject() const;
    void setText(const QString&);
    void setCacheText(bool);

public Q_SLOTS:
    void onClipboardChanged();

public:
    QClipboard* iClipboard;
    QString iText;
    bool iCacheText;
};

HarbourClipboard::Private::Private(
    HarbourClipboard* aParent) :
    QObject(aParent),
    iClipboard(qApp->clipboard()),
    iText(iClipboard->text()),
    iCacheText(true)
{
    connect(iClipboard, SIGNAL(dataChanged()), SLOT(onClipboardChanged()));
}

inline
HarbourClipboard*
HarbourClipboard::Private::parentObject() const
{
    return qobject_cast<HarbourClipboard*>(parent());
}

void
HarbourClipboard::Private::onClipboardChanged()
{
    const QString clipboardText(iClipboard->text());
    HDEBUG(clipboardText);
    if (iText != clipboardText && (!iCacheText || !clipboardText.isEmpty())) {
        iText = clipboardText;
        Q_EMIT parentObject()->textChanged();
    }
}

void
HarbourClipboard::Private::setText(
    const QString& aValue)
{
    if (iText != aValue) {
        iText = aValue;
        HDEBUG(aValue);
        Q_EMIT parentObject()->textChanged();
        if (aValue.isEmpty()) {
            iClipboard->clear();
        } else {
            iClipboard->setText(iText);
        }
    }
}

void
HarbourClipboard::Private::setCacheText(
    bool aValue)
{
    if (iCacheText != aValue) {
        iCacheText = aValue;
        HDEBUG(aValue);
        HarbourClipboard* obj = parentObject();
        Q_EMIT obj->cacheTextChanged();
        if (!iCacheText) {
            const QString clipboardText(iClipboard->text());
            if (iText != clipboardText) {
                iText = clipboardText;
                Q_EMIT obj->textChanged();
            }
        }
    }
}

// ==========================================================================
// HarbourClipboard::Private
// ==========================================================================

HarbourClipboard::HarbourClipboard(
    QObject* aParent) :
    QObject(aParent),
    iPrivate(new Private(this))
{
}

// Callback for qmlRegisterSingletonType<HarbourClipboard>
QObject*
HarbourClipboard::createSingleton(QQmlEngine*, QJSEngine*)
{
    return new HarbourClipboard();
}

QString
HarbourClipboard::text() const
{
    return iPrivate->iText;
}

void
HarbourClipboard::setText(
    QString aValue)
{
    iPrivate->setText(aValue);
}

bool
HarbourClipboard::cacheText() const
{
    return iPrivate->iCacheText;
}

void
HarbourClipboard::setCacheText(
    bool aValue)
{
    iPrivate->setCacheText(aValue);
}

#include "HarbourClipboard.moc"
