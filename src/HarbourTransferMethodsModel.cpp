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
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
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

// ==========================================================================
// HarbourTransferMethodsModel::TransferEngine
// ==========================================================================

class HarbourTransferMethodsModel::TransferEngine: public QDBusAbstractInterface
{
    Q_OBJECT

    static const char SERVICE[];
    static const char PATH[];
    static const char INTERFACE[];

public:
    TransferEngine(QObject* aParent) : QDBusAbstractInterface(SERVICE, PATH,
        INTERFACE, QDBusConnection::sessionBus(), aParent) {}

public: // METHODS
    inline QDBusPendingCall transferMethods()
        { return asyncCall("transferMethods"); }
    inline QDBusPendingCall transferMethods2()
        { return asyncCall("transferMethods2"); }

Q_SIGNALS: // SIGNALS
    void transferMethodListChanged();
};

const char HarbourTransferMethodsModel::TransferEngine::SERVICE[] = "org.nemo.transferengine";
const char HarbourTransferMethodsModel::TransferEngine::PATH[] = "/org/nemo/transferengine";
const char HarbourTransferMethodsModel::TransferEngine::INTERFACE[] = "org.nemo.transferengine";

// ==========================================================================
// HarbourTransferMethodsModel
// ==========================================================================

HarbourTransferMethodsModel::HarbourTransferMethodsModel(QObject* aParent):
    QAbstractListModel(aParent),
    iAccountIconSupported(false),
    iRequestUpdate(&HarbourTransferMethodsModel::checkTransferMethods),
    iUpdateWatcher(Q_NULLPTR)
{
    iTransferEngine = new TransferEngine(this);
    connect(iTransferEngine,
        SIGNAL(transferMethodListChanged()),
        SLOT(requestUpdate()));
    requestUpdate();
}

HarbourTransferMethodsModel::~HarbourTransferMethodsModel()
{
    delete iTransferEngine;
}

// Callback for qmlRegisterSingletonType<HarbourTransferMethodsModel>
QObject*
HarbourTransferMethodsModel::createSingleton(QQmlEngine*, QJSEngine*)
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

void HarbourTransferMethodsModel::requestUpdate()
{
    if (iUpdateWatcher) {
        HDEBUG("dropping pending call");
        iUpdateWatcher->disconnect(this);
        delete iUpdateWatcher;
        iUpdateWatcher = Q_NULLPTR;
    }
    iUpdateWatcher = (this->*iRequestUpdate)();
}

void HarbourTransferMethodsModel::setTransferMethods2(HarbourTransferMethodInfo2List aList)
{
    iRequestUpdate = &HarbourTransferMethodsModel::requestTransferMethods2;
    HDEBUG(aList.count() << "methods");
    if (iMethodList != aList) {
        iMethodList = aList;
        filterModel();
    }
    if (!iAccountIconSupported) {
        iAccountIconSupported = true;
        Q_EMIT accountIconSupportedChanged();
    }
}

QDBusPendingCallWatcher* HarbourTransferMethodsModel::checkTransferMethods()
{
    // First try transferMethods2() and see if it works
    QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher
        (iTransferEngine->transferMethods2(), this);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
        SLOT(onTransferMethodsCheckFinished(QDBusPendingCallWatcher*)));
    return watcher;
}

QDBusPendingCallWatcher* HarbourTransferMethodsModel::requestTransferMethods()
{
    QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher
        (iTransferEngine->transferMethods(), this);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
        SLOT(onTransferMethodsFinished(QDBusPendingCallWatcher*)));
    return watcher;
}

QDBusPendingCallWatcher* HarbourTransferMethodsModel::requestTransferMethods2()
{
    QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher
        (iTransferEngine->transferMethods2(), this);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
        SLOT(onTransferMethods2Finished(QDBusPendingCallWatcher*)));
    return watcher;
}

void HarbourTransferMethodsModel::onTransferMethodsCheckFinished(QDBusPendingCallWatcher* aWatch)
{
    QDBusPendingReply<HarbourTransferMethodInfo2List> reply(*aWatch);
    aWatch->deleteLater();
    HASSERT(aWatch == iUpdateWatcher);
    iUpdateWatcher = Q_NULLPTR;
    if (reply.isError()) {
        QDBusError error(reply.error());
        qWarning() << error;
        if (error.type() == QDBusError::UnknownMethod) {
            // Switch to the legacy interface
            iRequestUpdate = &HarbourTransferMethodsModel::requestTransferMethods;
            requestUpdate();
        }
    } else {
        setTransferMethods2(reply.value());
    }
}

