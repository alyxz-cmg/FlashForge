// ftl.cpp
#include "ftl.h"
#include <stdexcept>
#include <algorithm>

namespace FlashForge {

FtlEngine::FtlEngine(const SystemConfig& config) : config_(config), flash_(config) {
    
    l2p_table_.resize(config_.logical_pages, {UNMAPPED, UNMAPPED});

    for (uint32_t i = 0; i < config_.total_blocks; ++i) {
        free_blocks_.push_back(i);
    }

    active_block_ = get_next_free_block();
    active_page_offset_ = 0;
}

void FtlEngine::write(uint32_t lpn) {
    if (lpn >= config_.logical_pages) {
        throw std::out_of_range("LPN out of bounds");
    }

    metrics_.record_host_write();

    if (l2p_table_[lpn].is_valid()) {
        flash_.invalidate_page(l2p_table_[lpn]);
    }

    PhysicalAddress pba = allocate_page();

    flash_.program_page(pba, lpn);
    metrics_.record_flash_write();

    l2p_table_[lpn] = pba;
}

bool FtlEngine::read(uint32_t lpn) {
    metrics_.record_host_read();
    
    if (lpn >= config_.logical_pages) return false;
    
    PhysicalAddress pba = l2p_table_[lpn];
    if (!pba.is_valid()) return false;

    metrics_.record_flash_read();
    FlashPage page = flash_.read_page(pba);
    return page.state == PageState::VALID;
}

void FtlEngine::trim(uint32_t lpn) {
    metrics_.record_host_trim();
    if (lpn >= config_.logical_pages) return;

    if (l2p_table_[lpn].is_valid()) {
        flash_.invalidate_page(l2p_table_[lpn]);
        l2p_table_[lpn] = {UNMAPPED, UNMAPPED};
    }
}
}