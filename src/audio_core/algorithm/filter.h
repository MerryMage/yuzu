// Copyright 2018 yuzu Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <array>
#include <vector>
#include "common/common_types.h"

namespace AudioCore {

/// Digial biquad filter:
///
///          b0 + b1 z^-1 + b2 z^-2
///  H(z) = ------------------------
///          a0 + a1 z^-1 + b2 z^-2
struct Filter {
public:
    /// Creates a low-pass filter.
    /// @param cutoff Determines the cutoff frequency. A value from 0.0 to 1.0.
    /// @param quality Determines the quality factor of this filter.
    static Filter LowPass(double cutoff, double Q);

    /// Passthrough filter.
    Filter();

    Filter(double a0, double a1, double a2, double b0, double b1, double b2);

    void Process(std::vector<s16>& signal);

private:
    static constexpr size_t channel_count = 2;

    /// Coefficients are in normalized form (a0 = 1.0).
    double a1, a2, b0, b1, b2;
    /// Input History
    std::array<std::array<double, channel_count>, 3> in;
    /// Output History
    std::array<std::array<double, channel_count>, 3> out;
};

} // namespace AudioCore
