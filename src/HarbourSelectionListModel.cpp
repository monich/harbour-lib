/*
 * Copyright (C) 2019 Jolla Ltd.
 * Copyright (C) 2019-2024 Slava Monich <slava@monich.com>
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer
 *     in the documentation and/or other materials provided with the
 *     distribution.
 *
 *  3. Neither the names of the copyright holders nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
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
 *
 * The views and conclusions contained in the software and documentation
 * are those of the authors and should not be interpreted as representing
 * any official policies, either expressed or implied.
 */

#include "HarbourSelectionListModel.h"

#include "HarbourDebug.h"

#define ROLE "selected"

// ==========================================================================
// HarbourSelectionListModel::Private
// ==========================================================================

// s(SignalName,signalName)
#define QUEUED_SIGNALS(s) \
    s(NonSelectableRows,nonSelectableRows) \
    s(SelectedRows,selectedRows) \
    s(SelectableCount,selectableCount) \
    s(SelectionCount,selectionCount) \
    s(Count,count)

class HarbourSelectionListModel::Private : public QObject
{
    Q_OBJECT

public:
    typedef void (HarbourSelectionListModel::*SignalEmitter)();
    typedef uint SignalMask;

    enum Signal {
#define SIGNAL_ENUM_(Name,name) Signal##Name##Changed,
        QUEUED_SIGNALS(SIGNAL_ENUM_)
#undef SIGNAL_ENUM_
        SignalCount
    };

    Private(HarbourSelectionListModel*);

    static int binaryFind(const QList<int>, int);

    HarbourSelectionListModel* parentModel() const;
    void queueSignal(Signal aSignal);
    void emitQueuedSignals();
    bool isSelectionRole(int) const;
    bool isSelectedRow(int) const;
    bool isSelectableRow(int) const;
    int findSelectedRow(int) const;
    int findRole(const QString&) const;
    QVariantList selectedValues(const QString&) const;
    void updateCounts();
    void selectRow(int);
    void unselectRow(int);
    void toggleRows(const QList<int>);
    void setNonSelectableRows(const QList<int>);
    void selectedRowChanged(int);
    void selectionChangedAt(int);
    void clearSelection();
    void selectAll();
    void reset();

public Q_SLOTS:
    void onCountChanged();

public:
    SignalMask iQueuedSignals;
    Signal iFirstQueuedSignal;
    QList<int> iSelectedRows;
    QList<int> iNonSelectableRows;
    QList<int> iNormalizedNonSelectableRows;
    QVector<int> iSelectedRole; // Passed to dataChanged as an argument
    int iLastKnownSelectableCount;
    int iLastKnownCount;
};

