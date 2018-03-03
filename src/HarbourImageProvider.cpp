/*
 * Copyright (C) 2018 Jolla Ltd.
 * Copyright (C) 2018 Slava Monich <slava@monich.com>
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
 *   3. Neither the name of Jolla Ltd nor the names of its contributors
 *      may be used to endorse or promote products derived from this
 *      software without specific prior written permission.
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

#include "HarbourImageProvider.h"
#include "HarbourDebug.h"

#include <QImageReader>
#include <QQuickWindow>

#include <dlfcn.h>

#define SILICA_SO "/usr/lib/libsailfishsilica.so.1"
#define SILICA_FUNCTIONS(f) \
    f("_ZN6Silica5Theme8instanceEv", /* Silica::Theme* Silica::Theme::instance() */ \
        HarbourImageProvider::Theme*, Instance,()) \
    f("_ZNK6Silica5Theme5styleEv", /* Silica::Style  Silica::Theme::style() const */ \
        HarbourImageProvider::Theme::Style, Style,(HarbourImageProvider::Theme*)) \
    f("_ZNK6Silica5Theme12primaryColorEv", /* QColor Silica::Theme::primaryColor() const */ \
        QColor, PrimaryColor,(HarbourImageProvider::Theme*))

// ==========================================================================
// HarbourImageProvider::Theme
// ==========================================================================

class HarbourImageProvider::Theme
{
public:
    enum Style {
        StyleLight,
        StyleDark
    };

    typedef struct _SilicaFunctions {
    #define SILICA_TYPEDEF(sym,ret,name,args) ret (*name) args;
    SILICA_FUNCTIONS(SILICA_TYPEDEF)
    } SilicaFunctions;

    static Theme* instance();
    static Style style(Theme* theme);

public:
    static void* gHandle;
    static SilicaFunctions gSilica;
};

void* HarbourImageProvider::Theme::gHandle = NULL;
HarbourImageProvider::Theme::SilicaFunctions HarbourImageProvider::Theme::gSilica;

static const char* SilicaSymbols[] = {
#define SILICA_SYMBOL(sym,ret,name,args) sym,
    SILICA_FUNCTIONS(SILICA_SYMBOL)
};

#define _N(a) (sizeof(a)/sizeof((a)[0]))
#define NUM_FUNCTIONS _N(SilicaSymbols)
Q_STATIC_ASSERT(sizeof(HarbourImageProvider::Theme::SilicaFunctions) == NUM_FUNCTIONS*sizeof(void*));

HarbourImageProvider::Theme*
HarbourImageProvider::Theme::instance()
{
    return gSilica.Instance ? gSilica.Instance() : NULL;
}

HarbourImageProvider::Theme::Style
HarbourImageProvider::Theme::style(
    Theme* aTheme)
{
    return (aTheme && gSilica.Style) ? gSilica.Style(aTheme) : StyleDark;
}

// ==========================================================================
// HarbourImageProvider::TextureFactory
// ==========================================================================

class HarbourImageProvider::TextureFactory : public QQuickTextureFactory
{
public:
    TextureFactory(QString aPath, QSize requestedSize);

    QSGTexture *createTexture(QQuickWindow *window) const Q_DECL_OVERRIDE;
    QSize textureSize() const Q_DECL_OVERRIDE;
    int textureByteCount() const Q_DECL_OVERRIDE;
    QImage image() const Q_DECL_OVERRIDE;

    QImage load() const;
    static QImage colorize(QImage aImage, QColor aColor);

private:
    QString iPath;
    QString iHighlight;
    QSize iRequestedSize;
    mutable QImage iImage;
};

HarbourImageProvider::TextureFactory::TextureFactory(
    QString aPath,
    QSize aRequestedSize) :
    iRequestedSize(aRequestedSize)
{
    const int sep1 = aPath.lastIndexOf('?');
    const int sep2 = aPath.lastIndexOf('/');
    if (sep1 >= 0 && sep1 > sep2) {
        iPath = aPath.left(sep1);
        iHighlight = aPath.mid(sep1 + 1);
        HDEBUG(iPath << iHighlight);
    } else {
        iPath = aPath;
    }
}

