#pragma once

/**
    @brief: Handles one of more sets of two parameters, where the first parameter gets set to the value of the second one.
    Returns true if any of the parameter pairs were different.

    Used to apply cached values in the form of current/pending.

    @code
    if(isDiffThenAssign(
        currentA, pendingA,
        currentB, pendingB,
    )) {
        //execute neccesary state changes
    }
    @endcode
*/
template<typename T>
bool isDiffThenAssign(T& current, const T& pending) {
    if (current != pending) {
        current = pending;
        return true;
    } else {
        return false;
    }
}

template<typename T, typename ... TArgs>
bool isDiffThenAssign(T& current, const T& pending, TArgs&&...args) {
    return isDiffThenAssign(current, pending) || isDiffThenAssign(args...);
}