void HarbourTransferMethodsModel::onTransferMethods2Finished(QDBusPendingCallWatcher* aWatch)
{
    QDBusPendingReply<HarbourTransferMethodInfo2List> reply(*aWatch);
    aWatch->deleteLater();
    HASSERT(aWatch == iUpdateWatcher);
    iUpdateWatcher = Q_NULLPTR;
    if (reply.isError()) {
        qWarning() << reply.error();
    } else {
        setTransferMethods2(reply.value());
    }
}

void HarbourTransferMethodsModel::onTransferMethodsFinished(QDBusPendingCallWatcher* aWatch)
{
    QDBusPendingReply<HarbourTransferMethodInfoList> reply(*aWatch);
    aWatch->deleteLater();
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
}

QHash<int,QByteArray> HarbourTransferMethodsModel::roleNames() const
{
    QHash<int,QByteArray> roles;
    roles[DisplayNameRole] = "displayName";
    roles[UserNameRole]    = "userName";
    roles[MethodIdRole]    = "methodId";
    roles[ShareUIPathRole] = "shareUIPath";
    roles[AccountIdRole]   = "accountId";
    roles[AccountIconRole] = "accountIcon";
    return roles;
}

int HarbourTransferMethodsModel::rowCount(const QModelIndex &) const
{
    return iFilteredList.count();
}

QVariant HarbourTransferMethodsModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    if (row >= 0 && row < iFilteredList.count()) {
        const HarbourTransferMethodInfo2& info = iMethodList.at(iFilteredList.at(row));
        switch (role) {
        case DisplayNameRole: {
            QString s(qApp->translate(Q_NULLPTR, qPrintable(info.displayName)));
            if (!s.isEmpty()) return s;
            /* Otherwise default to methodId */
            HDEBUG("no translation for" << info.displayName);
        }
        /* fallthrough */
        case MethodIdRole:    return info.methodId;
        case UserNameRole:    return info.userName;
        case ShareUIPathRole: return info.shareUIPath;
        case AccountIdRole:   return info.accountId;
        case AccountIconRole: return info.accountIcon;
        }
    }
    qWarning() << index << role;
    return QVariant();
}

int HarbourTransferMethodsModel::count() const
{
    return iFilteredList.count();
}

bool HarbourTransferMethodsModel::accountIconSupported() const
{
    return iAccountIconSupported;
}

QString HarbourTransferMethodsModel::filter() const
{
    return iFilter;
}

void HarbourTransferMethodsModel::setFilter(QString aFilter)
{
    if (iFilter != aFilter) {
        iFilter = aFilter;
        filterModel();
        Q_EMIT filterChanged();
    }
}

QRegExp HarbourTransferMethodsModel::regExp(QString aRegExp)
{
    return QRegExp(aRegExp, Qt::CaseInsensitive, QRegExp::Wildcard);
}

void HarbourTransferMethodsModel::filterModel()
{
    beginResetModel();
    const int oldCount = iFilteredList.count();
    iFilteredList.clear();
    if (iFilter.isEmpty() || iFilter == "*") {
        HDEBUG("no filter");
        for (int i=0; i<iMethodList.count(); i++) {
            iFilteredList.append(i);
        }
    } else {
        QRegExp re(regExp(iFilter));
        for (int i=0; i<iMethodList.count(); i++) {
            const HarbourTransferMethodInfo2& info = iMethodList.at(i);
            for (int j=0; j<info.capabilitities.count(); j++) {
                const QString& cap = info.capabilitities.at(j);
                if (iFilter == cap ||
                    re.exactMatch(cap) ||
                    regExp(cap).exactMatch(iFilter)) {
                    HDEBUG(i << ":" << iFilter << "matches" << cap);
                    iFilteredList.append(i);
                    break;
                } else {
                    HDEBUG(i << ":" << iFilter << "doesn't match" << cap);
                }
            }
        }
    }
    if (oldCount != iFilteredList.count()) {
        Q_EMIT countChanged();
    }
    endResetModel();
}

#include "HarbourTransferMethodsModel.moc"
