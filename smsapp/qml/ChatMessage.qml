/**
 * SPDX-FileCopyrightText: 2020 Nicolas Fella <nicolas.fella@gmx.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as KirigamiComponents
import org.kde.kdeconnect.sms

Item {
    id: root

    height: messageColumn.implicitHeight

    property string messageBody
    property bool sentByMe
    property string selectedText
    property date dateTime
    property string name
    property bool multiTarget
    property var attachmentList: []
    property bool firstInGroup: true
    property bool lastInGroup: true
    property bool firstOfDay: false
    property int messageType: 0

    // ConversationMessage::Types
    readonly property bool sending: messageType === 4 || messageType === 6 // Outbox, Queued
    readonly property bool failed: messageType === 5
    readonly property string verificationCode: sentByMe ? "" : SmsHelper.findVerificationCode(messageBody)

    // Received messages in group chats have the sender's avatar next to them
    readonly property bool hasAvatar: !sentByMe && multiTarget
    readonly property real bubbleRadius: Kirigami.Units.cornerRadius * 3
    readonly property real groupedCornerRadius: Kirigami.Units.cornerRadius
    readonly property color bubbleTextColor: sentByMe ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.textColor

    signal messageCopyRequested(string message)
    signal retryRequested()

    function linkify(text, linkColor) {
        let s = text.replace(/[&<>]/g, function(c) {
            return {"&": "&amp;", "<": "&lt;", ">": "&gt;"}[c];
        });

        // Match http(s):// and www. URLs, and bare domains like "bit.ly/abc" that SMS often use.
        // Allow () in paths but exclude trailing punctuation.
        const urlEnd = "[^\\s<>]*[^\\s<>.,;:!?\\)\\]\"']";
        const domain = "(?:[a-z0-9-]+\\.)+(?:com|org|net|edu|gov|io|co|ly|me|gl|to|in|us|uk|ca|au|de|fr|app|dev|ai|gg|tv|info|biz|link|page|site|xyz)";
        const urlRegex = new RegExp("(^|[^\\w@./-])((?:https?:\\/\\/|www\\.)" + urlEnd + "|" + domain + "(?:\\/" + urlEnd + "|\\/)?(?![\\w@-]))", "gi");
        s = s.replace(urlRegex, function(match, before, url) {
            const href = /^https?:\/\//i.test(url) ? url : "https://" + url;
            const size = Kirigami.Units.iconSizes.small;
            return before + '<a href="' + href + '" style="color: ' + linkColor + '">' + url + '</a>'
                + '&nbsp;<a href="copy:' + href + '"><img src="image://icon/edit-copy-symbolic" width="' + size + '" height="' + size + '" align="middle"></a>';
        });

        return s.replace(/\n/g, "<br>");
    }

    Kirigami.Theme.colorSet: Kirigami.Theme.View
    Kirigami.Theme.inherit: false

    ColumnLayout {
        id: messageColumn
        width: parent.width
        spacing: 0

        Label {
            id: daySeparator
            visible: root.firstOfDay
            text: SmsHelper.formatDaySeparator(root.dateTime.getTime())
            font: Kirigami.Theme.smallFont
            color: Kirigami.Theme.disabledTextColor
            topPadding: Kirigami.Units.largeSpacing * 2
            bottomPadding: Kirigami.Units.smallSpacing
            Layout.alignment: Qt.AlignHCenter
        }

        Label {
            id: authorLabel
            visible: root.hasAvatar && root.firstInGroup
            text: root.name
            font: Kirigami.Theme.smallFont
            color: Kirigami.Theme.disabledTextColor
            elide: Text.ElideRight
            topPadding: Kirigami.Units.largeSpacing
            bottomPadding: Kirigami.Units.smallSpacing / 2
            Layout.leftMargin: avatarSlot.width + Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing + root.bubbleRadius / 2
            Layout.fillWidth: true
        }

        // Positioned with anchors rather than a layout: the height of the bubble depends on its
        // width, because the text wraps, which a layout would keep rearranging
        Item {
            id: bubbleRow
            implicitHeight: Math.max(messageBox.height, avatarSlot.visible ? avatarSlot.height : 0)
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.largeSpacing
            Layout.rightMargin: Kirigami.Units.largeSpacing
            // Messages in a group are close together, groups are further apart
            Layout.topMargin: root.firstInGroup && !authorLabel.visible ? Kirigami.Units.largeSpacing : Math.round(Kirigami.Units.smallSpacing / 2)

            Item {
                id: avatarSlot
                visible: root.hasAvatar
                width: Kirigami.Units.gridUnit * 1.5
                height: width
                anchors.left: parent.left
                anchors.bottom: parent.bottom

                KirigamiComponents.Avatar {
                    anchors.fill: parent
                    visible: root.lastInGroup
                    name: root.name
                }
            }

            Kirigami.ShadowedRectangle {
                id: messageBox

                readonly property real maximumWidth: bubbleRow.width * (applicationWindow().wideScreen ? 0.7 : 0.85) - (avatarSlot.visible ? avatarSlot.width : 0)
                readonly property real sideTopRadius: root.firstInGroup ? root.bubbleRadius : root.groupedCornerRadius

                width: Math.min(Math.max(naturalText.implicitWidth + messageLabel.leftPadding + messageLabel.rightPadding, attachmentGrid.implicitWidth), maximumWidth)

                // The width of the text without wrapping. Measured separately, since the wrapped
                // label's implicit width depends on the width of the bubble.
                Text {
                    id: naturalText
                    visible: false
                    text: messageLabel.text
                    textFormat: Text.RichText
                    font: messageLabel.font
                }
                height: bubbleContent.implicitHeight
                anchors.top: parent.top
                anchors.right: root.sentByMe ? parent.right : undefined
                anchors.left: root.sentByMe ? undefined : (avatarSlot.visible ? avatarSlot.right : parent.left)
                anchors.leftMargin: avatarSlot.visible ? Kirigami.Units.smallSpacing : 0

                color: root.sentByMe ? Kirigami.Theme.highlightColor
                                     : Kirigami.ColorUtils.tintWithAlpha(Kirigami.Theme.backgroundColor, Kirigami.Theme.textColor, 0.1)

                // The corners next to the other messages of the group, and the bottom corner on the
                // sender's side of each bubble, which points at the sender, are less rounded
                corners.topLeftRadius: root.sentByMe ? root.bubbleRadius : sideTopRadius
                corners.topRightRadius: root.sentByMe ? sideTopRadius : root.bubbleRadius
                corners.bottomLeftRadius: root.sentByMe ? root.bubbleRadius : root.groupedCornerRadius
                corners.bottomRightRadius: root.sentByMe ? root.groupedCornerRadius : root.bubbleRadius

                HoverHandler {
                    id: bubbleHover
                }

                ToolTip.visible: bubbleHover.hovered
                ToolTip.delay: Kirigami.Units.toolTipDelay
                ToolTip.text: root.dateTime.toLocaleString(Qt.locale(), Locale.LongFormat)

                Column {
                    id: bubbleContent
                    width: parent.width

                    Grid {
                        id: attachmentGrid
                        columns: 2
                        padding: attachmentList.length > 0 ? Kirigami.Units.smallSpacing : 0
                        layoutDirection: root.sentByMe ? Qt.RightToLeft : Qt.LeftToRight

                        Repeater {
                            model: attachmentList

                            delegate: MessageAttachments {
                                mimeType: modelData.mimeType
                                partID: modelData.partID
                                uniqueIdentifier: modelData.uniqueIdentifier
                            }
                        }
                    }

                    Kirigami.SelectableLabel {
                        id: messageLabel
                        visible: messageBody != ""
                        width: parent.width
                        leftPadding: Kirigami.Units.largeSpacing
                        rightPadding: Kirigami.Units.largeSpacing
                        topPadding: Kirigami.Units.mediumSpacing
                        bottomPadding: Kirigami.Units.mediumSpacing
                        wrapMode: Text.Wrap
                        textFormat: Text.RichText
                        color: root.bubbleTextColor
                        selectionColor: root.sentByMe ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.highlightColor
                        selectedTextColor: root.sentByMe ? Kirigami.Theme.highlightColor : Kirigami.Theme.highlightedTextColor
                        text: root.linkify(root.messageBody, root.sentByMe ? root.bubbleTextColor : Kirigami.Theme.linkColor)
                        onLinkActivated: (link) => {
                            if (link.startsWith("copy:")) {
                                SmsHelper.copyToClipboard(link.substring(5));
                                applicationWindow().showPassiveNotification(i18nd("kdeconnect-sms", "Link copied"));
                            } else {
                                Qt.openUrlExternally(link);
                            }
                        }
                    }
                }
            }

        }

        Button {
            id: copyCodeButton
            visible: root.verificationCode.length > 0
            text: i18ndc("kdeconnect-sms", "@action:button %1 is a verification code", "Copy %1", root.verificationCode)
            icon.name: "edit-copy"
            flat: false
            font: Kirigami.Theme.smallFont
            Layout.alignment: Qt.AlignLeft
            Layout.topMargin: Kirigami.Units.smallSpacing
            Layout.leftMargin: Kirigami.Units.largeSpacing + (root.hasAvatar ? avatarSlot.width + Kirigami.Units.smallSpacing : 0)
            onClicked: {
                SmsHelper.copyVerificationCode(root.verificationCode);
                applicationWindow().showPassiveNotification(i18ndc("kdeconnect-sms", "@info %1 is a verification code", "Copied %1", root.verificationCode));
            }
        }

        RowLayout {
            id: statusRow
            visible: root.lastInGroup || root.sending || root.failed
            spacing: Kirigami.Units.smallSpacing
            layoutDirection: root.sentByMe ? Qt.RightToLeft : Qt.LeftToRight
            Layout.alignment: root.sentByMe ? Qt.AlignRight : Qt.AlignLeft
            Layout.topMargin: Kirigami.Units.smallSpacing / 2
            Layout.leftMargin: Kirigami.Units.largeSpacing + (root.hasAvatar ? avatarSlot.width + Kirigami.Units.smallSpacing : 0) + root.bubbleRadius / 2
            Layout.rightMargin: Kirigami.Units.largeSpacing + root.bubbleRadius / 2

            Kirigami.Icon {
                visible: root.sending || root.failed
                source: root.failed ? "dialog-error-symbolic" : "chronometer-symbolic"
                color: root.failed ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.disabledTextColor
                isMask: true
                Layout.preferredWidth: Kirigami.Units.iconSizes.small
                Layout.preferredHeight: Kirigami.Units.iconSizes.small
            }

            Label {
                id: timeLabel
                text: root.failed ? i18nd("kdeconnect-sms", "Not sent")
                      : root.sending ? i18nd("kdeconnect-sms", "Sending…")
                      : root.dateTime.toLocaleTimeString(Qt.locale(), Locale.ShortFormat)
                font: Kirigami.Theme.smallFont
                color: root.failed ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.disabledTextColor
            }

            Kirigami.LinkButton {
                visible: root.failed
                text: i18nd("kdeconnect-sms", "Retry")
                font: Kirigami.Theme.smallFont
                onClicked: root.retryRequested()
            }
        }
    }
}
