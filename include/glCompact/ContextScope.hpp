#pragma once

namespace glCompact {
    class ContextGroup_;
    class ContextGroup;
    class ContextScope {
        public:
            ContextScope(void *(*getGlFunctionPointer)(const char*));
            ContextScope(void(*(*getGlFunctionPointer)(const char*))());
            ContextScope(const ContextScope* contextGroupSource);
            ~ContextScope();
        private:
            ContextGroup_* contextGroup_;
            ContextGroup*  contextGroup;
    };
}
