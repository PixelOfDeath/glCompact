#pragma once

namespace glCompact {
    class CastAnyPtr {
        public:
            template<typename T>
            constexpr CastAnyPtr(T* ptr):ptr(reinterpret_cast<void*>(ptr)){}
            template<typename T>
            constexpr operator T*(){return reinterpret_cast<T*>(ptr);}
        private:
            void* ptr;
    };
}
