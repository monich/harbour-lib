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

#ifndef HARBOUR_UTIL_H
#define HARBOUR_UTIL_H

#include <QByteArray>
#include <QColor>
#include <QObject>
#include <QRgb>
#include <QString>

class QQmlEngine;
class QJSEngine;

class HarbourUtil :
    public QObject
{
    Q_OBJECT
    class Private;

public:
    explicit HarbourUtil(QObject* aParent = Q_NULLPTR);

    // Callback for qmlRegisterSingletonType<HarbourUtil>
    static QObject* createSingleton(QQmlEngine*, QJSEngine*);

    // QML (and possibly native) utilities
    Q_INVOKABLE static QColor invertedColor(const QColor&);

    // Static utilities
    static QRgb invertedRgb(QRgb);
    static QByteArray toHexBytes(const void*, size_t);
    static QString toHex(const void*, size_t);
    static inline QString toHex(const QByteArray& aData)
        { return toHex(aData.constData(), aData.size()); }
};

#endif // HARBOUR_UTIL_H
