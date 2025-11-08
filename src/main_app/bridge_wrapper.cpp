#include <cstdio>
#include <functional>

// --- C++ Implementations of Callbacks ---

/**
 * @brief C++ function to be called when the playback lease ends.
 */
static void endLeaseCb(int const &c) {
    fprintf(stderr, "[INFO] Playback lease ended with code: %d\n", c);
}

/**
 * @brief C++ function to be called on a playback error.
 */
void pbErrCb(void *) {
    fprintf(stderr, "[WARN] A playback error occurred.\n");
}


// --- C-Linkage Wrappers ---

extern "C" {

/**
 * @brief A C-linkage std::function object wrapping the endLeaseCb.
 *
 * The C code can take the address of this variable and pass it to C++
 * functions that expect a std::function<void(int const&)>.
 */
std::function<void(int const &)> endLeaseCallback(endLeaseCb);

/**
 * @brief A C-linkage std::function object wrapping the pbErrCb.
 *
 * The C code can take the address of this variable and pass it to C++
 * functions that expect a std::function<void(void*)>.
 */
std::function<void(void *)> pbErrCallback(pbErrCb);

} // extern "C"
