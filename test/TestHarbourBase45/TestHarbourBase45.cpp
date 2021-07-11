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

#include <glib.h>

/*==========================================================================*
 * isValidBase45
 *==========================================================================*/

static
void
test_isValidBase45(
    void)
{
    g_assert(HarbourBase45::isValidBase45("")); // Empty is considered valid
    g_assert(HarbourBase45::isValidBase45("BB8"));
    g_assert(HarbourBase45::isValidBase45("%69 VD92EX0"));
    g_assert(HarbourBase45::isValidBase45("UJCLQE7W581"));
    g_assert(!HarbourBase45::isValidBase45("AA("));
    g_assert(!HarbourBase45::isValidBase45("A(A"));
    g_assert(!HarbourBase45::isValidBase45("(AA"));
    g_assert(!HarbourBase45::isValidBase45("A("));
    g_assert(!HarbourBase45::isValidBase45("A["));
    g_assert(!HarbourBase45::isValidBase45("[A"));
    g_assert(!HarbourBase45::isValidBase45("A"));
    g_assert(!HarbourBase45::isValidBase45("ZZ"));
    g_assert(!HarbourBase45::isValidBase45("ZZZ"));
}

/*==========================================================================*
 * toBase45
 *==========================================================================*/

static
void
test_toBase45(
    void)
{
    g_assert(HarbourBase45::toBase45(QByteArray()).isEmpty());
    // Examples from draft-faltstrom-base45-07
    g_assert(HarbourBase45::toBase45(QByteArray("AB")) == QString("BB8"));
    g_assert(HarbourBase45::toBase45(QByteArray("Hello!!")) == QString("%69 VD92EX0"));
    g_assert(HarbourBase45::toBase45(QByteArray("base-45")) == QString("UJCLQE7W581"));
}

/*==========================================================================*
 * fromBase45
 *==========================================================================*/

static
void
test_fromBase45(
    void)
{
    // Invalid
    g_assert(HarbourBase45::fromBase45(QByteArray("AA[")).isEmpty());
    g_assert(HarbourBase45::fromBase45(QByteArray("A[A")).isEmpty());
    g_assert(HarbourBase45::fromBase45(QByteArray("[AA")).isEmpty());
    g_assert(HarbourBase45::fromBase45(QByteArray("AA(")).isEmpty());
    g_assert(HarbourBase45::fromBase45(QByteArray("A(A")).isEmpty());
    g_assert(HarbourBase45::fromBase45(QByteArray("(AA")).isEmpty());
    g_assert(HarbourBase45::fromBase45(QByteArray("A(")).isEmpty());
    g_assert(HarbourBase45::fromBase45(QByteArray("A[")).isEmpty());
    g_assert(HarbourBase45::fromBase45(QByteArray("[A")).isEmpty());
    g_assert(HarbourBase45::fromBase45(QByteArray("A")).isEmpty());
    g_assert(HarbourBase45::fromBase45(QByteArray("ZZ")).isEmpty());
    g_assert(HarbourBase45::fromBase45(QByteArray("ZZZ")).isEmpty());
    // Valid
    g_assert(HarbourBase45::fromBase45(QByteArray()).isEmpty());
    g_assert(HarbourBase45::fromBase45(QByteArray("BB8")) == QByteArray("AB"));
    g_assert(HarbourBase45::fromBase45(QByteArray("%69 VD92EX0")) == QByteArray("Hello!!"));
    g_assert(HarbourBase45::fromBase45(QByteArray("UJCLQE7W581")) == QByteArray("base-45"));
}

/*==========================================================================*
 * Common
 *==========================================================================*/

#define TEST_(name) "/HarbourBase45/" name

int main(int argc, char* argv[])
{
    g_test_init(&argc, &argv, NULL);
    g_test_add_func(TEST_("isValidBase45"), test_isValidBase45);
    g_test_add_func(TEST_("fromBase45"), test_fromBase45);
    g_test_add_func(TEST_("toBase45"), test_toBase45);
    return g_test_run();
}

/*
 * Local Variables:
 * mode: C++
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
