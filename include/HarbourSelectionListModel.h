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

#ifndef HARBOUR_SELECTION_LIST_MODEL_H
#define HARBOUR_SELECTION_LIST_MODEL_H

#include <QtQml>
#include <QIdentityProxyModel>

class HarbourSelectionListModel : public QIdentityProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QObject* sourceModel READ sourceModel WRITE setSourceModelObject NOTIFY sourceModelObjectChanged)
    Q_PROPERTY(QList<int> nonSelectableRows READ nonSelectableRows WRITE setNonSelectableRows NOTIFY nonSelectableRowsChanged)
    Q_PROPERTY(QList<int> selectedRows READ selectedRows NOTIFY selectedRowsChanged)
    Q_PROPERTY(int selectableCount READ selectableCount NOTIFY selectableCountChanged)
    Q_PROPERTY(int selectionCount READ selectionCount NOTIFY selectionCountChanged)
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
    HarbourSelectionListModel(QObject* aParent = Q_NULLPTR);

    void setSourceModelObject(QObject*);

    QList<int> nonSelectableRows() const;
    void setNonSelectableRows(const QList<int>);

    QList<int> selectedRows() const;
    int selectableCount() const;
    int selectionCount() const;

    Q_INVOKABLE void selectAll();
    Q_INVOKABLE void clearSelection();
    Q_INVOKABLE void toggleRows(const QList<int>);

    // QAbstractItemModel
    Qt::ItemFlags flags(const QModelIndex&) const Q_DECL_OVERRIDE;
    QHash<int,QByteArray> roleNames() const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex&, int) const Q_DECL_OVERRIDE;
    bool setData(const QModelIndex&, const QVariant&, int) Q_DECL_OVERRIDE;

Q_SIGNALS:
    void sourceModelObjectChanged();
    void nonSelectableRowsChanged();
    void selectedRowsChanged();
    void selectableCountChanged();
    void selectionCountChanged();
    void countChanged();

private:
    class Private;
    Private* iPrivate;
};

QML_DECLARE_TYPE(HarbourSelectionListModel)

#endif // HARBOUR_SELECTION_LIST_MODEL_H
