/*
    glCompact
    Copyright (C) 2019-2020 Frederik Uje vom Hofe

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 3 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#include "../Framework.hpp" //also includes glCompact
#include <string>

using namespace std;
using namespace glm;
using namespace glCompact;

class MyPipeline : public PipelineRasterization {
    public:
        using PipelineRasterization::PipelineRasterization;
        //In case of old g++ version where constructor inherence is broken, replace upper line with this:
        //MyPipeline(const string& s1, const string& s2, const string& s3, const string& s4, const string& s5):PipelineRasterization(s1, s2, s3, s4, s5){}
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
