/*
 * Copyright (C) 2020 Jolla Ltd.
 * Copyright (C) 2020 Slava Monich <slava@monich.com>
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

#include "HarbourSystemInfo.h"
#include "HarbourDebug.h"

#include <QHash>
#include <QVector>
#include <QStringList>
#include <QFile>
#include <QTextStream>

// ==========================================================================
// HarbourSystemInfo::Private
// ==========================================================================

class HarbourSystemInfo::Private
{
public:
    static const QString NAME;
    static const QString VERSION_ID;

public:
    Private();

    static QHash<QString,QString> parseFile(QString aFileName, const QStringList aKeys);
    static QHash<QString,QString> parseOsRelease(const QStringList aKeys);
    static QVector<uint> parseVersion(QString aVersion);
    static int compareVersions(const QVector<uint> aVersion1, const QVector<uint> aVersion2);
    static int compareVersions(const QVector<uint> aVersion1, const QString aVersion2);

public:
    QString iName;
    QString iVersion;
    QVector<uint> iParsedVersion;
};

const QString HarbourSystemInfo::Private::NAME("NAME");
const QString HarbourSystemInfo::Private::VERSION_ID("VERSION_ID");

HarbourSystemInfo::Private::Private()
{
    QStringList keys;
    keys.append(NAME);
    keys.append(VERSION_ID);

    QHash<QString,QString> values(parseOsRelease(keys));
    iName = values.value(NAME);
    iVersion = values.value(VERSION_ID);
    iParsedVersion = parseVersion(iVersion);
}

inline QHash<QString,QString> HarbourSystemInfo::Private::parseOsRelease(const QStringList aKeys)
{
    return parseFile("/etc/os-release", aKeys);
}

QHash<QString,QString> HarbourSystemInfo::Private::parseFile(QString aPath, const QStringList aKeys)
{
    QFile file(aPath);
    QHash<QString,QString> result;
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        HDEBUG("Parsing" << qPrintable(aPath));
        QTextStream in(&file);
        while (!in.atEnd() && result.size() < aKeys.size()) {
            const QString line = in.readLine();
            const int sep = line.indexOf('=');
            if (sep > 0 && (sep + 1) < line.length()) {
                const QString key(line.left(sep));
                if (aKeys.contains(key)) {
                    QString value(line.mid(sep + 1));
                    const int len = value.length();
                    if (len > 1 && value.at(0) == '"' && value.at(len - 1) == '"') {
                        value = value.mid(1, len - 2);
                    }
                    value = value.replace("\\\"", "\"");
                    HDEBUG(qPrintable(key) << "=" << qPrintable(value));
                    result.insert(key, value);
                }
            }
        }
    }
    return result;
}

QVector<uint> HarbourSystemInfo::Private::parseVersion(QString aVersion)
{
    QVector<uint> parsed;
    QStringList parts(aVersion.split('.', QString::SkipEmptyParts));
    const int n = qMin(parts.count(),4);
    for (int i = 0; i < n; i++) {
        const QString part(parts.at(i));
        bool ok = false;
        int val = part.toUInt(&ok);
        if (ok) {
            parsed.append(val);
        } else {
            break;
        }
    }
    return parsed;
}

int HarbourSystemInfo::Private::compareVersions(const QVector<uint> aVersion1,
    const QVector<uint> aVersion2)
{
    const int n1 = aVersion1.size();
    const int n2 = aVersion2.size();
    const int n = qMin(n1, n2);
    for (int i = 0; i < n; i++) {
        const uint v1 = aVersion1.at(i);
        const uint v2 = aVersion2.at(i);
        if (v1 > v2) {
            return 1;
        } else if (v1 < v2) {
            return -1;
        }
    }
    return (n1 > n2) ? 1 : (n1 < n2) ? -1 : 0;
}

inline int HarbourSystemInfo::Private::compareVersions(const QVector<uint> aVersion1,
    const QString aVersion2)
{
    return compareVersions(aVersion1, Private::parseVersion(aVersion2));
}

// ==========================================================================
// HarbourSystemInfo
// ==========================================================================

HarbourSystemInfo::HarbourSystemInfo(QObject* aParent) :
    QObject(aParent),
    iPrivate(new Private)
{
    HDEBUG("created");
}

HarbourSystemInfo::~HarbourSystemInfo()
{
    HDEBUG("deleted");
    delete iPrivate;
}

// Callback for qmlRegisterSingletonType<HarbourSystemInfo>
QObject* HarbourSystemInfo::createSingleton(QQmlEngine*, QJSEngine*)
{
    return new HarbourSystemInfo(); // Singleton doesn't need a parent
}

QString HarbourSystemInfo::osName() const
{
    return iPrivate->iName;
}

QString HarbourSystemInfo::osVersion() const
{
    return iPrivate->iVersion;
}

int HarbourSystemInfo::osVersionCompare(QString aVersion)
{
    return Private::compareVersions(iPrivate->iParsedVersion, aVersion);
}

int HarbourSystemInfo::osVersionCompareWith(QString aVersion)
{
    const QStringList keys(Private::VERSION_ID);
    const QString os(Private::parseOsRelease(keys).value(Private::VERSION_ID));
    return Private::compareVersions(Private::parseVersion(os), aVersion);
}