HarbourSelectionListModel::Private::Private(
    HarbourSelectionListModel* aParent) :
    QObject(aParent),
    iLastKnownSelectableCount(0),
    iLastKnownCount(0)
{
    connect(aParent, SIGNAL(modelReset()), SLOT(onCountChanged()));
    connect(aParent, SIGNAL(rowsInserted(QModelIndex,int,int)), SLOT(onCountChanged()));
    connect(aParent, SIGNAL(rowsRemoved(QModelIndex,int,int)), SLOT(onCountChanged()));
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

inline
HarbourSelectionListModel*
HarbourSelectionListModel::Private::parentModel() const
{
    return qobject_cast<HarbourSelectionListModel*>(parent());
}

void
HarbourSelectionListModel::Private::queueSignal(
    Signal aSignal)
{
    if (aSignal >= 0 && aSignal < SignalCount) {
        const SignalMask signalBit = (SignalMask(1) << aSignal);
        if (iQueuedSignals) {
            iQueuedSignals |= signalBit;
            if (iFirstQueuedSignal > aSignal) {
                iFirstQueuedSignal = aSignal;
            }
        } else {
            iQueuedSignals = signalBit;
            iFirstQueuedSignal = aSignal;
        }
    }
}

void
HarbourSelectionListModel::Private::emitQueuedSignals()
{
    static const SignalEmitter emitSignal [] = {
#define SIGNAL_EMITTER_(Name,name) &HarbourSelectionListModel::name##Changed,
        QUEUED_SIGNALS(SIGNAL_EMITTER_)
#undef SIGNAL_EMITTER_
    };
    if (iQueuedSignals) {
        HarbourSelectionListModel* model = parentModel();
        // Reset first queued signal before emitting the signals.
        // Signal handlers may emit new signals.
        uint i = iFirstQueuedSignal;
        iFirstQueuedSignal = SignalCount;
        for (; i < SignalCount && iQueuedSignals; i++) {
            const SignalMask signalBit = (SignalMask(1) << i);
            if (iQueuedSignals & signalBit) {
                iQueuedSignals &= ~signalBit;
                Q_EMIT (model->*(emitSignal[i]))();
            }
        }
    }
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

inline
bool
HarbourSelectionListModel::Private::isSelectableRow(
    int aRow) const
{
    return binaryFind(iNonSelectableRows, aRow) < 0;
}

int
HarbourSelectionListModel::Private::findSelectedRow(
    int aRow) const
{
    return binaryFind(iSelectedRows, aRow);
}

int
HarbourSelectionListModel::Private::findRole(
    const QString& aRole) const
{
    if (!aRole.isEmpty()) {
        const QByteArray roleName(aRole.toUtf8());
        QHashIterator<int,QByteArray> roles(parentModel()->roleNames());
        while (roles.hasNext()) {
            if (roles.next().value() == roleName) {
                return roles.key();
            }
        }
        HDEBUG("Unknown role" << aRole);
    }
    return -1;
}

QVariantList
HarbourSelectionListModel::Private::selectedValues(
    const QString& aRole) const
{
    QVariantList values;
    const int n = iSelectedRows.count();
    if (n > 0) {
        const int role = findRole(aRole);
        if (role >= 0) {
            HarbourSelectionListModel* model = parentModel();
            values.reserve(n);
            for (int i = 0; i < n; i++) {
                const int row = iSelectedRows.at(i);
                values.append(model->data(model->index(row, 0), role));
            }
        }
    }
    HDEBUG(aRole << values);
    return values;
}

void
HarbourSelectionListModel::Private::updateCounts()
{
    const int count = parentModel()->rowCount();

    int selectableCount = count;
    const int n = iNormalizedNonSelectableRows.count();
    for (int i = 0; i < n; i++) {
        const int row = iNormalizedNonSelectableRows.at(i);

        if (row < count) {
            selectableCount--;
        } else {
            break;
        }
    }

    if (iLastKnownSelectableCount != selectableCount) {
        iLastKnownSelectableCount = selectableCount;
        queueSignal(SignalSelectableCountChanged);
    }

    if (iLastKnownCount != count) {
        iLastKnownCount = count;
        queueSignal(SignalCountChanged);
        while (!iSelectedRows.isEmpty() && iSelectedRows.last() >= count) {
            iSelectedRows.removeAt(iSelectedRows.count() - 1);
            queueSignal(SignalSelectionCountChanged);
            queueSignal(SignalSelectedRowsChanged);
        }
    }
}

void
HarbourSelectionListModel::Private::onCountChanged()
{
    updateCounts();
    emitQueuedSignals();
}

void
HarbourSelectionListModel::Private::clearSelection()
{
    if (!iSelectedRows.isEmpty()) {
        HDEBUG("clearing selection");
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

        queueSignal(SignalSelectionCountChanged);
        queueSignal(SignalSelectedRowsChanged);
        emitQueuedSignals();
    }
}

void
HarbourSelectionListModel::Private::selectAll()
{
    const int count = parentModel()->rowCount();
    QList<int> rows;
    int i;

    rows.reserve(count);
    for (i = 0; i < count; i++) {
        if (binaryFind(iNormalizedNonSelectableRows, i) < 0) {
            rows.append(i);
        }
    }

    if (iSelectedRows.count() != rows.count()) {
        queueSignal(SignalSelectionCountChanged);
    }

    const QList<int> prevSelection(iSelectedRows);
    if (iSelectedRows != rows) {
        iSelectedRows = rows;
        queueSignal(SignalSelectedRowsChanged);
    }

    for (i = 0; i < iSelectedRows.count(); i++) {
        const int row = iSelectedRows.at(i);

        if (binaryFind(prevSelection, row) < 0) {
            selectionChangedAt(row);
        }
    }
    emitQueuedSignals();
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
HarbourSelectionListModel::Private::selectedRowChanged(
    int aRow)
{
    queueSignal(SignalSelectionCountChanged);
    queueSignal(SignalSelectedRowsChanged);
    selectionChangedAt(aRow);
    emitQueuedSignals();
}

void
HarbourSelectionListModel::Private::selectRow(
    int aRow)
{
    if (aRow >= 0 && aRow < parentModel()->rowCount()) {
        const int pos = findSelectedRow(aRow);
        if (pos < 0) {
            if (isSelectableRow(aRow)) {
                HDEBUG(aRow << "selected at" << (-pos - 1));
                iSelectedRows.insert(-pos - 1, aRow);
                selectedRowChanged(aRow);
            } else {
                HDEBUG("Row" << aRow << "is not selectable");
            }
        }
    }
}

void
HarbourSelectionListModel::Private::unselectRow(
    int aRow)
{
    if (aRow >= 0 && aRow < parentModel()->rowCount()) {
        const int pos = findSelectedRow(aRow);
        if (pos >= 0) {
            HDEBUG(aRow << "unselected at" << pos);
            iSelectedRows.removeAt(pos);
            selectedRowChanged(aRow);
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
    const int k = rows.count();
    const int prevSelectedCount = iSelectedRows.count();

    for (i = 0; i < k; i++) {
        const int row = rows.at(i);
        if (row >= 0 && row < model->rowCount()) {
            const int pos = findSelectedRow(row);
            if (pos >= 0) {
                HDEBUG(row << "unselected at" << pos);
                iSelectedRows.removeAt(pos);
                selectionChangedAt(row);
            } else if (isSelectableRow(row)) {
                HDEBUG(row << "selected at" << (-pos - 1));
                iSelectedRows.insert(-pos - 1, row);
                selectionChangedAt(row);
            }
            queueSignal(SignalSelectedRowsChanged);
        }
    }

    if (prevSelectedCount != iSelectedRows.count()) {
        queueSignal(SignalSelectionCountChanged);
    }

    emitQueuedSignals();
}

void
HarbourSelectionListModel::Private::setNonSelectableRows(
    const QList<int> aRows)
{
    if (iNonSelectableRows != aRows) {
        iNonSelectableRows = aRows;
        queueSignal(SignalNonSelectableRowsChanged);

        // Sort and remove negatives and duplicates
        iNormalizedNonSelectableRows.clear();
        iNormalizedNonSelectableRows.reserve(aRows.count());

        const int n = aRows.count();
        for (int i = 0; i < n; i++) {
            const int row = aRows.at(i);
            if (row >= 0) {
                const int pos = binaryFind(iNormalizedNonSelectableRows, row);
                if (pos < 0) iNormalizedNonSelectableRows.insert(-pos - 1, row);
            }
        }

        HDEBUG(iNormalizedNonSelectableRows);
        updateCounts();
        emitQueuedSignals();
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
HarbourSelectionListModel::nonSelectableRows() const
{
    return iPrivate->iNonSelectableRows;
}

void
HarbourSelectionListModel::setNonSelectableRows(
    const QList<int> aRows)
{
    iPrivate->setNonSelectableRows(aRows);
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

int
HarbourSelectionListModel::selectableCount() const
{
    return iPrivate->iLastKnownSelectableCount;
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

QVariantList
HarbourSelectionListModel::selectedValues(
    QString aRole) const
{
    return iPrivate->selectedValues(aRole);
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
