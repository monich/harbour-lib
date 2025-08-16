/*
 * Copyright (C) 2019-2022 Jolla Ltd.
 * Copyright (C) 2019-2025 Slava Monich <slava@monich.com>
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

#include "HarbourBase32.h"

#include "HarbourDebug.h"

#define BASE32_BITS_PER_NIBBLE (5)
#define BASE32_BYTES_PER_CHUNK (5)
#define BASE32_NIBBLES_PER_CHUNK (8)
#define BASE32_BITS_PER_CHUNK (BASE32_BITS_PER_NIBBLE*BASE32_NIBBLES_PER_CHUNK)
#define BASE32_NIBBLE_MASK  ((1 << BASE32_BITS_PER_NIBBLE) - 1)
Q_STATIC_ASSERT(BASE32_BITS_PER_CHUNK == BASE32_BYTES_PER_CHUNK * 8);

// ==========================================================================
// HarbourBase32::Private
// ==========================================================================

class HarbourBase32::Private
{
public:
    static char nibbleToBase32(int, char);
    static int base32ToNibble(char);
};

// static
inline
char
HarbourBase32::Private::nibbleToBase32(
    int aNibble,
    char aBaseChar)
{
    return (aNibble < 26) ? (aBaseChar + aNibble) : ('2' + (aNibble - 26));
}

// static
inline
int
HarbourBase32::Private::base32ToNibble(
    char aChar)
{
    if (aChar >= 'a' && aChar <= 'z') {
        return aChar - 'a';
    } else if (aChar >= 'A' && aChar <= 'Z') {
        return aChar - 'A';
    } else if (aChar >= '2' && aChar <= '7') {
        return 26 + (aChar - '2');
    } else {
        return -1;
    }
}

// ==========================================================================
// HarbourBase32
// ==========================================================================

// static
bool
HarbourBase32::isValidBase32(
    const QString aBase32)
{
    const int n = aBase32.length();
    const QChar* chars = aBase32.constData();

    qint64 buf = 0;
    int i, bits = 0;
    bool empty = true;

    for (i = 0; i < n; i++) {
        const QChar c = chars[i];

        if (!c.isSpace()) {
            const char l = c.toLatin1();

            empty = false;
            if (l == '=') {
                i++;
                break;
            } else {
                const int nibble = Private::base32ToNibble(l);

                if (nibble < 0) {
                    HDEBUG("Invalid BASE32 character" << c);
                    return false;
                }
                bits += BASE32_BITS_PER_NIBBLE;
                buf <<= BASE32_BITS_PER_NIBBLE;
                buf += nibble;
                if (bits == BASE32_BITS_PER_CHUNK) {
                    bits = 0;
                    buf = 0;
                }
            }
        }
    }

    // Handle padding per RFC 4648
    if (bits) {
        while (i < n) {
            const QChar c = chars[i++];

            if (!c.isSpace()) {
                empty = false;
                if (c != '=') {
                    HDEBUG("Invalid BASE32 string" << aBase32);
                    return false;
                }
            }
        }
        const int realbits = bits/8*8;
        const int mask = (1 << (bits - realbits)) - 1;
        if (buf & mask) {
            return false;
        }
    }
    return !empty;
}

// static
QByteArray
HarbourBase32::fromBase32(
    const QString aBase32)
{
    QByteArray out;

    const int n = aBase32.length();
    const QChar* chars = aBase32.constData();

    qint64 buf = 0;
    int i, bits = 0;
    char bytes[BASE32_BYTES_PER_CHUNK];

    out.reserve((n * 5 + 7)/8);

    for (i = 0; i < n; i++) {
        const QChar c = chars[i];

        if (!c.isSpace()) {
            const char l = c.toLatin1();

            if (l == '=') {
                i++;
                break;
            } else {
                const int nibble = Private::base32ToNibble(l);

                if (nibble < 0) {
                    HDEBUG("Invalid BASE32 character" << c);
                    return QByteArray();
                } else {
                    bits += BASE32_BITS_PER_NIBBLE;
                    buf <<= BASE32_BITS_PER_NIBBLE;
                    buf += nibble;
                    if (bits == BASE32_BITS_PER_CHUNK) {
                        bits = 0;
                        for (int k = BASE32_BYTES_PER_CHUNK; k > 0; k--) {
                            bytes[k - 1] = (char)buf;
                            buf >>= 8;
                        }
                        out.append(bytes, BASE32_BYTES_PER_CHUNK);
                    }
                }
            }
        }
    }

    // Handle padding per RFC 4648
    if (bits) {
        while (i < n) {
            const QChar c = chars[i++];

            if (!c.isSpace() && c != '=') {
                HDEBUG("Invalid BASE32 string" << aBase32);
                return QByteArray();
            }
        }
        const int realbits = bits/8*8;
        const int mask = (1 << (bits - realbits)) - 1;
        if (buf & mask) {
            HDEBUG("Invalid BASE32 string" << aBase32);
            return QByteArray();
        } else {
            buf >>= (bits - realbits);
            for (int k = realbits / 8; k > 0; k--) {
                bytes[k - 1] = (char)buf;
                buf >>= 8;
            }
            out.append(bytes, realbits / 8);
        }
    }
    return out;
}

// static
QString
HarbourBase32::toBase32(
    const QByteArray aBinary,
    bool aLowerCase)
{
    QString str;
    const int n = aBinary.size();
    const uchar* ptr = (uchar*)aBinary.constData();
    char base32[BASE32_NIBBLES_PER_CHUNK + 1];
    const char a = aLowerCase ? 'a' : 'A';
    quint64 chunk = 0;
    int i, k, bits = 0;

    base32[BASE32_NIBBLES_PER_CHUNK] = 0;
    for (i = 0; i < n; i++) {
        chunk <<= 8;
        chunk |= *ptr++;
        bits += 8;
        if (bits == BASE32_BITS_PER_CHUNK) {
            for (k = BASE32_NIBBLES_PER_CHUNK; k > 0; k--) {
                const int nibble = chunk & BASE32_NIBBLE_MASK;
                base32[k - 1] = Private::nibbleToBase32(nibble, a);
                chunk >>= BASE32_BITS_PER_NIBBLE;
            }
            str.append(base32);
            bits = 0;
        }
    }
    // Add padding per RFC 4648
    if (bits) {
        const int outnibbles = (bits + BASE32_BITS_PER_NIBBLE - 1) /
            BASE32_BITS_PER_NIBBLE;

        chunk <<= (outnibbles * BASE32_BITS_PER_NIBBLE - bits);
        k = BASE32_NIBBLES_PER_CHUNK;
        while (k > outnibbles) {
            base32[--k] = '=';
        }
        while (k > 0) {
            const int nibble = chunk & BASE32_NIBBLE_MASK;

            base32[--k] = Private::nibbleToBase32(nibble, a);
            chunk >>= BASE32_BITS_PER_NIBBLE;
        }
        str.append(base32);
    }
    return str;
}
