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

Dialog {
    id: thisDialog

    readonly property color selectedColor: sampleItem.color
    property color initialColor: Theme.highlightColor
    property string acceptText: "Add color"
    property alias hueSliderText: hueSlider.label // "Color"
    property alias brightnessSliderText: brightnessSlider.label // "Brightness"
    property string hexNotationText: "Hex notation"

    canAccept: hexText.acceptableInput
    forwardNavigation: !hueItem.pressed
    backNavigation: !hueItem.pressed

    DialogHeader {
        id: header

        acceptText: forwardNavigation ?
            (thisDialog.acceptText ? thisDialog.acceptText : defaultAcceptText) : ""
    }

    Component.onCompleted: hexText.text = initialColor.toString().substr(1)

    // Otherwise width is changing with a delay, causing visible layout changes
    onIsLandscapeChanged: width = isLandscape ? Screen.height : Screen.width

    SilicaFlickable {
        clip: true
        interactive: !hueItem.pressed
        anchors {
            left: parent.left
            right: parent.right
            top: header.bottom
            bottom: parent.bottom
        }

        HarbourColorHueItem {
            id: hueItem

            x: Theme.horizontalPageMargin
            width: parent.width - 2 * x
            anchors {
                top: parent.top
                bottom: toolPanel.top
                bottomMargin: Theme.paddingSmall
            }
            brightness: brightnessSlider.sliderValue
            onTapped: hueSlider.value = h
        }

        Column {
            id: toolPanel

            width: parent.width
            anchors.bottom: parent.bottom

            Slider {
                id: hueSlider

                width: parent.width
                leftMargin: Theme.horizontalPageMargin
                rightMargin: Theme.horizontalPageMargin
                minimumValue: 0
                maximumValue: 1
                value: 1
                label: "Color"
                opacity: (y + parent.y >= 0) ? 1 : 0
                onSliderValueChanged: hexText.updateText()
            }

            Slider {
                id: brightnessSlider

                width: parent.width
                leftMargin: Theme.horizontalPageMargin
                rightMargin: Theme.horizontalPageMargin
                label: "Brightness"
                minimumValue: 0
                maximumValue: 1
                value: 1
                opacity: (y + parent.y >= 0) ? 1 : 0
                onSliderValueChanged: hexText.updateText()
            }

            Item {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * x
                height: hexText.height
                Row {
                    spacing: Theme.paddingSmall

                    Label {
                        id: hexLabel

                        y: hexText.textTopMargin
                        text: "#"
                        font.pixelSize: Theme.fontSizeLarge
                    }

                    Item {
                        readonly property int maxWidth: toolPanel.width - 2 * Theme.horizontalPageMargin - hexLabel.width - parent.spacing - Theme.paddingLarge - sample.width
                        width: Math.min(Math.max(textLabelTemplate.implicitWidth, Theme.itemSizeHuge), maxWidth)
                        height: hexText.height

                        Label {
                            id: textLabelTemplate

                            // Same text as hexText.label
                            text: hexNotationText
                            font.pixelSize: Theme.fontSizeSmall
                            opacity: 0
                        }

                        TextField {
                            id: hexText

                            property int ignoreTextUpdates // to avoid binding loops
                            property color tmpColor

                            font.pixelSize: Theme.fontSizeLarge
                            width: parent.width
                            textLeftMargin: 0
                            textRightMargin: 0
                            label: hexNotationText
                            validator: RegExpValidator { regExp: /^[0-9a-fA-F]{6}$/ }
                            inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase
                            EnterKey.iconSource: "image://theme/icon-m-enter-close"
                            EnterKey.onClicked: hexText.focus = false

                            onTextChanged: {
                                if (!ignoreTextUpdates) {
                                    // acceptableInput hasn't been updated yet
                                    syncTimer.restart()
                                }
                            }

                            onActiveFocusChanged: {
                                if (!activeFocus && !acceptableInput) {
                                    updateText()
                                }
                            }

                            function syncColor() {
                                if (acceptableInput) {
                                    tmpColor = "#" + text
                                    ignoreTextUpdates++
                                    brightnessSlider.value = hueItem.getV(tmpColor)
                                    hueSlider.value = hueItem.getH(tmpColor)
                                    ignoreTextUpdates--
                                }
                            }

                            function updateText() {
                                if (!ignoreTextUpdates) {
                                    ignoreTextUpdates++
                                    var s = hueItem.getColor(hueSlider.sliderValue).toString()
                                    text = (s.length > 0 && s.charAt(0) === '#') ? s.substr(1) : s
                                    ignoreTextUpdates--
                                }
                            }

                            Timer {
                                id: syncTimer

                                interval: 0
                                onTriggered: hexText.syncColor()
                            }
                        }
                    }
                }

                MouseArea {
                    id: sample

                    y: Theme.paddingLarge
                    width: 2 * height
                    height: hexText.height - 2 * Theme.paddingLarge
                    anchors.right: parent.right
                    visible: hexText.acceptableInput

                    onClicked: thisDialog.accept()

                    Rectangle {
                        id: sampleItem

                        radius: Theme.paddingSmall
                        anchors.fill: parent
                        color: "#" + hexText.text
                        layer.enabled: sample.pressed && sample.containsMouse
                        layer.effect: HarbourPressEffect { source: sampleItem }
                    }
                }
            }
        }
    }
}
