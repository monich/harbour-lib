/*
 * Copyright (C) 2015-2018 Jolla Ltd.
 * Copyright (C) 2015-2018 Slava Monich <slava.monich@jolla.com>
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Jolla Ltd nor the names of its contributors
 *     may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
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

#include "HarbourLib.h"
#include "HarbourDebug.h"
#include "HarbourDisplayBlanking.h"
#include "HarbourSystemState.h"
#include "HarbourTemporaryFile.h"
#include "HarbourTransferMethodsModel.h"

#if QT_VERSION >= 0x050000
#  include <QtQml>
#else
#  include <QtDeclarative/qdeclarative.h>
#endif

QML_DECLARE_TYPE(HarbourDisplayBlanking)
QML_DECLARE_TYPE(HarbourSystemState)
QML_DECLARE_TYPE(HarbourTransferMethodsModel)
QML_DECLARE_TYPE(HarbourTemporaryFile)

void
HarbourLib::registerTypes(
    const char* aUri,
    int aMajor,
    int aMinor)
{
    HDEBUG(aUri << aMinor << aMajor);
    qmlRegisterType<HarbourDisplayBlanking>(aUri, aMajor, aMinor, "DisplayBlanking");
    qmlRegisterType<HarbourSystemState>(aUri, aMajor, aMinor, "SystemState");
    qmlRegisterType<HarbourTemporaryFile>(aUri, aMajor, aMinor, "TemporaryFile");
    qmlRegisterType<HarbourTransferMethodsModel>(aUri, aMajor, aMinor, "TransferMethodsModel");
}
