/*
 * Copyright (C) 2017-2020 Jolla Ltd.
 * Copyright (C) 2017-2020 Slava Monich <slava@monich.com>
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
    QQmlAttachedPropertiesFunc attachedPropertiesFunction(QQmlEnginePrivate *engine) const;
    const QMetaObject *attachedPropertiesType(QQmlEnginePrivate *engine) const;
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
// HarbourPluginLoader::Private
// ==========================================================================

class HarbourPluginLoader::Private {
public:
    Private(QQmlEngine* aEngine, QString aModule, int aMajor, int aMinor);
    ~Private();

    QQmlType* qmlType(QString aName);

    void reRegisterType(QQmlType* aType, const char* aQmlName,
        const char* aModule, int aMajor, int aMinor);
    void reRegisterType(const char* aOriginalQmlName, const char* aNewQmlName,
        const char* aModule, int aMajor, int aMinor);
    void reRegisterType(const char* aQmlName,
        const char* aModule, int aMajor, int aMinor);

public:
    QQmlEngine* iEngine;
    QString iModule;
    int iMajor;
    int iMinor;
    bool iLoaded;
    void* iHandle;
    union {
        QmlTypeFunctions fn;
        void* ptr[NUM_FUNCTIONS];
    } iLibQt5Qml;
};

HarbourPluginLoader::Private::Private(
    QQmlEngine* aEngine,
    QString aModule,
    int aMajor,
    int aMinor) :
    iEngine(aEngine),
    iModule(aModule),
    iMajor(aMajor),
    iMinor(aMinor),
    iLoaded(false),
    iHandle(NULL)
{
    memset(&iLibQt5Qml, 0, sizeof(iLibQt5Qml));
    // Load the actual import library
    QQmlComponent* component = new QQmlComponent(iEngine);
    component->setData(QString("import QtQuick 2.0\nimport %1 %2.%3\nQtObject {}").
        arg(iModule).arg(iMajor).arg(iMinor).toUtf8(), QUrl());
    if (component->status() == QQmlComponent::Ready) {
        delete component->create();
        // Resolve unstable symbols
        iHandle = HarbourDlopen(LIBQT5QML_SO, RTLD_LAZY);
        if (iHandle) {
            for (uint i = 0; i < NUM_FUNCTIONS; i++) {
                iLibQt5Qml.ptr[i] = dlsym(iHandle, QmlTypeSymbols[i]);
                HDEBUG(QmlTypeSymbols[i] << (iLibQt5Qml.ptr[i] ? "OK" : "missing"));
            }
        } else {
            HWARN("Failed to load" << LIBQT5QML_SO);
        }
        iLoaded = true;
    } else {
        HWARN(component->errors());
    }
    delete component;
}

HarbourPluginLoader::Private::~Private()
{
    if (iHandle) {
        dlclose(iHandle);
    }
}

QQmlType*
HarbourPluginLoader::Private::qmlType(
    QString aName)
{
    QString fullName(iModule + '/' + aName);
    QQmlType* type = QQmlMetaType::qmlType(fullName, iMajor, iMinor);
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
    if (aType && iEngine) {
        // Get around the ABI break in Qt 5.6
        QQmlAttachedPropertiesFunc attachedPropertiesFunction =
            iLibQt5Qml.fn.AttachedPropertiesFunctionProc ?
            iLibQt5Qml.fn.AttachedPropertiesFunctionProc(aType) :
            iLibQt5Qml.fn.AttachedPropertiesFunctionProc6 ?
            iLibQt5Qml.fn.AttachedPropertiesFunctionProc6(aType, iEngine->d_func()) :
            NULL;
        const QMetaObject *attachedPropertiesMetaObject =
            iLibQt5Qml.fn.AttachedPropertiesTypeProc ?
            iLibQt5Qml.fn.AttachedPropertiesTypeProc(aType) :
            iLibQt5Qml.fn.AttachedPropertiesTypeProc6 ?
            iLibQt5Qml.fn.AttachedPropertiesTypeProc6(aType, iEngine->d_func()) :
            NULL;
        QQmlPrivate::RegisterType type = {
            0, // int version;
            aType->typeId(),  // int typeId;
            aType->qListTypeId(), // int listId;
            aType->createSize(), // int objectSize;
            aType->createFunction(), // void (*create)(void *);
            QString(), // QString noCreationReason;
            aModule, // const char *uri;
            aMajor, // int versionMajor;
            aMinor, // int versionMinor;
            aQmlName, // const char *elementName;
            aType->metaObject(), // const QMetaObject *metaObject;
            attachedPropertiesFunction,
            attachedPropertiesMetaObject,
            aType->parserStatusCast(), // int parserStatusCast;
            aType->propertyValueSourceCast(), // int valueSourceCast;
            aType->propertyValueInterceptorCast(), // int valueInterceptorCast;
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
