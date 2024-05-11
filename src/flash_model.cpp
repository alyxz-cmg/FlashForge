// flash_model.cpp
#include "flash_model.h"

namespace FlashForge {

FlashModel::FlashModel(const SystemConfig& config) : config_(config) {
    blocks_.resize(config.total_blocks);
    pages_.resize(config.total_blocks, std::vector<FlashPage>(config.pages_per_block));

    for (uint32_t i = 0; i < config.total_blocks; ++i) {
        blocks_[i].id = i;
        blocks_[i].free_pages = config.pages_per_block;
    }
}

void FlashModel::program_page(PhysicalAddress pba, uint32_t lpn) {
    if (pba.block_id >= config_.total_blocks || pba.page_offset >= config_.pages_per_block) {
        throw std::out_of_range("Physical address out of bounds.");
    }
    
    auto& page = pages_[pba.block_id][pba.page_offset];
    auto& block = blocks_[pba.block_id];

    if (page.state != PageState::FREE) {
        throw std::logic_error("Attempted to program a non-free page. Erase required.");
    }

    page.state = PageState::VALID;
    page.lpn = lpn;
    
    block.free_pages--;
    block.valid_pages++;
}

void FlashModel::invalidate_page(PhysicalAddress pba) {
    auto& page = pages_[pba.block_id][pba.page_offset];
    auto& block = blocks_[pba.block_id];

    if (page.state == PageState::VALID) {
        page.state = PageState::INVALID;
        block.valid_pages--;
        block.invalid_pages++;
    }
}

void FlashModel::erase_block(uint32_t block_id) {
    auto& block = blocks_[block_id];
    auto& block_pages = pages_[block_id];

    for (auto& page : block_pages) {
        page.state = PageState::FREE;
        page.lpn = UNMAPPED;
    }

    block.erase_count++;
    block.free_pages = config_.pages_per_block;
    block.valid_pages = 0;
    block.invalid_pages = 0;
}

FlashPage FlashModel::read_page(PhysicalAddress pba) const {
    return pages_[pba.block_id][pba.page_offset];
}

const FlashBlock& FlashModel::get_block_meta(uint32_t block_id) const {
    return blocks_[block_id];
}

std::vector<uint32_t> FlashModel::get_all_erase_counts() const {
    std::vector<uint32_t> counts;
    counts.reserve(blocks_.size());
    
    for (const auto& b : blocks_) {
        counts.push_back(b.erase_count);
    }

    return counts;
}
}