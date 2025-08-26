/*
 * Copyright (C) 2019-2025 Slava Monich <slava@monich.com>
 * Copyright (C) 2019 Jolla Ltd.
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

#ifndef HARBOUR_ORGANIZE_LIST_MODEL_H
#define HARBOUR_ORGANIZE_LIST_MODEL_H

#include <QtCore/QSortFilterProxyModel>

// This model helps to implement rearranging list items by dragging.
// When drag starts, QML sets dragIndex property and then updates
// dragPos when with the position of the dragged list item. When
// drag is finished, QML sets dragIndex to -1 and the model calls
// moveRow() on the underlying (source) model, to finish the move.

class HarbourOrganizeListModel :
    public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QObject* sourceModel READ sourceModel WRITE setSourceModelObject NOTIFY sourceModelObjectChanged)
    Q_PROPERTY(int dragIndex READ dragIndex WRITE setDragIndex NOTIFY dragIndexChanged)
    Q_PROPERTY(int dragPos READ dragPos WRITE setDragPos NOTIFY dragPosChanged)

public:
    HarbourOrganizeListModel(QObject* aParent = Q_NULLPTR);
    ~HarbourOrganizeListModel();

    void setSourceModelObject(QObject*);

    int dragIndex() const;
    void setDragIndex(int);

    int dragPos() const;
    void setDragPos(int);

    // QAbstractProxyModel
    QModelIndex mapToSource(const QModelIndex&) const Q_DECL_OVERRIDE;
    QModelIndex mapFromSource(const QModelIndex&) const Q_DECL_OVERRIDE;

Q_SIGNALS:
    void sourceModelObjectChanged();
    void dragIndexChanged();
    void dragPosChanged();

private:
    class Private;
    Private* iPrivate;
};

#endif // HARBOUR_ORGANIZE_LIST_MODEL_H
