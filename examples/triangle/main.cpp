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

int main (int argc, char *argv[]) {
    Framework framework(640, 480);

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

    AttributeLayout attributeLayout;
    attributeLayout.addBufferIndex();
    attributeLayout.addLocation(0, AttributeFormat::R32G32_SFLOAT);

    float attributeData[] = {
        -1.0f, -1.0f,
         0.0f,  1.0f,
         1.0f, -1.0f
    };
    Buffer attributeBuffer(false, sizeof(attributeData), attributeData);

    MyPipeline myPipeline(vertexShader, "", "", "", fragmentShader);
    myPipeline.setVertexStageInputPrimitiveTopology(PrimitiveTopology::triangleList);
    myPipeline.setAttributeLayout(attributeLayout);
    myPipeline.setAttributeBuffer(0, attributeBuffer);
    myPipeline.rgba = vec4(1.0f, 0.0f, 0.0f, 0.0f);

    setDrawFrame(getWindowFrame());
    while (!framework.isClosing()) {
        framework.handleEvents();

        getDrawFrame().clearRgba();
        myPipeline.draw(3, 1, 0, 0);

        framework.swap();
    }

    return 0;
}
