/*
 * Copyright (C) 2018-2020 Jolla Ltd.
 * Copyright (C) 2018-2020 Slava Monich <slava@monich.com>
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
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "HarbourTheme.h"
#include "HarbourDebug.h"

#include <QQmlEngine>

#include <dlfcn.h>

#define SILICA_SO "/usr/lib/libsailfishsilica.so.1"
#define SILICA_FUNCTIONS(f) \
    f("_ZN6Silica5Theme8instanceEv", /* Silica::Theme* Silica::Theme::instance() */ \
        QObject*, instance,()) \
    f("_ZNK6Silica5Theme5styleEv", /* Silica::Style Silica::Theme::style() const */ \
        HarbourTheme::Private::Style, style,(QObject*)) \
    f("_ZNK6Silica5Theme11colorSchemeEv", /* Silica::ColorScheme Silica::Theme::colorScheme() const */ \
        HarbourTheme::ColorScheme, colorScheme,(QObject*)) \
    f("_ZNK6Silica5Theme12primaryColorEv", /* QColor Silica::Theme::primaryColor() const */ \
        QColor, primaryColor,(QObject*)) \
    f("_ZNK6Silica5Theme12opacityFaintEv", /* qreal Silica::Theme::opacityFaint() const */ \
        qreal, opacityFaint,(QObject*)) \
    f("_ZNK6Silica5Theme10opacityLowEv", /* qreal Silica::Theme::opacityLow() const */ \
        qreal, opacityLow,(QObject*)) \
    f("_ZNK6Silica5Theme11opacityHighEv", /* qreal Silica::Theme::opacityHigh() const */ \
        qreal, opacityHigh,(QObject*)) \
    f("_ZNK6Silica5Theme14opacityOverlayEv", /* qreal Silica::Theme::opacityOverlay() const */ \
        qreal, opacityOverlay,(QObject*))

// ==========================================================================
// HarbourTheme::Private
// ==========================================================================

class HarbourTheme::Private
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

    static const qreal DEFAULT_OPACITY_FAINT;
    static const qreal DEFAULT_OPACITY_LOW;
    static const qreal DEFAULT_OPACITY_HIGH;
    static const qreal DEFAULT_OPACITY_OVERLAY;

    static void* gHandle;
    static SilicaFunctions gSilica;

public:
    static QObject* instance();
    static ColorScheme colorScheme();
    static QColor primaryColor();
    static qreal opacityFaint();
    static qreal opacityLow();
    static qreal opacityHigh();
    static qreal opacityOverlay();
};

void* HarbourTheme::Private::gHandle;
HarbourTheme::Private::SilicaFunctions HarbourTheme::Private::gSilica;

const qreal HarbourTheme::Private::DEFAULT_OPACITY_FAINT = 0.2;
const qreal HarbourTheme::Private::DEFAULT_OPACITY_LOW = 0.4;
const qreal HarbourTheme::Private::DEFAULT_OPACITY_HIGH = 0.6;
const qreal HarbourTheme::Private::DEFAULT_OPACITY_OVERLAY = 0.8;

static const char* SilicaSymbols[] = {
#define SILICA_SYMBOL(sym,ret,name,args) sym,
    SILICA_FUNCTIONS(SILICA_SYMBOL)
};

#define _N(a) (sizeof(a)/sizeof((a)[0]))
#define NUM_FUNCTIONS _N(SilicaSymbols)
Q_STATIC_ASSERT(sizeof(HarbourTheme::Private::SilicaFunctions) == NUM_FUNCTIONS*sizeof(void*));

QObject* HarbourTheme::Private::instance()
{
    return gSilica.instance ? gSilica.instance() : Q_NULLPTR;
}

HarbourTheme::ColorScheme HarbourTheme::Private::colorScheme()
{
    QObject* silica = instance();
    if (silica) {
        if (gSilica.colorScheme) {
            return gSilica.colorScheme(silica);
        } else if (gSilica.style) {
            gSilica.style(silica);
        }
    }
    return LightOnDark;
}

