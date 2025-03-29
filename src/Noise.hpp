#ifndef FIRMWARE_REWRITE_NOISE_HPP
#define FIRMWARE_REWRITE_NOISE_HPP

#include <cstdint>
#include <array>

class Noise {

public:

    typedef struct {
        int32_t x;
        int32_t y;
        int32_t z;
    } Vector3;

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

    int32_t dotGridGradient(const int32_t X, const int32_t Y, const int32_t Z, const int32_t x, const int32_t y, const int32_t z) const {
        auto [gx, gy, gz] = randomGradient(X, Y, Z);

        int32_t dx = x - X;
        int32_t dy = y - Y;
        int32_t dz = z - Z;

        int32_t d = (dx * gx) + (dy * gy) + (dz * gz);
        return d / (FIXED_SCALE * FIXED_SCALE);
    }

    // Calculate 3D fixed-point noise value.
    Vector3 getValue(int32_t x, int32_t y, int32_t z) const {
        int32_t X0 = (x / FIXED_SCALE) * FIXED_SCALE;
        int32_t Y0 = (y / FIXED_SCALE) * FIXED_SCALE;
        int32_t Z0 = (z / FIXED_SCALE) * FIXED_SCALE;

        int32_t X1 = x + FIXED_SCALE;
        int32_t Y1 = y + FIXED_SCALE;
        int32_t Z1 = z + FIXED_SCALE;

        int32_t wx = x - X0;
        int32_t wy = y - Y0;
        int32_t wz = z - Z0;

        // lower top two corners
        int32_t n0 = dotGridGradient(X0, Y0, Z0, x, y, z);
        int32_t n1 = dotGridGradient(X1, Y0, Z0, x, y, z);
        int32_t lt = lerp(n0, n1, wx);

        // lower bottom two corners
        n0 = dotGridGradient(X0, Y1, Z0, x, y, z);
        n1 = dotGridGradient(X1, Y1, Z0, x, y, z);
        int32_t lb = lerp(n0, n1, wx);
        int32_t l = lerp(lt, lb, wy);

        // upper top two corners
        n0 = dotGridGradient(X0, Y0, Z1, x, y, z);
        n1 = dotGridGradient(X1, Y0, Z1, x, y, z);
        int32_t ut = lerp(n0, n1, wx);

        // upper bottom two corners
        n0 = dotGridGradient(X0, Y1, Z1, x, y, z);
        n1 = dotGridGradient(X1, Y1, Z1, x, y, z);
        int32_t ub = lerp(n0, n1, wx);
        int32_t u = lerp(ut, ub, wy);

        int32_t value = lerp(l, u, wz);
        return {value, value, value};
    }

    Vector3 randomGradient(int32_t x, int32_t y, int32_t z) const {
        int32_t ix = x / FIXED_SCALE;
        int32_t iy = y / FIXED_SCALE;
        int32_t iz = z / FIXED_SCALE;
        // Compute a pseudo-random seed from the input coordinates.
        uint32_t seed = permutation[(ix + permutation[(iy + permutation[iz & 255]) & 255]) & 255];

        // Generate pseudo-random fixed-point components in [-FIXED_SCALE, FIXED_SCALE].
        int32_t gx = static_cast<int32_t>((seed ^ 0xF45325) % (2 * FIXED_SCALE + 1)) - FIXED_SCALE;
        int32_t gy = static_cast<int32_t>((seed ^ 0xA7463B) % (2 * FIXED_SCALE + 1)) - FIXED_SCALE;
        int32_t gz = static_cast<int32_t>((seed ^ 0xC83D21) % (2 * FIXED_SCALE + 1)) - FIXED_SCALE;

        // Normalize the gradient vector to have a unit length in fixed-point precision.
        int64_t lengthSquared = static_cast<int64_t>(gx) * gx +
                                static_cast<int64_t>(gy) * gy +
                                static_cast<int64_t>(gz) * gz;

        if (lengthSquared > 0) {
            int64_t scaleFactor = (static_cast<int64_t>(FIXED_SCALE) * FIXED_SCALE) / isqrt(lengthSquared);
            gx = (gx * scaleFactor);
            gy = (gy * scaleFactor);
            gz = (gz * scaleFactor);
        }

        return { gx, gy, gz };
    }

    // Fixed-point integer square root function.
    int32_t isqrt(int64_t value) const {
        int64_t result = 0;
        int64_t bit = 1LL << 62; // The second-to-top bit is set.

        // The input value must be non-negative (handle appropriately).
        while (bit > value) {
            bit >>= 2;
        }
        while (bit != 0) {
            if (value >= result + bit) {
                value -= result + bit;
                result = (result >> 1) + bit;
            } else {
                result >>= 1;
            }
            bit >>= 2;
        }
        return static_cast<int32_t>(result);
    }

    int32_t lerp(int32_t a, int32_t b, int32_t weight) const {
        return a - (a * weight / FIXED_SCALE) + (b * weight / FIXED_SCALE);
    }

private:
    std::array<uint8_t, 512> permutation;
};

#endif //FIRMWARE_REWRITE_NOISE_HPP