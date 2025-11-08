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
        property color colour: Theme.rgba(Theme.primaryColor, 0.75)
        property Image pattern: Image { source: Theme._patternImage; fillMode: Image.Tile }
        property size backgroundScale: Qt.size(width / pattern.width, height / pattern.height)
        property size centerSize: Qt.size(0.5 * (width - height) / width, 0.1);

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
            uniform lowp vec4 colour;
            uniform lowp sampler2D pattern;
            uniform lowp vec2 backgroundScale;
            uniform lowp vec2 centerSize;

            const float PI = 3.14159265358979323846264;
            const float border = 0.3;
            const float sigma = 0.35;
            const float brightness = 0.5;
            const float dropoff = 0.1;
            const float pulse = 0.15f;

            void main() {
                lowp float width = 1.0 + dropoff;
                lowp float upper = (progress * width);
                lowp float lower = upper - dropoff;
                lowp float x = coord.x * width;
                lowp float paint;
                float cycle = pulse + (pulse * sin(2.0 * PI * (1.0 - (float(count) / 50.0f) + (5.0f * coord.x) + (0.0f * coord.y))));
                lowp vec2 patternCoord;
                lowp float patternValue;
                mediump vec2 normalize;
                mediump vec2 edge;
                mediump float disc;
                mediump float shi;

                if (x < lower) {
                    paint = 1.0 + cycle;
                }
                else {
                    if (x > upper) {
                        paint = cycle;
                    }
                    else {
                        paint = (0.5 * (cos((PI / 1.0) * ((x - lower) / dropoff)) + 1.0)) + cycle;
                    }
                }

                patternCoord = coord * backgroundScale;
                patternCoord = vec2(mod(patternCoord.x, 1.0), mod(patternCoord.y, 1.0));
                patternValue = texture2D(pattern, patternCoord).a;

                normalize = 1.0 - (2.0 * centerSize);
                edge = max(vec2(0.0), abs(coord - 0.5) - centerSize) / normalize;
                disc = 1.0 - smoothstep(0.0, 1.0, length(edge) / border);
                shi = min(1.0, exp2(-dot(edge, edge) / (sigma * sigma)));
                gl_FragColor = paint * qt_Opacity * mix(vec4(0.0) * (shi * (1.0 - disc)), colour, min(1.0, (brightness * shi * (1.0 + shi + 2.0 * disc) *  0.25 + shi * patternValue)));

                return;
            }"
    }
}
