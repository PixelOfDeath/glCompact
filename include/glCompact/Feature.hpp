#pragma once

namespace glCompact {
    struct Feature {
        bool drawIndirect;
        bool blendModePerDrawbuffer;
        bool drawBaseInstance;
        bool bptc;
        bool atomicCounter;
        bool shaderStorageBufferObject;
        bool astc;
        bool textureView;
        bool drawIndirectCount;
        bool polygonOffsetClamp;
        bool anisotropicFilter;
        bool spirv;
    };
}
