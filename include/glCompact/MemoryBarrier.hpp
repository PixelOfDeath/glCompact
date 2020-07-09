#pragma once

namespace glCompact {
    class MemoryBarrier {
        public:
            MemoryBarrier() = delete;
            //read
            static void attributeBuffer();
            static void attributeIndexBuffer();
            static void parameterBuffer();
            static void uniformBuffer();
            static void texture();
            //read/write
            static void image();
            static void imageUploadDownloadClear();
            static void shaderStorageBuffer();
            static void atomicCounterBuffer();

            static void bufferImageTransfer();
            static void bufferCreateClearCopyInvalidate();

            static void frame();
            static void transformFeedback();
            static void query();

            static void flushMappedMemoryWrites();
            static void all();

            class RasterizationRegion {
                public:
                    RasterizationRegion() = delete;
                    //read
                    static void uniformBuffer();
                    static void texture();
                    //read/write
                    static void image();
                    static void shaderStorageBuffer();
                    static void atomicCounterBuffer();
                    static void frame();
                    static void all();
            };
    };
    //void setFrameTextureBarrier(); //void glTextureBarrier(void); NV_texture_barrier or GL_ARB_texture_barrier (Core since 4.5), barrier to sample from a texture that is still bound as the current frame render target
}
