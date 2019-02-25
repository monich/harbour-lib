/*
 * Copyright (C) 2019 Jolla Ltd.
 * Copyright (C) 2019 Slava Monich <slava.monich@jolla.com>
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

#include "HarbourOrganizeListModel.h"

#include "HarbourDebug.h"

// ==========================================================================
// HarbourOrganizeListModel::Private
// ==========================================================================

class HarbourOrganizeListModel::Private {
public:
    Private();

    int mapToSource(int aRow) const;
    int mapFromSource(int aRow) const;

public:
    int iDragIndex;
    int iDragPos;
    const char* iRowsAboutToBeMovedSlot;
    const char* iRowsMovedSlot;
};

HarbourOrganizeListModel::Private::Private() :
    iDragIndex(-1),
    iDragPos(-1),
    iRowsAboutToBeMovedSlot(SLOT(_q_sourceRowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int))),
    iRowsMovedSlot(SLOT(_q_sourceRowsMoved(QModelIndex,int,int,QModelIndex,int)))
{
}

int HarbourOrganizeListModel::Private::mapToSource(int aRow) const
{
    if (iDragIndex < iDragPos) {
        if (aRow < iDragIndex || aRow > iDragPos) {
            return aRow;
        } else if (aRow == iDragPos) {
            return iDragIndex;
        } else {
            return aRow + 1;
        }
    } else if (iDragPos < iDragIndex) {
        if (aRow < iDragPos || aRow > iDragIndex) {
            return aRow;
        } else if (aRow == iDragPos) {
            return iDragIndex;
        } else {
            return aRow - 1;
        }
    }
    return aRow;
}

int HarbourOrganizeListModel::Private::mapFromSource(int aRow) const
{
    if (iDragIndex < iDragPos) {
        if (aRow < iDragIndex || aRow > iDragPos) {
            return aRow;
        } else if (aRow == iDragIndex) {
            return iDragPos;
        } else {
            return aRow - 1;
        }
    } else if (iDragPos < iDragIndex) {
        if (aRow < iDragPos || aRow > iDragIndex) {
            return aRow;
        } else if (aRow == iDragIndex) {
            return iDragPos;
        } else {
            return aRow + 1;
        }
    }
    return aRow;
}

// ==========================================================================
// HarbourOrganizeListModel
// ==========================================================================

#define SUPER QSortFilterProxyModel

HarbourOrganizeListModel::HarbourOrganizeListModel(QObject* aParent) :
    SUPER(aParent),
    iPrivate(new Private)
{
}

HarbourOrganizeListModel::~HarbourOrganizeListModel()
{
    delete iPrivate;
}

void HarbourOrganizeListModel::setSourceModelObject(QObject* aModel)
{
    setSourceModel(qobject_cast<QAbstractItemModel*>(aModel));
}

int HarbourOrganizeListModel::dragIndex() const
{
    return iPrivate->iDragIndex;
}

int HarbourOrganizeListModel::dragPos() const
{
    return iPrivate->iDragPos;
}

QModelIndex HarbourOrganizeListModel::mapToSource(const QModelIndex& aIndex) const
{
    QAbstractItemModel* source = sourceModel();
    if (source) {
        return source->index(iPrivate->mapToSource(aIndex.row()), aIndex.column());
    }
    return aIndex;
}

QModelIndex HarbourOrganizeListModel::mapFromSource(const QModelIndex& aIndex) const
{
    return index(iPrivate->mapFromSource(aIndex.row()), aIndex.column());
}

void HarbourOrganizeListModel::setDragIndex(int aIndex)
{
    HDEBUG(aIndex);
    if (aIndex < 0) {
        if (iPrivate->iDragIndex >= 0) {
            // Drag is finished
            if (iPrivate->iDragPos != iPrivate->iDragIndex) {
                const int dragIndex = iPrivate->iDragIndex;
                const int dragPos = iPrivate->iDragPos;
                iPrivate->iDragPos = iPrivate->iDragIndex = -1;
                QAbstractItemModel* source = sourceModel();
                if (source) {
                    //
                    // Now, this is getting a bit hackish.
                    //
                    // We don't need QSortFilterProxyModel to react to
                    // rowsAboutToBeMoved and rowsMoved signals emitted
                    // by the source model when we invoke moveRow() because
                    // that would result in layoutChanged signal which
                    // would reset the view (its current position etc).
                    // That's totally unnecessary because all the rows
                    // are already in the right place as long as the view
                    // is concerned.
                    //
                    // It looks a bit fragile since these slots are
                    // internal and can change at any point of time.
                    // Well, in the worst case we would get the view
                    // reset.
                    //
                    QObject::disconnect(source,
                        SIGNAL(rowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)),
                        this, iPrivate->iRowsAboutToBeMovedSlot);
                    QObject::disconnect(source,
                        SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)),
                        this, iPrivate->iRowsMovedSlot);

                    // Actually move the row
                    source->moveRow(QModelIndex(), dragIndex, QModelIndex(), dragPos);

                    // And (hopefully) reconnect the handlers.
                    QObject::connect(source,
                        SIGNAL(rowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)),
                        this, iPrivate->iRowsAboutToBeMovedSlot);
                    QObject::connect(source,
                        SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)),
                        this, iPrivate->iRowsMovedSlot);

                    // Now the base class doesn't know that rows have
                    // moved, tell it to update the data by explicitely
                    // emitting dataChanged signal.
                    const int top = qMin(dragIndex, dragPos);
                    const int bottom = qMax(dragIndex, dragPos);
                    source->dataChanged(source->index(top, 0),
                        source->index(bottom, 0));
                }
            } else {
                iPrivate->iDragPos = iPrivate->iDragIndex = -1;
            }
            Q_EMIT dragIndexChanged();
            Q_EMIT dragPosChanged();
        }
    } else if (aIndex != iPrivate->iDragIndex && aIndex < rowCount()) {
        // Drag is starting
        iPrivate->iDragPos = iPrivate->iDragIndex = aIndex;
        Q_EMIT dragIndexChanged();
        Q_EMIT dragPosChanged();
    }
}

void HarbourOrganizeListModel::setDragPos(int aPos)
{
    if (aPos >= 0 && aPos < rowCount() &&
        iPrivate->iDragIndex >= 0 && iPrivate->iDragPos != aPos) {
        HDEBUG(aPos);
        const int dest = (aPos > iPrivate->iDragPos) ? (aPos + 1) : aPos;
        beginMoveRows(QModelIndex(), iPrivate->iDragPos, iPrivate->iDragPos,
            QModelIndex(), dest);
        iPrivate->iDragPos = aPos;
        endMoveRows();
        Q_EMIT dragPosChanged();
    }
}
