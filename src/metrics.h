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

    double get_write_amplification() const {
        if (host_writes == 0) return 1.0;
        return static_cast<double>(flash_writes) / host_writes;
    }

    void print_report(const std::vector<uint32_t>& block_erase_counts) const {
        std::cout << "\n=== FlashForge Metrics Report ===\n";
        std::cout << "Host Writes: " << host_writes << "\n";
        std::cout << "Host Reads:  " << host_reads << "\n";
        std::cout << "Flash Writes: " << flash_writes << "\n";
        std::cout << "GC Events: " << gc_events << " (" << gc_pages_moved << " pages moved)\n";
        std::cout << "Erase Ops: " << erase_ops << "\n";
        std::cout << "Write Amplification (WA): " << get_write_amplification() << "\n";

        if (!block_erase_counts.empty()) {
            double sum = 0, mean = 0, variance = 0;
            uint32_t min_e = UINT32_MAX, max_e = 0;
            
            for (auto e : block_erase_counts) {
                sum += e;
                if (e < min_e) min_e = e;
                if (e > max_e) max_e = e;
            }

            mean = sum / block_erase_counts.size();

            for (auto e : block_erase_counts) {
                variance += (e - mean) * (e - mean);
            }

            variance /= block_erase_counts.size();

            std::cout << "Wear Mean: " << mean << " erases\n";
            std::cout << "Wear Min/Max: " << min_e << " / " << max_e << "\n";
            std::cout << "Wear StdDev: " << std::sqrt(variance) << "\n";
        }

        std::cout << "=================================\n";
    }
};
}