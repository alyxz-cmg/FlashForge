// metrics.h
#pragma once
#include <cstdint>
#include <vector>
#include <cmath>
#include <iostream>

namespace FlashForge {

class MetricsCollector {
public:
    uint64_t host_writes = 0;
    uint64_t host_reads = 0;
    uint64_t host_trims = 0;
    uint64_t flash_writes = 0;
    uint64_t flash_reads = 0;
    uint64_t erase_ops = 0;
    uint64_t gc_events = 0;
    uint64_t gc_pages_moved = 0;

    void record_host_write() { host_writes++; }
    void record_host_read() { host_reads++; }
    void record_host_trim() { host_trims++; }
    void record_flash_write() { flash_writes++; }
    void record_flash_read() { flash_reads++; }
    void record_erase() { erase_ops++; }
    void record_gc(uint32_t pages_moved) {
        gc_events++;
        gc_pages_moved += pages_moved;
    }
};
}