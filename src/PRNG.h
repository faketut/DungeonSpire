#ifndef PRNG_H
#define PRNG_H

#include <random>
#include <cstdint>

// Process-wide PRNG. Single-threaded game, so a global is fine.
// Use seed() at startup; use randInt(n) for [0, n).
class PRNG {
public:
    static std::mt19937& engine() {
        static std::mt19937 eng{std::random_device{}()};
        return eng;
    }
    static void seed(std::uint32_t s) { engine().seed(s); }
    // Uniform integer in [0, n). Requires n > 0.
    static int randInt(int n) {
        std::uniform_int_distribution<int> dist(0, n - 1);
        return dist(engine());
    }
};

#endif
