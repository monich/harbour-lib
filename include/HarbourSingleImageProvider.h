/*
 * Copyright (C) 2019-2024 Slava Monich <slava@monich.com>
 * Copyright (C) 2019 Jolla Ltd.
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

#ifndef HARBOUR_SINGLE_IMAGE_PROVIDER_H
#define HARBOUR_SINGLE_IMAGE_PROVIDER_H

#include <QtCore/QObject>
#include <QtGui/QImage>
#include <QtQml/QQmlParserStatus>

class HarbourSingleImageProvider :
    public QObject,
    public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(bool mirrorHorizontally READ mirrorHorizontally WRITE setMirrorHorizontally NOTIFY mirrorHorizontallyChanged)
    Q_PROPERTY(bool mirrorVertically READ mirrorVertically WRITE setMirrorVertically NOTIFY mirrorVerticallyChanged)
    Q_PROPERTY(QImage image READ image WRITE setImage NOTIFY imageChanged)
    Q_PROPERTY(QString source READ source NOTIFY sourceChanged)
    Q_FLAGS(Options)

public:
    enum Option {
        Default = 0,
        MirrorHorizontally = 0x01,
        MirrorVertically = 0x02
    };

    Q_DECLARE_FLAGS(Options, Option)

    HarbourSingleImageProvider(QObject* aParent = Q_NULLPTR);

    QImage image() const;
    void setImage(QImage);

    bool mirrorHorizontally() const;
    void setMirrorHorizontally(bool);

    bool mirrorVertically() const;
    void setMirrorVertically(bool);

    QString source() const;

    Q_INVOKABLE void set(QImage, Options);
    Q_INVOKABLE void clear();

    // QQmlParserStatus
    void classBegin() Q_DECL_OVERRIDE;
    void componentComplete() Q_DECL_OVERRIDE;

Q_SIGNALS:
    void mirrorHorizontallyChanged();
    void mirrorVerticallyChanged();
    void imageChanged();
    void sourceChanged();

private:
    class Private;
    class ImageProvider;
    Private* iPrivate;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(HarbourSingleImageProvider::Options)

#endif // HARBOUR_SINGLE_IMAGE_PROVIDER_H
