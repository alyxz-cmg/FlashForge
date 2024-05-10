// config.h
#pragma once
#include <cstdint>

namespace FlashForge {

struct SystemConfig {
    uint32_t pages_per_block = 256;
    uint32_t total_blocks = 1024;
    uint32_t logical_pages = (1024 * 256) * 0.8;
    uint32_t gc_threshold_blocks = 10;
};
}