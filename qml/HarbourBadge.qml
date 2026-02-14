/*
 * Copyright (C) 2018-2026 Slava Monich <slava@monich.com>
 * Copyright (C) 2018-2022 Jolla Ltd.
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
    id: badge

    property real maxWidth: parent.width
    property alias text: label.text
    property alias backgroundColor: background.color
    property alias textColor: label.color

    readonly property real _radius: height/2

    width: Math.max(label.contentWidth + _radius, height)
    height: Theme.itemSizeSmall/2
    visible: opacity > 0
    opacity: text.length ? 1 : 0

    Behavior on opacity { FadeAnimation {} }

    Rectangle {
        id: background

        anchors.fill: parent
        color: Theme.rgba(Theme.primaryColor, 0.2)
        radius: badge._radius
    }

    Label {
        id: label

        anchors.centerIn: background
        color: Theme.highlightColor
        font.bold: true
        height: Math.floor(parent.height - _radius/2)
        horizontalAlignment: Text.AlignHCenter
        minimumPixelSize: Theme.fontSizeTiny
        fontSizeMode: Text.Fit
    }
}
