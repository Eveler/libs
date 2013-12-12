import QtQuick 2.1
import QtGraphicalEffects 1.0


Rectangle {
    id: menuButton

    property string text: ""
    readonly property bool pressed: mouseArea.pressed
    property bool checked: false
    property bool checkable: true
    readonly property bool hovered: mouseArea.containsMouse
    property string textEnabledColor: "black"
    property string textDisabledColor: "#ff555555"
    property string enabledColor: "#00ffffff"
    property string disabledColor: "transparent"
    property string checkedEnabledColor: "#55ff5511"
    property string checkedDisabledColor: "#55555555"
    property string hoveredEnabledColor: "#33000000"
    property string hoveredDisabledColor: "transparent"

    signal clicked

    color: ( checked ?
                ( enabled ? checkedEnabledColor : checkedDisabledColor ) :
                ( hovered ? ( enabled ? hoveredEnabledColor : hoveredDisabledColor ) :
                            ( enabled ? enabledColor : "transparent" ) ) )

    Behavior on color {
        ColorAnimation { duration: 200 }
    }

    Text {
        id: text_Label
        anchors.fill: parent
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter

        text: menuButton.text
        color: ( enabled ? menuButton.textEnabledColor :
                           menuButton.textDisabledColor )

        font.pixelSize: 15
    }
    DropShadow {
        anchors.fill: text_Label

        horizontalOffset: 3
        verticalOffset: 3
        radius: 4.0
        samples: 32
        color: "#80000000"
        source: text_Label
        visible: enabled
        smooth: false
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent

        hoverEnabled: true

        onClicked: {
            if ( menuButton.checkable )
                menuButton.checked = true
            else menuButton.clicked()
        }
    }

    function bottomAnchor()
    {
        if ( visible || anchors.top === undefined )
            return bottom
        else return anchors.top
    }
}
