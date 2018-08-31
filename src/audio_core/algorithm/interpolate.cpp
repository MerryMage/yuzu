// Copyright 2018 yuzu Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#define _USE_MATH_DEFINES

#include <algorithm>
#include <cmath>
#include <vector>
#include "audio_core/algorithm/interpolate.h"
#include "common/assert.h"
#include "common/common_types.h"
#include "common/logging/log.h"

namespace AudioCore {

/// The Lanczos kernel
static double Lanczos(size_t a, double x) {
    if (x == 0.0)
        return 1.0;
    const double px = M_PI * x;
    return a * std::sin(px) * std::sin(px / a) / (px * px);
}

void Interpolator::SetRatio(double ratio) {
    if (ratio != current_ratio) {
        const double cutoff_frequency = std::min(0.5 / ratio, 0.5 * ratio);
        nyquist = CascadingFilter::LowPass(std::clamp(cutoff_frequency, 0.0, 0.4), 3);
        current_ratio = ratio;
        Reset();
    }
}

void Interpolator::Reset() {
    h = {};
    pos = 0;
}

std::vector<s16> Interpolator::Process(std::vector<s16> input) {
    if (input.size() < 2)
        return {};

    ASSERT(current_ratio != 0.0);
    nyquist.Process(input);

    const size_t num_frames = input.size() / 2;

    std::vector<s16> output;
    output.reserve(static_cast<size_t>(input.size() / current_ratio + 4));

    for (size_t i = 0; i < num_frames; ++i) {
        std::rotate(h.begin(), h.end() - 1, h.end());
        h[0][0] = input[i * 2 + 0];
        h[0][1] = input[i * 2 + 1];

        while (pos <= 1.0) {
            double l = 0.0;
            double r = 0.0;
            for (size_t j = 0; j < h.size(); j++) {
                l += Lanczos(lanczos_taps, pos + j - lanczos_taps + 1) * h[j][0];
                r += Lanczos(lanczos_taps, pos + j - lanczos_taps + 1) * h[j][1];
            }
            output.emplace_back(static_cast<s16>(std::clamp(l, -32768.0, 32767.0)));
            output.emplace_back(static_cast<s16>(std::clamp(r, -32768.0, 32767.0)));

            pos += current_ratio;
        }
        pos -= 1.0;
    }

    return output;
}

} // namespace AudioCore
