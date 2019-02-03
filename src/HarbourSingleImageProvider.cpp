/*
 * Copyright (C) 2019 Jolla Ltd.
 * Copyright (C) 2019 Slava Monich <slava.monich@jolla.com>
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
 *      notice, this list of conditions and the following disclaimer in
 *      the documentation and/or other materials provided with the
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

#include "HarbourSingleImageProvider.h"

#include "HarbourDebug.h"

#include <QQuickImageProvider>
#include <QQmlEngine>
#include <QQmlContext>

// ==========================================================================
// HarbourSingleImageProvider::ImageProvider
// ==========================================================================

class HarbourSingleImageProvider::ImageProvider : public QQuickImageProvider {
public:
    static const QString IMAGE_NAME;

    ImageProvider(QImage aImage);

    QImage requestImage(const QString& aId, QSize* aSize,
        const QSize& aRequested) Q_DECL_OVERRIDE;

private:
    QImage iImage;
};

const QString HarbourSingleImageProvider::ImageProvider::IMAGE_NAME("image");

HarbourSingleImageProvider::ImageProvider::ImageProvider(QImage aImage) :
    QQuickImageProvider(Image),
    iImage(aImage)
{
}

QImage HarbourSingleImageProvider::ImageProvider::requestImage(const QString& aId,
    QSize* aSize, const QSize& aRequested)
{
    HDEBUG(aId);
    if (aSize) {
        *aSize = iImage.size();
    }
    return iImage;
}

// ==========================================================================
// HarbourSingleImageProvider::Private
// ==========================================================================

class HarbourSingleImageProvider::Private : public QObject {
    Q_OBJECT

public:
    Private(HarbourSingleImageProvider* aParent);
    ~Private();

    bool setEngine(QQmlEngine* aEngine);
    bool setImage(QImage aImage);
    void registerProvider();

public Q_SLOTS:
    void onEngineDied();

public:
    QImage iImage;
    QString iId;
    QString iSourceUri;
    QQmlEngine* iEngine;
};

HarbourSingleImageProvider::Private::Private(HarbourSingleImageProvider* aParent) :
    QObject(aParent),
    iEngine(Q_NULLPTR)
{
}

HarbourSingleImageProvider::Private::~Private()
{
    if (!iId.isEmpty()) {
        iEngine->removeImageProvider(iId);
    }
}

void HarbourSingleImageProvider::Private::registerProvider()
{
    QQmlImageProviderBase* provider = new ImageProvider(iImage);
    iId = QString().sprintf("HarbourSingleImageProvider%p", provider);
    iSourceUri = QString("image://%1/%2").arg(iId, ImageProvider::IMAGE_NAME);
    HDEBUG("registering provider" << iId);
    iEngine->addImageProvider(iId, provider);
}

bool HarbourSingleImageProvider::Private::setEngine(QQmlEngine* aEngine)
{
    if (iEngine) {
        if (!iId.isEmpty()) {
            iEngine->removeImageProvider(iId);
            iId  = QString();
        }
        iEngine->disconnect(this);
    }
    iEngine = aEngine;
    if (iEngine) {
        connect(iEngine, SIGNAL(destroyed(QObject*)), SLOT(onEngineDied()));
        if (!iImage.isNull()) {
            registerProvider();
            return true;
        }
    }
    return false;
}

bool HarbourSingleImageProvider::Private::setImage(QImage aImage)
{
    HDEBUG(aImage);
    if (!iId.isEmpty()) {
        iEngine->removeImageProvider(iId);
    }
    iImage = aImage;
    if (iEngine && !iImage.isNull()) {
        registerProvider();
        return true;
    }
    return false;
}

void HarbourSingleImageProvider::Private::onEngineDied()
{
    HDEBUG("engine died");
    iEngine = Q_NULLPTR;
    iId = QString();
}

// ==========================================================================
// HarbourSingleImageProvider
// ==========================================================================

HarbourSingleImageProvider::HarbourSingleImageProvider(QObject* aParent) :
    QObject(aParent),
    iPrivate(new Private(this))
{
}

void HarbourSingleImageProvider::classBegin()
{
}

void HarbourSingleImageProvider::componentComplete()
{
    QQmlContext* context = QQmlEngine::contextForObject(this);
    HDEBUG(context);
    if (context) {
        QQmlEngine* engine = context->engine();
        HDEBUG(engine);
        if (iPrivate->setEngine(engine)) {
            Q_EMIT sourceChanged();
        }
    }
}

QImage HarbourSingleImageProvider::image() const
{
    return iPrivate->iImage;
}

void HarbourSingleImageProvider::setImage(QImage aImage)
{
    HDEBUG(aImage);
    if (iPrivate->setImage(aImage)) {
        Q_EMIT sourceChanged();
    }
    Q_EMIT imageChanged();
}

QString HarbourSingleImageProvider::source() const
{
    return iPrivate->iSourceUri;
}

#include "HarbourSingleImageProvider.moc"
