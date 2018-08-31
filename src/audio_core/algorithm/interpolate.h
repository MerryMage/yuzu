// Copyright 2018 yuzu Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <array>
#include <vector>
#include "audio_core/algorithm/filter.h"
#include "common/common_types.h"

namespace AudioCore {

class Interpolator {
public:
    /// @param ratio Interpolation ratio.
    ///              ratio > 1.0 results in fewer output samples.
    ///              ratio < 1.0 results in more output samples.
    void SetRatio(double ratio);

    /// Clear all history.
    void Reset();

    inline void SetRatio(u32 input_rate, u32 output_rate) {
        SetRatio(static_cast<double>(input_rate) / static_cast<double>(output_rate));
    }

    /// Interpolates input signal to produce output signal.
    /// @returns Output signal.
    std::vector<s16> Process(std::vector<s16> input);

private:
    static constexpr size_t lanczos_taps = 4;
    static constexpr size_t history_size = lanczos_taps * 2 - 1;

    double current_ratio = 0.0;
    CascadingFilter nyquist;
    std::array<std::array<s16, 2>, history_size> h = {}; ///< history
    double pos = 0;
};

} // namespace AudioCore
