/*
 * Copyright (C) 2016 Jolla Ltd.
 * Contact: Slava Monich <slava.monich@jolla.com>
 *
 * You may use this file under the terms of BSD license as follows:
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

#ifndef HARBOUR_TRANSFER_METHODS_MODEL_H
#define HARBOUR_TRANSFER_METHODS_MODEL_H

#include "HarbourTransferMethodInfo.h"
#include <QLocale>
#include <QRegExp>

class QTranslator;
class OrgNemoTransferEngine;

class HarbourTransferMethodsModel: public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QString filter READ filter WRITE setFilter NOTIFY filterChanged)

public:
    enum TransferMethodsRole {
        DisplayNameRole = Qt::UserRole + 1,
        UserNameRole,
        MethodIdRole,
        ShareUIPathRole,
        AccountIdRole
    };

public:
    explicit HarbourTransferMethodsModel(QObject* aParent = NULL);
    ~HarbourTransferMethodsModel();

    static bool loadTranslations(QTranslator* aTranslator, QLocale aLocale);

    int count() const;
    QString filter() const;
    void setFilter(QString filter);

    int rowCount(const QModelIndex& aParent) const;
    QVariant data(const QModelIndex& aIndex, int aRole) const;

protected:
    QHash<int,QByteArray> roleNames() const;

private:
    void filterModel();
    static QRegExp regExp(QString aRegExp);

private Q_SLOTS:
    void requestUpdate();
    void onTransferMethodsFinished(QDBusPendingCallWatcher* aWatch);

Q_SIGNALS:
    void countChanged();
    void filterChanged();

private:
    OrgNemoTransferEngine* iTransferEngine;
    QString iFilter;
    QList<HarbourTransferMethodInfo> iMethodList;
    QList<int> iFilteredList;
};

#endif // HARBOUR_TRANSFER_METHODS_MODEL_H
