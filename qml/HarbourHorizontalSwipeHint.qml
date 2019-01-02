import QtQuick 2.2
import Sailfish.Silica 1.0

Item {
    id: hint

    anchors.fill: parent

    property bool swipeRight
    property bool hintEnabled
    property alias text: label.text
    property alias hintDelay: hintDelayTimer.interval
    readonly property bool hintRunning: hintDelayTimer.running || touchInteractionHint.running

    signal hintShown()

    function showHint() {
        touchInteractionHint.start()
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
        Behavior on opacity { FadeAnimator { duration: 1000 } }
    }

    TouchInteractionHint {
        id: touchInteractionHint

        direction: swipeRight ? TouchInteraction.Right : TouchInteraction.Left
        anchors.verticalCenter: parent.verticalCenter
        onRunningChanged: if (!running) hint.hintShown()
    }

    Timer {
        id: hintDelayTimer

        interval: 1000
        onTriggered: showHint()
    }
}
