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

#include "HarbourDebug.h"
#include "HarbourProtoBuf.h"

// https://developers.google.com/protocol-buffers/docs/encoding

QByteArray*
HarbourProtoBuf::appendVarInt(
    QByteArray* aOutput,
    quint64 aValue)
{
    uchar out[10];
    quint64 value = aValue;
    int i = sizeof(out) - 1;

    out[i] = value & 0x7f;
    value >>= 7;
    while (value) {
        out[--i] = 0x80 | (uchar)value;
        value >>= 7;
    }

    const int n = sizeof(out) - i;

    aOutput->reserve(aOutput->size() + n);
    aOutput->append((char*)(out + i), n);
    return aOutput;
}

QByteArray*
HarbourProtoBuf::appendVarIntKeyValue(
    QByteArray* aOutput,
    quint64 aKey,
    quint64 aValue)
{
    HASSERT((aKey & TYPE_MASK) == TYPE_VARINT);
    return appendVarInt(appendVarInt(aOutput, aKey), aValue);
}

QByteArray*
HarbourProtoBuf::appendDelimitedValue(
    QByteArray* aOutput,
    const QByteArray aValue)
{
    appendVarInt(aOutput, aValue.size())->append(aValue);
    return aOutput;
}

QByteArray*
HarbourProtoBuf::appendDelimitedKeyValue(
    QByteArray* aOutput,
    quint64 aKey,
    const QByteArray aValue)
{
    HASSERT((aKey & TYPE_MASK) == TYPE_DELIMITED);
    return appendDelimitedValue(appendVarInt(aOutput, aKey), aValue);
}

bool
HarbourProtoBuf::parseVarInt(
    GUtilRange* aPos,
    quint64* aResult)
{
    quint64 value = 0;
    const guint8* ptr = aPos->ptr;

    for (int i = 0; i < 10 && ptr < aPos->end; i++, ptr++) {
        value = (value << 7) | (*ptr & 0x7f);
        if (!(*ptr & 0x80)) {
            aPos->ptr = ptr + 1;
            *aResult = value;
            return true;
        }
    }

    // Premature end of stream or too many bytes
    *aResult = 0;
    return false;
}

// A delimited value is stored as a size, encoded as a varint, followed
// by the payload of type (message | string | bytes | packed)
bool
HarbourProtoBuf::parseDelimitedValue(
    GUtilRange* aPos,
    GUtilData* aPayload)
{
    GUtilRange pos = *aPos;
    quint64 size;

    if (parseVarInt(&pos, &size) && (pos.ptr + size) <= pos.end) {
        aPayload->bytes = pos.ptr;
        aPayload->size = size;
        aPos->ptr = pos.ptr + size;
        return true;
    }
    return false;
}
