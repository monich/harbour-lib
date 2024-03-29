/*
 * Copyright (C) 2022 Jolla Ltd.
 * Copyright (C) 2022 Slava Monich <slava.monich@jolla.com>
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

#ifndef HARBOUR_BATTERY_H
#define HARBOUR_BATTERY_H

#include <QObject>

class QQmlEngine;
class QJSEngine;

// D-Bus calls used by this object require mce 1.86.0 (Sailfish OS 2.1)
class HarbourBattery: public QObject
{
    Q_OBJECT
    Q_PROPERTY(BatteryStatus batteryStatus READ batteryStatus NOTIFY batteryStatusChanged)
    Q_PROPERTY(BatteryState batteryState READ batteryState NOTIFY batteryStateChanged)
    Q_PROPERTY(int batteryLevel READ batteryLevel NOTIFY batteryLevelChanged)
    Q_ENUMS(BatteryStatus)
    Q_ENUMS(BatteryState)

public:
    enum BatteryStatus {
        BatteryStatusUnknown,
        BatteryStatusEmpty,
        BatteryStatusLow,
        BatteryStatusOk,
        BatteryStatusFull
    };

    enum BatteryState {
        BatteryStateUnknown,
        BatteryStateCharging,
        BatteryStateDischarging,
        BatteryStateNotCharging,
        BatteryStateFull
    };

    explicit HarbourBattery(QObject* aParent = Q_NULLPTR);
    ~HarbourBattery();

    // Callback for qmlRegisterSingletonType<HarbourBattery>
    static QObject* createSingleton(QQmlEngine*, QJSEngine*);

    BatteryStatus batteryStatus() const;
    BatteryState batteryState() const;
    int batteryLevel() const;

Q_SIGNALS:
    void batteryStatusChanged();
    void batteryStateChanged();
    void batteryLevelChanged();

private:
    class Private;
    Private* iPrivate;
};

#endif // HARBOUR_BATTERY_H
