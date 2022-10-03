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

#include "HarbourBattery.h"
#include "HarbourDebug.h"
#include "HarbourMce.h"

#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>

// ==========================================================================
// HarbourBattery::Private
// ==========================================================================

class HarbourBattery::Private : public HarbourMce
{
    Q_OBJECT

public:
    static const QString MCE_BATTERY_STATUS_UNKNOWN;
    static const QString MCE_BATTERY_STATUS_FULL;
    static const QString MCE_BATTERY_STATUS_OK;
    static const QString MCE_BATTERY_STATUS_LOW;
    static const QString MCE_BATTERY_STATUS_EMPTY;

    static const QString MCE_BATTERY_STATE_UNKNOWN;
    static const QString MCE_BATTERY_STATE_DISCHARGING;
    static const QString MCE_BATTERY_STATE_CHARGING;
    static const QString MCE_BATTERY_STATE_NOT_CHARGING;
    static const QString MCE_BATTERY_STATE_FULL;

    Private(HarbourBattery*);

    HarbourBattery* parentObject() const;

public Q_SLOTS:
    void updateBatteryStatus(const QString);
    void updateBatteryState(const QString);
    void updateBatteryLevel(int);
    void onBatteryStatusQueryDone(QDBusPendingCallWatcher*);
    void onBatteryStateQueryDone(QDBusPendingCallWatcher*);
    void onBatteryLevelQueryDone(QDBusPendingCallWatcher*);

public:
    BatteryStatus iBatteryStatus;
    BatteryState iBatteryState;
    int iBatteryLevel;
};

const QString HarbourBattery::Private::MCE_BATTERY_STATUS_UNKNOWN("unknown");
const QString HarbourBattery::Private::MCE_BATTERY_STATUS_FULL("full");
const QString HarbourBattery::Private::MCE_BATTERY_STATUS_OK("ok");
const QString HarbourBattery::Private::MCE_BATTERY_STATUS_LOW("low");
const QString HarbourBattery::Private::MCE_BATTERY_STATUS_EMPTY("empty");

const QString HarbourBattery::Private::MCE_BATTERY_STATE_UNKNOWN("unknown");
const QString HarbourBattery::Private::MCE_BATTERY_STATE_DISCHARGING("discharging");
const QString HarbourBattery::Private::MCE_BATTERY_STATE_CHARGING("charging");
const QString HarbourBattery::Private::MCE_BATTERY_STATE_NOT_CHARGING("not_charging");
const QString HarbourBattery::Private::MCE_BATTERY_STATE_FULL("full");

HarbourBattery::Private::Private(
    HarbourBattery* aParent) :
    HarbourMce(aParent),
    iBatteryStatus(BatteryStatusUnknown),
    iBatteryState(BatteryStateUnknown),
    iBatteryLevel(0)
{
    setupProperty("get_battery_status", "battery_status_ind",
        SLOT(onBatteryStatusQueryDone(QDBusPendingCallWatcher*)),
        SLOT(updateBatteryStatus(QString)));
    setupProperty("get_battery_state", "battery_state_ind",
        SLOT(onBatteryStateQueryDone(QDBusPendingCallWatcher*)),
        SLOT(updateBatteryState(QString)));
    setupProperty("get_battery_level", "battery_level_ind",
        SLOT(onBatteryLevelQueryDone(QDBusPendingCallWatcher*)),
        SLOT(updateBatteryLevel(int)));
}

inline
HarbourBattery*
HarbourBattery::Private::parentObject() const
{
    return qobject_cast<HarbourBattery*>(parent());
}

void
HarbourBattery::Private::updateBatteryStatus(
    const QString aStatus)
{
    BatteryStatus status = BatteryStatusUnknown;

    if (aStatus == MCE_BATTERY_STATUS_OK) {
        status = BatteryStatusOk;
    } else if (aStatus == MCE_BATTERY_STATUS_FULL) {
        status = BatteryStatusFull;
    } else if (aStatus == MCE_BATTERY_STATUS_LOW) {
        status = BatteryStatusLow;
    } else if (aStatus == MCE_BATTERY_STATUS_EMPTY) {
        status = BatteryStatusEmpty;
    } else if (aStatus != MCE_BATTERY_STATUS_UNKNOWN) {
        HWARN("Unexpected battery status" << aStatus);
        return;
    }

    if (iBatteryStatus != status) {
        iBatteryStatus = status;
        HDEBUG(aStatus);
        Q_EMIT parentObject()->batteryStatusChanged();
    }
}

void
HarbourBattery::Private::updateBatteryState(
    const QString aState)
{
    BatteryState state = BatteryStateUnknown;

    if (aState == MCE_BATTERY_STATE_CHARGING) {
        state = BatteryStateCharging;
    } else if (aState == MCE_BATTERY_STATE_DISCHARGING) {
        state = BatteryStateDischarging;
    } else if (aState == MCE_BATTERY_STATE_NOT_CHARGING) {
        state = BatteryStateNotCharging;
    } else if (aState == MCE_BATTERY_STATE_FULL) {
        state = BatteryStateFull;
    } else if (aState != MCE_BATTERY_STATE_UNKNOWN) {
        HWARN("Unexpected battery state" << aState);
        return;
    }

    if (iBatteryState != state) {
        iBatteryState = state;
        HDEBUG(aState);
        Q_EMIT parentObject()->batteryStateChanged();
    }
}

void
HarbourBattery::Private::updateBatteryLevel(
    int aLevel)
{
    if (iBatteryLevel != aLevel) {
        iBatteryLevel = aLevel;
        HDEBUG(aLevel);
        Q_EMIT parentObject()->batteryLevelChanged();
    }
}

void
HarbourBattery::Private::onBatteryStatusQueryDone(
    QDBusPendingCallWatcher* aWatcher)
{
    QDBusPendingReply<QString> reply(*aWatcher);

    if (reply.isError()) {
        HWARN(reply.error().message());
    } else {
        updateBatteryStatus(reply.value());
    }
    aWatcher->deleteLater();
}

void
HarbourBattery::Private::onBatteryStateQueryDone(
    QDBusPendingCallWatcher* aWatcher)
{
    QDBusPendingReply<QString> reply(*aWatcher);

    if (reply.isError()) {
        HWARN(reply.error().message());
    } else {
        updateBatteryState(reply.value());
    }
    aWatcher->deleteLater();
}

void
HarbourBattery::Private::onBatteryLevelQueryDone(
    QDBusPendingCallWatcher* aWatcher)
{
    QDBusPendingReply<int> reply(*aWatcher);

    if (reply.isError()) {
        HWARN(reply.error().message());
    } else {
        updateBatteryLevel(reply.value());
    }
    aWatcher->deleteLater();
}

// ==========================================================================
// HarbourBattery
// ==========================================================================

HarbourBattery::HarbourBattery(
    QObject* aParent) :
    QObject(aParent),
    iPrivate(new Private(this))
{
    HDEBUG("created");
}

HarbourBattery::~HarbourBattery()
{
    HDEBUG("destroyed");
}

// Callback for qmlRegisterSingletonType<HarbourBattery>
QObject*
HarbourBattery::createSingleton(
    QQmlEngine*,
    QJSEngine*)
{
    return new HarbourBattery();
}

HarbourBattery::BatteryState
HarbourBattery::batteryState() const
{
    return iPrivate->iBatteryState;
}

HarbourBattery::BatteryStatus
HarbourBattery::batteryStatus() const
{
    return iPrivate->iBatteryStatus;
}

int
HarbourBattery::batteryLevel() const
{
    return iPrivate->iBatteryLevel;
}

#include "HarbourBattery.moc"
