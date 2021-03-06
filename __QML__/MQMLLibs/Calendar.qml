import QtQuick 2.1
import QtQuick.Controls 1.0
import QtQuick.Controls.Styles 1.0
import QtGraphicalEffects 1.0
import com.mihail.qmlcomponents 1.0

Item {
    id: calendar

    property int contentWidth: ( listView.currentItem ?
                                    listView.currentItem.width+(rect_ContentBackground.radius*2) : 0 )
    property int contentHeight: ( listView.currentItem ?
                                     headerContainer.height+listView.currentItem.height+
                                     (rect_ContentBackground.radius*2) : 0 )

    readonly property Component headerDelegate: component_Header
    readonly property Component monthDelegate: component_Month

    property bool fontBold
    property int fontPixelSize
    property string fontFamily

    readonly property int visibleYear: dataContainer.dateModel.get( listView.currentIndex ).yearValue
    readonly property int visibleMonth: dataContainer.dateModel.get( listView.currentIndex ).monthValue

    property date minimumDate: "0001-01-01"
    property date currentDate: minimumDate
    onCurrentDateChanged: {
        if ( currentDate < minimumDate ) currentDate = minimumDate
    }

    signal clicked

    property bool changeDateOnHover: false


    Component {
        id: component_Header

        Item {
            id: headerItem

            Rectangle {
                anchors.fill: parent

                color: "#99000000"
            }

            ListView {
                id: list_Months
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.bottom: parent.bottom
                width: parent.width/2

                clip: true
                interactive: false

                property int currentMonth: headerItem.parent.visibleMonth
                onCurrentMonthChanged: {
                    if ( model.count === 0 )
                        for ( var month = 1; month < 13; month++ )
                            model.append( { "month": month } )
                    currentIndex = currentMonth-1
                }

                model: ListModel {}
                delegate: Item {
                    width: list_Months.width
                    height: list_Months.height

                    Text {
                        anchors.fill: parent

                        MDate {
                            id: dateInfo
                        }

                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter

                        text: dateInfo.longMonthName( month )
                        color: "white"

                        font.pixelSize: 13
                        font.bold: true
                    }
                }

                MouseArea {
                    anchors.fill: parent

                    onWheel: {
                        if ( wheel.angleDelta.y > 0 )
                            headerItem.parent.prevMonth()
                        else
                            headerItem.parent.nextMonth()
                    }
                }
            }

            SpinBox {
                anchors.top: parent.top
                anchors.left: list_Months.right
                anchors.bottom: parent.bottom
                anchors.right: rect_TodayButton.left

                font.pixelSize: 15

                minimumValue: 1
                maximumValue: 5000

                value: headerItem.parent.visibleYear
                onValueChanged: headerItem.parent.setYear( value )

                style: Rectangle{
                    property QtObject padding: QtObject {}
                    property Component panel: TextInput {
                        property int horizontalTextAlignment: Text.AlignHCenter
                        property int verticalTextAlignment: Text.AlignVCenter
                        property color foregroundColor: "white"
                        property color selectionColor: "#55ffffff"
                        property color selectedTextColor: "white"
                    }
                }
            }

            Rectangle {
                id: rect_TodayButton
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.right: parent.right
                anchors.margins: 2
                width: height

                radius: 3

                color: ( mouse_TodayButton.containsMouse ? "#55000000" : "transparent" )

                Image {
                    anchors.fill: parent
                    anchors.margins: 2

                    source: "CalendarImages/today.png"
                }

                MouseArea {
                    id: mouse_TodayButton
                    anchors.fill: parent

                    hoverEnabled: true

                    onClicked: headerItem.parent.toCurrentDate()
                }
            }
        }
    }

    Component {
        id: component_Month

        Item {
            id: calendarItem
            width: parent.parent.width/*rect_LeftButton.width+item_GridContainer.width+rect_RightButton.width*/
            height: parent.parent.height/*item_GridContainer.height*/

            scale: ( calendarItem.ListView.isCurrentItem ? 1.0 : 0.0 )
            Behavior on scale {
                NumberAnimation { duration: 500 }
            }
            onScaleChanged: {
                if ( scale === 1.0 )
                {
                    onShow = true
                    onHide = false
                }
                else if ( scale === 0.0 && onShow )
                {
                    onShow = false
                    onHide = true
                    month = 0
                }
                else if ( onHide )
                    month = monthValue
            }

            enabled: scale === 1.0

            property bool onShow: false
            property bool onHide: false

            property int year: yearValue
            property int month: monthValue

            MDate { id: dateInfo }

            property date itemDate
            onMonthChanged: {
                grid_Content.model.clear()
                if ( monthValue < 1 || monthValue > 12 ) return
                var selectIndex = -1
                var date = dateInfo.date( year, month, 1 )
                var date2 = dateInfo.addDays( date, -(dateInfo.dayOfWeak( date )) )
                for ( var day = 1; day<dateInfo.dayOfWeak( date ); day++ )
                {
                    date2 = dateInfo.addDays( date2, 1 )
                    if ( calendarItem.parent.parent.currentDate.toString() === date2.toString() )
                        selectIndex = grid_Content.model.count
                    grid_Content.model.append( { "dateValue": date2 } )
                }

                date = dateInfo.date( year, month, 1 )
                date = dateInfo.addDays( date, -1 )
                for ( var day = 0; day < dateInfo.daysInMonth( year, month ); day++ )
                {
                    date = dateInfo.addDays( date, 1 )
                    if ( calendarItem.parent.parent.currentDate.toString() === date.toString() )
                        selectIndex = grid_Content.model.count
                    grid_Content.model.append( { "dateValue": date } )
                }

                while ( grid_Content.model.count < 7*6 )
                {
                    date = dateInfo.addDays( date, 1 )
                    if ( calendarItem.parent.parent.currentDate.toString() === date.toString() )
                        selectIndex = grid_Content.model.count
                    grid_Content.model.append( { "dateValue": date } )
                }

                grid_Content.currentIndex = selectIndex
            }

            Rectangle {
                id: rect_LeftButton
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.bottom: parent.bottom
                width: 12

                color: ( mouse_LeftButton.containsMouse ? "#ccff8833" : "#99ff8833" )
                Behavior on color {
                    ColorAnimation { duration: 120 }
                }

                MouseArea {
                    id: mouse_LeftButton
                    anchors.fill: parent

                    hoverEnabled: true

                    onClicked: calendarItem.parent.parent.prevMonth()
                }
            }
            Rectangle {
                id: rect_RightButton
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                width: 12

                color: ( mouse_RightButton.containsMouse ? "#ccff8833" : "#99ff8833" )
                Behavior on color {
                    ColorAnimation { duration: 120 }
                }

                MouseArea {
                    id: mouse_RightButton
                    anchors.fill: parent

                    hoverEnabled: true

                    onClicked: calendarItem.parent.parent.nextMonth()
                }
            }
            Item {
                id: item_GridContainer
                anchors.fill: parent
                anchors.leftMargin: rect_LeftButton.width
                anchors.rightMargin: rect_RightButton.width

                GridView {
                    id: grid_Content
                    anchors.fill: parent
                    anchors.margins: 1
                    anchors.leftMargin: 3
                    anchors.rightMargin: 3

                    cellWidth: width/7
                    cellHeight: height/6

                    interactive: false

                    model: ListModel{}
                    delegate: Item {
                        id: item_Day
                        width: grid_Content.cellWidth
                        height: grid_Content.cellHeight

                        property date currentDate: calendarItem.parent.parent.currentDate
                        onCurrentDateChanged: {
                            if ( currentDate.toString() === itemDate.toString() )
                                grid_Content.currentIndex = model.index
                        }
                        property date itemDate: dateValue
                        onItemDateChanged: {
                            if ( currentDate.toString() === itemDate.toString() )
                                grid_Content.currentIndex = model.index
                        }

                        Rectangle {
                            anchors.fill: parent
                            anchors.margins: 1

                            radius: 4

                            color: ( item_Day.itemDate.getMonth()+1 !== calendarItem.month ?
                                        "#77000000" : ( mouse_Day.containsMouse ?
                                                           "#bbffffff" : "#77ffffff" ) )
                            Behavior on color {
                                ColorAnimation { duration: 120 }
                            }

                            scale: ( mouse_Day.containsMouse ? 1.2 : 1.0 )
                            Behavior on scale {
                                NumberAnimation { duration: 120 }
                            }

                            Text {
                                id: text_ItemDay
                                anchors.fill: parent

                                text: item_Day.itemDate.getDate()
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter

                                color: ( item_Day.itemDate.getMonth()+1 !== calendarItem.month ?
                                            "white" : "black" )
                                font.pixelSize: text_ItemDay.height*0.6
                                Behavior on color {
                                    ColorAnimation { duration: 120 }
                                }
                                scale: ( mouse_Day.containsMouse ? 1.3 : 1.0 )
                                Behavior on scale {
                                    NumberAnimation { duration: 120 }
                                }
                            }

                            MouseArea {
                                id: mouse_Day
                                anchors.fill: parent

                                hoverEnabled: true

                                onClicked: {
                                    calendarItem.parent.parent.setCurrentDate( item_Day.itemDate )
                                    calendarItem.parent.parent.clicked()
                                }
                                onContainsMouseChanged: {
                                    if ( calendarItem.parent.parent.changeDateOnHover &&
                                            item_Day.itemDate.getMonth()+1 === calendarItem.month )
                                        calendarItem.parent.parent.setCurrentDate( item_Day.itemDate )
                                }
                            }
                        }
                    }

                    highlight: Rectangle {
                        width: grid_Content.cellWidth; height: grid_Content.cellHeight
                        color: "white"; radius: 5
                        x: grid_Content.currentItem.x
                        y: grid_Content.currentItem.y
                        Behavior on x { SpringAnimation { spring: 3; damping: 0.2 } }
                        Behavior on y { SpringAnimation { spring: 3; damping: 0.2 } }
                    }
                }
            }
        }
    }

    QtObject {
        id: dataContainer

        property ListModel dateModel: ListModel {}
    }

    Image {
        id: backgroundImage
        anchors.fill: parent

        asynchronous: true
        clip: true

        source: "CalendarImages/month_"+visibleMonth+".jpg"
    }

    Rectangle {
        id: rect_ContentBackground
        anchors.fill: parent

        color: "#440077ff"
    }

    Item {
        id: headerContainer
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: rect_ContentBackground.radius

        height: ( parent.height > 35 ? 35 : parent.height )

        property int visibleYear: calendar.visibleYear
        property int visibleMonth: calendar.visibleMonth

        Loader {
            id: headerLoader
            sourceComponent: component_Header

            onLoaded: {
                item.parent = headerContainer
                item.anchors.fill = headerContainer
            }
        }

        function prevMonth() { listView.prevMonth() }
        function nextMonth() { listView.nextMonth() }
        function prevYear() { listView.prevYear() }
        function nextYear() { listView.prevYear() }
        function setYear( year ) { listView.setYear( year ) }
        function toCurrentDate() { listView.setCurrentMonth( calendar.currentDate.getFullYear(), calendar.currentDate.getMonth()+1 ) }
    }

    ListView {
        id: listView
        anchors.fill: rect_ContentBackground
        anchors.margins: rect_ContentBackground.radius
        anchors.topMargin: headerContainer.height+anchors.margins

        clip: true

        model: dataContainer.dateModel
        delegate: calendar.monthDelegate

        orientation: ListView.Horizontal
        snapMode: ListView.SnapOneItem

        interactive: false

        readonly property date currentDate: calendar.currentDate
        onCurrentDateChanged: {
            listView.setCurrentMonth( currentDate.getFullYear(), currentDate.getMonth()+1 )
        }

        readonly property bool changeDateOnHover: calendar.changeDateOnHover

        function clicked() {
            calendar.clicked()
        }

        function nextMonth() {
            var year = listView.model.get( listView.currentIndex ).yearValue
            var month = listView.model.get( listView.currentIndex ).monthValue
            if ( month === 12 )
            {
                year++
                month = 1
            }
            else month++
            setCurrentMonth( year, month )
        }

        function prevMonth() {
            var year = listView.model.get( listView.currentIndex ).yearValue
            var month = listView.model.get( listView.currentIndex ).monthValue
            if ( month === 1 )
            {
                year--
                month = 12
            }
            else month--
            setCurrentMonth( year, month )
        }

        function nextYear() {
            var year = listView.model.get( listView.currentIndex ).yearValue
            var month = listView.model.get( listView.currentIndex ).monthValue
            year++
            setCurrentMonth( year, month )
        }

        function prevYear() {
            var year = listView.model.get( listView.currentIndex ).yearValue
            var month = listView.model.get( listView.currentIndex ).monthValue
                year--
            setCurrentMonth( year, month )
        }

        function setYear( year ) {
            var month = listView.model.get( listView.currentIndex ).monthValue
            setCurrentMonth( year, month )
        }

        function setCurrentMonth( year, month ) {
            var oldIndex = listView.currentIndex
            var newIndex = oldIndex

            if ( year < calendar.minimumDate.getFullYear() ||
                    ( year === calendar.minimumDate.getFullYear() &&
                     month < calendar.minimumDate.getMonth()+1 ) )
                return;

            if ( newIndex === -1 )
            {
                newIndex = 0
                listView.model.append( { "yearValue": year, "monthValue": month } )
                listView.currentIndex = newIndex
            }
            else
            {
                var oldYear = listView.model.get( listView.currentIndex ).yearValue
                var oldMonth = listView.model.get( listView.currentIndex ).monthValue
                if ( year < oldYear || ( year === oldYear && month < oldMonth ) )
                {
                    if ( newIndex === 0 )
                    {
                        listView.model.insert( newIndex, { "yearValue": year, "monthValue": month } )
                        listView.currentIndex++
                        listView.positionViewAtIndex( 1, ListView.SnapPosition )
                    }
                    else
                    {
                        newIndex--
                        listView.model.setProperty( newIndex, "yearValue", year )
                        listView.model.setProperty( newIndex, "monthValue", month )
                    }

                    if ( listView.count > 2 )
                        listView.model.remove( listView.count-1 )
                    listView.currentIndex = newIndex
                }
                else if ( year > oldYear || ( year === oldYear && month > oldMonth ) )
                {
                    newIndex++
                    if ( newIndex === listView.count )
                    {
                        listView.model.append( { "yearValue": year, "monthValue": month } )
                        if ( listView.count > 2 )
                        {
                            listView.model.remove( 0 )
                            newIndex--
                        }
                    }
                    else
                    {
                        listView.model.setProperty( newIndex, "yearValue", year )
                        listView.model.setProperty( newIndex, "monthValue", month )
                    }
                    listView.currentIndex = newIndex
                }
            }
        }

        function setCurrentDate( newDate ) {
            calendar.currentDate = newDate
        }
    }

    Text {
        id: text_ClickedDate
        anchors.fill: parent

        opacity: 0.0

        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter

        text: calendar.currentDate.getDate()
        font.pixelSize: parent.height
        color: "white"

        ParallelAnimation {
            id: clickAnimation

            NumberAnimation { target: text_ClickedDate; property: "opacity"; from: 1.0; to: 0.0; duration: 200 }
            NumberAnimation { target: text_ClickedDate; property: "scale"; from: 0.5; to: 1.0; duration: 200 }
        }

        Connections {
            target: calendar

            onClicked: {
                clickAnimation.stop()
                clickAnimation.start()
            }
        }
    }

    DropShadow {
        anchors.fill: text_ClickedDate
        horizontalOffset: 3
        verticalOffset: 3
        spread: 0.5
        radius: 16.0
        samples: 16
        color: "black"
        opacity: text_ClickedDate.opacity
        scale: text_ClickedDate.scale
        source: text_ClickedDate
    }

    function toCurrentDate() {
        headerContainer.toCurrentDate()
    }
}
