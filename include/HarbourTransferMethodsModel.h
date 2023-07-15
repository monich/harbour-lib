/*
 * Copyright (C) 2016-2023 Slava Monich <slava@monich.com>
 * Copyright (C) 2016-2020 Jolla Ltd.
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

#ifndef HARBOUR_TRANSFER_METHODS_MODEL_H
#define HARBOUR_TRANSFER_METHODS_MODEL_H

#include "HarbourTransferMethodInfo.h"

#include <QLocale>

class QTranslator;
class QQmlEngine;
class QJSEngine;

// N.B. This model (and in-process sharing in general) doesn't
// work since Sailfish OS 4.2.0 (or more specifically, since
// declarative-transferengine-qt5 package >= 0.4.0)
//
// It can only be of interest to those who want to make their
// apps compatible with older releases of Sailfish OS.
class HarbourTransferMethodsModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QString filter READ filter WRITE setFilter NOTIFY filterChanged)
    Q_PROPERTY(bool accountIconSupported READ accountIconSupported NOTIFY accountIconSupportedChanged)
    Q_PROPERTY(bool showAccountsPending READ showAccountsPending NOTIFY showAccountsPendingChanged)
    Q_PROPERTY(bool canShowAccounts READ canShowAccounts NOTIFY canShowAccountsChanged)
    Q_PROPERTY(bool valid READ isValid NOTIFY validChanged)

public:
    explicit HarbourTransferMethodsModel(QObject* aParent = Q_NULLPTR);
    ~HarbourTransferMethodsModel() Q_DECL_OVERRIDE;

    // Callback for qmlRegisterSingletonType<HarbourTransferMethodsModel>
    static QObject* createSingleton(QQmlEngine*, QJSEngine*);

    static bool loadTranslations(QTranslator*, QLocale);

    bool isValid() const;
    int count() const;
    QString filter() const;
    void setFilter(QString);
    bool accountIconSupported() const;
    bool showAccountsPending() const;
    bool canShowAccounts() const;

    Q_INVOKABLE void showAccounts();

    // QAbstractListModel
    QHash<int,QByteArray> roleNames() const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex&) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex&, int) const Q_DECL_OVERRIDE;

Q_SIGNALS:
    void validChanged();
    void countChanged();
    void filterChanged();
    void accountIconSupportedChanged();
    void showAccountsPendingChanged();
    void canShowAccountsChanged();

private:
    class Private;
    class TransferEngine;
    Private* iPrivate;
};

#endif // HARBOUR_TRANSFER_METHODS_MODEL_H
