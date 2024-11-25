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

#include "HarbourSingleImageProvider.h"

#include "HarbourDebug.h"

#include <QtQml/QQmlEngine>
#include <QtQml/QQmlContext>
#include <QtQuick/QQuickImageProvider>

#if QT_VERSION >= QT_VERSION_CHECK(5,5,0)
#  define qStringPrintf(format...) (QString::asprintf(format))
#else
#  define qStringPrintf(format...) (QString().sprintf(format))
#endif

// ==========================================================================
// HarbourSingleImageProvider::ImageProvider
// ==========================================================================

class HarbourSingleImageProvider::ImageProvider :
    public QQuickImageProvider
{
public:
    static const QString IMAGE_NAME;

    ImageProvider(QImage);

    QImage requestImage(const QString&, QSize*, const QSize&) Q_DECL_OVERRIDE;

private:
    const QImage iImage;
    const QSize iImageSize;
    const Qt::TransformationMode iTransformationMode;
};

const QString HarbourSingleImageProvider::ImageProvider::IMAGE_NAME("image");

HarbourSingleImageProvider::ImageProvider::ImageProvider(
    QImage aImage) :
    QQuickImageProvider(Image),
    iImage(aImage),
    iImageSize(aImage.size()),
    iTransformationMode(Qt::SmoothTransformation)
{
}

QImage
HarbourSingleImageProvider::ImageProvider::requestImage(
    const QString& aId,
    QSize* aSize,
    const QSize& aRequested)
{
    HDEBUG(aId << aRequested);
    if (aSize) {
        *aSize = iImageSize;
    }
    return (aRequested.isEmpty() || iImageSize == aRequested) ? iImage :
        iImage.scaled(aRequested, Qt::IgnoreAspectRatio, iTransformationMode);
}

// ==========================================================================
// HarbourSingleImageProvider::Private
// ==========================================================================

class HarbourSingleImageProvider::Private :
    public QObject
{
    Q_OBJECT

public:
    class SaveState
    {
    public:
        SaveState(HarbourSingleImageProvider*);
        void emitSignals(HarbourSingleImageProvider*) const;

    private:
        const Options iOptions;
        const QString iId;
    };

    Private(HarbourSingleImageProvider*);
    ~Private();

    bool setEngine(QQmlEngine*);
    void set(const QImage&, Options);
    void registerProvider();

public Q_SLOTS:
    void onEngineDied();

public:
    Options iOptions;
    QImage iImage;
    QString iId;
    QString iSourceUri;
    QQmlEngine* iEngine;
};

HarbourSingleImageProvider::Private::Private(HarbourSingleImageProvider* aParent) :
    QObject(aParent),
    iOptions(Default),
    iEngine(Q_NULLPTR)
{
}

HarbourSingleImageProvider::Private::~Private()
{
    if (!iId.isEmpty()) {
        iEngine->removeImageProvider(iId);
    }
}

void
HarbourSingleImageProvider::Private::registerProvider()
{
    QQmlImageProviderBase* provider = new ImageProvider(iImage);
    iId = qStringPrintf("HarbourSingleImageProvider%p", provider);
    iSourceUri = QString("image://%1/%2").arg(iId, ImageProvider::IMAGE_NAME);
    HDEBUG("registering provider" << iId);
    iEngine->addImageProvider(iId, provider);
}

bool
HarbourSingleImageProvider::Private::setEngine(
    QQmlEngine* aEngine)
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

void
HarbourSingleImageProvider::Private::set(
    const QImage& aImage,
    Options aOptions)
{
    const bool mirrorHorizontally = aOptions.testFlag(MirrorHorizontally);
    const bool mirrorVertically = aOptions.testFlag(MirrorVertically);
    HDEBUG(aImage << mirrorHorizontally << mirrorVertically);
    if (!iId.isEmpty()) {
        iEngine->removeImageProvider(iId);
    }
    iOptions = aOptions;
    iImage = (aImage.width() && (mirrorHorizontally | mirrorVertically)) ?
        aImage.mirrored(mirrorHorizontally, mirrorVertically) : aImage;
    if (iEngine && !iImage.isNull()) {
        registerProvider();
    }
}