QImage
HarbourImageProvider::TextureFactory::load() const
{
    if (iImage.isNull() && !iPath.isEmpty()) {
        QImageReader imageReader(iPath);
        if (iRequestedSize.width() > 0 && iRequestedSize.height() > 0) {
            imageReader.setScaledSize(iRequestedSize);
        }
        if (imageReader.read(&iImage) && !iImage.isNull()) {
            HDEBUG("loaded" << qPrintable(iPath));
            if (iHighlight.isEmpty()) {
                // Invert grayscale icon colors on inverted ambience
                Theme* theme = Theme::instance();
                if (Theme::style(theme) == Theme::StyleLight &&
                    iImage.isGrayscale() &&
                    Theme::gSilica.PrimaryColor) {
                    iImage = colorize(iImage, Theme::gSilica.PrimaryColor(theme));
                }
            } else {
                // Colorization required
                QColor highlightColor(iHighlight);
                if (highlightColor.isValid()) {
                    HDEBUG(highlightColor);
                    iImage = colorize(iImage, highlightColor);
                }
            }
        } else {
            HWARN("can't load" << qPrintable(iPath));
        }
    }
    return iImage;
}

QSize
HarbourImageProvider::TextureFactory::textureSize() const
{
    return image().size();
}

int
HarbourImageProvider::TextureFactory::textureByteCount() const
{
    QSize size(image().size());
    return size.width() * size.height() * 4;
}

QImage
HarbourImageProvider::TextureFactory::image() const
{
    return load();
}

QSGTexture*
HarbourImageProvider::TextureFactory::createTexture(
    QQuickWindow* aWindow) const
{
    if (!load().isNull()) {
        QSGTexture* texture = aWindow->createTextureFromImage(iImage);
        if (qEnvironmentVariableIsSet("QSG_TRANSIENT_IMAGES")) {
            iImage = QImage();
        }
        return texture;
    }
    return NULL;
}

QImage
HarbourImageProvider::TextureFactory::colorize(
    QImage aImage,
    QColor aColor)
{
    QImage::Format format = aImage.format();
    switch (format) {
    case QImage::Format_ARGB32:
    case QImage::Format_ARGB32_Premultiplied:
        break;
    default:
        HWARN("TextureFactory: Image format not supported, doing format conversion");
        aImage = aImage.convertToFormat(QImage::Format_ARGB32_Premultiplied);
        format = QImage::Format_ARGB32_Premultiplied;
    }

    const QRgb rgb = aColor.rgba() & 0x00ffffff;
    QRgb* bits = (QRgb*)aImage.bits();
    const int numPixels = aImage.byteCount()/sizeof(QRgb);

    if (format == QImage::Format_ARGB32) {
        for (int i=0; i<numPixels; i++) {
            QRgb alpha = bits[i] & 0xff000000;
            bits[i] = alpha | rgb;
        }
    } else if (format == QImage::Format_ARGB32_Premultiplied) {
        QRgb colorPremultiplied[256];
        for (int alpha = 0; alpha<256; alpha++) {
            colorPremultiplied[alpha] =
                    (alpha << 24) |
                    (alpha*qRed(rgb)/255 << 16) |
                    (alpha*qGreen(rgb)/255 <<  8) |
                    (alpha*qBlue(rgb) /255);
        }
        for (int i=0; i<numPixels; i++) {
            int alpha = (bits[i] & 0xff000000) >> 24;
            bits[i] = colorPremultiplied[alpha];
        }
    }

    return aImage;
}

// ==========================================================================
// HarbourImageProvider
// ==========================================================================

HarbourImageProvider::HarbourImageProvider() :
    QQuickImageProvider(QQuickImageProvider::Texture)
{
    if (!Theme::gHandle) {
        Theme::gHandle = dlopen(SILICA_SO, RTLD_LAZY);
        if (Theme::gHandle) {
            void** ptr = (void**)&Theme::gSilica;
            for (uint i = 0; i < NUM_FUNCTIONS; i++) {
                ptr[i] = dlsym(Theme::gHandle, SilicaSymbols[i]);
                HDEBUG(SilicaSymbols[i] << (ptr[i] ? "OK" : "missing"));
            }
        }
    }
}

QQuickTextureFactory*
HarbourImageProvider::requestTexture(
    const QString& aId,
    QSize* aSize,
    const QSize& aRequestedSize)
{
    QString path(aId);
    HDEBUG(aId << aRequestedSize);
    if (path.startsWith("file://")) {
        path = path.mid(7);
        HDEBUG(path);
    }
    TextureFactory* factory = new TextureFactory(path, aRequestedSize);
    if (aSize) {
        *aSize = factory->textureSize();
    }
    return factory;
}