QColor HarbourTheme::Private::primaryColor()
{
    QObject* silica = instance();
    return (silica && gSilica.primaryColor) ? gSilica.primaryColor(silica) : QColor();
}

qreal HarbourTheme::Private::opacityFaint()
{
    QObject* silica = instance();
    return (silica && gSilica.opacityFaint) ? gSilica.opacityFaint(silica) :
        DEFAULT_OPACITY_FAINT;
}

qreal HarbourTheme::Private::opacityLow()
{
    QObject* silica = instance();
    return (silica && gSilica.opacityLow) ? gSilica.opacityLow(silica) :
        DEFAULT_OPACITY_LOW;
}

qreal HarbourTheme::Private::opacityHigh()
{
    QObject* silica = instance();
    return (silica && gSilica.opacityHigh) ? gSilica.opacityHigh(silica) :
        DEFAULT_OPACITY_HIGH;
}

qreal HarbourTheme::Private::opacityOverlay()
{
    QObject* silica = instance();
    return (silica && gSilica.opacityOverlay) ? gSilica.opacityOverlay(silica) :
        DEFAULT_OPACITY_OVERLAY;
}

// ==========================================================================
// HarbourTheme
// ==========================================================================

HarbourTheme::HarbourTheme(QObject* aParent) : QObject(aParent)
{
    if (!Private::gHandle) {
        Private::gHandle = dlopen(SILICA_SO, RTLD_LAZY);
        if (Private::gHandle) {
            void** ptr = (void**)&Private::gSilica;
            for (uint i = 0; i < NUM_FUNCTIONS; i++) {
                ptr[i] = dlsym(Private::gHandle, SilicaSymbols[i]);
                HDEBUG(SilicaSymbols[i] << (ptr[i] ? "OK" : "missing"));
            }
        }
    }
    QObject* silica = Private::instance();
    if (silica) {
        if (Private::gSilica.colorScheme) {
            // Colors update is delayed, queue this signal
            QObject::connect(silica,
                SIGNAL(colorSchemeChanged()),
                SIGNAL(colorSchemeChanged()),
                Qt::QueuedConnection);
        }
        if (Private::gSilica.primaryColor) {
            QObject::connect(silica,
                SIGNAL(primaryColorChanged()),
                SIGNAL(primaryColorChanged()));
        }
    }
}

HarbourTheme::~HarbourTheme()
{
}

// Callback for qmlRegisterSingletonType<HarbourTheme>
QObject* HarbourTheme::createSingleton(QQmlEngine*, QJSEngine*)
{
    return new HarbourTheme();
}

QColor HarbourTheme::primaryColor() const
{
    return Private::primaryColor();
}

QColor HarbourTheme::invertedPrimaryColor() const
{
    return invertedColor(Private::primaryColor());
}

QColor HarbourTheme::invertedColor(QColor aColor)
{
    if (aColor.isValid()) {
        const QRgb rgba = aColor.rgba();
        return QColor(((~(rgba & RGB_MASK)) & RGB_MASK) | (rgba & (~RGB_MASK)));
    } else {
        return aColor;
    }
}

HarbourTheme::ColorScheme HarbourTheme::colorScheme() const
{
    return Private::colorScheme();
}

bool HarbourTheme::lightOnDark() const
{
    return Private::colorScheme() == LightOnDark;
}

bool HarbourTheme::darkOnLight() const
{
    return Private::colorScheme() == DarkOnLight;
}

qreal HarbourTheme::opacityFaint() const
{
    return Private::opacityFaint();
}

qreal HarbourTheme::opacityLow() const
{
    return Private::opacityLow();
}

qreal HarbourTheme::opacityHigh() const
{
    return Private::opacityHigh();
}

qreal HarbourTheme::opacityOverlay() const
{
    return Private::opacityOverlay();
}