void
HarbourSingleImageProvider::Private::onEngineDied()
{
    HDEBUG("engine died");
    iEngine = Q_NULLPTR;
    iId = QString();
}


// ==========================================================================
// HarbourSingleImageProvider::Private::SaveState
// ==========================================================================

HarbourSingleImageProvider::Private::SaveState::SaveState(
    HarbourSingleImageProvider* aObject) :
    iOptions(aObject->iPrivate->iOptions),
    iId(aObject->iPrivate->iId)
{}

void
HarbourSingleImageProvider::Private::SaveState::emitSignals(
    HarbourSingleImageProvider* aObject) const
{
    Private* priv = aObject->iPrivate;
    const Options newOpts(priv->iOptions);

    if (iId != aObject->iPrivate->iId) {
        Q_EMIT aObject->sourceChanged();
    }
    if ((iOptions & MirrorHorizontally) != (newOpts & MirrorHorizontally)) {
        Q_EMIT aObject->mirrorHorizontallyChanged();
    }
    if ((iOptions & MirrorVertically) != (newOpts & MirrorVertically)) {
        Q_EMIT aObject->mirrorVerticallyChanged();
    }
}

// ==========================================================================
// HarbourSingleImageProvider
// ==========================================================================

HarbourSingleImageProvider::HarbourSingleImageProvider(
    QObject* aParent) :
    QObject(aParent),
    iPrivate(new Private(this))
{
}

void
HarbourSingleImageProvider::classBegin()
{
}

void
HarbourSingleImageProvider::componentComplete()
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

QImage
HarbourSingleImageProvider::image() const
{
    return iPrivate->iImage;
}

void
HarbourSingleImageProvider::setImage(QImage aImage)
{
    const Private::SaveState state(this);
    HDEBUG(aImage);
    iPrivate->set(aImage, iPrivate->iOptions);
    Q_EMIT imageChanged(); // Not emitted by SaveState
    state.emitSignals(this);
}

bool
HarbourSingleImageProvider::mirrorHorizontally() const
{
    return iPrivate->iOptions.testFlag(MirrorHorizontally);
}

void
HarbourSingleImageProvider::setMirrorHorizontally(
    bool aMirrorHorizontally)
{
    if (mirrorHorizontally() != aMirrorHorizontally) {
        const Private::SaveState state(this);
        HDEBUG(aMirrorHorizontally);
        iPrivate->set(iPrivate->iImage, iPrivate->iOptions ^ MirrorHorizontally);
        state.emitSignals(this);
    }
}

bool
HarbourSingleImageProvider::mirrorVertically() const
{
    return iPrivate->iOptions.testFlag(MirrorVertically);
}

void
HarbourSingleImageProvider::setMirrorVertically(
    bool aMirrorVertically)
{
    if (mirrorVertically() != aMirrorVertically) {
        const Private::SaveState state(this);
        HDEBUG(aMirrorVertically);
        iPrivate->set(iPrivate->iImage, iPrivate->iOptions ^ MirrorVertically);
        state.emitSignals(this);
    }
}

QString
HarbourSingleImageProvider::source() const
{
    return iPrivate->iSourceUri;
}

void
HarbourSingleImageProvider::set(
    QImage aImage,
    Options aOptions)
{
    const Private::SaveState state(this);
    HDEBUG(aImage << aOptions);
    iPrivate->set(aImage, aOptions);
    Q_EMIT imageChanged(); // Not emitted by SaveState
    state.emitSignals(this);
}

void
HarbourSingleImageProvider::clear()
{
    if (!iPrivate->iImage.isNull()) {
        HDEBUG("clearing image");
        iPrivate->set(QImage(), iPrivate->iOptions);
        Q_EMIT sourceChanged();
        Q_EMIT imageChanged();
    }
}

#include "HarbourSingleImageProvider.moc"
