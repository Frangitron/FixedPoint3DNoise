#ifndef FIRMWARE_REWRITE_NOISE_HPP
#define FIRMWARE_REWRITE_NOISE_HPP

#include <cstdint>
#include <array>
#include <cstdio>

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

        const int64_t dot =
            static_cast<int64_t>(dx) * gx +
            static_cast<int64_t>(dy) * gy +
            static_cast<int64_t>(dz) * gz;

        return static_cast<int32_t>(dot / FIXED_SCALE);
    }

    // Calculate 3D fixed-point noise value.
    int32_t getValue(int32_t x, int32_t y, int32_t z) const {
        int32_t X0 = (x / FIXED_SCALE) * FIXED_SCALE;
        int32_t Y0 = (y / FIXED_SCALE) * FIXED_SCALE;
        int32_t Z0 = (z / FIXED_SCALE) * FIXED_SCALE;

        int32_t X1 = X0 + FIXED_SCALE;
        int32_t Y1 = Y0 + FIXED_SCALE;
        int32_t Z1 = Z0 + FIXED_SCALE;

        int32_t wx = x - X0;
        int32_t wy = y - Y0;
        int32_t wz = z - Z0;

        // lower top x 0 0
        int32_t n0 = dotGridGradient(X0, Y0, Z0, x, y, z);
        int32_t n1 = dotGridGradient(X1, Y0, Z0, x, y, z);
        const int32_t lt = lerp(n0, n1, wx);

        // lower bottom x 1 0
        n0 = dotGridGradient(X0, Y1, Z0, x, y, z);
        n1 = dotGridGradient(X1, Y1, Z0, x, y, z);
        const int32_t lb = lerp(n0, n1, wx);

        // lower x y 0
        const int32_t l = lerp(lt, lb, wy);

        // upper top x 0 1
        n0 = dotGridGradient(X0, Y0, Z1, x, y, z);
        n1 = dotGridGradient(X1, Y0, Z1, x, y, z);
        const int32_t ut = lerp(n0, n1, wx);

        // upper bottom x 1 1
        n0 = dotGridGradient(X0, Y1, Z1, x, y, z);
        n1 = dotGridGradient(X1, Y1, Z1, x, y, z);
        const int32_t ub = lerp(n0, n1, wx);

        // upper x y 1
        const int32_t u = lerp(ut, ub, wy);

        // value
        const int32_t value = lerp(l, u, wz);

        return value;
    }

    Vector3 randomGradient(const int32_t x, const int32_t y, const int32_t z) const {
        const int32_t ix = x / FIXED_SCALE;
        const int32_t iy = y / FIXED_SCALE;
        const int32_t iz = z / FIXED_SCALE;

        // Compute a pseudo-random seed from the input coordinates.
        const uint32_t seed = permutation[(ix + permutation[(iy + permutation[iz & 255]) & 255]) & 255];

        // Generate pseudo-random fixed-point components in [-FIXED_SCALE, FIXED_SCALE].
        int32_t gx = static_cast<int32_t>((seed ^ 0xF45325) % (2 * FIXED_SCALE + 1)) - FIXED_SCALE;
        int32_t gy = static_cast<int32_t>((seed ^ 0xA7463B) % (2 * FIXED_SCALE + 1)) - FIXED_SCALE;
        int32_t gz = static_cast<int32_t>((seed ^ 0xC83D21) % (2 * FIXED_SCALE + 1)) - FIXED_SCALE;

        // Normalize the gradient vector to have a unit length in fixed-point precision.
        if (const int32_t lengthSquared = gx * gx + gy * gy + gz * gz; lengthSquared > 0) {
            const int32_t length = sqrt_i32(lengthSquared);
            gx = FIXED_SCALE * gx / length;
            gy = FIXED_SCALE * gy / length;
            gz = FIXED_SCALE * gz / length;
        }

        return { gx, gy, gz };
    }

    // Fixed-point integer square root function.
    [[nodiscard]] static int32_t sqrt_i32(int32_t v) {
        uint32_t b = 1<<30, q = 0, r = v;
        while (b > r)
            b >>= 2;
        while( b > 0 ) {
            const uint32_t t = q + b;
            q >>= 1;
            if( r >= t ) {
                r -= t;
                q += b;
            }
            b >>= 2;
        }
        return static_cast<int32_t>(q);
    }

    static int32_t lerp(const int32_t a, const int32_t b, const int32_t weight) {
        //  return (a1 - a0) * (3.0 - w * 2.0) * w * w + a0;  <- Cubic float
        return a - (a * weight / FIXED_SCALE) + (b * weight / FIXED_SCALE);
    }

private:
    std::array<uint8_t, 512> permutation;
};

#endif //FIRMWARE_REWRITE_NOISE_HPP