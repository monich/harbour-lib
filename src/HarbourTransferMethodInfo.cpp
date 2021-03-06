/*
 * Copyright (C) 2016-2018 Jolla Ltd.
 * Copyright (C) 2016-2018 Slava Monich <slava.monich@jolla.com>
 *
 * You may use this file under the terms of BSD license as follows:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *   3. Neither the name of Jolla Ltd nor the names of its contributors may
 *      be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "HarbourTransferMethodInfo.h"

// ==========================================================================
// HarbourTransferMethodInfo
// ==========================================================================

QDBusArgument &operator<<(QDBusArgument& aArg, const HarbourTransferMethodInfo& aInfo)
{
    aArg.beginStructure();
    aArg << aInfo.displayName << aInfo.userName << aInfo.methodId
         << aInfo.shareUIPath << aInfo.capabilitities << aInfo.accountId;
    aArg.endStructure();
    return aArg;
}

const QDBusArgument &operator>>(const QDBusArgument& aArg, HarbourTransferMethodInfo& aInfo)
{
    aArg.beginStructure();
    aArg >> aInfo.displayName >> aInfo.userName >> aInfo.methodId
         >> aInfo.shareUIPath >> aInfo.capabilitities >> aInfo.accountId;
    aArg.endStructure();
    return aArg;
}

HarbourTransferMethodInfo::HarbourTransferMethodInfo():
    accountId(0)
{
}

HarbourTransferMethodInfo::HarbourTransferMethodInfo(const HarbourTransferMethodInfo& aInfo):
    displayName(aInfo.displayName),
    userName(aInfo.userName),
    methodId(aInfo.methodId),
    shareUIPath(aInfo.shareUIPath),
    capabilitities(aInfo.capabilitities),
    accountId(aInfo.accountId)
{
}

HarbourTransferMethodInfo& HarbourTransferMethodInfo::operator=(const HarbourTransferMethodInfo& aInfo)
{
    displayName    = aInfo.displayName;
    userName       = aInfo.userName;
    methodId       = aInfo.methodId;
    shareUIPath    = aInfo.shareUIPath;
    capabilitities = aInfo.capabilitities;
    accountId      = aInfo.accountId;
    return *this;
}

bool HarbourTransferMethodInfo::equals(const HarbourTransferMethodInfo& aInfo) const
{
    return displayName    == aInfo.displayName    &&
           userName       == aInfo.userName       &&
           methodId       == aInfo.methodId       &&
           shareUIPath    == aInfo.shareUIPath    &&
           capabilitities == aInfo.capabilitities &&
           accountId      == aInfo.accountId;
}

void HarbourTransferMethodInfo::registerTypes()
{
    qDBusRegisterMetaType<HarbourTransferMethodInfo>();
    qDBusRegisterMetaType<HarbourTransferMethodInfoList>();
}

// ==========================================================================
// HarbourTransferMethodInfo2
// ==========================================================================

QDBusArgument &operator<<(QDBusArgument& aArg, const HarbourTransferMethodInfo2& aInfo)
{
    aArg.beginStructure();
    aArg << aInfo.displayName << aInfo.userName << aInfo.methodId
         << aInfo.shareUIPath << aInfo.capabilitities << aInfo.accountId
         << aInfo.accountIcon << aInfo.hints;
    aArg.endStructure();
    return aArg;
}

const QDBusArgument &operator>>(const QDBusArgument& aArg, HarbourTransferMethodInfo2& aInfo)
{
    aArg.beginStructure();
    aArg >> aInfo.displayName >> aInfo.userName >> aInfo.methodId
         >> aInfo.shareUIPath >> aInfo.capabilitities >> aInfo.accountId
         >> aInfo.accountIcon >> aInfo.hints;
    aArg.endStructure();
    return aArg;
}

HarbourTransferMethodInfo2::HarbourTransferMethodInfo2():
    accountId(0)
{
}

HarbourTransferMethodInfo2::HarbourTransferMethodInfo2(const HarbourTransferMethodInfo2& aInfo):
    displayName(aInfo.displayName),
    userName(aInfo.userName),
    methodId(aInfo.methodId),
    shareUIPath(aInfo.shareUIPath),
    capabilitities(aInfo.capabilitities),
    accountId(aInfo.accountId),
    accountIcon(aInfo.accountIcon),
    hints(aInfo.hints)
{
}

HarbourTransferMethodInfo2::HarbourTransferMethodInfo2(const HarbourTransferMethodInfo& aInfo):
    displayName(aInfo.displayName),
    userName(aInfo.userName),
    methodId(aInfo.methodId),
    shareUIPath(aInfo.shareUIPath),
    capabilitities(aInfo.capabilitities),
    accountId(aInfo.accountId)
{
}

HarbourTransferMethodInfo2& HarbourTransferMethodInfo2::operator=(const HarbourTransferMethodInfo2& aInfo)
{
    displayName    = aInfo.displayName;
    userName       = aInfo.userName;
    methodId       = aInfo.methodId;
    shareUIPath    = aInfo.shareUIPath;
    capabilitities = aInfo.capabilitities;
    accountId      = aInfo.accountId;
    accountIcon    = aInfo.accountIcon;
    hints          = aInfo.hints;
    return *this;
}

bool HarbourTransferMethodInfo2::equals(const HarbourTransferMethodInfo2& aInfo) const
{
    return displayName    == aInfo.displayName    &&
           userName       == aInfo.userName       &&
           methodId       == aInfo.methodId       &&
           shareUIPath    == aInfo.shareUIPath    &&
           capabilitities == aInfo.capabilitities &&
           accountId      == aInfo.accountId      &&
           accountIcon    == aInfo.accountIcon    &&
           hints          == aInfo.hints;
}

void HarbourTransferMethodInfo2::registerTypes()
{
    HarbourTransferMethodInfo::registerTypes();
    qDBusRegisterMetaType<HarbourTransferMethodInfo2>();
    qDBusRegisterMetaType<HarbourTransferMethodInfo2List>();
}
