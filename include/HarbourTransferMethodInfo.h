/*
 * Copyright (C) 2016 Jolla Ltd.
 * Contact: Slava Monich <slava.monich@jolla.com>
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

#ifndef HARBOUR_TRANSFER_METHOD_INFO_H
#define HARBOUR_TRANSFER_METHOD_INFO_H

#include <QStringList>
#include <QtDBus/QtDBus>

class HarbourTransferMethodInfo
{
public:
    HarbourTransferMethodInfo();
    HarbourTransferMethodInfo& operator=(const HarbourTransferMethodInfo& aInfo);
    HarbourTransferMethodInfo(const HarbourTransferMethodInfo& aInfo);
    bool operator==(const HarbourTransferMethodInfo& Info) const;
    bool operator!=(const HarbourTransferMethodInfo& Info) const;
    bool equals(const HarbourTransferMethodInfo& Info) const;

    static void registerTypes();

    QString displayName;
    QString userName;
    QString methodId;
    QString shareUIPath;
    QStringList capabilitities;
    quint32 accountId;
};

inline bool HarbourTransferMethodInfo::operator==(const HarbourTransferMethodInfo& aInfo) const
    { return equals(aInfo); }
inline bool HarbourTransferMethodInfo::operator!=(const HarbourTransferMethodInfo& aInfo) const
    { return !equals(aInfo); }

typedef QList<HarbourTransferMethodInfo> HarbourTransferMethodInfoList;
QDBusArgument& operator<<(QDBusArgument& aArg, const HarbourTransferMethodInfo& aInfo);
const QDBusArgument& operator>>(const QDBusArgument& aArg, HarbourTransferMethodInfo& aInfo);

Q_DECLARE_METATYPE(HarbourTransferMethodInfo)
Q_DECLARE_METATYPE(QList<HarbourTransferMethodInfo>)

#endif // HARBOUR_TRANSFER_METHOD_INFO_H
