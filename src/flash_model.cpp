// flash_model.cpp
#pragma once
#include "types.h"
#include "config.h"
#include <vector>
#include <stdexcept>

namespace FlashForge {

class FlashModel {
public:
    FlashModel(const SystemConfig& config);

    void program_page(PhysicalAddress pba, uint32_t lpn);
    void invalidate_page(PhysicalAddress pba);
    void erase_block(uint32_t block_id);
    
    FlashPage read_page(PhysicalAddress pba) const;
    const FlashBlock& get_block_meta(uint32_t block_id) const;
    
    std::vector<uint32_t> get_all_erase_counts() const;
    uint32_t get_total_blocks() const { return config_.total_blocks; }
    uint32_t get_pages_per_block() const { return config_.pages_per_block; }

private:
    SystemConfig config_;
    std::vector<FlashBlock> blocks_;
    std::vector<std::vector<FlashPage>> pages_;
};
}