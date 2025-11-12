import QtQuick 2.0
import Sailfish.Silica 1.0

Item {
    id: detailItem
    width: parent.width
    height: Math.max(labelText.height, valueText.height)

    property alias label: labelText.text
    property alias value: valueText.text
    property real leftMargin: Theme.horizontalPageMargin
    property real rightMargin: Theme.horizontalPageMargin
    property real midlineRatio: 0.4
    property real midlineMin: Theme.fontSizeSmall * 5
    property real midlineMax: Theme.fontSizeSmall * 10
    property real midline: Math.min(Math.max((width * midlineRatio), midlineMin), midlineMax)
    property int pixelSize: Theme.fontSizeSmall

    Label {
        id: labelText

        x: detailItem.leftMargin
        y: Theme.paddingSmall
        width: midline - detailItem.leftMargin - Theme.paddingSmall
        horizontalAlignment: Text.AlignLeft
        color: Theme.secondaryColor
        font.pixelSize: pixelSize
        wrapMode: Text.Wrap
    }

    Label {
        id: valueText

        x: midline
        y: Theme.paddingSmall
        width: parent.width - midline - detailItem.rightMargin
        horizontalAlignment: Text.AlignLeft
        color: Theme.primaryColor
        font.pixelSize: pixelSize
  }
}
