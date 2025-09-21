/*
 * Copyright (C) 2021-2025 Slava Monich <slava@monich.com>
 * Copyright (C) 2021 Jolla Ltd.
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

#include <glib.h>

/*==========================================================================*
 * isValidBase45
 *==========================================================================*/

static
void
test_isValidBase32(
    void)
{
    g_assert(HarbourBase32::isValidBase32("AEBAGBAFAYDQQCIKBMGA2DQPCAIREEYUCULBOGI2DMOB2HQ7"));
    g_assert(HarbourBase32::isValidBase32("aebagbaf aydqqcik bmga2dqp caireeyu culbogi2 dmob2hq7"));

    // Padding
    g_assert(HarbourBase32::isValidBase32("ae======"));
    g_assert(HarbourBase32::isValidBase32("aeba===="));
    g_assert(HarbourBase32::isValidBase32("aebag==="));
    g_assert(HarbourBase32::isValidBase32("aebagba="));
    g_assert(HarbourBase32::isValidBase32("aebagbaf"));
    g_assert(HarbourBase32::isValidBase32("aeba gbaf")); // Space is ignored inside base32
    g_assert(HarbourBase32::isValidBase32("ae ======")); // before the padding
    g_assert(HarbourBase32::isValidBase32("ae= =====")); // and inside the padding

    // Ignore missing padding
    g_assert(HarbourBase32::isValidBase32("ae"));
    g_assert(HarbourBase32::isValidBase32("aeba"));
    g_assert(HarbourBase32::isValidBase32("aebag"));
    g_assert(HarbourBase32::isValidBase32("aebagba"));
    g_assert(HarbourBase32::isValidBase32("aebagbaf"));

    // Don't ignore missing padding
    g_assert(!HarbourBase32::isValidBase32("ae", true));
    g_assert(!HarbourBase32::isValidBase32("aeba", true));
    g_assert(!HarbourBase32::isValidBase32("aebag", true));
    g_assert(!HarbourBase32::isValidBase32("aebagba", true));

    g_assert(!HarbourBase32::isValidBase32("af")); // Unused bits not 0
    g_assert(!HarbourBase32::isValidBase32("ae=====")); // Not enough padding
    g_assert(!HarbourBase32::isValidBase32("ae=======")); // Too much padding
    g_assert(!HarbourBase32::isValidBase32("ae======a")); // Unexpected char
    g_assert(!HarbourBase32::isValidBase32("aebagbaf========")); // Unexpected padding
    g_assert(!HarbourBase32::isValidBase32("aeb"));
    g_assert(!HarbourBase32::isValidBase32("aebagb"));
    g_assert(!HarbourBase32::isValidBase32("aebagbafa"));
    g_assert(!HarbourBase32::isValidBase32("aebagb= x"));
    g_assert(!HarbourBase32::isValidBase32(QString()));
    g_assert(!HarbourBase32::isValidBase32(" "));
    g_assert(!HarbourBase32::isValidBase32("01234567"));
    g_assert(!HarbourBase32::isValidBase32("88888888"));
    g_assert(!HarbourBase32::isValidBase32("{}"));
    g_assert(!HarbourBase32::isValidBase32("[]"));
    g_assert(!HarbourBase32::isValidBase32("="));
}

/*==========================================================================*
 * fromBase32
 *==========================================================================*/

static
void
test_fromBase32(
    void)
{
    static const char out[] = {
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A,
        0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14,
        0x15, 0x16, 0x17, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F
    };
    QString in1("AEBAGBAFAYDQQCIKBMGA2DQPCAIREEYUCULBOGI2DMOB2HQ7");
    QString in2("aebagbaf aydqqcik bmga2dqp caireeyu culbogi2 dmob2hq7");
    QByteArray out1(HarbourBase32::fromBase32(in1));
    QByteArray out2(HarbourBase32::fromBase32(in2));
    g_assert(out1 == out2);
    g_assert(out1 == QByteArray(out, sizeof(out)));
    g_assert(HarbourBase32::fromBase32("ae======") == QByteArray(out, 1));
    g_assert(HarbourBase32::fromBase32("ae") == QByteArray(out, 1));
    g_assert(HarbourBase32::fromBase32("af======").isEmpty()); // Unused bits not 0
    g_assert(HarbourBase32::fromBase32("af").isEmpty()); // Unused bits not 0
    g_assert(HarbourBase32::fromBase32("aeb", true).isEmpty());
    g_assert(HarbourBase32::fromBase32("aeb").isEmpty());
    g_assert(HarbourBase32::fromBase32("aeba", true).isEmpty());
    g_assert(HarbourBase32::fromBase32("aeba===.").isEmpty()); // Garbage padding
    g_assert(HarbourBase32::fromBase32("aeba====") == QByteArray(out, 2));
    g_assert(HarbourBase32::fromBase32("aeba") == QByteArray(out, 2));
    g_assert(HarbourBase32::fromBase32("aebag", true).isEmpty());
    g_assert(HarbourBase32::fromBase32("aebag===") == QByteArray(out, 3));
    g_assert(HarbourBase32::fromBase32("aebag") == QByteArray(out, 3));
    g_assert(HarbourBase32::fromBase32("aebagb").isEmpty());
    g_assert(HarbourBase32::fromBase32("aebagb=x").isEmpty());
    g_assert(HarbourBase32::fromBase32("aebagb= x").isEmpty());
    g_assert(HarbourBase32::fromBase32("aebagba=") == QByteArray(out, 4));
    g_assert(HarbourBase32::fromBase32("aebagba") == QByteArray(out, 4));
    g_assert(HarbourBase32::fromBase32("aebagba", true).isEmpty());
    g_assert(HarbourBase32::fromBase32("aebagbaf") == QByteArray(out, 5));
    g_assert(HarbourBase32::fromBase32("aebagbafa").isEmpty());
    g_assert(HarbourBase32::fromBase32("aebagbafay") == QByteArray(out, 6));
    g_assert(HarbourBase32::fromBase32(QString()).isEmpty());
    g_assert(HarbourBase32::fromBase32(" ").isEmpty());
    g_assert(HarbourBase32::fromBase32("01234567").isEmpty());
    g_assert(HarbourBase32::fromBase32("88888888").isEmpty());
    g_assert(HarbourBase32::fromBase32("{}").isEmpty());
    g_assert(HarbourBase32::fromBase32("[]").isEmpty());
}

