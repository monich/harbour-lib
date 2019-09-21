/*
 * Copyright (C) 2015-2019 Jolla Ltd.
 * Copyright (C) 2015-2019 Slava Monich <slava@monich.com>
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
 *      notice, this list of conditions and the following disclaimer
 *      in the documentation and/or other materials provided with the
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

#ifndef HARBOUR_DEBUG_H
#define HARBOUR_DEBUG_H

#ifndef HARBOUR_DEBUG
#  define HARBOUR_DEBUG 0
#endif // HARBOUR_DEBUG

#ifndef HARBOUR_VERBOSE
#  define HARBOUR_VERBOSE 0
#endif // HARBOUR_VERBOSE

#include <QDebug>

#if HARBOUR_VERBOSE
#if QT_VERSION >= 0x050000
#  define HVERBOSE(x) qDebug() << x
#else
#  define HVERBOSE(x) qDebug() << Q_FUNC_INFO << x
#endif
#else
#  define HVERBOSE(expr) ((void)0)
#endif // HARBOUR_VERBOSE

#if HARBOUR_DEBUG
#if QT_VERSION >= 0x050000
#  define HDEBUG(x) qDebug() << x
#else
#  define HDEBUG(x) qDebug() << Q_FUNC_INFO << x
#endif
#  define HASSERT(x) ((x) ? ((void)0) : qt_assert(#x,__FILE__,__LINE__))
#  define HVERIFY(x) HASSERT(x)
#else
#  define HDEBUG(expr) ((void)0)
#  define HASSERT(expr) ((void)0)
#  define HVERIFY(x) (x)
#endif // HARBOUR_DEBUG

#if QT_VERSION >= 0x050000
# define HWARN(x) qWarning() << x
#else
# define HWARN(x) qWarning() << Q_FUNC_INFO << x
#endif

// Usage: TODO("Remember to fix this")
#ifndef TODO
#  define HARBOUR_PRAGMA(x) _Pragma(#x)
#  define TODO(x) HARBOUR_PRAGMA(message("TODO - " x))
#endif

#endif // HARBOUR_DEBUG_H
