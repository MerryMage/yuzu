// Copyright 2018 yuzu Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <algorithm>
#include <cmath>
#include <vector>
#include "audio_core/algorithm/interpolate.h"
#include "common/common_types.h"
#include "common/logging/log.h"

namespace AudioCore {

std::vector<s16> Interpolate(InterpolationState& state, std::vector<s16> input, double ratio) {
    if (input.size() < 2)
        return {};

    if (ratio <= 0) {
        LOG_CRITICAL(Audio, "Nonsensical interpolation ratio {}", ratio);
        ratio = 1.0;
    } else if (ratio != state.current_ratio) {
        state.nyquist = Filter::LowPass(std::min(0.5, 0.5 / ratio), 1 / std::sqrt(2.0));
    }

    state.nyquist.Process(input);

    const size_t num_frames = input.size() / 2;
    double position = state.position;
    std::vector<s16> output;
    output.reserve(static_cast<size_t>(input.size() / ratio + 4));

    const auto sample = [&](double f, s16 x0l, s16 x0r, s16 x1l, s16 x1r) {
        // Linear interpolation.
        const double l = x0l * f + x1l * (1 - f);
        const double r = x0r * f + x1r * (1 - f);
        output.emplace_back(static_cast<s16>(l));
        output.emplace_back(static_cast<s16>(r));
    };

    if (position < 0) {
        const double f = std::fmod(position + 1, 1);
        sample(f, state.last_frame[0], state.last_frame[1], input[0], input[1]);
        position = std::max(position + ratio, 0.0);
    } else {
        output.emplace_back(state.last_frame[0]);
        output.emplace_back(state.last_frame[1]);
    }

    for (; static_cast<size_t>(position) < (num_frames - 1); position += ratio) {
        const size_t i = static_cast<size_t>(position);
        const double f = std::fmod(position, 1);
        sample(f, input[i * 2 + 0], input[i * 2 + 1], input[i * 2 + 2], input[i * 2 + 3]);
    }

    state.position = position - num_frames;
    state.last_frame[0] = input[(num_frames - 1) * 2 + 0];
    state.last_frame[1] = input[(num_frames - 1) * 2 + 1];

    return output;
}

} // namespace AudioCore
