/*
 * Copyright (C) 2015-2016 Jolla Ltd.
 * Copyright (C) 2015-2017 Slava Monich <slava.monich@jolla.com>
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   - Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   - Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   - Neither the name of Jolla Ltd nor the names of its contributors
 *     may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
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

#ifndef HARBOUR_SYSTEM_STATE_H
#define HARBOUR_SYSTEM_STATE_H

#include <QObject>
#include <QString>
#include <QSharedPointer>

class HarbourSystemState: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString displayStatus READ displayStatus NOTIFY displayStatusChanged)
    Q_PROPERTY(QString lockMode READ lockMode NOTIFY lockModeChanged)
    Q_PROPERTY(bool displayOff READ displayOff NOTIFY displayOffChanged)
    Q_PROPERTY(bool locked READ locked NOTIFY lockedChanged)

    Q_PROPERTY(QString MCE_DISPLAY_ON READ _MCE_DISPLAY_ON CONSTANT)
    Q_PROPERTY(QString MCE_DISPLAY_DIM READ _MCE_DISPLAY_DIM CONSTANT)
    Q_PROPERTY(QString MCE_DISPLAY_OFF READ _MCE_DISPLAY_OFF CONSTANT)
    Q_PROPERTY(QString MCE_TK_LOCKED READ _MCE_TK_LOCKED CONSTANT)
    Q_PROPERTY(QString MCE_TK_UNLOCKED READ _MCE_TK_UNLOCKED CONSTANT)

public:
    static const QString MCE_DISPLAY_ON;
    static const QString MCE_DISPLAY_DIM;
    static const QString MCE_DISPLAY_OFF;
    static const QString MCE_TK_LOCKED;
    static const QString MCE_TK_UNLOCKED;

public:
    explicit HarbourSystemState(QObject* aParent = NULL);
    ~HarbourSystemState();

    static QSharedPointer<HarbourSystemState> sharedInstance();

    QString displayStatus() const;
    QString lockMode() const;

    bool displayOff() const;
    bool locked() const;

Q_SIGNALS:
    void displayStatusChanged();
    void displayOffChanged();
    void lockModeChanged();
    void lockedChanged();

private:
    // Getters for QML constants
    QString _MCE_DISPLAY_ON() const { return MCE_DISPLAY_ON; }
    QString _MCE_DISPLAY_DIM() const { return MCE_DISPLAY_DIM; }
    QString _MCE_DISPLAY_OFF() const { return MCE_DISPLAY_OFF; }
    QString _MCE_TK_LOCKED() const { return MCE_TK_LOCKED; }
    QString _MCE_TK_UNLOCKED() const { return MCE_TK_UNLOCKED; }

private:
    class Private;
    Private* iPrivate;
};

#endif // HARBOUR_SYSTEM_STATE_H
