// Copyright 2020 yuzu Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include "common/assert.h"
#include "common/common_funcs.h"
#include "common/fastmem.h"
#include "common/virtual_buffer.h"

namespace Core {

class System;

namespace DramMemoryMap {
enum : u64 {
    Base = 0x80000000ULL,
    Size = 0x100000000ULL,
    End = Base + Size,
    KernelReserveBase = Base + 0x60000,
    SlabHeapBase = KernelReserveBase + 0x85000,
    SlapHeapSize = 0xa21000,
    SlabHeapEnd = SlabHeapBase + SlapHeapSize,
};
}; // namespace DramMemoryMap

inline u64 GetDramOffset(PAddr addr) {
    return addr - DramMemoryMap::Base;
}

class DeviceMemory : NonCopyable {
public:
    explicit DeviceMemory(Core::System& system);
    ~DeviceMemory();

    template <typename T>
    PAddr GetPhysicalAddr(const T* ptr) const {
        return (reinterpret_cast<uintptr_t>(ptr) -
                reinterpret_cast<uintptr_t>(fastmem.GetDramPointer())) +
               DramMemoryMap::Base;
    }

    u8* GetPointer(PAddr addr) {
        return fastmem.GetDramPointer() + GetDramOffset(addr);
    }

    const u8* GetPointer(PAddr addr) const {
        return fastmem.GetDramPointer() + GetDramOffset(addr);
    }

    void ResetFastmemRegion(Common::FastmemRegion& region,
                            std::size_t address_space_width_in_bits) {
        return fastmem.ResetRegion(region, address_space_width_in_bits);
    }

private:
    Common::FastmemManager fastmem;
    Core::System& system;
};

} // namespace Core
