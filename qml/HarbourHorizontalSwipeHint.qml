/*
 * Copyright (C) 2019-2020 Jolla Ltd.
 * Copyright (C) 2019-2020 Slava Monich <slava@monich.com>
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

import QtQuick 2.0
import Sailfish.Silica 1.0

Item {
    id: hint

    anchors.fill: parent

    property bool swipeRight
    property bool bothWays
    property bool hintEnabled
    property alias text: label.text
    property alias hintDelay: hintDelayTimer.interval
    property alias loops: touchInteractionHint.loops
    readonly property bool hintRunning: hintDelayTimer.running || touchInteractionHint.running || label.opacity > 0

    signal hintShown()

    function showHint() {
        if (!touchInteractionHint.running) {
            touchInteractionHint.direction = touchInteractionHint.defaultDirection
            touchInteractionHint.start()
        }
    }

    onHintEnabledChanged: {
        if (hint.hintEnabled) {
            hintDelayTimer.restart()
        } else {
            hintDelayTimer.stop()
        }
    }

    Component.onCompleted: if (hint.hintEnabled) hintDelayTimer.start()

    InteractionHintLabel {
        id: label

        anchors.bottom: parent.bottom
        opacity: touchInteractionHint.running ? 1.0 : 0.0
        Behavior on opacity { FadeAnimation { duration: 1000 } }
    }

    TouchInteractionHint {
        id: touchInteractionHint

        readonly property int defaultDirection: swipeRight ? TouchInteraction.Right : TouchInteraction.Left
        readonly property int otherDirection: swipeRight ? TouchInteraction.Left : TouchInteraction.Right
        anchors.verticalCenter: parent.verticalCenter
        onRunningChanged: {
            if (!running) {
                if (bothWays && direction === defaultDirection) {
                    direction = otherDirection
                    start()
                } else {
                    hint.hintShown()
                }
            }
        }
    }

    Timer {
        id: hintDelayTimer

        interval: 1000
        onTriggered: showHint()
    }
}
