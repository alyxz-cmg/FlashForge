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

PhysicalAddress FtlEngine::allocate_page(bool is_for_gc) {
    if (active_page_offset_ >= config_.pages_per_block) {
        if (!is_for_gc && free_blocks_.size() <= config_.gc_threshold_blocks) {
            trigger_garbage_collection();
        }
        active_block_ = get_next_free_block();
        active_page_offset_ = 0;
    }

    PhysicalAddress pba = {active_block_, active_page_offset_};
    active_page_offset_++;
    return pba;
}

uint32_t FtlEngine::get_next_free_block() {
    if (free_blocks_.empty()) {
        throw std::runtime_error("SSD is entirely full, no free blocks available. OP ratio too low or GC failed.");
    }

    uint32_t block = free_blocks_.front();
    free_blocks_.pop_front();
    return block;
}

void FtlEngine::trigger_garbage_collection() {
    while (free_blocks_.size() < (config_.gc_threshold_blocks + 2)) {
        
        uint32_t victim_block = UNMAPPED;
        uint32_t max_invalid = 0;

        for (uint32_t i = 0; i < config_.total_blocks; ++i) {

            if (i == active_block_) continue;
            
            const auto& meta = flash_.get_block_meta(i);
            if (meta.free_pages == config_.pages_per_block) continue;

            if (meta.invalid_pages > max_invalid) {
                max_invalid = meta.invalid_pages;
                victim_block = i;
            }
        }

        if (victim_block == UNMAPPED) break; 

        uint32_t pages_moved = 0;
        for (uint32_t offset = 0; offset < config_.pages_per_block; ++offset) {
            PhysicalAddress old_pba = {victim_block, offset};
            FlashPage page = flash_.read_page(old_pba);

            if (page.state == PageState::VALID) {
                PhysicalAddress new_pba = allocate_page(true); 
                flash_.program_page(new_pba, page.lpn);
                metrics_.record_flash_write();
                metrics_.record_flash_read();
                l2p_table_[page.lpn] = new_pba;
                pages_moved++;
            }
        }

        flash_.erase_block(victim_block);
        metrics_.record_erase();
        metrics_.record_gc(pages_moved);
        free_blocks_.push_back(victim_block);
    }
}
}