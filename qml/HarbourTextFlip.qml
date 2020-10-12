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

Rotation {
    id: rotation

    property string text
    property Item target
    property real duration: 500
    property bool enabled: true
    property string property: "text"

    property var animation: SequentialAnimation {
        alwaysRunToEnd: true

        onRunningChanged: {
            if (!running && target[property] != rotation.text) {
                start()
            }
        }

        NumberAnimation {
            easing.type: Easing.InOutSine
            target: rotation
            property: "angle"
            from: 0
            to: 90
            duration: rotation.duration/2
        }
        ScriptAction { script: target[property] = rotation.text; }
        NumberAnimation {
            easing.type: Easing.InOutSine
            target: rotation
            property: "angle"
            to: 0
            duration: rotation.duration/2
        }
    }

    origin {
        x: target.width / 2
        y: target.height / 2
    }

    axis {
        x: 1
        y: 0
        z: 0
    }

    function flipTo(value) {
        if (!!target) {
            animation.start()
            text = value
        }
    }

    function _updateTargetProperty() {
        if (!!target) {
            if (enabled) {
                animation.start()
            } else if (!animation.running) {
                target[property] = text
            }
        }
    }

    onTextChanged: _updateTargetProperty()
    onTargetChanged: _updateTargetProperty()
}
