/*
 * Copyright (C) 2016-2019 Jolla Ltd.
 * Copyright (C) 2016-2019 Slava Monich <slava.monich@jolla.com>
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

#ifndef HARBOUR_TRANSFER_METHODS_MODEL_H
#define HARBOUR_TRANSFER_METHODS_MODEL_H

#include "HarbourTransferMethodInfo.h"

#include <QLocale>
#include <QRegExp>

class QTranslator;
class QQmlEngine;
class QJSEngine;

class HarbourTransferMethodsModel: public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QString filter READ filter WRITE setFilter NOTIFY filterChanged)
    Q_PROPERTY(bool accountIconSupported READ accountIconSupported NOTIFY accountIconSupportedChanged)
    typedef QDBusPendingCallWatcher* (HarbourTransferMethodsModel::*RequestUpdate)();

public:
    enum TransferMethodsRole {
        DisplayNameRole = Qt::UserRole + 1,
        UserNameRole,
        MethodIdRole,
        ShareUIPathRole,
        AccountIdRole,
        AccountIconRole
    };

public:
    explicit HarbourTransferMethodsModel(QObject* aParent = Q_NULLPTR);
    ~HarbourTransferMethodsModel();

    // Callback for qmlRegisterSingletonType<HarbourTransferMethodsModel>
    static QObject* createSingleton(QQmlEngine* aEngine, QJSEngine* aScript);

    static bool loadTranslations(QTranslator* aTranslator, QLocale aLocale);

    int count() const;
    QString filter() const;
    void setFilter(QString filter);
    bool accountIconSupported() const;

    QHash<int,QByteArray> roleNames() const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex& aParent) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex& aIndex, int aRole) const Q_DECL_OVERRIDE;

private:
    void filterModel();
    static QRegExp regExp(QString aRegExp);
    QDBusPendingCallWatcher* checkTransferMethods();
    QDBusPendingCallWatcher* requestTransferMethods();
    QDBusPendingCallWatcher* requestTransferMethods2();
    void setTransferMethods2(HarbourTransferMethodInfo2List aList);

private Q_SLOTS:
    void onTransferMethodsCheckFinished(QDBusPendingCallWatcher* aWatch);
    void onTransferMethodsFinished(QDBusPendingCallWatcher* aWatch);
    void onTransferMethods2Finished(QDBusPendingCallWatcher* aWatch);
    void requestUpdate();

Q_SIGNALS:
    void countChanged();
    void filterChanged();
    void accountIconSupportedChanged();

private:
    class TransferEngine;
    TransferEngine* iTransferEngine;
    QString iFilter;
    QList<HarbourTransferMethodInfo2> iMethodList;
    QList<int> iFilteredList;
    bool iAccountIconSupported;
    RequestUpdate iRequestUpdate;
    QDBusPendingCallWatcher* iUpdateWatcher;
};

#endif // HARBOUR_TRANSFER_METHODS_MODEL_H
