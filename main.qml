import QtQuick 2.9
import QtQuick.Controls 2.3
import QtQuick.Window 2.9
import QtQuick.Controls.Material 2.0
import Qt.labs.settings 1.0
import TgAnalizer 1.0

Window {
    id: win
    visible: true
    width: 1700
    height: 900
    title: qsTr("Analizer")

    Page {
        anchors.fill: parent
    }

    Settings {
        id: settings
        property string lastJsonPath
        property string lastLearnPath: "/home/bardia/Train/Train/"
    }

    DataFetcher {
        id: fetcher
        mergables: {
            if (!mixSwitch.checked)
                return new Array

            return [
                {
                    title: "Friends",
                    list: ["Casual rel", "Close rel"]
                }
            ]
        }
    }

    ScrollView {
        id: scrollView
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: panelColumn.right
        anchors.right: parent.right
        anchors.leftMargin: 10

        ScrollBar.vertical.policy: ScrollBar.AlwaysOff

        Flickable {
            id: flick
            contentWidth: rowScene.width
            contentHeight: rowScene.height
            clip: true

            Item {
                id: rowScene
                height: flick.height
                width: Math.max(flick.width, row.width + 100)

                Row {
                    id: row
                    height: flick.height
                    visible: !indicator.running
                    anchors.centerIn: parent

                    Repeater {
                        id: mainRepeater
                        model: fetcher.byProperties

                        Item {
                            id: item
                            height: parent.height
                            width: 80

                            property variant list: modelData.list
                            property real maximum: modelData.maximum
                            property real minimum: modelData.minimum
                            property string property: modelData.property

                            Label {
                                id: label
                                anchors.horizontalCenter: parent.horizontalCenter
                                anchors.bottom: parent.bottom
                                anchors.bottomMargin: index%2==0? 20 : 0
                                text: modelData.property
                                font.pointSize: 9
                            }

                            Column {
                                id: titlesLabel
                                width: parent.width
                                anchors.top: parent.top
                                clip: true

                                Label { text: "Max: %1".arg(item.maximum); anchors.horizontalCenter: parent.horizontalCenter; font.pointSize: 9 }
                                Label { text: "Min: %1".arg(item.minimum); anchors.horizontalCenter: parent.horizontalCenter; font.pointSize: 9 }
                            }

                            Rectangle {
                                id: rect
                                width: 1
                                anchors.horizontalCenter: parent.horizontalCenter
                                anchors.bottom: label.top
                                anchors.top: parent.bottom
                                anchors.topMargin: -80
                                color: index%2==0? "#f00" : "#18f"
                            }

                            Rectangle {
                                width: 1
                                color: "#888"
                                opacity: 0.5
                                height: parent.height
                            }

                            Item {
                                id: scene
                                width: parent.width
                                anchors.top: titlesLabel.bottom
                                anchors.bottom: rect.top
                                clip: true

                                Repeater {
                                    model: item.list

                                    Rectangle {
                                        x: modelData.labelIndex * parent.width/propertyLabelsRepeater.count
                                        color: modelData.color
                                        width: parent.width / propertyLabelsRepeater.count
                                        height: 1
                                        y: parent.height - ((modelData.value - item.minimum) * (parent.height - 1) / (item.maximum - item.minimum)) - height/2
                                    }
                                }

                                Repeater {
                                    model: {
                                        var res = new Array
                                        try {
                                            res = fetcher.checkedMap[item.property]
                                        } catch (e) {
                                        }
                                        if (res == undefined)
                                            res = new Array
                                        return res
                                    }

                                    Rectangle {
                                        color: "#18f"
                                        width: parent.width
                                        height: 3
                                        y: parent.height - ((modelData.value - item.minimum) * (parent.height - 1) / (item.maximum - item.minimum)) - height/2
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    BusyIndicator {
        id: indicator
        anchors.centerIn: parent
        running: false
    }

    Column {
        id: panelColumn
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 10
        spacing: 2
        visible: mainRepeater.count != 0

        Repeater {
            id: propertyLabelsRepeater
            model: fetcher.labels

            Row {
                spacing: 6

                Rectangle {
                    color: modelData.color
                    width: 20
                    height: 4
                    anchors.verticalCenter: parent.verticalCenter
                }

                Label {
                    text: modelData.label
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }

        Item { width: 1; height: 20 }

        SwitchDelegate {
            id: mixSwitch
            text: qsTr("Friends")
            checked: true
        }

        Item { width: 1; height: 20 }

        Button {
            text: "Learn Again"
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: learn()
        }

        Button {
            text: "Calculate"
            highlighted: true
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: open()
        }
    }

    Column {
        anchors.centerIn: parent
        visible: mainRepeater.count == 0 && !indicator.running
        spacing: 10

        Label {
            text: "To Learn the machine, Click below button and select data folder"
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Button {
            text: "Click To Learn"
            anchors.horizontalCenter: parent.horizontalCenter
            highlighted: true
            onClicked: learn()
        }
    }

    Dialog {
        id: resultDialog
        x: parent.width/2 - width/2
        y: parent.height/2 - height/2
        dim: true
        modal: true
        standardButtons: Dialog.Ok

        Column {
            spacing: 2
            Label {
                id: resultLabel
                font.pointSize: 26
                font.bold: true
            }

            Item { width: 1; height: 8 }

            Label {
                font.pointSize: 10
                font.bold: true
                text: "Possibilities:"
                Material.foreground: Material.accent
            }

            Label {
                id: percentsLabel
                font.family: "monospace"
                font.pointSize: 10
            }

            Item { width: 1; height: 8 }

            Label {
                font.pointSize: 10
                font.bold: true
                text: "Full Details:"
                Material.foreground: Material.accent
            }

            Label {
                id: stringLabel
                font.family: "monospace"
                font.pointSize: 10
            }
        }
    }

    function learn() {
        var source = Tools.getExistingDirectory(win, "Select Path to Learn", settings.lastLearnPath)
        if (source.trim().legnth === "")
            return

        settings.lastLearnPath = source

        indicator.running = true
        Tools.jsDelayCall(100, function(){
            fetcher.source = source
            indicator.running = false
        })
    }

    function open() {
        var file = Tools.getOpenFileName(win, "Select JSON", ["*.json"], settings.lastJsonPath)
        if (file.trim() === "")
            return

        settings.lastJsonPath = Tools.fileParent(file)

        var res = fetcher.check(file)

        resultLabel.text = res.result
        stringLabel.text = res.string
        percentsLabel.text = res.percents
        win.title = qsTr("Analizer: %1").arg(Tools.fileName(file))
        resultDialog.open()
    }
}
