/*
 * Copyright (C) 2022 Jolla Ltd.
 * Copyright (C) 2022 Slava Monich <slava.monich@jolla.com>
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

#include "HarbourUtil.h"

HarbourUtil::HarbourUtil(
    QObject* aParent) :
    QObject(aParent)
{
}

// Callback for qmlRegisterSingletonType<HarbourUtil>
QObject*
HarbourUtil::createSingleton(
    QQmlEngine*,
    QJSEngine*)
{
    return new HarbourUtil();
}

QColor
HarbourUtil::invertedColor(
    const QColor& aColor)
{
    if (aColor.isValid()) {
        const QRgb inv = invertedRgb(aColor.rgba());

        return QColor(qRed(inv), qGreen(inv), qBlue(inv), qAlpha(inv));
    } else {
        return aColor;
    }
}

QRgb
HarbourUtil::invertedRgb(
    QRgb aRgb)
{
    return ((~(aRgb & RGB_MASK)) & RGB_MASK) | (aRgb & (~RGB_MASK));
}

QString
HarbourUtil::toHex(
    const void* aData,
    size_t aSize)
{
    QString str;

    if (aSize > 0) {
        const uchar* bytes = (const uchar*)aData;

        str.reserve(2 * aSize);
        for (size_t i = 0; i < aSize; i++) {
            static const char hex[] = "0123456789abcdef";
            const uchar b = bytes[i];

            str.append(QChar(hex[(b & 0xf0) >> 4]));
            str.append(QChar(hex[b & 0x0f]));
        }
    }
    return str;
}
