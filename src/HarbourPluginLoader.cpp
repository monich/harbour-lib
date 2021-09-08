/*
 * Copyright (C) 2017-2021 Jolla Ltd.
 * Copyright (C) 2017-2021 Slava Monich <slava.monich@jolla.com>
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
 */

#include "HarbourPluginLoader.h"
#include "HarbourSystem.h"
#include "HarbourDebug.h"

// These are included from qqmlengine.h
#include <QUrl>
#include <QObject>
#include <QMap>
#include <QJSEngine>
#include <QQmlError>

#if QT_VERSION >= 0x050000

// Unprotect QQmlEngine::d_func
#define private public
#include <QQmlEngine>
#undef private

#include <qqml.h>
#include <QQmlEngine>
#include <QQmlComponent>

#include <dlfcn.h>

// This hack allows (in most cases) to use prohibited QML imports by
// re-registering them under a different name

// These:
// QQmlAttachedPropertiesFunc QQmlType::attachedPropertiesFunction() const
// _ZNK8QQmlType26attachedPropertiesFunctionEv
//
// const QMetaObject* QQmlType::attachedPropertiesType() const
// _ZNK8QQmlType22attachedPropertiesTypeEv
//
// have been replaced with these in Qt 5.6:
//
// QQmlAttachedPropertiesFunc QQmlType::attachedPropertiesFunction(QQmlEnginePrivate*) const
// _ZNK8QQmlType26attachedPropertiesFunctionEP17QQmlEnginePrivate
//
// const QMetaObject* QQmlType::attachedPropertiesType(QQmlEnginePrivate*) const
// _ZNK8QQmlType22attachedPropertiesTypeEP17QQmlEnginePrivate
//

#define LIBQT5QML_SO "libQt5Qml.so.5"

// sym,ret,name,args
#define QMLTYPE_FUNCTIONS(f) \
    f("_ZN12QQmlMetaType7qmlTypeERK7QStringii", \
        QQmlType*, qmlType, (QString const&, int, int)) \
    f("_ZNK8QQmlType23propertyValueSourceCastEv", \
        int, propertyValueSourceCast, (QQmlType*)) \
    f("_ZNK8QQmlType10createSizeEv", \
        int, createSize, (QQmlType*)) \
    f("_ZNK8QQmlType10metaObjectEv", \
        const QMetaObject*, metaObject, (QQmlType*)) \
    f("_ZNK8QQmlType16parserStatusCastEv", \
        int, parserStatusCast, (QQmlType*)) \
    f("_ZNK8QQmlType28propertyValueInterceptorCastEv", \
        int, propertyValueInterceptorCast, (QQmlType*)) \
    f("_ZNK8QQmlType6typeIdEv", \
        int, typeId, (QQmlType*)) \
    f("_ZNK8QQmlType14createFunctionEv", \
        QQmlType::CreateFunc, createFunction, (QQmlType*)) \
    f("_ZNK8QQmlType11qListTypeIdEv", \
        int, qListTypeId, (QQmlType*)) \
    f("_ZNK8QQmlType26attachedPropertiesFunctionEv", \
        QQmlAttachedPropertiesFunc, AttachedPropertiesFunctionProc,(QQmlType*)) \
    f("_ZNK8QQmlType22attachedPropertiesTypeEv", \
        const QMetaObject*, AttachedPropertiesTypeProc,(QQmlType*)) \
    f("_ZNK8QQmlType26attachedPropertiesFunctionEP17QQmlEnginePrivate", \
        QQmlAttachedPropertiesFunc, AttachedPropertiesFunctionProc6, (QQmlType*,QQmlEnginePrivate*)) \
    f("_ZNK8QQmlType22attachedPropertiesTypeEP17QQmlEnginePrivate", \
        const QMetaObject*, AttachedPropertiesTypeProc6, (QQmlType*,QQmlEnginePrivate*))

// PRIVATE QT API!
class Q_QML_EXPORT QQmlType
{
public:
    int typeId() const;
    int qListTypeId() const;
    typedef void (*CreateFunc)(void *);
    CreateFunc createFunction() const;
    int createSize() const;
    const QMetaObject *metaObject() const;
    int parserStatusCast() const;
    int propertyValueSourceCast() const;
    int propertyValueInterceptorCast() const;
    // attachedPropertiesFunction
    // attachedPropertiesType
};

// PRIVATE QT API!
class Q_QML_EXPORT QQmlMetaType
{
public:
    static QQmlType* qmlType(const QString &qualifiedName, int, int);
};

static const char* QmlTypeSymbols[] = {
#define QMLTYPE_SYMBOL(sym,ret,name,args) sym,
    QMLTYPE_FUNCTIONS(QMLTYPE_SYMBOL)
};

