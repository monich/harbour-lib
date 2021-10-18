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

import QtQuick 2.0
import Sailfish.Silica 1.0

Rectangle {
    id: thisItem

    color: "black"

    property alias brightness: mouseArea.opacity
    property alias pressed: mouseArea.pressed

    readonly property color color0: "#ffffff" // 1.0
    readonly property color color1: "#ff0000" // colorStop7
    readonly property color color2: "#ff00ff" // colorStop6
    readonly property color color3: "#0000ff" // colorStop5
    readonly property color color4: "#00ffff" // colorStop4
    readonly property color color5: "#00ff00" // colorStop3
    readonly property color color6: "#ffff00" // colorStop2
    readonly property color color7: "#ff0000" // colorStop1
    readonly property color color8: "#ffffff" // 0.0

    readonly property real colorStop1: 0.125
    readonly property real colorStop2: 0.250
    readonly property real colorStop3: 0.375
    readonly property real colorStop4: 0.500
    readonly property real colorStop5: 0.625
    readonly property real colorStop6: 0.750
    readonly property real colorStop7: 0.875
    readonly property real hueLast: colorStop7
    readonly property real hueRange: hueLast - colorStop1

    signal tapped(var h)

    function getColor(h)  {
        if (h < colorStop4) {
            if (h > colorStop3) {
                // colorStop3..colorStop4
                return averageColor(color3, color4, (h - colorStop3)/(colorStop4 - colorStop3), brightness)
            } else if (h > colorStop2) {
                // colorStop2..colorStop3
                return averageColor(color2, color3, (h - colorStop2)/(colorStop3 - colorStop2), brightness)
            } else if (h > colorStop1) {
                // colorStop1..colorStop2
                return averageColor(color1, color2, (h - colorStop1)/(colorStop2 - colorStop1), brightness)
            } else if (h > 0) {
                // 0..colorStop1
                return averageColor(color0, color1, h/colorStop1, brightness)
            } else {
                return adjustColor(color0, brightness)
            }
        } else {
            if (h < colorStop5) {
                // colorStop4..colorStop5
                return averageColor(color4, color5, (h - colorStop4)/(colorStop5 - colorStop4), brightness)
            } else if (h < colorStop6) {
                // colorStop5..colorStop6
                return averageColor(color5, color6, (h - colorStop5)/(colorStop6 - colorStop5), brightness)
            } else if (h < colorStop7) {
                // colorStop6..colorStop7
                return averageColor(color6, color7, (h - colorStop6)/(colorStop7 - colorStop6), brightness)
            } else if (h < 1) {
                // colorStop7..1
                return averageColor(color7, color8, (h - colorStop7)/(1 - colorStop7), brightness)
            } else {
                return adjustColor(color8, brightness)
            }
        }
    }

    function averageColor(c1,c2,h,v)  {
        return Qt.rgba((c1.r + (c2.r - c1.r) * h) * v,
                       (c1.g + (c2.g - c1.g) * h) * v,
                       (c1.b + (c2.b - c1.b) * h) * v, 1)
    }

    function adjustColor(c,v)  {
        return Qt.rgba(c.r * v, c.g * v, c.b * v, 1)
    }

    function getV(c) {
        return Math.max(c.r, c.g, c.b)
    }

    function getH(c) {
        var max = Math.max(c.r, c.g, c.b)
        var min = Math.min(c.r, c.g, c.b)
        var chroma = max - min
        if (chroma === 0) {
            return 0
        } else {
            if (max === c.r) {
                return hueLast - (c.g - c.b)/chroma/6 * hueRange
            } else if (max === c.g) {
                return hueLast - (2 + (c.b - c.r)/chroma)/6 * hueRange
            } else {
                return hueLast - (4 + (c.r - c.g)/chroma)/6 * hueRange
            }
        }
    }

    MouseArea {
        id: mouseArea

        width: parent.height
        height: parent.width
        anchors.centerIn: parent
        rotation: -90

        ShaderEffectSource {
            anchors.fill: parent
            sourceItem: Rectangle {
                width: mouseArea.width
                height: mouseArea.height
                radius: Theme.paddingSmall
                gradient: Gradient {
                    GradientStop { position: 0.0;    color: color0 }
                    GradientStop { position: colorStop1; color: color1 }
                    GradientStop { position: colorStop2; color: color2 }
                    GradientStop { position: colorStop3; color: color3 }
                    GradientStop { position: colorStop4; color: color4 }
                    GradientStop { position: colorStop5; color: color5 }
                    GradientStop { position: colorStop6; color: color6 }
                    GradientStop { position: colorStop7; color: color7 }
                    GradientStop { position: 1.0;        color: color8 }
                }
            }
        }
        // mouseY/height because we are rotated
        onClicked: thisItem.tapped(mouseY/height)
        onPositionChanged: thisItem.tapped(mouseY/height)
    }
}
