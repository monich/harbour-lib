/*
 * Copyright (C) 2016-2020 Jolla Ltd.
 * Copyright (C) 2016-2020 Slava Monich <slava.monich@jolla.com>
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

#include "HarbourTransferMethodsModel.h"
#include "HarbourDebug.h"

#include <QTranslator>
#include <QQmlEngine>
#include <QRegExp>

// ==========================================================================
// HarbourTransferMethodsModel::TransferEngine
// ==========================================================================

class HarbourTransferMethodsModel::TransferEngine: public QDBusAbstractInterface
{
    Q_OBJECT

public:
    TransferEngine(QObject* aParent) :
        QDBusAbstractInterface(QStringLiteral("org.nemo.transferengine"),
            QStringLiteral("/org/nemo/transferengine"), "org.nemo.transferengine",
            QDBusConnection::sessionBus(), aParent) {}

public: // METHODS
    inline QDBusPendingCall transferMethods()
        { return asyncCall(QStringLiteral("transferMethods")); }
    inline QDBusPendingCall transferMethods2()
        { return asyncCall(QStringLiteral("transferMethods2")); }

Q_SIGNALS: // SIGNALS
    void transferMethodListChanged();
};

// ==========================================================================
// HarbourTransferMethodsModel::Private
// ==========================================================================

class HarbourTransferMethodsModel::Private: public QObject
{
    Q_OBJECT
    typedef QDBusPendingCallWatcher* (Private::*RequestUpdate)();

public:
    enum Role {
        DisplayNameRole = Qt::UserRole + 1,
        UserNameRole,
        MethodIdRole,
        ShareUIPathRole,
        AccountIdRole,
        AccountIconRole
    };

    Private(HarbourTransferMethodsModel* aModel);
    ~Private();

public:
    HarbourTransferMethodsModel* parentModel();
    static QRegExp regExp(QString aRegExp);
    void filterModel();
    QDBusPendingCallWatcher* checkTransferMethods();
    QDBusPendingCallWatcher* requestTransferMethods();
    QDBusPendingCallWatcher* requestTransferMethods2();
    void setTransferMethods2(HarbourTransferMethodInfo2List aList);
    bool showAccounts();

private Q_SLOTS:
    void onTransferMethodsCheckFinished(QDBusPendingCallWatcher* aWatch);
    void onTransferMethodsFinished(QDBusPendingCallWatcher* aWatch);
    void onTransferMethods2Finished(QDBusPendingCallWatcher* aWatch);
    void onShowAccountsFinished(QDBusPendingCallWatcher* aWatch);
    void requestUpdate();

public:
    QString iFilter;
    QList<HarbourTransferMethodInfo2> iMethodList;
    QList<int> iFilteredList;
    bool iAccountIconSupported;
    RequestUpdate iRequestUpdate;
    QDBusPendingCallWatcher* iUpdateWatcher;
    QDBusPendingCallWatcher* iShowAccountsWatcher;
    bool iShowAccountsFailed;
    TransferEngine* iTransferEngine;
};

HarbourTransferMethodsModel::Private::Private(HarbourTransferMethodsModel* aModel) :
    QObject(aModel),
    iAccountIconSupported(false),
    iRequestUpdate(&Private::checkTransferMethods),
    iUpdateWatcher(Q_NULLPTR),
    iShowAccountsWatcher(Q_NULLPTR),
    iShowAccountsFailed(false),
    iTransferEngine(new TransferEngine(this))
{
    connect(iTransferEngine,
        SIGNAL(transferMethodListChanged()),
        SLOT(requestUpdate()));
    requestUpdate();
}

HarbourTransferMethodsModel::Private::~Private()
{
    delete iTransferEngine;
}

inline HarbourTransferMethodsModel* HarbourTransferMethodsModel::Private::parentModel()
{
    return qobject_cast<HarbourTransferMethodsModel*>(parent());
}

void HarbourTransferMethodsModel::Private::requestUpdate()
{
    if (iUpdateWatcher) {
        HDEBUG("dropping pending method list query");
        iUpdateWatcher->disconnect(this);
        delete iUpdateWatcher;
    }
    iUpdateWatcher = (this->*iRequestUpdate)();
}

void HarbourTransferMethodsModel::Private::setTransferMethods2(HarbourTransferMethodInfo2List aList)
{
    iRequestUpdate = &Private::requestTransferMethods2;
    HDEBUG(aList.count() << "methods");
    if (iMethodList != aList) {
        iMethodList = aList;
        filterModel();
    }
    if (!iAccountIconSupported) {
        iAccountIconSupported = true;
        Q_EMIT parentModel()->accountIconSupportedChanged();
    }
}

QDBusPendingCallWatcher* HarbourTransferMethodsModel::Private::checkTransferMethods()
{
    // First try transferMethods2() and see if it works
    QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher
        (iTransferEngine->transferMethods2(), this);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
        SLOT(onTransferMethodsCheckFinished(QDBusPendingCallWatcher*)));
    return watcher;
}

QDBusPendingCallWatcher* HarbourTransferMethodsModel::Private::requestTransferMethods()
{
    QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher
        (iTransferEngine->transferMethods(), this);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
        SLOT(onTransferMethodsFinished(QDBusPendingCallWatcher*)));
    return watcher;
}

QDBusPendingCallWatcher* HarbourTransferMethodsModel::Private::requestTransferMethods2()
{
    QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher
        (iTransferEngine->transferMethods2(), this);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
        SLOT(onTransferMethods2Finished(QDBusPendingCallWatcher*)));
    return watcher;
}

void HarbourTransferMethodsModel::Private::onTransferMethodsCheckFinished(QDBusPendingCallWatcher* aWatch)
{
    QDBusPendingReply<HarbourTransferMethodInfo2List> reply(*aWatch);
    HASSERT(aWatch == iUpdateWatcher);
    iUpdateWatcher = Q_NULLPTR;
    if (reply.isError()) {
        QDBusError error(reply.error());
        qWarning() << error;
        if (error.type() == QDBusError::UnknownMethod) {
            // Switch to the legacy interface
            iRequestUpdate = &Private::requestTransferMethods;
            requestUpdate();
        }
    } else {
        setTransferMethods2(reply.value());
    }
    aWatch->deleteLater();
}

void HarbourTransferMethodsModel::Private::onTransferMethods2Finished(QDBusPendingCallWatcher* aWatch)
{
    QDBusPendingReply<HarbourTransferMethodInfo2List> reply(*aWatch);
    HASSERT(aWatch == iUpdateWatcher);
    iUpdateWatcher = Q_NULLPTR;
    if (reply.isError()) {
        qWarning() << reply.error();
    } else {
        setTransferMethods2(reply.value());
    }
    aWatch->deleteLater();
}

void HarbourTransferMethodsModel::Private::onTransferMethodsFinished(QDBusPendingCallWatcher* aWatch)
{
    QDBusPendingReply<HarbourTransferMethodInfoList> reply(*aWatch);
    HASSERT(aWatch == iUpdateWatcher);
    iUpdateWatcher = Q_NULLPTR;
    if (reply.isError()) {
        qWarning() << reply.error();
    } else {
        const HarbourTransferMethodInfoList list = reply.value();
        HarbourTransferMethodInfo2List list2;
        const int n = list.count();
        for (int i = 0; i < n; i++) {
            list2.append(HarbourTransferMethodInfo2(list.at(i)));
        }
        HDEBUG(n << "methods");
        if (iMethodList != list2) {
            iMethodList = list2;
            filterModel();
        }
    }
    aWatch->deleteLater();
}

QRegExp HarbourTransferMethodsModel::Private::regExp(QString aRegExp)
{
    return QRegExp(aRegExp, Qt::CaseInsensitive, QRegExp::Wildcard);
}

void HarbourTransferMethodsModel::Private::filterModel()
{
    QList<int> filteredList;
    if (iFilter.isEmpty() || iFilter == "*") {
        HDEBUG("no filter");
        for (int i = 0; i < iMethodList.count(); i++) {
            filteredList.append(i);
        }
    } else {
        QRegExp re(regExp(iFilter));
        for (int i = 0; i < iMethodList.count(); i++) {
            const HarbourTransferMethodInfo2& info = iMethodList.at(i);
            for (int j = 0; j < info.capabilitities.count(); j++) {
                const QString& cap = info.capabilitities.at(j);
                if (iFilter == cap ||
                    re.exactMatch(cap) ||
                    regExp(cap).exactMatch(iFilter)) {
                    HDEBUG(i << ":" << iFilter << "matches" << cap);
                    filteredList.append(i);
                    break;
                } else {
                    HDEBUG(i << ":" << iFilter << "doesn't match" << cap);
                }
            }
        }
    }
    if (iFilteredList != filteredList) {
        HDEBUG("Methods changed");
        HarbourTransferMethodsModel* model = parentModel();
        model->beginResetModel();
        const int oldCount = iFilteredList.count();
        iFilteredList = filteredList;
        if (oldCount != iFilteredList.count()) {
            Q_EMIT model->countChanged();
        }
        model->endResetModel();
    }
}

bool HarbourTransferMethodsModel::Private::showAccounts()
{
    bool wasPending;
    if (iShowAccountsWatcher) {
        HDEBUG("dropping pending showAccounts");
        iShowAccountsWatcher->disconnect(this);
        delete iShowAccountsWatcher;
        wasPending = true;
    } else {
        wasPending = true;
    }
    connect(iShowAccountsWatcher = new QDBusPendingCallWatcher(QDBusConnection::sessionBus().
        asyncCall(QDBusMessage::createMethodCall(QStringLiteral("com.jolla.settings"),
        QStringLiteral("/com/jolla/settings/ui"), QStringLiteral("com.jolla.settings.ui"),
        QStringLiteral("showAccounts"))), this),
        SIGNAL(finished(QDBusPendingCallWatcher*)),
        SLOT(onShowAccountsFinished(QDBusPendingCallWatcher*)));
    return !wasPending;
}

void HarbourTransferMethodsModel::Private::onShowAccountsFinished(QDBusPendingCallWatcher* aWatch)
{
    HarbourTransferMethodsModel* model = parentModel();
    QDBusPendingReply<> reply(*aWatch);
    HASSERT(aWatch == iShowAccountsWatcher);
    iShowAccountsWatcher = Q_NULLPTR;
    if (reply.isError()) {
        qWarning() << reply.error();
        if (!iShowAccountsFailed) {
            iShowAccountsFailed = true;
            Q_EMIT model->canShowAccountsChanged();
        }
    }
    Q_EMIT model->showAccountsPendingChanged();
    aWatch->deleteLater();
}

// ==========================================================================
// HarbourTransferMethodsModel
// ==========================================================================

HarbourTransferMethodsModel::HarbourTransferMethodsModel(QObject* aParent):
    QAbstractListModel(aParent),
    iPrivate(new Private(this))
{
}

HarbourTransferMethodsModel::~HarbourTransferMethodsModel()
{
    delete iPrivate;
}

// Callback for qmlRegisterSingletonType<HarbourTransferMethodsModel>
QObject* HarbourTransferMethodsModel::createSingleton(QQmlEngine*, QJSEngine*)
{
    return new HarbourTransferMethodsModel();
}

bool HarbourTransferMethodsModel::loadTranslations(QTranslator* aTranslator, QLocale aLocale)
{
    if (aTranslator->load(aLocale, "sailfish_transferengine_plugins", "-",
        "/usr/share/translations")) {
        return true;
    } else {
        HWARN("Failed to load transferengine plugin translator for" << aLocale);
        return false;
    }
}

QHash<int,QByteArray> HarbourTransferMethodsModel::roleNames() const
{
    QHash<int,QByteArray> roles;
    roles[Private::DisplayNameRole] = "displayName";
    roles[Private::UserNameRole]    = "userName";
    roles[Private::MethodIdRole]    = "methodId";
    roles[Private::ShareUIPathRole] = "shareUIPath";
    roles[Private::AccountIdRole]   = "accountId";
    roles[Private::AccountIconRole] = "accountIcon";
    return roles;
}

int HarbourTransferMethodsModel::rowCount(const QModelIndex &) const
{
    return iPrivate->iFilteredList.count();
}

QVariant HarbourTransferMethodsModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    if (row >= 0 && row < iPrivate->iFilteredList.count()) {
        const int pos = iPrivate->iFilteredList.at(row);
        const HarbourTransferMethodInfo2& info = iPrivate->iMethodList.at(pos);
        switch (role) {
        case Private::DisplayNameRole: {
            QString s(qApp->translate(Q_NULLPTR, qPrintable(info.displayName)));
            if (!s.isEmpty()) return s;
            /* Otherwise default to methodId */
            HDEBUG("no translation for" << info.displayName);
        }
        /* fallthrough */
        case Private::MethodIdRole:    return info.methodId;
        case Private::UserNameRole:    return info.userName;
        case Private::ShareUIPathRole: return info.shareUIPath;
        case Private::AccountIdRole:   return info.accountId;
        case Private::AccountIconRole: return info.accountIcon;
        }
    }
    qWarning() << index << role;
    return QVariant();
}

int HarbourTransferMethodsModel::count() const
{
    return iPrivate->iFilteredList.count();
}

bool HarbourTransferMethodsModel::accountIconSupported() const
{
    return iPrivate->iAccountIconSupported;
}

QString HarbourTransferMethodsModel::filter() const
{
    return iPrivate->iFilter;
}

void HarbourTransferMethodsModel::setFilter(QString aFilter)
{
    if (iPrivate->iFilter != aFilter) {
        iPrivate->iFilter = aFilter;
        iPrivate->filterModel();
        Q_EMIT filterChanged();
    }
}

bool HarbourTransferMethodsModel::showAccountsPending() const
{
    return iPrivate->iShowAccountsWatcher != Q_NULLPTR;
}

bool HarbourTransferMethodsModel::canShowAccounts() const
{
    return !iPrivate->iShowAccountsFailed;
}

void HarbourTransferMethodsModel::showAccounts()
{
    if (iPrivate->showAccounts()) {
        Q_EMIT showAccountsPendingChanged();
    }
}

#include "HarbourTransferMethodsModel.moc"
