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

#ifndef HARBOUR_PROTOBUF_H
#define HARBOUR_PROTOBUF_H

#include <QByteArray>

#include <gutil_types.h>

// https://developers.google.com/protocol-buffers/docs/encoding

class HarbourProtoBuf
{
    HarbourProtoBuf() Q_DECL_EQ_DELETE;

public:
    enum {
        TYPE_SHIFT = 3,
        TYPE_MASK = ((1 << TYPE_SHIFT)-1),
        TYPE_VARINT = 0,
        TYPE_DELIMITED = 2
    };

    static QByteArray* appendVarInt(QByteArray*, quint64);
    static QByteArray* appendVarIntKeyValue(QByteArray*, quint64, quint64);
    static QByteArray* appendDelimitedValue(QByteArray*, const QByteArray);
    static QByteArray* appendDelimitedKeyValue(QByteArray*, quint64, const QByteArray);

    static bool parseVarInt(GUtilRange*, quint64*);
    static bool parseDelimitedValue(GUtilRange*, GUtilData*);
};

#endif // HARBOUR_PROTOBUF_H
