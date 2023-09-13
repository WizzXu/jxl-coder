//
// Created by Radzivon Bartoshyk on 11/09/2023.
//

#include "F32ToRGB1010102.h"
#include <vector>
#include "HalfFloats.h"
#include <arpa/inet.h>
#include <cmath>
#include <algorithm>
#include "ThreadPool.hpp"

#undef HWY_TARGET_INCLUDE
#define HWY_TARGET_INCLUDE "F32ToRGB1010102.cpp"

#include "hwy/foreach_target.h"
#include "hwy/highway.h"

HWY_BEFORE_NAMESPACE();

namespace coder {
    namespace HWY_NAMESPACE {

        using hwy::HWY_NAMESPACE::FixedTag;
        using hwy::HWY_NAMESPACE::Rebind;
        using hwy::HWY_NAMESPACE::Vec;
        using hwy::HWY_NAMESPACE::Set;
        using hwy::HWY_NAMESPACE::Zero;
        using hwy::HWY_NAMESPACE::Load;
        using hwy::HWY_NAMESPACE::Mul;
        using hwy::HWY_NAMESPACE::Max;
        using hwy::HWY_NAMESPACE::Min;
        using hwy::HWY_NAMESPACE::BitCast;
        using hwy::HWY_NAMESPACE::ExtractLane;
        using hwy::HWY_NAMESPACE::DemoteTo;
        using hwy::HWY_NAMESPACE::ConvertTo;
        using hwy::HWY_NAMESPACE::LoadInterleaved4;
        using hwy::HWY_NAMESPACE::Or;
        using hwy::HWY_NAMESPACE::ShiftLeft;
        using hwy::HWY_NAMESPACE::Store;

        void
        F32ToRGBA1010102RowC(const float *HWY_RESTRICT data, uint8_t *HWY_RESTRICT dst, int width,
                             const int *permuteMap) {
            float range10 = powf(2, 10) - 1;
            const FixedTag<float, 4> df;
            const Rebind<int32_t, FixedTag<float, 4>> di32;
            const FixedTag<uint32_t, 4> du;
            using V = Vec<decltype(df)>;
            using VU = Vec<decltype(du)>;
            const auto vRange10 = Set(df, range10);
            const auto zeros = Zero(df);
            const auto alphaMax = Set(df, 3.0);
            int x = 0;
            auto dst32 = reinterpret_cast<uint32_t *>(dst);
            for (x = 0; x + 4 < width; x += 4) {
                V pixels1;
                V pixels2;
                V pixels3;
                V pixels4;
                LoadInterleaved4(df, data, pixels1, pixels2, pixels3, pixels4);
                pixels1 = Min(Max(Mul(pixels1, vRange10), zeros), vRange10);
                pixels2 = Min(Max(Mul(pixels2, vRange10), zeros), vRange10);
                pixels3 = Min(Max(Mul(pixels3, vRange10), zeros), vRange10);
                pixels4 = Min(Max(Mul(pixels4, alphaMax), zeros), alphaMax);
                VU pixelsu1 = BitCast(du, ConvertTo(di32, pixels1));
                VU pixelsu2 = BitCast(du, ConvertTo(di32, pixels2));
                VU pixelsu3 = BitCast(du, ConvertTo(di32, pixels3));
                VU pixelsu4 = BitCast(du, ConvertTo(di32, pixels4));

                VU pixelsStore[4] = { pixelsu1, pixelsu2, pixelsu3, pixelsu4 };
                VU AV = pixelsStore[permuteMap[0]];
                VU RV = pixelsStore[permuteMap[1]];
                VU GV = pixelsStore[permuteMap[2]];
                VU BV = pixelsStore[permuteMap[3]];
                VU upper = Or(ShiftLeft<30>(AV), ShiftLeft<20>(RV));
                VU lower = Or(ShiftLeft<10>(GV), BV);
                VU final = Or(upper, lower);
                Store(final, du, dst32);
                data += 4;
                dst32 += 4;
            }

            for (; x < width; ++x) {
                auto A16 = (float) data[permuteMap[0]];
                auto R16 = (float) data[permuteMap[1]];
                auto G16 = (float) data[permuteMap[2]];
                auto B16 = (float) data[permuteMap[3]];
                auto R10 = (uint32_t) (std::clamp(R16 * range10, 0.0f, (float) range10));
                auto G10 = (uint32_t) (std::clamp(G16 * range10, 0.0f, (float) range10));
                auto B10 = (uint32_t) (std::clamp(B16 * range10, 0.0f, (float) range10));
                auto A10 = (uint32_t) std::clamp(std::round(A16 * 3), 0.0f, 3.0f);

                dst32[0] = (A10 << 30) | (R10 << 20) | (G10 << 10) | B10;

                data += 4;
                dst32 += 1;
            }
        }

        void
        F32ToRGBA1010102HWY(std::vector<uint8_t> &data, int srcStride, int *HWY_RESTRICT dstStride,
                            int width,
                            int height) {
            int newStride = (int) width * 4 * (int) sizeof(uint8_t);
            *dstStride = newStride;
            std::vector<uint8_t> newData(newStride * height);
            int permuteMap[4] = {3, 2, 1, 0};
//            int permuteMap[4] = {3, 1, 2, 0};
            int minimumTilingAreaSize = 850 * 850;
            int currentAreaSize = width * height;
            if (minimumTilingAreaSize > currentAreaSize) {
                for (int y = 0; y < height; ++y) {
                    F32ToRGBA1010102RowC(
                            reinterpret_cast<const float *>(data.data() + srcStride * y),
                            newData.data() + newStride * y,
                            width, &permuteMap[0]);
                }
            } else {
                ThreadPool pool;
                std::vector<std::future<void>> results;

                for (int y = 0; y < height; ++y) {
                    auto r = pool.enqueue(F32ToRGBA1010102RowC,
                                          reinterpret_cast<const float *>(data.data() +
                                                                          srcStride * y),
                                          newData.data() + newStride * y,
                                          width, &permuteMap[0]);
                    results.push_back(std::move(r));
                }

                for (auto &result: results) {
                    result.wait();
                }

            }
            data = newData;
        }

// NOLINTNEXTLINE(google-readability-namespace-comments)
    }
}

HWY_AFTER_NAMESPACE();

#if HWY_ONCE

namespace coder {
    HWY_EXPORT(F32ToRGBA1010102HWY);
    HWY_DLLEXPORT void
    F32ToRGBA1010102(std::vector<uint8_t> &data, int srcStride, int *HWY_RESTRICT dstStride,
                     int width,
                     int height) {
        HWY_DYNAMIC_DISPATCH(F32ToRGBA1010102HWY)(data, srcStride, dstStride, width, height);
    }
}

#endif