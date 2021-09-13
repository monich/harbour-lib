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

    property color color
    property alias colorModel: grid.model // HarbourColorEditorModel

    // Localizable strings
    property alias resetColorsMenuText: resetColorsMenuItem.text // "Reset colors"
    property string acceptText: "Select color"
    property string addColorAcceptText: "Add color"
    property string addColorHueSliderText: "Color"
    property string addColorBrightnessSliderText: "Brightness"
    property string addColorHexNotationText: "Hex notation"

    forwardNavigation: false

    signal resetColors()

    // Constants used by HarbourColorEditorModel
    readonly property int itemTypeColor: 0
    readonly property int itemTypeTrash: 1
    readonly property int itemTypeAdd: 2

    SilicaGridView {
        id: grid

        cellWidth: Math.floor(width/cellsPerRow)
        cellHeight: cellWidth
        anchors.fill: parent

        VerticalScrollDecorator {}

        PullDownMenu {
            MenuItem {
                id: resetColorsMenuItem

                text: "Reset colors"
                onClicked: thisDialog.resetColors()
            }
        }

        property Item dragItem
        property int pageHeaderHeight
        readonly property int cellsPerRow: Math.floor(width / Theme.itemSizeHuge)

        header: DialogHeader {
            dialog: thisDialog
            acceptText: thisDialog.acceptText
            spacing: 0
            Component.onCompleted: grid.pageHeaderHeight = height
            onHeightChanged: grid.pageHeaderHeight = height
        }

        delegate: MouseArea {
            id: colorDelegate

            width: grid.cellWidth
            height: grid.cellHeight

            drag.target: dragging ? colorItem : null
            drag.axis: Drag.XandYAxis

            readonly property bool highlighted: (pressed && containsMouse)
            readonly property bool dragging: grid.dragItem === colorDelegate
            readonly property int index: model.index

            Rectangle {
                id: colorItem

                width: grid.cellWidth
                height: grid.cellHeight
                color: model.color
                scale: !colorDelegate.dragging ? 1 : isTrashed ? 0.8 : 1.1
                opacity: isTrashed ? 0.8 : 1
                layer.enabled: colorDelegate.highlighted || colorDelegate.dragging || scale > 1
                layer.effect: HarbourPressEffect { source: colorItem }

                readonly property bool isTrashed: model.itemType === itemTypeTrash
                property real returnX
                property real returnY
                property bool snappingBack

                Behavior on opacity { FadeAnimation { } }
                Behavior on scale {
                    NumberAnimation {
                        easing.type: Easing.InQuad
                        duration: 150
                    }
                }

                Image {
                    anchors.centerIn: parent
                    source: opacity ? ("image://theme/graphic-close-app?" + deriveColor(model.color)) : ""
                    sourceSize: Qt.size(Theme.iconSizeLarge, Theme.iconSizeLarge)
                    opacity: (model.itemType === itemTypeTrash) ? 0.8 : 0
                    visible: opacity > 0
                    Behavior on opacity { FadeAnimation { } }

                    function deriveColor(color) {
                        var gray = 0.299 * color.r + 0.587 * color.g + 0.114 *color.b
                        var v = (gray > 0.5) ? 0 : 1
                        return Qt.rgba(v, v, v, color.a)
                    }
                }

                states: [
                    State {
                        name: "dragging"
                        when: colorDelegate.dragging
                        ParentChange {
                            target: colorItem
                            parent: grid
                            x: colorDelegate.mapToItem(grid, 0, 0).x
                            y: colorDelegate.mapToItem(grid, 0, 0).y
                        }
                    },
                    State {
                        name: "snappingBack"
                        when: !colorDelegate.dragging && colorItem.snappingBack
                        ParentChange {
                            target: colorItem
                            parent: grid
                            x: colorItem.returnX
                            y: colorItem.returnY
                        }
                    },
                    State {
                        name: "idle"
                        when: !colorDelegate.dragging && !colorItem.snappingBack
                        ParentChange {
                            target: colorItem
                            parent: colorDelegate
                            x: 0
                            y: 0
                        }
                    }
                ]

                transitions: [
                    Transition {
                        to: "snappingBack"
                        SequentialAnimation {
                            SmoothedAnimation {
                                properties: "x,y"
                                duration: 150
                            }
                            ScriptAction { script: colorItem.snappingBack = false }
                        }
                    },
                    Transition {
                        to: "idle"
                        ScriptAction { script: colorModel.dragPos = -1 }
                    }
                ]

                Connections {
                    target: colorDelegate.dragging ? colorItem : null
                    onXChanged: colorItem.updateDragPos()
                    onYChanged: colorItem.scrollAndUpdateDragPos()
                }

                Connections {
                    target: colorDelegate.dragging ? grid : null
                    onContentXChanged: colorItem.updateDragPos()
                    onContentYChanged: colorItem.updateDragPos()
                }

                function scrollAndUpdateDragPos() {
                    if (y < grid.pageHeaderHeight) {
                        if (grid.contentY > grid.originY) {
                            scrollAnimation.to = grid.originY
                            if (!scrollAnimation.running) {
                                scrollAnimation.duration = Math.max(scrollAnimation.minDuration,
                                    Math.min(scrollAnimation.maxDuration,
                                    (grid.contentY - grid.originY)*1000/grid.height))
                                scrollAnimation.start()
                            }
                        }
                    } else if ((y + height) > grid.height) {
                        var maxContentY = grid.contentHeight - grid.height - grid.pageHeaderHeight
                        if (grid.contentY < maxContentY) {
                            scrollAnimation.to = maxContentY
                            if (!scrollAnimation.running) {
                                scrollAnimation.duration = Math.max(scrollAnimation.minDuration,
                                    Math.min(scrollAnimation.maxDuration,
                                    (maxContentY - grid.contentY)*1000/grid.height))
                                scrollAnimation.start()
                            }
                        }
                    } else {
                        scrollAnimation.stop()
                    }
                    updateDragPos()
                }

                function updateDragPos() {
                    var row = Math.floor((grid.contentY + y + height/2)/grid.cellHeight)
                    colorModel.dragPos = grid.cellsPerRow * row + Math.floor((grid.contentX + x + width/2)/grid.cellWidth)
                }

                Rectangle {
                    border {
                        width: Theme.paddingSmall
                        color: Theme.highlightBackgroundColor
                    }
                    anchors.fill: parent
                    color: "transparent"
                    visible: model.color === thisDialog.color &&
                        model.index === colorModel.indexOf(thisDialog.color)
                }
            }

            Rectangle {
                anchors.fill: parent
                visible: model.itemType === itemTypeAdd
                color: colorDelegate.highlighted ? highlightColor : "transparent"
                readonly property color highlightColor: Theme.rgba(Theme.highlightBackgroundColor, Theme.highlightBackgroundOpacity)

                Image {
                    anchors.centerIn: parent
                    source: parent.visible ? ("image://theme/icon-m-add?" + (colorDelegate.highlighted ? Theme.highlightColor : Theme.primaryColor)) : ""
                    sourceSize: Qt.size(Theme.iconSizeExtraLarge, Theme.iconSizeExtraLarge)
                }
            }

            onClicked: {
                if (model.itemType === itemTypeColor) {
                    thisDialog.color = model.color
                    thisDialog.forwardNavigation = true
                    thisDialog.accept()
                } else {
                    var editor = pageStack.push(Qt.resolvedUrl("HarbourColorEditorDialog.qml"), {
                        allowedOrientations: thisDialog.allowedOrientations,
                        initialColor: thisDialog.color,
                        acceptText: thisDialog.addColorAcceptText,
                        hueSliderText: thisDialog.addColorHueSliderText,
                        brightnessSliderText: thisDialog.addColorBrightnessSliderText,
                        hexNotationText: thisDialog.addColorHexNotationText
                    })
                    editor.accepted.connect(function() {
                        colorModel.addColor(editor.selectedColor)
                    })
                }
            }

            onPressAndHold: {
                if (model.itemType === itemTypeColor) {
                    grid.dragItem = colorDelegate
                }
            }
            onReleased: stopDrag()
            onCanceled: stopDrag()

            onDraggingChanged: {
                if (dragging) {
                    colorModel.dragPos = index
                }
            }

            function stopDrag() {
                if (grid.dragItem === colorDelegate) {
                    var returnPoint = mapToItem(grid, 0, 0)
                    colorItem.returnX = returnPoint.x
                    colorItem.returnY = returnPoint.y
                    colorItem.snappingBack = (colorItem.returnX !== colorItem.x || colorItem.returnY !== colorItem.y)
                    grid.dragItem = null
                }
            }
        }

        SmoothedAnimation {
            id: scrollAnimation
            target: grid
            duration: minDuration
            properties: "contentY"

            readonly property int minDuration: 250
            readonly property int maxDuration: 5000
        }

        addDisplaced: Transition { SmoothedAnimation { properties: "x,y"; duration: 150 } }
        moveDisplaced: Transition { SmoothedAnimation { properties: "x,y"; duration: 150 } }
        removeDisplaced: Transition { SmoothedAnimation { properties: "x,y"; duration: 150 } }
    }
}
