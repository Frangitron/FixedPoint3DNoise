#ifndef FIRMWARE_REWRITE_NOISE_HPP
#define FIRMWARE_REWRITE_NOISE_HPP

#include <cstdint>
#include <array>
#include <cmath>

class Noise {
public:
    // Fixed-point scaling factor to adjust precision.
    static constexpr int FIXED_SCALE = 1024;

    Noise(uint32_t seed = 0) {
        // Initialize permutation array with a simple pseudo-random sequence.
        for (uint32_t i = 0; i < 256; ++i) {
            permutation[i] = i;
        }

        // Shuffle the permutation using the provided seed.
        for (uint32_t i = 255; i > 0; --i) {
            uint32_t j = seed % (i + 1);
            std::swap(permutation[i], permutation[j]);
            seed = seed * 1103515245 + 12345;  // Simple LCG for new 'random' seed.
        }

        // Duplicate the permutation array to avoid overflow when accessing.
        for (uint32_t i = 0; i < 256; ++i) {
            permutation[256 + i] = permutation[i];
        }
    }

    // Calculate 3D fixed-point noise value.
    int32_t getValue(int32_t x, int32_t y, int32_t z) const {
        // Convert to fixed-point space (scaled int coordinates).
        int32_t fx = x / FIXED_SCALE;
        int32_t fy = y / FIXED_SCALE;
        int32_t fz = z / FIXED_SCALE;

        // Get the integer parts.
        int32_t X = fx & 255;
        int32_t Y = fy & 255;
        int32_t Z = fz & 255;

        // Get the fractional parts as fixed-point (scaled [0, FIXED_SCALE)).
        fx = x % FIXED_SCALE;
        fy = y % FIXED_SCALE;
        fz = z % FIXED_SCALE;

        // Fade curves for interpolation.
        int32_t u = fade(fx);
        int32_t v = fade(fy);
        int32_t w = fade(fz);

        // Hash coordinates of the cube corners.
        uint8_t A = permutation[X] + Y;
        uint8_t AA = permutation[A] + Z;
        uint8_t AB = permutation[A + 1] + Z;
        uint8_t B = permutation[X + 1] + Y;
        uint8_t BA = permutation[B] + Z;
        uint8_t BB = permutation[B + 1] + Z;

        // Add blended results from the corners.
        return lerp(w,
                    lerp(v,
                         lerp(u, grad(permutation[AA], fx, fy, fz),
                              grad(permutation[BA], fx - FIXED_SCALE, fy, fz)),
                         lerp(u, grad(permutation[AB], fx, fy - FIXED_SCALE, fz),
                              grad(permutation[BB], fx - FIXED_SCALE, fy - FIXED_SCALE, fz))),
                    lerp(v,
                         lerp(u, grad(permutation[AA + 1], fx, fy, fz - FIXED_SCALE),
                              grad(permutation[BA + 1], fx - FIXED_SCALE, fy, fz - FIXED_SCALE)),
                         lerp(u, grad(permutation[AB + 1], fx, fy - FIXED_SCALE, fz - FIXED_SCALE),
                              grad(permutation[BB + 1], fx - FIXED_SCALE, fy - FIXED_SCALE, fz - FIXED_SCALE))));
    }

private:
    // Permutation table for pseudo-random gradients.
    std::array<uint8_t, 512> permutation;

    // Fade function (scaled with FIXED_SCALE for fixed-point).
    static int32_t fade(int32_t t) {
        int32_t scaled_t = t / FIXED_SCALE;  // Bring t into 0 to 1 [fixed-represented].
        return scaled_t * scaled_t * (3 * FIXED_SCALE - (2 * scaled_t));
    }

    // Linear interpolator (lerp).
    static int32_t lerp(int32_t t, int32_t a, int32_t b) {
        return a + ((b - a) * t) / FIXED_SCALE;
    }

    // Gradient function for 3D noise.
    static int32_t grad(int32_t hash, int32_t x, int32_t y, int32_t z) {
        int32_t h = hash & 15;              // Drop top 4 bits for 16 possible outcomes.
        int32_t u = h < 8 ? x : y;          // First gradient component is either x or y.
        int32_t v = h < 4 ? y : (h == 12 || h == 14 ? x : z);  // Second component may be x, y, or z.

        // Depending on the gradient direction, return adjusted u/v.
        return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
    }
};

#endif //FIRMWARE_REWRITE_NOISE_HPP