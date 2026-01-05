/*
 * Copyright (C) 2019-2025 Slava Monich <slava@monich.com>
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
#include "HarbourParentSignalQueueObject.h"

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

// s(SignalName,signalName)
#define QUEUED_SIGNALS(s) \
    s(Image,image) \
    s(Source,source) \
    s(MirrorHorizontally,mirrorHorizontally) \
    s(MirrorVertically,mirrorVertically)


enum HarbourSingleImageProviderSignal {
    #define SIGNAL_ENUM_(Name,name) Signal##Name##Changed,
    QUEUED_SIGNALS(SIGNAL_ENUM_)
    #undef SIGNAL_ENUM_
    HarbourSingleImageProviderSignalCount
};

typedef HarbourParentSignalQueueObject<HarbourSingleImageProvider,
    HarbourSingleImageProviderSignal, HarbourSingleImageProviderSignalCount>
    HarbourSingleImageProviderPrivateBase;

class HarbourSingleImageProvider::Private :
    public HarbourSingleImageProviderPrivateBase
{
    Q_OBJECT

    static const SignalEmitter gSignalEmitters [];

public:
    Private(HarbourSingleImageProvider*);
    ~Private();

    bool setEngine(QQmlEngine*);
    void set(const QImage&, Options);
    void registerProvider(ImageProvider*);

public Q_SLOTS:
    void onEngineDied();

public:
    Options iOptions;
    QImage iImage;
    QString iId;
    QString iSourceUri;
    QQmlEngine* iEngine;
};

const HarbourSingleImageProvider::Private::SignalEmitter
HarbourSingleImageProvider::Private::gSignalEmitters [] = {
    #define SIGNAL_EMITTER_(Name,name) &HarbourSingleImageProvider::name##Changed,
    QUEUED_SIGNALS(SIGNAL_EMITTER_)
    #undef  SIGNAL_EMITTER_
};

HarbourSingleImageProvider::Private::Private(
    HarbourSingleImageProvider* aParent) :
    HarbourSingleImageProviderPrivateBase(aParent, gSignalEmitters),
    iOptions(Default),
    iEngine(Q_NULLPTR)
{}

HarbourSingleImageProvider::Private::~Private()
{
    if (!iId.isEmpty()) {
        iEngine->removeImageProvider(iId);
    }
}

void
HarbourSingleImageProvider::Private::registerProvider(
    ImageProvider* aProvider)
{
    iId = qStringPrintf("HarbourSingleImageProvider%p", aProvider);
    iSourceUri = QString("image://%1/%2").arg(iId, ImageProvider::IMAGE_NAME);
    HDEBUG("registering provider" << iId);
    iEngine->addImageProvider(iId, aProvider);
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
            registerProvider(new ImageProvider(iImage));
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
    queueSignal(SignalImageChanged);
    queueSignal(SignalSourceChanged);
    if ((iOptions & MirrorHorizontally) != mirrorHorizontally) {
        queueSignal(SignalMirrorHorizontallyChanged);
    }
    if ((iOptions & MirrorVertically) != mirrorVertically) {
        queueSignal(SignalMirrorVerticallyChanged);
    }

    iOptions = aOptions;
    iImage = (aImage.width() && (mirrorHorizontally | mirrorVertically)) ?
        aImage.mirrored(mirrorHorizontally, mirrorVertically) : aImage;

    // Allocate new provider before unregistering the previous one,
    // to ensure that the provider's pointer (and therefore, its id
    // which is derived from the pointer) really changes. Deallocating
    // the object and immediately allocating a new one may (and often
    // does) reuse the same address. The id (and the image source URL)
    // must change, or else the image may remain in the cache and won't
    // be updated.
    ImageProvider* provider = (iEngine && !iImage.isNull()) ?
        new ImageProvider(aImage) : Q_NULLPTR;

    if (!iId.isEmpty()) {
        iEngine->removeImageProvider(iId);
    }
    if (provider) {
        registerProvider(provider);
    }
}

void
HarbourSingleImageProvider::Private::onEngineDied()
{
    HDEBUG("engine died");
    iEngine = Q_NULLPTR;
    iId = QString();
    queueSignal(SignalImageChanged);
    queueSignal(SignalSourceChanged);
    emitQueuedSignals();
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
    HDEBUG(aImage);
    iPrivate->set(aImage, iPrivate->iOptions);
    iPrivate->emitQueuedSignals();
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
        iPrivate->set(iPrivate->iImage, iPrivate->iOptions ^ MirrorHorizontally);
        iPrivate->emitQueuedSignals();
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
        iPrivate->set(iPrivate->iImage, iPrivate->iOptions ^ MirrorVertically);
        iPrivate->emitQueuedSignals();
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
    HDEBUG(aImage << aOptions);
    iPrivate->set(aImage, aOptions);
    iPrivate->emitQueuedSignals();
}

void
HarbourSingleImageProvider::clear()
{
    if (!iPrivate->iImage.isNull()) {
        HDEBUG("clearing image");
        iPrivate->set(QImage(), iPrivate->iOptions);
        iPrivate->emitQueuedSignals();
    }
}

#include "HarbourSingleImageProvider.moc"
