// ftl.h
#pragma once
#include "config.h"
#include "types.h"
#include "flash_model.h"
#include "metrics.h"
#include <vector>
#include <list>

namespace FlashForge {

class FtlEngine {
public:
    FtlEngine(const SystemConfig& config);

    void write(uint32_t lpn);
    bool read(uint32_t lpn);
    void trim(uint32_t lpn);

    const FlashModel& get_flash() const { return flash_; }

private:
    SystemConfig config_;
    FlashModel flash_;

    std::vector<PhysicalAddress> l2p_table_;
    
    std::list<uint32_t> free_blocks_;
    uint32_t active_block_;
    uint32_t active_page_offset_;

    PhysicalAddress allocate_page();
    void trigger_garbage_collection();
    uint32_t get_next_free_block();
};
}