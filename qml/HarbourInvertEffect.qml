import QtQuick 2.0

ShaderEffect {
    property variant source
    fragmentShader: "
        uniform sampler2D source;
        uniform lowp float qt_Opacity;
        varying highp vec2 qt_TexCoord0;
        void main(void) {
            highp vec4 pixelColor = texture2D(source, qt_TexCoord0);
            gl_FragColor = vec4(vec3(1,1,1) - pixelColor.rgb, pixelColor.a) * qt_Opacity;
        }"
}
