/*
 * Copyright (C) 2019 Jolla Ltd.
 * Copyright (C) 2019-2023 Slava Monich <slava@jolla.com>
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

#include "HarbourSelectionListModel.h"

#include "HarbourDebug.h"

#define ROLE "selected"

// ==========================================================================
// HarbourSelectionListModel::Private
// ==========================================================================

class HarbourSelectionListModel::Private : public QObject
{
    Q_OBJECT

public:
    Private(HarbourSelectionListModel*);

    HarbourSelectionListModel* parentModel();
    static int binaryFind(const QList<int>, int);
    bool isSelectionRole(int) const;
    bool isSelectedRow(int) const;
    int findSelectedRow(int) const;
    void selectRow(int);
    void unselectRow(int);
    void toggleRows(const QList<int>);
    void selectionChangedAt(int);
    void clearSelection();
    void selectAll();
    void reset();

public Q_SLOTS:
    void onCountChanged();

public:
    QList<int> iSelectedRows;
    QVector<int> iSelectedRole;
    int iLastKnownCount;
};

HarbourSelectionListModel::Private::Private(
    HarbourSelectionListModel* aParent) :
    QObject(aParent),
    iLastKnownCount(0)
{
    connect(aParent, SIGNAL(modelReset()), SLOT(onCountChanged()));
    connect(aParent, SIGNAL(rowsInserted(QModelIndex,int,int)), SLOT(onCountChanged()));
    connect(aParent, SIGNAL(rowsRemoved(QModelIndex,int,int)), SLOT(onCountChanged()));
}

inline
HarbourSelectionListModel*
HarbourSelectionListModel::Private::parentModel()
{
    return qobject_cast<HarbourSelectionListModel*>(parent());
}

inline
bool
HarbourSelectionListModel::Private::isSelectionRole(
    int aRole) const
{
    return !iSelectedRole.isEmpty() && iSelectedRole.first() == aRole;
}

inline
bool
HarbourSelectionListModel::Private::isSelectedRow(
    int aRow) const
{
    return findSelectedRow(aRow) >= 0;
}

int
HarbourSelectionListModel::Private::binaryFind(
    const QList<int> aList,
    int aValue)
{
    // It turned out to be significantly easier to copy/paste this code
    // than to fight with qBinaryFind and iterators which behave strangely
    // with empty QList and when you convert them to index. Oh well...
    int low = 0;
    int high = aList.count() - 1;

    while (low <= high) {
        int mid = (low + high)/2;
        const int val = aList.at(mid);
        if (val < aValue) {
            low = mid + 1;
        } else if (val > aValue) {
            high = mid - 1;
        } else {
            // Found
            return mid;
        }
    }

    // Not found, return -(INSERTION POINT+1)
    return -(low + 1);
}

int
HarbourSelectionListModel::Private::findSelectedRow(
    int aRow) const
{
    return binaryFind(iSelectedRows, aRow);
}

void
HarbourSelectionListModel::Private::onCountChanged()
{
    HarbourSelectionListModel* model = parentModel();
    const int count = model->rowCount();
    if (iLastKnownCount != count) {
        iLastKnownCount = count;
        bool changed = false;
        while (!iSelectedRows.isEmpty() && iSelectedRows.last() >= count) {
            iSelectedRows.removeAt(iSelectedRows.count() - 1);
            changed = true;
        }
        if (changed) {
            Q_EMIT model->selectedRowsChanged();
        }
        Q_EMIT model->countChanged();
    }
}

void
HarbourSelectionListModel::Private::clearSelection()
{
    if (!iSelectedRows.isEmpty()) {
        HDEBUG("clearing selection");
        HarbourSelectionListModel* model = parentModel();
        if (!iSelectedRole.isEmpty()) {
            const QList<int> rows(iSelectedRows);
            iSelectedRows.clear();
            const int n = rows.count();
            for (int i = 0; i < n; i++) {
                selectionChangedAt(rows.at(i));
            }
        } else {
            iSelectedRows.clear();
        }
        Q_EMIT model->selectedRowsChanged();
    }
}

void
HarbourSelectionListModel::Private::selectAll()
{
    HarbourSelectionListModel* model = parentModel();
    const int count = model->rowCount();
    if (iSelectedRows.count() < count) {
        const QList<int> prevSelection(iSelectedRows);
        iSelectedRows.reserve(count);
        int i;
        for (i = 0; i < iSelectedRows.count(); i++) iSelectedRows[i] = i;
        while (i < count) iSelectedRows.append(i++);
        for (i = 0; i < count; i++) {
            if (binaryFind(prevSelection, i) < 0) {
                selectionChangedAt(i);
            }
        }
        Q_EMIT model->selectedRowsChanged();
    }
}

void
HarbourSelectionListModel::Private::reset()
{
    iSelectedRole.clear();
    clearSelection();
}

void
HarbourSelectionListModel::Private::selectionChangedAt(
    int aRow)
{
    if (!iSelectedRole.isEmpty()) {
        HarbourSelectionListModel* model = parentModel();
        QModelIndex modelIndex(model->index(aRow, 0));
        Q_EMIT model->dataChanged(modelIndex, modelIndex, iSelectedRole);
    }
}

void
HarbourSelectionListModel::Private::selectRow(
    int aRow)
{
    HarbourSelectionListModel* model = parentModel();
    if (aRow >= 0 && aRow < model->rowCount()) {
        const int pos = findSelectedRow(aRow);
        if (pos < 0) {
            HDEBUG(aRow << "selected at" << (-pos - 1));
            iSelectedRows.insert(-pos - 1, aRow);
            selectionChangedAt(aRow);
            Q_EMIT model->selectedRowsChanged();
        }
    }
}

void
HarbourSelectionListModel::Private::unselectRow(
    int aRow)
{
    HarbourSelectionListModel* model = parentModel();
    if (aRow >= 0 && aRow < model->rowCount()) {
        const int pos = findSelectedRow(aRow);
         if (pos >= 0) {
             HDEBUG(aRow << "unselected at" << pos);
             iSelectedRows.removeAt(pos);
             selectionChangedAt(aRow);
             Q_EMIT model->selectedRowsChanged();
        }
    }
}

void
HarbourSelectionListModel::Private::toggleRows(
    const QList<int> aRows)
{
    int i;
    const int n = aRows.count();

    // Sort and remove duplicates
    QList<int> rows;
    rows.reserve(n);
    for (i = 0; i < n; i++) {
        const int row = aRows.at(i);
        const int pos = binaryFind(rows, row);
        if (pos < 0) rows.insert(-pos - 1, row);
    }

    HarbourSelectionListModel* model = parentModel();
    bool changed = false;
    const int k = rows.count();

    for (i = 0; i < k; i++) {
        const int row = rows.at(i);
        if (row >= 0 && row < model->rowCount()) {
            const int pos = findSelectedRow(row);
            if (pos < 0) {
                HDEBUG(row << "selected at" << (-pos - 1));
                iSelectedRows.insert(-pos - 1, row);
                selectionChangedAt(row);
            } else {
                HDEBUG(row << "unselected at" << pos);
                iSelectedRows.removeAt(pos);
                selectionChangedAt(row);
            }
            changed = true;
        }
    }

    if (changed) {
        Q_EMIT model->selectedRowsChanged();
    }
}

// ==========================================================================
// HarbourSelectionListModel
// ==========================================================================

#define SUPER QIdentityProxyModel

HarbourSelectionListModel::HarbourSelectionListModel(
    QObject* aParent) :
    SUPER(aParent),
    iPrivate(new Private(this))
{
    connect(this, SIGNAL(sourceModelChanged()), SIGNAL(sourceModelObjectChanged()));
}

void
HarbourSelectionListModel::setSourceModelObject(
    QObject* aModel)
{
    if (sourceModel() != aModel) {
        HDEBUG(aModel);
        setSourceModel(qobject_cast<QAbstractItemModel*>(aModel));
        iPrivate->reset();
    }
}

QList<int>
HarbourSelectionListModel::selectedRows() const
{
    return iPrivate->iSelectedRows;
}

int
HarbourSelectionListModel::selectionCount() const
{
    return iPrivate->iSelectedRows.count();
}

void
HarbourSelectionListModel::selectAll()
{
    iPrivate->selectAll();
}

void
HarbourSelectionListModel::clearSelection()
{
    iPrivate->clearSelection();
}

void
HarbourSelectionListModel::toggleRows(
    const QList<int> aRows)
{
    iPrivate->toggleRows(aRows);
}

Qt::ItemFlags
HarbourSelectionListModel::flags(
    const QModelIndex& aIndex) const
{
    return SUPER::flags(aIndex) | Qt::ItemIsEditable;
}

QHash<int,QByteArray>
HarbourSelectionListModel::roleNames() const
{
    QHash<int,QByteArray> roles = SUPER::roleNames();
    int selectedRole;
    if (!roles.isEmpty()) {
        QList<int> keys(roles.keys());
        qSort(keys);
        selectedRole = qMax((int)Qt::UserRole, keys.last() + 1);
    } else {
        selectedRole = Qt::UserRole;
    }
    HDEBUG(roles.keys() << selectedRole);
    iPrivate->iSelectedRole.resize(0);
    iPrivate->iSelectedRole.append(selectedRole);
    roles.insert(selectedRole, ROLE);
    return roles;
}

QVariant
HarbourSelectionListModel::data(
    const QModelIndex& aIndex,
    int aRole) const
{
    if (iPrivate->isSelectionRole(aRole)) {
        return iPrivate->isSelectedRow(aIndex.row());
    } else {
        return SUPER::data(aIndex, aRole);
    }
}

bool
HarbourSelectionListModel::setData(
    const QModelIndex& aIndex,
    const QVariant& aValue,
    int aRole)
{
    if (iPrivate->isSelectionRole(aRole)) {
        if (aValue.toBool()) {
            iPrivate->selectRow(aIndex.row());
        } else {
            iPrivate->unselectRow(aIndex.row());
        }
        return true;
    } else {
        return SUPER::setData(aIndex, aValue, aRole);
    }
}

#include "HarbourSelectionListModel.moc"
