import QtQuick 2.0
import Sailfish.Silica 1.0

Item {
    id: root
    property alias running: progress.running
    property alias progress: progress.progress

    Timer {
        interval: 100
        repeat: true
        running: root.running
        onTriggered: progress.count = (progress.count + 1) % 50
    }

    ShaderEffect {
        id: progress

        anchors.fill: parent

        property bool running: false
        property real progress: 0.5
        property int count: 0
        property color colour: Theme.rgba(Theme.highlightColor, 0.75)
        property Image pattern: Image { source: Theme._patternImage }

        vertexShader: "
            uniform highp mat4 qt_Matrix;
            attribute highp vec4 qt_Vertex;
            attribute highp vec2 qt_MultiTexCoord0;
            varying highp vec2 coord;
            void main() {
                coord = qt_MultiTexCoord0;
                gl_Position = qt_Matrix * qt_Vertex;
            }"
        fragmentShader: "
            varying highp vec2 coord;
            uniform lowp float qt_Opacity;
            uniform lowp float progress;
            uniform lowp int count;
            lowp float cycle;
            uniform lowp vec4 colour;
            uniform lowp sampler2D pattern;

            lowp float PI = 3.14159265358979323846264;
            lowp vec2 dash = vec2(0.1, 0.1);

            void main() {
                if (coord.x > progress) {
                    cycle = 0.5f + (0.5f * sin(2.0 * PI * (1.0 - (float(count) / 50.0f) + (5.0f * coord.x) + (0.3f * coord.y))));
                    gl_FragColor = cycle * colour * qt_Opacity;
                }
                else {
                    gl_FragColor = colour * qt_Opacity;
                }
                return;
            }"
    }
}
