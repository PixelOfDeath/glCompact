#pragma once
#include <cstdint> //C++11

namespace glCompact {
    class Tribool {
        public:
            constexpr Tribool():value(2){}
            constexpr Tribool(bool value):value(value){}
            constexpr Tribool(const Tribool& tribool):value(tribool.value){}
            bool isFalse  () const {return value == 0;}
            bool isTrue   () const {return value == 1;}
            bool isUnknown() const {return value == 2;}
        private:
            uint8_t value;
    };
}
