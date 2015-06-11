/*
 * Copyright (C) 2015 Jolla Ltd.
 * Contact: Slava Monich <slava.monich@jolla.com>
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
 *   * Neither the name of Nemo Mobile nor the names of its contributors
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

#include "HarbourJson.h"
#include "HarbourDebug.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>

#if QT_VERSION >= 0x050000
#  include <QJsonDocument>
#  include <QJsonObject>
#else
#  include <qjson/parser.h>
#  include <qjson/serializer.h>
#endif

bool HarbourJson::save(QString aPath, const QVariantMap& aMap)
{
    QFileInfo file(aPath);
    QDir dir(file.dir());
    if (dir.mkpath(dir.absolutePath())) {
        QFile f(file.absoluteFilePath());
        if (!aMap.isEmpty()) {
            if (f.open(QIODevice::WriteOnly)) {
#if QT_VERSION >= 0x050000
                if (f.write(QJsonDocument::fromVariant(aMap).toJson()) >= 0) {
                    return true;
                }
#else
                QJson::Serializer serializer;
                QByteArray json = serializer.serialize(aMap);
                if (!json.isNull()) {
                    if (f.write(json ) >= 0) {
                        return true;
                    }
                } else {
                    HWARN("Json serialization error");
                }
#endif
                HWARN("Error writing" << aPath << f.errorString());
            } else {
                HWARN("Error opening" << aPath << f.errorString());
            }
        } else if (!f.remove()) {
            HWARN("Error removing" << aPath << f.errorString());
        }
    } else {
        HWARN("Failed to create" << dir.absolutePath());
    }
    return false;
}

bool HarbourJson::load(QString aPath, QVariantMap& aRoot)
{
    QFile f(aPath);
    if (f.exists()) {
        if (f.open(QIODevice::ReadOnly)) {
            HDEBUG("reading" << aPath);
#if QT_VERSION >= 0x050000
            QJsonDocument doc(QJsonDocument::fromJson(f.readAll()).object());
            if (!doc.isEmpty()) {
                aRoot = doc.toVariant().toMap();
                return true;
            }
#else
            QJson::Parser parser;
            QVariant result = parser.parse(&f);
            if (result.isValid()) {
                aRoot = result.toMap();
                return true;
            } else {
                HWARN("Failed to parse" << qPrintable(aPath));
            }
#endif
        } else {
            HDEBUG("can't open" << aPath << f.errorString());
        }
    }
    return false;
}