typedef struct _QmlTypeFunctions {
#define QMLTYPE_TYPEDEF(sym,ret,name,args) ret (*name) args;
QMLTYPE_FUNCTIONS(QMLTYPE_TYPEDEF)
} QmlTypeFunctions;

#define _N(a) (sizeof(a)/sizeof((a)[0]))
#define NUM_FUNCTIONS _N(QmlTypeSymbols)
Q_STATIC_ASSERT(sizeof(QmlTypeFunctions) == NUM_FUNCTIONS*sizeof(void*));

// ==========================================================================
// HarbourPluginLoader::LibQt5Qml
// ==========================================================================

class HarbourPluginLoader::LibQt5Qml {
public:

    LibQt5Qml();

public:
    void* iHandle;
    union {
        QmlTypeFunctions fn;
        void* ptr[NUM_FUNCTIONS];
    } iSym;
};

HarbourPluginLoader::LibQt5Qml::LibQt5Qml() :
    iHandle(HarbourDlopen(LIBQT5QML_SO, RTLD_LAZY))
{
    memset(&iSym, 0, sizeof(iSym));
    if (iHandle) {
        // Resolve unstable symbols
        for (uint i = 0; i < NUM_FUNCTIONS; i++) {
            iSym.ptr[i] = dlsym(iHandle, QmlTypeSymbols[i]);
            HDEBUG(QmlTypeSymbols[i] << (iSym.ptr[i] ? "OK" : "missing"));
        }
        // No need to ever call dlclose(iHandle);
    } else {
        HWARN("Failed to load" << LIBQT5QML_SO);
    }
}

// ==========================================================================
// HarbourPluginLoader::Private
// ==========================================================================

class HarbourPluginLoader::Private {
public:
    Private(QQmlEngine* aEngine, QString aModule, int aMajor, int aMinor);

    QQmlType* qmlType(QString aName);

    void reRegisterType(QQmlType* aType, const char* aQmlName,
        const char* aModule, int aMajor, int aMinor);
    void reRegisterType(const char* aOriginalQmlName, const char* aNewQmlName,
        const char* aModule, int aMajor, int aMinor);
    void reRegisterType(const char* aQmlName,
        const char* aModule, int aMajor, int aMinor);

public:
    static const LibQt5Qml gLibQt5Qml;

public:
    QQmlEngine* iEngine;
    QString iModule;
    int iMajor;
    int iMinor;
    bool iLoaded;
};

const HarbourPluginLoader::LibQt5Qml HarbourPluginLoader::Private::gLibQt5Qml;

HarbourPluginLoader::Private::Private(
    QQmlEngine* aEngine,
    QString aModule,
    int aMajor,
    int aMinor) :
    iEngine(aEngine),
    iModule(aModule),
    iMajor(aMajor),
    iMinor(aMinor),
    iLoaded(false)
{
    HASSERT(gLibQt5Qml.iHandle);
    if (gLibQt5Qml.iHandle) {
        // Load the actual import library
        QQmlComponent* component = new QQmlComponent(iEngine);
        component->setData(QString("import QtQuick 2.0\nimport %1 %2.%3\nQtObject {}").
            arg(iModule).arg(iMajor).arg(iMinor).toUtf8(), QUrl());
        if (component->status() == QQmlComponent::Ready) {
            delete component->create();
            iLoaded = true;
        } else {
            HWARN(component->errors());
        }
        delete component;
    }
}

QQmlType*
HarbourPluginLoader::Private::qmlType(
    QString aName)
{
    const QString fullName(iModule + '/' + aName);
    QQmlType* type = gLibQt5Qml.iSym.fn.qmlType ?
        gLibQt5Qml.iSym.fn.qmlType(fullName, iMajor, iMinor) :
        Q_NULLPTR;
    if (!type) {
        HWARN("Failed to load" << fullName);
    }
    return type;
}

void
HarbourPluginLoader::Private::reRegisterType(
    const char* aQmlName,
    const char* aModule,
    int aMajor,
    int aMinor)
{
    // Re-register with the same type name (in different module)
    reRegisterType(qmlType(aQmlName), aQmlName, aModule, aMajor, aMinor);
}

void
HarbourPluginLoader::Private::reRegisterType(
    const char* aOrigName,
    const char* aNewName,
    const char* aModule,
    int aMajor,
    int aMinor)
{
    // Re-register with a different type name (and a module)
    reRegisterType(qmlType(aOrigName), aNewName, aModule, aMajor, aMinor);
}

