import QtQuick 2.0
import Sailfish.Silica 1.0
import Sailfish.TextLinking 1.0

LinkedText {
    property string key
    property string value
    property color keyColor: highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor

    width: parent.width
    color: highlighted ? Theme.highlightColor : Theme.primaryColor
    linkColor: color
    textFormat: Text.StyledText
    wrapMode: Text.Wrap
    font.pixelSize: Theme.fontSizeSmall
    text: "<font color=\"" + keyColor + "\">" + key + "</font> " + value.replace(/\n/g, '<br>')
}
