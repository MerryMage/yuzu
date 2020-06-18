// Copyright 2020 yuzu emulator team
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <cstdlib>
#include <vector>
#include "common/common_types.h"
#include "common/fastmem.h"

namespace Common {

FastmemRegion::FastmemRegion() = default;

void FastmemRegion::Map(VAddr vaddr, size_t page_count, u64 dram_offset) {}

void FastmemRegion::Unmap(VAddr vaddr, size_t page_count) {}

FastmemManager::FastmemManager(std::size_t dram_size)
    : dram_pointer{static_cast<u8*>(std::malloc(dram_size))} {}

FastmemManager::~FastmemManager() {
    std::free(dram_pointer);
}

void FastmemManager::Reset(FastmemRegion& region, std::size_t address_space_width_in_bits) {}

} // namespace Common
