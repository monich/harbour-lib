/*
 * Copyright (C) 2020-2023 Slava Monich <slava@monich.com>
 * Copyright (C) 2020 Jolla Ltd.
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
 * HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

import QtQuick 2.0
import Sailfish.Silica 1.0

// In-process sharing doesn't work since Sailfish OS 4.2.0 (or more
// specifically, since declarative-transferengine-qt5 package >= 0.4.0)
//
// This code can only be of interest to those who want to make their
// apps compatible with older releases of Sailfish OS.

SilicaListView {
    id: view

    property url source
    property string subject
    property string emailTo
    property string type

    width: parent.width
    height: Theme.itemSizeSmall * model.count

    property string addAccountText: "Add account"
    readonly property bool accountIconSupported: model.accountIconSupported
    property var content: []
    // model: HarbourTransferMethodsModel

    onTypeChanged: content.type = type

    Component.onCompleted: content.type = type

    delegate: BackgroundItem {
        id: backgroundItem
        width: view.width

        Image {
            id: icon

            x: Theme.horizontalPageMargin
            anchors.verticalCenter: parent.verticalCenter
            source: accountIcon ? accountIcon :
                view.accountIconSupported ? "image://theme/icon-m-share" : ""
            visible: view.accountIconSupported
        }

        Label {
            id: displayNameLabel

            text: displayName
            color: backgroundItem.highlighted ? Theme.highlightColor : Theme.primaryColor
            truncationMode: TruncationMode.Fade
            x: Theme.horizontalPageMargin
            anchors {
                left: icon.visible ? icon.right : parent.left
                leftMargin: icon.visible ? Theme.paddingMedium : Theme.horizontalPageMargin
                verticalCenter: parent.verticalCenter
            }
            width: Math.min(implicitWidth, parent.width - 2*Theme.horizontalPageMargin)
        }

        Label {
            text: userName
            font.pixelSize: Theme.fontSizeMedium
            color: backgroundItem.highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor
            truncationMode: TruncationMode.Fade
            anchors {
                left: displayNameLabel.right
                leftMargin: Theme.paddingSmall
                right: parent.right
                rightMargin: Theme.horizontalPageMargin
                verticalCenter: parent.verticalCenter
            }
            visible: text.length > 0
        }

        onClicked: {
            pageStack.push(model.shareUIPath, {
                source: view.source,
                content: content,
                methodId: model.methodId,
                displayName: model.displayName,
                accountId: model.accountId,
                accountName: model.userName,
                emailTo: view.emailTo,
                emailSubject: view.subject
            })
        }
    }

    footer: BackgroundItem {
        // Don't show the "Add account" item before the initial query completes
        enabled: model.valid && model.canShowAccounts && !model.showAccountsPending
        opacity: model.valid ? (enabled ? 1.0 : 0.4) : 0.0

        Image {
            id: addAccountIcon

            x: Theme.horizontalPageMargin
            anchors.verticalCenter: parent.verticalCenter
            source: view.accountIconSupported ?
                ("image://theme/icon-m-add" + (parent.highlighted ? "?" + Theme.highlightColor : "")) : ""
            visible: view.accountIconSupported
        }

        Label {
            text: addAccountText
            x: Theme.horizontalPageMargin
            anchors {
                left: addAccountIcon.visible ? addAccountIcon.right : parent.left
                leftMargin: addAccountIcon.visible ? Theme.paddingMedium : Theme.horizontalPageMargin
                verticalCenter: parent.verticalCenter
            }
            color: highlighted ? Theme.highlightColor : Theme.primaryColor
        }

        onClicked: model.showAccounts()
    }

    VerticalScrollDecorator { }
}