// Re-registers the existing QML type under a different name/module
void
HarbourPluginLoader::Private::reRegisterType(
    QQmlType* aType,
    const char* aQmlName,
    const char* aModule,
    int aMajor,
    int aMinor)
{
    if (aType && iEngine &&
        gLibQt5Qml.iSym.fn.typeId &&
        gLibQt5Qml.iSym.fn.qListTypeId &&
        gLibQt5Qml.iSym.fn.createSize &&
        gLibQt5Qml.iSym.fn.createFunction &&
        gLibQt5Qml.iSym.fn.metaObject &&
        gLibQt5Qml.iSym.fn.parserStatusCast &&
        gLibQt5Qml.iSym.fn.propertyValueSourceCast &&
        gLibQt5Qml.iSym.fn.propertyValueInterceptorCast) {
        // Get around the ABI break in Qt 5.6
        QQmlAttachedPropertiesFunc attachedPropertiesFunction =
            gLibQt5Qml.iSym.fn.AttachedPropertiesFunctionProc ?
            gLibQt5Qml.iSym.fn.AttachedPropertiesFunctionProc(aType) :
            gLibQt5Qml.iSym.fn.AttachedPropertiesFunctionProc6 ?
            gLibQt5Qml.iSym.fn.AttachedPropertiesFunctionProc6(aType, iEngine->d_func()) :
            NULL;
        const QMetaObject *attachedPropertiesMetaObject =
            gLibQt5Qml.iSym.fn.AttachedPropertiesTypeProc ?
            gLibQt5Qml.iSym.fn.AttachedPropertiesTypeProc(aType) :
            gLibQt5Qml.iSym.fn.AttachedPropertiesTypeProc6 ?
            gLibQt5Qml.iSym.fn.AttachedPropertiesTypeProc6(aType, iEngine->d_func()) :
            NULL;
        QQmlPrivate::RegisterType type = {
            0, // int version;
            gLibQt5Qml.iSym.fn.typeId(aType),  // int typeId;
            gLibQt5Qml.iSym.fn.qListTypeId(aType), // int listId;
            gLibQt5Qml.iSym.fn.createSize(aType), // int objectSize;
            gLibQt5Qml.iSym.fn.createFunction(aType), // void (*create)(void *);
            QString(), // QString noCreationReason;
            aModule, // const char *uri;
            aMajor, // int versionMajor;
            aMinor, // int versionMinor;
            aQmlName, // const char *elementName;
            gLibQt5Qml.iSym.fn.metaObject(aType), // const QMetaObject *metaObject;
            attachedPropertiesFunction,
            attachedPropertiesMetaObject,
            gLibQt5Qml.iSym.fn.parserStatusCast(aType), // int parserStatusCast;
            gLibQt5Qml.iSym.fn.propertyValueSourceCast(aType), // int valueSourceCast;
            gLibQt5Qml.iSym.fn.propertyValueInterceptorCast(aType), // int valueInterceptorCast;
            Q_NULLPTR, // QObject *(*extensionObjectCreate)(QObject *);
            Q_NULLPTR, // const QMetaObject *extensionMetaObject;
            Q_NULLPTR, // QQmlCustomParser *customParser;
            0  // int revision;
        };
        QQmlPrivate::qmlregister(QQmlPrivate::TypeRegistration, &type);
    }
}

#endif // QT_VERSION >= 0x050000

// ==========================================================================
// HarbourPluginLoader
// ==========================================================================

HarbourPluginLoader::HarbourPluginLoader(
    QQmlEngine* aEngine,
    QString aModule,
    int aMajor,
    int aMinor)
{
#if QT_VERSION >= 0x050000
    iPrivate = new Private(aEngine, aModule, aMajor, aMinor);
#else
    iPrivate = NULL;
#pragma message "HarbourPluginLoader only supports Qt5"
#endif
}

HarbourPluginLoader::~HarbourPluginLoader()
{
    delete iPrivate;
}

void
HarbourPluginLoader::reRegisterType(
    const char* aQmlName,
    const char* aModule,
    int aMajor,
    int aMinor)
{
#if QT_VERSION >= 0x050000
    // Re-register with the same type name (in different module)
    iPrivate->reRegisterType(aQmlName, aModule, aMajor, aMinor);
#endif
}

void HarbourPluginLoader::reRegisterType(
    const char* aOrigName,
    const char* aNewName,
    const char* aModule,
    int aMajor,
    int aMinor)
{
#if QT_VERSION >= 0x050000
    // Re-register with a different type name (and a module)
    iPrivate->reRegisterType(aOrigName, aNewName, aModule, aMajor, aMinor);
#endif
}

bool
HarbourPluginLoader::isValid() const
{
#if QT_VERSION >= 0x050000
    return iPrivate->iLoaded;
#else
    return false;
#endif
}
