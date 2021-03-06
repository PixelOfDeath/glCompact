#include "../Framework.hpp" //also includes glCompact
#include <string>

using namespace std;
using namespace glm;
using namespace glCompact;

class MyPipeline : public PipelineRasterization {
    public:
        using PipelineRasterization::PipelineRasterization;
        UniformSetter<vec4> rgba{this, "rgba"};
};

string vertexShader = R"""(
    #version 330
    layout(location = 0) in vec2 position;
    void main() {
        gl_Position = vec4(position.x, position.y, 0.0, 1.0);
    }
)""";

string fragmentShader = R"""(
    #version 330
    uniform vec4 rgba;
    out vec4 outputColor;
    void main() {
        outputColor = rgba;
    }
)""";

int main (int argc, char *argv[]) {
    //glMajor, glMinor, gles, resolutionX, resolutioY
    Framework framework(3, 3, false, 640, 480);

    AttributeLayout attributeLayout;
    attributeLayout.addBufferIndex();
    attributeLayout.addLocation(0, AttributeFormat::R32G32_SFLOAT);

    float attributeData[] = {
        -1.0f, -1.0f,
         0.0f,  1.0f,
         1.0f, -1.0f
    };
    BufferGpu attributeBuffer(false, sizeof(attributeData), attributeData);

    MyPipeline myPipeline(Primitive::triangles, vertexShader, "", "", "", fragmentShader);
    myPipeline.setAttributeLayout(attributeLayout);
    myPipeline.setAttributeBuffer(0, attributeBuffer);
    myPipeline.rgba = vec4(1.0f, 0.0f, 0.0f, 0.0f);

    setDrawFrame(getDisplayFrame());
    while (!framework.isClosing()) {
        framework.handleEvents();

        getDrawFrame().clearRgba();
        myPipeline.draw(3, 1, 0, 0);

        framework.swap();
    }

    return 0;
}
