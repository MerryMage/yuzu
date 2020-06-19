// Copyright 2020 yuzu emulator team
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <algorithm>
#include <string>
#include <vector>
#include "common/assert.h"
#include "common/common_types.h"
#include "common/fastmem.h"
#include "common/logging/log.h"

#ifdef _WIN32
#include <cstdlib>
#else
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

namespace Common {

#ifndef _WIN32
static constexpr size_t PAGE_SIZE = 0x1000;
#endif

FastmemRegion::FastmemRegion() = default;

void FastmemRegion::Map(VAddr vaddr, size_t page_count, u64 dram_offset) {
#ifdef _WIN32
    (void)vaddr;
    (void)page_count;
    (void)dram_offset;
#else
    if (!base_pointer) {
        return;
    }
    mmap(base_pointer + vaddr, page_count * PAGE_SIZE, PROT_READ | PROT_WRITE,
         MAP_SHARED | MAP_FIXED, fd, dram_offset);
#endif
}

void FastmemRegion::Unmap(VAddr vaddr, size_t page_count) {
#ifdef _WIN32
    (void)vaddr;
    (void)page_count;
#else
    if (!base_pointer) {
        return;
    }
    mmap(base_pointer + vaddr, page_count * PAGE_SIZE, PROT_NONE,
         MAP_ANON | MAP_PRIVATE | MAP_FIXED, -1, 0);
#endif
}

FastmemManager::FastmemManager(std::size_t dram_size_) {
#ifdef _WIN32
    dram_pointer = static_cast<u8*>(std::malloc(dram_size_));
#else
    dram_size = dram_size_;

    const std::string shm_filename = "/yuzu.fastmem." + std::to_string(getpid());
    fd = shm_open(shm_filename.c_str(), O_RDWR | O_CREAT | O_EXCL, 0600);
    if (fd == -1) {
        LOG_WARNING(Common_Memory, "Unable to fastmem: shm_open failed");
        goto fastmem_failure;
    }
    shm_unlink(shm_filename.c_str());
    if (ftruncate(fd, dram_size) < 0) {
        LOG_WARNING(Common_Memory, "Unable to fastmem: could not allocate shared memory");
        goto fastmem_failure;
    }
    dram_pointer =
        static_cast<u8*>(mmap(nullptr, dram_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    if (!dram_pointer || dram_pointer == MAP_FAILED) {
        LOG_WARNING(Common_Memory, "Unable to fastmem: could not map shared memory");
        goto fastmem_failure;
    }
    return;

fastmem_failure:
    if (fd != -1) {
        close(fd);
        fd = -1;
    }
    dram_pointer = static_cast<u8*>(std::malloc(dram_size));
#endif
}

FastmemManager::~FastmemManager() {
#ifdef _WIN32
    std::free(dram_pointer);
#else
    if (fd == -1) {
        std::free(dram_pointer);
        return;
    }

    // munmap MUST be called before closing the file descriptor
    for (const auto [region, size] : fastmem_regions) {
        munmap(region, size);
    }
    munmap(dram_pointer, dram_size);
    close(fd);
#endif
}

void FastmemManager::ResetRegion(FastmemRegion& r, std::size_t address_space_width_in_bits) {
#ifdef _WIN32
    (void)r;
    (void)address_space_width_in_bits
#else
    if (fd == -1) {
        return;
    }

    if (r.base_pointer) {
        fastmem_regions.erase(
            std::remove_if(fastmem_regions.begin(), fastmem_regions.end(),
                           [&](const auto& x) { return x.first == r.base_pointer; }),
            fastmem_regions.end());
        munmap(r.base_pointer, r.region_size);
    }

    r.fd = fd;
    r.region_size = static_cast<size_t>(1) << address_space_width_in_bits;
    r.base_pointer =
        static_cast<u8*>(mmap(nullptr, r.region_size, PROT_NONE, MAP_ANON | MAP_PRIVATE, -1, 0));
    if (!r.base_pointer || r.base_pointer == MAP_FAILED) {
        LOG_WARNING(Common_Memory, "Unable to fastmem: failed alloc of fastmem region of size {}",
                    r.region_size);
        r.fd = -1;
        r.base_pointer = nullptr;
        return;
    }

    fastmem_regions.emplace_back(std::pair(r.base_pointer, r.region_size));
#endif
}

} // namespace Common
