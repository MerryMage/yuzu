// Copyright 2020 yuzu emulator team
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <vector>
#include "common/common_types.h"

namespace Common {

class FastmemManager;

class FastmemRegion final {
public:
    FastmemRegion();

    void Map(VAddr vaddr, size_t page_count, u64 dram_offset);
    void Unmap(VAddr vaddr, size_t page_count);

    u8* GetPointer() const {
        return base_pointer;
    }

private:
    friend class FastmemManager;

    u8* base_pointer = nullptr;
};

class FastmemManager final {
public:
    explicit FastmemManager(std::size_t dram_size);
    ~FastmemManager();

    u8* GetDramPointer() const {
        return dram_pointer;
    }

    void ResetRegion(FastmemRegion& region, std::size_t address_space_width_in_bits);

private:
    u8* dram_pointer = nullptr;
};

} // namespace Common
