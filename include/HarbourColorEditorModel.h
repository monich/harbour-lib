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

#ifndef HARBOUR_COLOR_EDITOR_MODEL_H
#define HARBOUR_COLOR_EDITOR_MODEL_H

#include <QList>
#include <QColor>
#include <QAbstractListModel>

class HarbourColorEditorModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QStringList colors READ getColors WRITE setColors NOTIFY colorsChanged)
    Q_PROPERTY(int dragPos READ getDragPos WRITE setDragPos NOTIFY dragPosChanged)
    Q_ENUMS(ItemType)

public:
    enum ItemType {
        ColorItem,
        TrashedItem,
        AddItem
    };

    HarbourColorEditorModel(QObject* parent = Q_NULLPTR);

    QStringList getColors() const;
    void setColors(QStringList colors);

    int getDragPos() const;
    void setDragPos(int pos);

    Q_INVOKABLE void addColor(QColor color);

    // QAbstractListModel
    QHash<int,QByteArray> roleNames() const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex& parent) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex& index, int role) const Q_DECL_OVERRIDE;

Q_SIGNALS:
    void colorsChanged();
    void dragPosChanged();

private:
    class Private;
    Private* iPrivate;
};

#endif // HARBOUR_COLOR_EDITOR_MODEL_H