/*==========================================================================*
 * base32pad
 *==========================================================================*/

static
void
test_base32pad(
    void)
{
    static const char out[] = {
        0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23
    };
    static const int n = sizeof(out);

    g_assert(HarbourBase32::fromBase32("DMOB2HQ7EAQSEIY", false) == QByteArray(out, n));
    g_assert(HarbourBase32::fromBase32("DMOB2HQ7EAQSEIY=") == QByteArray(out, n));
    g_assert(HarbourBase32::fromBase32("DMOB2HQ7EAQSE===") == QByteArray(out, n - 1));
    g_assert(HarbourBase32::fromBase32("DMOB2HQ7EAQSE== =") == QByteArray(out, n - 1));
    g_assert(HarbourBase32::fromBase32("DMOB2HQ7EAQSE", false) == QByteArray(out, n - 1));
    g_assert(HarbourBase32::fromBase32("DMOB2HQ7EA=====").isEmpty()); // Not enough padding
    g_assert(HarbourBase32::fromBase32("DMOB2HQ7EAQQ=====").isEmpty()); // Too much padding
    g_assert(HarbourBase32::fromBase32(QString("=================")).isEmpty());
    g_assert(HarbourBase32::fromBase32(QString("=DMOB2HQ7EAQSEIY=")).isEmpty());
    g_assert(HarbourBase32::fromBase32(QString("DMOB2HQ7EB=")).isEmpty());
}

/*==========================================================================*
 * toBase32
 *==========================================================================*/

static
void
test_toBase32(
    void)
{
    static const char inBytes[] = {
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A,
        0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14,
        0x15, 0x16, 0x17, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F
    };
    const QByteArray in(inBytes, sizeof(inBytes));
    const QString OUT("AEBAGBAFAYDQQCIKBMGA2DQPCAIREEYUCULBOGI2DMOB2HQ7");
    const QString out(OUT.toLower());

    g_assert(HarbourBase32::toBase32(QByteArray()).isEmpty());
    g_assert(HarbourBase32::toBase32(in) == OUT);
    g_assert(HarbourBase32::toBase32(in, true) == out);
    g_assert(HarbourBase32::toBase32(in, HarbourBase32::EncodeLowerCase) == out);
}

/*==========================================================================*
 * rfc4648
 *==========================================================================*/

static
void
test_rfc4648(
    void)
{
    // Test vectors from RFC 4648
    static const char* test[][2] = {
        { "f", "MY======" },
        { "fo", "MZXQ====" },
        { "foo", "MZXW6===" },
        { "foob", "MZXW6YQ=" },
        { "fooba", "MZXW6YTB" },
        { "foobar", "MZXW6YTBOI======" }
    };

    for (guint i = 0; i < G_N_ELEMENTS(test); i++) {
        QByteArray data(test[i][0]);
        QString base32(test[i][1]);
        g_assert(HarbourBase32::isValidBase32(base32));
        g_assert(HarbourBase32::fromBase32(base32) == data);
        g_assert(HarbourBase32::toBase32(data, false) == base32);
    }
}

/*==========================================================================*
 * Common
 *==========================================================================*/

#define TEST_(name) "/HarbourBase32/" name

int main(int argc, char* argv[])
{
    g_test_init(&argc, &argv, NULL);
    g_test_add_func(TEST_("isValidBase32"), test_isValidBase32);
    g_test_add_func(TEST_("fromBase32"), test_fromBase32);
    g_test_add_func(TEST_("base32pad"), test_base32pad);
    g_test_add_func(TEST_("rfc4648"), test_rfc4648);
    g_test_add_func(TEST_("toBase32"), test_toBase32);
    return g_test_run();
}

/*
 * Local Variables:
 * mode: C++
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
