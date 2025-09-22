/*
 * Copyright (C) 2018-2025 Slava Monich <slava@monich.com>
 * Copyright (C) 2018-2021 Jolla Ltd.
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

TextField {
    id: thisItem

    property bool showEchoModeToggle: activeFocus
    property int passwordEchoMode: TextInput.Password

    // Sailfish OS 4.0 renamed TextBase._contentItem into contentItem
    readonly property var editContentItem: ('contentItem' in thisItem) ? contentItem : ('_contentItem' in thisItem) ? _contentItem : null

    property bool _usePasswordEchoMode: true
    property int _buttonLeftMargin: Theme.paddingLarge

    width: (parent ? parent.width : Screen.width) - 2*x
    textRightMargin: textMargin
    echoMode: _usePasswordEchoMode ? passwordEchoMode : TextInput.Normal
    inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase
    placeholderText: label

    //: Default label for password field
    //% "Password"
    label: qsTrId("components-la-password")

    function requestFocus() {
        focusOutBehavior = FocusBehavior.KeepFocus
        forceActiveFocus()
        keepFocusTimer.start()
    }

    Connections {
        target: Qt.application
        onActiveChanged: {
            if (!Qt.application.active) {
                _usePasswordEchoMode = true
                if (!readOnly) text = "" // Reset whatever has been typed
            }
        }
    }

    Timer {
        id: keepFocusTimer

        interval: 500
        onTriggered: thisItem.focusOutBehavior = FocusBehavior.ClearItemFocus
    }

    states: State {
        name: "showToggle"
        when: thisItem.showEchoModeToggle
        PropertyChanges {
            target: thisItem
            textRightMargin: thisItem._buttonLeftMargin/2 + passwordVisibilityButton.width + thisItem.textMargin
        }
        PropertyChanges {
            target: passwordVisibilityButton
            opacity: 1
            enabled: true
        }
    }

    transitions: Transition {
        from: ""
        to: "showToggle"
        reversible: true
        NumberAnimation {
            property: "textRightMargin"
            duration: 50
        }
        FadeAnimation {
            target: passwordVisibilityButton
        }
    }

    MouseArea {
        id: passwordVisibilityButton

        parent: thisItem    // ensure the field is visible, avoid auto-parenting to TextBase contentItem
        x: thisItem.width - width - thisItem.textMargin
        width: Math.max(textAbc.implicitWidth, textDots.implicitWidth) + thisItem._buttonLeftMargin/2
        height: thisItem.height - Theme.paddingLarge
        opacity: 0
        enabled: false

        onClicked: {
            thisItem._usePasswordEchoMode = !thisItem._usePasswordEchoMode
        }

        Text {
            id: textAbc

            anchors {
                top: parent.top
                topMargin: thisItem.textTopMargin
                horizontalCenter: parent.horizontalCenter
            }
            visible: thisItem.echoMode == TextInput.Password
            font.pixelSize: Theme.fontSizeMedium
            text: "abc"
            textFormat: Text.PlainText
            color: parent.pressed && parent.containsMouse ? Theme.highlightColor : Theme.primaryColor
        }

        Text {
            id: textDots

            anchors {
                top: parent.top
                topMargin: thisItem.textTopMargin
                horizontalCenter: parent.horizontalCenter
            }
            visible: thisItem.echoMode == TextInput.Normal
            font.pixelSize: Theme.fontSizeMedium
            text: "\u2022\u2022\u2022"
            textFormat: Text.PlainText
            color: parent.pressed && parent.containsMouse ? Theme.highlightColor : Theme.primaryColor
        }
    }
}
