/*
 * Copyright (C) 2021 Jolla Ltd.
 * Copyright (C) 2021 Slava Monich <slava.monich@jolla.com>
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

#include "HarbourBase45.h"

#include "HarbourDebug.h"

// ==========================================================================
// HarbourBase45::Private
// ==========================================================================

class HarbourBase45::Private {
public:
    enum {
        BASE = 45,
        BASE2 = BASE * BASE,
        REVERSE_MAP_SIZE = 91
    };
    static const char mapBase45[BASE];
    static const int reverseMapBase45[REVERSE_MAP_SIZE];
    static bool isValidChar(uint x);
};

const char HarbourBase45::Private::mapBase45[HarbourBase45::Private::BASE] = {
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
    'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
    'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
    'W', 'X', 'Y', 'Z', ' ', '$', '%', '*',
    '+', '-', '.', '/', ':'
};

const int HarbourBase45::Private::reverseMapBase45[HarbourBase45::Private::REVERSE_MAP_SIZE] = {
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,
    36, -1, -1, -1, 37, 38, -1, -1,
    -1, -1, 39, 40, -1, 41, 42, 43,
     0,  1,  2,  3,  4,  5,  6,  7,
     8,  9, 44, -1, -1, -1, -1, -1,
    -1, 10, 11, 12, 13, 14, 15, 16,
    17, 18, 19, 20, 21, 22, 23, 24,
    25, 26, 27, 28, 29, 30, 31, 32,
    33, 34, 35
};

inline bool HarbourBase45::Private::isValidChar(uint x)
{
    return (x < Private::REVERSE_MAP_SIZE) && Private::reverseMapBase45[x] >= 0;
}

// ==========================================================================
// HarbourBase45::Private
// ==========================================================================

bool HarbourBase45::isValidBase45(QString aBase45)
{
    const int len = aBase45.length();

    if (!((len % 3) % 2)) {
        const QChar* chars = aBase45.constData();
        int i = 0;

        while ((i + 2) < len) {
            const uint c = chars[i++].unicode();
            const uint d = chars[i++].unicode();
            const uint e = chars[i++].unicode();

            if (!Private::isValidChar(c) ||
                !Private::isValidChar(d) ||
                !Private::isValidChar(e) ||
                (Private::reverseMapBase45[c] +
                 Private::reverseMapBase45[d] * Private::BASE +
                 Private::reverseMapBase45[e] * Private::BASE2) > 0xffff) {
                return false;
            }
        }

        if (i < len) {
            const uint c = chars[i++].unicode();
            const uint d = chars[i++].unicode();

            if (!Private::isValidChar(c) ||
                !Private::isValidChar(d) ||
                (Private::reverseMapBase45[c] +
                 Private::reverseMapBase45[d] * Private::BASE) > 0xff) {
                return false;
            }
        }
        return true;
    }
    return false;
}

QByteArray HarbourBase45::fromBase45(QString aBase45)
{
    QByteArray out;
    const int len = aBase45.length();
    const int tail = (len % 3);

    if (!(tail % 2)) {
        const QChar* chars = aBase45.constData();
        int i = 0;

        out.reserve(len/3*2 + tail/2);
        while ((i + 2) < len) {
            const uint c = chars[i++].unicode();
            const uint d = chars[i++].unicode();
            const uint e = chars[i++].unicode();

            if (!Private::isValidChar(c) ||
                !Private::isValidChar(d) ||
                !Private::isValidChar(e)) {
                return QByteArray();
            }
            const uint n = Private::reverseMapBase45[c] +
                Private::reverseMapBase45[d] * Private::BASE +
                Private::reverseMapBase45[e] * Private::BASE2;
            if (n > 0xffff) {
                return QByteArray();
            }
            char ab[2];
            ab[0] = (char)(n >> 8);
            ab[1] = (char)n;
            out.append(ab, sizeof(ab));
        }

        if (i < len) {
            const uint c = chars[i++].unicode();
            const uint d = chars[i++].unicode();

            if (!Private::isValidChar(c) ||
                !Private::isValidChar(d)) {
                return QByteArray();
            }
            const uint a = Private::reverseMapBase45[c] +
                Private::reverseMapBase45[d] * Private::BASE;
            if (a > 0xff) {
                return QByteArray();
            }
            out.append((char)a);
        }
    }
    return out;
}

QString HarbourBase45::toBase45(QByteArray aBinary)
{
    const uchar* ptr = (uchar*)aBinary.constData();
    const int n = aBinary.size();
    int i;

    QString str;
    str.reserve(3 * (n / 2) + 2 * (n % 2));
    for (i = 0; (i + 1) < n; i += 2) {
        uint e = (uint)(ptr[i]) * 256 + ptr[i+1];
        const uchar c = (uchar)(e % Private::BASE);
        e = (e - c)/Private::BASE;
        const uchar d = (uchar)(e % Private::BASE);
        e = (e - d)/Private::BASE;
        str.append(QChar::fromLatin1(Private::mapBase45[c]));
        str.append(QChar::fromLatin1(Private::mapBase45[d]));
        str.append(QChar::fromLatin1(Private::mapBase45[e]));
    }
    if (i < n) {
        uchar d = ptr[i];
        const uchar c = d % Private::BASE;
        d = (d - c)/Private::BASE;
        str.append(QChar::fromLatin1(Private::mapBase45[c]));
        str.append(QChar::fromLatin1(Private::mapBase45[d]));
    }
    return str;
}
