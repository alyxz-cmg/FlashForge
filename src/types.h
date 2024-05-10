// types.h
#pragma once
#include <cstdint>

namespace FlashForge {

constexpr uint32_t UNMAPPED = UINT32_MAX;

enum class PageState {
    FREE,
    VALID,
    INVALID,
    BAD
};

struct PhysicalAddress {
    uint32_t block_id;
    uint32_t page_offset;
    
    bool is_valid() const { return block_id != UNMAPPED; }
};

struct FlashPage {
    PageState state = PageState::FREE;
    uint32_t lpn = UNMAPPED;
};

struct FlashBlock {
    uint32_t id;
    uint32_t erase_count = 0;
    uint32_t free_pages;
    uint32_t valid_pages = 0;
    uint32_t invalid_pages = 0;
    bool is_bad = false;
};
}