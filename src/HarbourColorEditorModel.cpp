/*
 * Copyright (C) 2021 Jolla Ltd.
 * Copyright (C) 2021 Slava Monich <slava.monich@jolla.com>
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

#include "HarbourColorEditorModel.h"
#include "HarbourDebug.h"

// ==========================================================================
// HarbourColorEditorModel::Private
// ==========================================================================

class HarbourColorEditorModel::Private
{
public:
    enum ModelRole {
        ColorRole = Qt::UserRole,
        ItemTypeRole
    };

    Private();

    bool resetDragPos();
    QStringList getColors() const;
    QList<QColor> makeColors(const QStringList& aColors, bool* aChanged) const;
    QVariant data(int aRow, ModelRole aRole) const;

public:
    int iDragPos;
    int iDragStartPos;
    QList<QColor> iColors;
    const QColor iAddColor;
};

HarbourColorEditorModel::Private::Private() :
    iDragPos(-1),
    iDragStartPos(-1),
    iAddColor(0, 0, 0, 0)
{
}

bool HarbourColorEditorModel::Private::resetDragPos()
{
    if (iDragPos >= 0) {
        iDragPos = -1;
        iDragStartPos = -1;
        return true;
    } else {
        return false;
    }
}

QStringList HarbourColorEditorModel::Private::getColors() const
{
    const int n = iColors.count();
    QStringList colors;
    colors.reserve(n);
    for (int i = 0; i < n; i++) {
        colors.append(iColors.at(i).name());
    }
    return colors;
}

QList<QColor> HarbourColorEditorModel::Private::makeColors(const QStringList& aColors, bool* aChanged) const
{
    const int n = aColors.count();
    *aChanged = iColors.count() != n;
    QList<QColor> newColors;
    newColors.reserve(n);
    for (int i = 0; i < n; i++) {
        const QColor c(aColors.at(i));
        newColors.append(c);
        if (!*aChanged && iColors.at(i) != c) {
            *aChanged = true;
        }
    }
    return newColors;
}

QVariant HarbourColorEditorModel::Private::data(int aRow, ModelRole aRole) const
{
    const int n = iColors.count();
    if (aRow >= 0 && aRow <= n) {
        switch (aRole) {
        case ColorRole:
            if (iDragPos >= 0) {
                if (aRow == iDragPos) {
                    // This includes the item dragged outside
                    return iColors.at(iDragStartPos);
                } else {
                    if (iDragPos == n && aRow == (n - 1)) {
                        // When item is dragged outside, it becomes
                        // the last one and shifts the "add" item.
                        return iAddColor;
                    } else if (aRow >= iDragStartPos && aRow < iDragPos) {
                        return iColors.at(aRow + 1);
                    } else if (aRow > iDragPos && aRow <= iDragStartPos) {
                        return iColors.at(aRow - 1);
                    }
                }
            }
            return (aRow == n) ? iAddColor : iColors.at(aRow);
        case ItemTypeRole:
            if (iDragPos == n) {
                // Item is dragged outside
                return (aRow == n) ? TrashedItem :
                    (aRow == (n - 1)) ? AddItem : ColorItem;
            } else {
                return (aRow == n) ? AddItem : ColorItem;
            }
        }
    }
    return QVariant();
}

// ==========================================================================
// HarbourColorEditorModel::Private
// ==========================================================================

HarbourColorEditorModel::HarbourColorEditorModel(QObject* aParent) :
    QAbstractListModel(aParent),
    iPrivate(new Private)
{
}

QHash<int,QByteArray> HarbourColorEditorModel::roleNames() const
{
    QHash<int,QByteArray> roles;
    roles.insert(Private::ColorRole, "color");
    roles.insert(Private::ItemTypeRole, "itemType");
    return roles;
}

int HarbourColorEditorModel::rowCount(const QModelIndex&) const
{
    return iPrivate->iColors.count() + 1;
}

QVariant HarbourColorEditorModel::data(const QModelIndex& aIndex, int aRole) const
{
    return iPrivate->data(aIndex.row(), (Private::ModelRole)aRole);
}

QStringList HarbourColorEditorModel::getColors() const
{
    return iPrivate->getColors();
}

void HarbourColorEditorModel::setColors(QStringList aColors)
{
    bool changed;
    const QList<QColor> prevColors(iPrivate->iColors);
    const QList<QColor> newColors(iPrivate->makeColors(aColors, &changed));
    if (changed) {
        const int prevCount = prevColors.count();
        const int newCount = newColors.count();
        const QVector<int> colorRole(1, Private::ColorRole);
        if (newCount < prevCount) {
            beginRemoveRows(QModelIndex(), newCount + 1, prevCount);
            iPrivate->iColors = newColors;
            endRemoveRows();
        } else if (newCount > prevCount) {
            beginInsertRows(QModelIndex(), prevCount + 1, newCount);
            iPrivate->iColors = newColors;
            endInsertRows();
        } else {
            iPrivate->iColors = newColors;
        }
        // Change the colors
        int rangeStart = -1;
        const int n = qMin(prevCount, newCount);
        for (int i = 0; i < n; i++) {
            if (newColors.at(i) != prevColors.at(i)) {
                if (rangeStart < 0) {
                    rangeStart = i;
                }
            } else if (rangeStart >= 0) {
                // End of range
                Q_EMIT dataChanged(index(rangeStart), index(i - 1), colorRole);
                rangeStart = -1;
            }
        }
        if (rangeStart >= 0) {
            // Last range
            Q_EMIT dataChanged(index(rangeStart), index(newCount - 1), colorRole);
        }
        if (newCount != prevCount) {
            // Update the special last item
            const QModelIndex lastIndex(index(n));
            Q_EMIT dataChanged(lastIndex, lastIndex);
        }
        if (iPrivate->resetDragPos()) {
            Q_EMIT dragPosChanged();
        }
        Q_EMIT colorsChanged();
    }
}

void HarbourColorEditorModel::addColor(QColor aColor)
{
    if (aColor.isValid()) {
        const int n = iPrivate->iColors.count();
        beginInsertRows(QModelIndex(), n, n);
        iPrivate->iColors.append(aColor);
        endInsertRows();
        Q_EMIT colorsChanged();
    }
}

int HarbourColorEditorModel::getDragPos() const
{
    return iPrivate->iDragPos;
}

void HarbourColorEditorModel::setDragPos(int aPos)
{
    const int n = iPrivate->iColors.count();
    if (aPos < 0) {
        if (iPrivate->iDragPos >= 0) {
            // The drag is finished
            if (iPrivate->iDragPos != iPrivate->iDragStartPos) {
                if (iPrivate->iDragPos == n) {
                    HDEBUG("trashed" << iPrivate->iDragStartPos);
                    beginRemoveRows(QModelIndex(), iPrivate->iDragPos, iPrivate->iDragPos);
                    iPrivate->iColors.removeAt(iPrivate->iDragStartPos);
                    iPrivate->resetDragPos();
                    endRemoveRows();
                } else {
                    HDEBUG("dragged" << iPrivate->iDragStartPos << "=>" << iPrivate->iDragPos);
                    iPrivate->iColors.move(iPrivate->iDragStartPos, iPrivate->iDragPos);
                    iPrivate->resetDragPos();
                }
                Q_EMIT colorsChanged();
            } else {
                HDEBUG("drag cancelled");
                iPrivate->resetDragPos();
            }
            Q_EMIT dragPosChanged();
        }
    } else {
        if (aPos >= n) aPos = n;
        if (aPos != iPrivate->iDragPos) {
            if (iPrivate->iDragPos >= 0) {
                const bool wasTrashed = (iPrivate->iDragPos == n);
                const bool isTrashed = (aPos == n);
                HDEBUG(aPos << (isTrashed ? "(outside)" : ""));
                const int dest = (aPos > iPrivate->iDragPos) ? (aPos + 1) : aPos;
                beginMoveRows(QModelIndex(), iPrivate->iDragPos, iPrivate->iDragPos, QModelIndex(), dest);
                iPrivate->iDragPos = aPos;
                endMoveRows();
                if (wasTrashed || isTrashed) {
                    const QVector<int> roles(1, Private::ItemTypeRole);
                    const QModelIndex modelIndex(index(iPrivate->iDragPos));
                    Q_EMIT dataChanged(modelIndex, modelIndex, roles);
                }
                Q_EMIT dragPosChanged();
            } else if (aPos < n /* Must be within bounds */) {
                // Drag is starting
                HDEBUG("dragging" << aPos);
                iPrivate->iDragPos = iPrivate->iDragStartPos = aPos;
                Q_EMIT dragPosChanged();
            }
        }
    }
}

int HarbourColorEditorModel::indexOf(QColor aColor) const
{
    return iPrivate->iColors.indexOf(aColor);
}
