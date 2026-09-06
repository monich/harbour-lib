/*
 * Copyright (C) 2020-2026 Slava Monich <slava@monich.com>
 * Copyright (C) 2020-2021 Jolla Ltd.
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

#ifndef HARBOUR_SYSTEM_INFO_H
#define HARBOUR_SYSTEM_INFO_H

#include <QtCore/QObject>
#include <QtCore/QVector>

class QQmlEngine;
class QJSEngine;

// Note that package version queries may not work in a sadbox if rpm
// binary isn't there.
class HarbourSystemInfo :
    public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString osName READ osName CONSTANT)
    Q_PROPERTY(QString osVersion READ osVersion CONSTANT)

public:
    explicit HarbourSystemInfo(QObject* aParent = NULL);
    ~HarbourSystemInfo();

    // Callback for qmlRegisterSingletonType<HarbourSystemInfo>
    static QObject* createSingleton(QQmlEngine*, QJSEngine*);

    QString osName() const;
    QString osVersion() const;

    Q_INVOKABLE QString packageVersion(QString);
    Q_INVOKABLE int osVersionCompare(QString);
    Q_INVOKABLE static int compareVersions(QString, QString);

    QString queryPackageVersion(QString) const;
    static int osVersionCompareWith(QString);

private:
    class Private;
    Private* iPrivate;
};

#endif // HARBOUR_SYSTEM_INFO_H
