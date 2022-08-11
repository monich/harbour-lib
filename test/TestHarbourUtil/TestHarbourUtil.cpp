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

#include <glib.h>

/*==========================================================================*
 * object
 *==========================================================================*/

static
void
test_object(
    void)
{
    QObject* obj = HarbourUtil::createSingleton(Q_NULLPTR, Q_NULLPTR);

    g_assert(qobject_cast<HarbourUtil*>(obj));
    delete obj;
}

/*==========================================================================*
 * invertColor
 *==========================================================================*/

static
void
test_invertColor(
    void)
{
    g_assert_cmpuint(HarbourUtil::invertedRgb(0x12345678), == ,0x12cba987);
    const QColor original(0xcb,0xa9,0x87,0x12);
    const QColor inverted(0x34,0x56,0x78,0x12);
    g_assert(HarbourUtil::invertedColor(original) == inverted);
    g_assert(!HarbourUtil::invertedColor(QColor()).isValid());
}

/*==========================================================================*
 * toHex
 *==========================================================================*/

static
void
test_toHex(
    void)
{
    static const char data[] = {
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A,
        0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14
    };

    const QByteArray dataBytes(data, sizeof(data));
    const QByteArray hex(dataBytes.toHex());

    g_assert(HarbourUtil::toHex(QByteArray()).isEmpty());
    g_assert(HarbourUtil::toHex(Q_NULLPTR, 0).isEmpty());
    g_assert(HarbourUtil::toHexBytes(Q_NULLPTR, 0).isEmpty());
    g_assert(HarbourUtil::toHex(data, sizeof(data)) == QString::fromLatin1(hex));
    g_assert_cmpstr(HarbourUtil::toHexBytes(data, sizeof(data)).constData(), == ,
        hex.constData());
}

/*==========================================================================*
 * Common
 *==========================================================================*/

#define TEST_(name) "/HarbourUtil/" name

int main(int argc, char* argv[])
{
    g_test_init(&argc, &argv, NULL);
    g_test_add_func(TEST_("object"), test_object);
    g_test_add_func(TEST_("invertColor"), test_invertColor);
    g_test_add_func(TEST_("toHex"), test_toHex);
    return g_test_run();
}

/*
 * Local Variables:
 * mode: C++
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
