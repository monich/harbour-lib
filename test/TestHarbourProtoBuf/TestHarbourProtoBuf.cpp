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

#include "HarbourProtoBuf.h"

#include <glib.h>

/*==========================================================================*
 * null
 *==========================================================================*/

static
void
test_null(
    void)
{
    g_assert(!HarbourProtoBuf::appendVarInt(NULL, 0));
    g_assert(!HarbourProtoBuf::appendVarIntKeyValue(NULL, 0, 0));
    g_assert(!HarbourProtoBuf::appendDelimitedValue(NULL, QByteArray()));
    g_assert(!HarbourProtoBuf::appendDelimitedKeyValue(NULL,
        HarbourProtoBuf::TYPE_DELIMITED, QByteArray()));
    g_assert(!HarbourProtoBuf::parseVarInt(NULL, NULL));
    g_assert(!HarbourProtoBuf::parseDelimitedValue(NULL, NULL));
}

/*==========================================================================*
 * int
 *==========================================================================*/

static
void
test_int(
    void)
{
    QByteArray buf;
    GUtilRange range;
    quint64 res;

    static const guint8 enc0[] = { 0 };

    g_assert(HarbourProtoBuf::appendVarInt(&buf, 0) == &buf);
    g_assert(buf == QByteArray::fromRawData((char*)enc0, sizeof(enc0)));

    range.end = (range.ptr = enc0);
    g_assert(!HarbourProtoBuf::parseVarInt(&range, NULL));
    range.end = (range.ptr = enc0);

    res = 42;
    g_assert(!HarbourProtoBuf::parseVarInt(&range, &res));
    g_assert_cmpuint(res, == ,0);

    range.end = (range.ptr = enc0) + sizeof(enc0);
    g_assert(HarbourProtoBuf::parseVarInt(&range, NULL));
    g_assert(range.ptr == range.end);

    res = 42;
    range.end = (range.ptr = enc0) + sizeof(enc0);
    g_assert(HarbourProtoBuf::parseVarInt(&range, &res));
    g_assert_cmpuint(res, == ,0);

    static const guint8 enc257[] = { 0x82, 0x01 };

    buf.clear();
    g_assert(HarbourProtoBuf::appendVarInt(&buf, 257) == &buf);
    g_assert(buf == QByteArray::fromRawData((char*)enc257, sizeof(enc257)));

    res = 42;
    range.end = (range.ptr = enc257) + 1;
    g_assert(!HarbourProtoBuf::parseVarInt(&range, &res));
    g_assert_cmpuint(res, == ,0);

    res = 42;
    range.end = (range.ptr = enc257) + sizeof(enc257);
    g_assert(HarbourProtoBuf::parseVarInt(&range, &res));
    g_assert(range.ptr == range.end);
    g_assert_cmpuint(res, == ,257);
}

/*==========================================================================*
 * delimited
 *==========================================================================*/

static
void
test_delimited(
    void)
{
    QByteArray buf;
    GUtilRange range;
    GUtilData payload;
    static const guint8 value[] = { 0x01, 0x02 };
    static const guint8 encodedValue[] = { 0x02, 0x01, 0x02 };

    g_assert(HarbourProtoBuf::appendDelimitedValue(&buf,
        QByteArray::fromRawData((char*)value, sizeof(value))) == &buf);
    g_assert(buf == QByteArray::fromRawData((char*)encodedValue,
        sizeof(encodedValue)));

    range.end = (range.ptr = encodedValue);
    g_assert(!HarbourProtoBuf::parseDelimitedValue(&range, NULL));
    g_assert(range.ptr == encodedValue);

    range.end = (range.ptr = encodedValue) + sizeof(encodedValue) - 1;
    g_assert(!HarbourProtoBuf::parseDelimitedValue(&range, NULL));
    g_assert(range.ptr == encodedValue);

    range.end = (range.ptr = encodedValue) + sizeof(encodedValue);
    g_assert(HarbourProtoBuf::parseDelimitedValue(&range, NULL));

    memset(&payload, 0, sizeof(payload));
    range.end = (range.ptr = encodedValue) + sizeof(encodedValue);
    g_assert(HarbourProtoBuf::parseDelimitedValue(&range, &payload));
    g_assert_cmpuint(payload.size, == ,sizeof(value));
    g_assert(!memcmp(payload.bytes, value, sizeof(value)));
}

/*==========================================================================*
 * Common
 *==========================================================================*/

#define TEST_(name) "/HarbourProtoBuf/" name

int main(int argc, char* argv[])
{
    g_test_init(&argc, &argv, NULL);
    g_test_add_func(TEST_("null"), test_null);
    g_test_add_func(TEST_("int"), test_int);
    g_test_add_func(TEST_("delimited"), test_delimited);
    return g_test_run();
}

/*
 * Local Variables:
 * mode: C++
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
