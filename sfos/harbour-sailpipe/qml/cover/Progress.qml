import QtQuick 2.0
import Sailfish.Silica 1.0

Item {
    id: root
    property alias running: progressShader.running
    property alias progress: progressShader.progress

    Timer {
        interval: 32
        repeat: true
        running: root.running
        onTriggered: progressShader.count = (progressShader.count + 1) % 100
    }

    ShaderEffect {
        id: progressShader

        anchors.fill: parent

        property bool running: false
        property real progress: 0.0
        property int count: 0
        property color colour: Theme.rgba(Theme.highlightColor, 0.75)
        property size size: Qt.size(width, height)
        property size p1: Qt.size(20.0, 20.0)
        property size p2: Qt.size((progress * (width - 40.0)) + 20.0, height - 20.0)
        property real corner: 20.0
        property real radius: height / 2.2;

        vertexShader: "
            uniform highp mat4 qt_Matrix;
            attribute highp vec4 qt_Vertex;
            attribute highp vec2 qt_MultiTexCoord0;
            varying highp vec2 pos;

            void main() {
                gl_Position = qt_Matrix * qt_Vertex;
                pos = qt_Vertex.xy;
            }"

        fragmentShader: "
            varying highp vec2 pos;
            uniform lowp float qt_Opacity;
            uniform lowp float progress;
            uniform lowp int count;
            uniform lowp vec4 colour;
            uniform lowp vec2 size;
            uniform lowp vec2 p1;
            uniform lowp vec2 p2;
            uniform lowp float corner;
            uniform lowp float radius;
            const float minpaint = 0.4;
            const float maxpaint = 0.6;

            void main() {
                lowp float x = pos.x;
                lowp float y = pos.y;
                lowp float paint;
                lowp float centre;
                lowp float effectiveRadius;

                lowp float dx = (x < p1.x) ? (p1.x - x) : ((x < p2.x) ? 0.0 : (x - p2.x));
                lowp float dy = (y < p1.y) ? (p1.y - y) : ((y < p2.y) ? 0.0 : (y - p2.y));
                lowp float d = sqrt(pow(dx, 2.0) + pow(dy, 2.0));
                lowp float z = max(corner - d, 0.0);
                paint = z / corner;

                for (centre = (float(count) / 100.0) * (size.x / 4.0); centre < size.x + radius; centre += size.x / 4.0) {
                    effectiveRadius = min(radius, max(radius + (0.9 * size.x * ((centre / size.x) - (progress + (corner / (size.x * 0.6))))), 0.0));
                    effectiveRadius *= min(corner * ((centre - 1.3 * corner) / size.x), 1.0);
                    effectiveRadius *= min(corner * ((size.x - centre - corner) / size.x), 1.0);
                    d = sqrt(pow(centre - x, 2.0) + pow((size.y / 2.0) - y, 2.0));
                    z = max(effectiveRadius - d, 0.0) * effectiveRadius / corner;
                    paint += z / effectiveRadius;
                }

                paint = ((paint > minpaint) && (paint < maxpaint)) ? 1.0 : 0.0;

                gl_FragColor = paint * qt_Opacity * colour;
                return;
            }"
    }
}
