/*
 * Copyright (C) 2020 Jolla Ltd.
 * Copyright (C) 2020 Slava Monich <slava@monich.com>
 *
 * You may use this file under the terms of BSD license as follows:
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

import QtQuick 2.0
import Sailfish.Silica 1.0

MouseArea {
    property alias icon: image
    property bool down: pressed && containsMouse
    property bool highlighted: down
    property string hint

    signal showHint(var text)
    signal hideHint()

    width: Theme.itemSizeSmall
    height: Theme.itemSizeSmall

    readonly property bool _showPress: highlighted || pressTimer.running

    onPressedChanged: {
        if (pressed) {
            pressTimer.start()
        }
    }

    onClicked: hideHint()

    onReleased: hideHint()

    onPressAndHold: if (hint !== "") showHint(hint)

    onCanceled: {
        pressTimer.stop()
        hideHint()
    }

    Timer {
        id: pressTimer

        interval: 50
    }

    HarbourHighlightIcon {
        id: image

        highlightColor: _showPress ? Theme.highlightColor : Theme.primaryColor
        sourceSize: Qt.size(Theme.iconSizeMedium, Theme.iconSizeMedium)
        anchors.centerIn: parent
        opacity: parent.enabled ? 1.0 : 0.4
    }
}
