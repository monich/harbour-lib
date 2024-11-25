/*
 * Copyright (C) 2018-2024 Slava Monich <slava@monich.com>
 * Copyright (C) 2018-2021 Jolla Ltd.
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

#include "HarbourTemporaryFile.h"
#include "HarbourDebug.h"

#include <QtCore/QDir>
#include <QtCore/QTemporaryFile>
#include <QtCore/QTextStream>
#include <QtCore/QStandardPaths>

#ifndef qMove
#  define qMove(x) (x)
#endif

// ==========================================================================
// HarbourTemporaryFile::Private
// ==========================================================================

class HarbourTemporaryFile::Private
{
public:
    Private();
    ~Private();

    void reopen(HarbourTemporaryFile*);
    QString directoryPath() const;
    QString fileName() const;

public:
    Location iLocation;
    QTemporaryFile* iFile;
    QString iFileTemplate;
    QString iContent;
    QUrl iUrl;
};

HarbourTemporaryFile::Private::Private() :
    iLocation(Tmp),
    iFile(NULL)
{
}

HarbourTemporaryFile::Private::~Private()
{
    if (iFile) {
        iFile->close();
        delete iFile;
    }
}

inline QString
HarbourTemporaryFile::Private::fileName() const
{
    return iFile ? iFile->fileName() : QString();
}

QString
HarbourTemporaryFile::Private::directoryPath() const
{
    QStandardPaths::StandardLocation type = QStandardPaths::TempLocation;
    switch (iLocation) {
    case Downloads:
        type = QStandardPaths::DownloadLocation;
        break;
    case Tmp:
        // This is the default
        break;
    }
    return QStandardPaths::writableLocation(type);
}

void
HarbourTemporaryFile::Private::reopen(
    HarbourTemporaryFile* aObject)
{
    const QString oldFileName = fileName();
    QString newFileName;
    if (iFile && iFile->isOpen()) {
        iFile->close();
        delete iFile;
        iFile = NULL;
        iUrl = QUrl();
    }
    if (!iContent.isEmpty() && !iFileTemplate.isEmpty()) {
        iFile = new QTemporaryFile(directoryPath() +
            QDir::separator() + iFileTemplate);
        iFile->setAutoRemove(true);
        if (iFile->open()) {
            newFileName = iFile->fileName();
            iUrl = QUrl::fromLocalFile(newFileName);
            HDEBUG("writing" << qPrintable(newFileName));
            QTextStream stream(iFile);
            stream << iContent;
            stream.flush();
        } else {
            HWARN("Failed to open temporary file");
            delete iFile;
            iFile = NULL;
        }
    }
    if (fileName() != oldFileName) {
        Q_EMIT aObject->fileNameChanged();
        Q_EMIT aObject->urlChanged();
    }
}

// ==========================================================================
// HarbourTemporaryFile
// ==========================================================================

HarbourTemporaryFile::HarbourTemporaryFile(
    QObject* aParent) :
    QObject(aParent),
    iPrivate(new Private())
{
}

HarbourTemporaryFile::~HarbourTemporaryFile()
{
    delete iPrivate;
}

QString
HarbourTemporaryFile::content() const
{
    return iPrivate->iContent;
}

void
HarbourTemporaryFile::setContent(
    QString aValue)
{
    if (iPrivate->iContent != aValue) {
        iPrivate->iContent = qMove(aValue);
        iPrivate->reopen(this);
        Q_EMIT contentChanged();
    }
}

QString
HarbourTemporaryFile::fileTemplate() const
{
    return iPrivate->iFileTemplate;
}

void
HarbourTemporaryFile::setFileTemplate(
    QString aValue)
{
    if (iPrivate->iFileTemplate != aValue) {
        iPrivate->iFileTemplate = qMove(aValue);
        iPrivate->reopen(this);
        Q_EMIT fileTemplateChanged();
    }
}

HarbourTemporaryFile::Location
HarbourTemporaryFile::location() const
{
    return iPrivate->iLocation;
}

void
HarbourTemporaryFile::setLocation(
    Location aValue)
{
    if (iPrivate->iLocation != aValue) {
        iPrivate->iLocation = aValue;
        iPrivate->reopen(this);
        Q_EMIT locationChanged();
    }
}

QString
HarbourTemporaryFile::fileName() const
{
    return iPrivate->fileName();
}

QUrl
HarbourTemporaryFile::url() const
{
    return iPrivate->iUrl;
}
