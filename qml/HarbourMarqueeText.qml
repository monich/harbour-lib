/*
 * Copyright (C) 2026 Slava Monich <slava@monich.com>
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

import QtQuick 2.0
import Sailfish.Silica 1.0

Item {
    id: thisItem

    clip: true
    implicitWidth: label.implicitWidth
    implicitHeight: label.implicitHeight

    property bool startOnTap: true
    property bool autoStart: true
    property alias autoStartDelay: autoStartTimer.interval
    property alias text: label.text
    property alias font: label.font
    property alias color: label.color
    property alias truncationMode: label.truncationMode
    property alias verticalAlignment: label.verticalAlignment
    property int horizontalAlignment: Text.AlignLeft
    property int loops: 1
    property real speed: 1

    readonly property bool _needScroll: label.contentWidth > thisItem.width

    function start() {
        if (_needScroll) {
            if (horizontalAlignment === Text.AlignRight) {
                leftFadeMarqueeAnimation.start()
            } else {
                rightFadeMarqueeAnimation.start()
            }
        }
    }

    function stop() {
        leftFadeMarqueeAnimation.stop()
        rightFadeMarqueeAnimation.stop()
    }

    onAutoStartChanged: {
        if (autoStart) {
            if (visible) {
                autoStartTimer.start()
            }
        } else {
            autoStartTimer.stop()
        }
    }

    onVisibleChanged: {
        if (visible) {
            if (autoStart) {
                autoStartTimer.start()
            }
        } else {
            autoStartTimer.stop()
            leftFadeMarqueeAnimation.complete()
            rightFadeMarqueeAnimation.complete()
        }
    }

    onTextChanged: {
        if (visible && autoStart) {
            autoStartTimer.start()
        }
    }

    Timer {
        id: autoStartTimer

        interval: 1000
        onTriggered: thisItem.start()
    }

    Label {
        id: label

        readonly property real _x: (thisItem.horizontalAlignment === Text.AlignHCenter) ?
            Math.max(0, (thisItem.width - contentWidth) / 2) : 0

        x: _x
        width: parent.width - x
        height: parent.height
        maximumLineCount: 1
        truncationMode: TruncationMode.Fade
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: thisItem.horizontalAlignment === Text.AlignHCenter ?
            Text.AlignLeft : thisItem.horizontalAlignment
    }

    MouseArea {
        enabled: thisItem.startOnTap && _needScroll
        anchors.fill: parent
        onClicked: start()
    }

    function _duration(span) {
        // By default, covering Theme.itemSizeHuge in 400 ms
        return (span > 0) ? (span * speed * 400 / Theme.itemSizeHuge) : 0
    }

    SequentialAnimation {
        id: rightFadeMarqueeAnimation

        loops: thisItem.loops
        alwaysRunToEnd: true

        NumberAnimation {
            target: label
            property: "leftPadding"
            duration: thisItem._duration(label._x + label.contentWidth)
            easing.type: Easing.Linear
            from: label._x
            to: -label.contentWidth
        }

        NumberAnimation {
            target: label
            property: "leftPadding"
            duration: thisItem._duration(thisItem.width - label._x)
            easing.type: Easing.Linear
            from: thisItem.width - label._x
            to: 0
        }
    }

    SequentialAnimation {
        id: leftFadeMarqueeAnimation

        loops: thisItem.loops
        alwaysRunToEnd: true

        NumberAnimation {
            target: label
            property: "rightPadding"
            duration: thisItem._duration(thisItem.width)
            easing.type: Easing.Linear
            from: 0
            to: thisItem.width
        }

        NumberAnimation {
            target: label
            property: "rightPadding"
            duration: thisItem._duration(label.contentWidth)
            easing.type: Easing.Linear
            from: -label.contentWidth
            to: 0
        }
    }
}
