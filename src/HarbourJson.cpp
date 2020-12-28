/*
 * Copyright (C) 2015-2020 Jolla Ltd.
 * Copyright (C) 2015-2020 Slava Monich <slava@monich.com>
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

#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

bool HarbourJson::save(const QString& aPath, const QVariantMap& aMap)
{
    QFileInfo file(aPath);
    QDir dir(file.dir());
    if (dir.mkpath(dir.absolutePath())) {
        const QString absPath(file.absoluteFilePath());
        QFile f(absPath);
        if (!aMap.isEmpty()) {
            struct stat st;
            const QByteArray pathBytes(absPath.toLocal8Bit());
            const char* path = pathBytes.constData();
            const bool haveFileAttr = (stat(path, &st) == 0);

            if (f.open(QIODevice::WriteOnly)) {
                bool ok;
#if QT_VERSION >= 0x050000
                if (f.write(QJsonDocument::fromVariant(aMap).toJson()) >= 0) {
                    ok = true;
                } else {
                    HWARN("Error writing" << absPath << f.errorString());
                }
#else
                QJson::Serializer serializer;
                QByteArray json = serializer.serialize(aMap);
                if (!json.isNull()) {
                    if (f.write(json) >= 0) {
                        ok = true;
                    } else {
                        HWARN("Error writing" << absPath << f.errorString());
                    }
                } else {
                    HWARN("Json serialization error");
                }
#endif
                if (ok) {
                    if (haveFileAttr) {
                        // Try to restore ownership and mode
                        if (chown(path, st.st_uid, st.st_gid)) {
                            HWARN("Failed to chown" << path << ":" <<
                                strerror(errno));
                        }
                        if (chmod(path, st.st_mode & ~S_IFMT)) {
                            HWARN("Failed to chmod" << path << ":" <<
                                strerror(errno));
                        }
                    }
                    return true;
                }
            } else {
                HWARN("Error opening" << absPath << f.errorString());
            }
        } else if (!f.remove()) {
            HWARN("Error removing" << absPath << f.errorString());
        }
    } else {
        HWARN("Failed to create" << dir.absolutePath());
    }
    return false;
}

bool HarbourJson::load(const QString& aPath, QVariantMap& aRoot)
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
