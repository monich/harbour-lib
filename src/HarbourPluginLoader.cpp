/*
 * Copyright (C) 2017 Jolla Ltd.
 * Copyright (C) 2017 Slava Monich <slava.monich@jolla.com>
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
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *   3. Neither the name of Jolla Ltd nor the names of its contributors may
 *      be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "HarbourPluginLoader.h"
#include "HarbourDebug.h"

#include <qqml.h>
#include <QQmlEngine>
#include <QQmlComponent>

// This hack allows (in some cases) to use prohibited QML imports by
// re-registering them under a different name

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
    bool iLoaded;
    QQmlEngine* iEngine;
    QString iModule;
    int iMajor;
    int iMinor;
};

HarbourPluginLoader::Private::Private(
    QQmlEngine* aEngine,
    QString aModule,
    int aMajor,
    int aMinor) :
    iLoaded(false),
    iEngine(aEngine),
    iModule(aModule),
    iMajor(aMajor),
    iMinor(aMinor)
{
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

HarbourPluginLoader::Private::~Private()
{
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
            aType->attachedPropertiesFunction(NULL /*iEngine->d_func()*/),
            aType->attachedPropertiesType(NULL /*iEngine->d_func()*/),
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

// ==========================================================================
// HarbourPluginLoader
// ==========================================================================

HarbourPluginLoader::HarbourPluginLoader(
    QQmlEngine* aEngine,
    QString aModule,
    int aMajor,
    int aMinor) :
    iPrivate(new Private(aEngine, aModule, aMajor, aMinor))
{}

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
    // Re-register with the same type name (in different module)
    iPrivate->reRegisterType(aQmlName, aModule, aMajor, aMinor);
}

void HarbourPluginLoader::reRegisterType(
    const char* aOrigName,
    const char* aNewName,
    const char* aModule,
    int aMajor,
    int aMinor)
{
    // Re-register with a different type name (and a module)
    iPrivate->reRegisterType(aOrigName, aNewName, aModule, aMajor, aMinor);
}

bool
HarbourPluginLoader::isValid() const
{
    return iPrivate->iLoaded;
}
