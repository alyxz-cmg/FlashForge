// main.cpp
#include "config.h"
#include "ftl.h"
#include <iostream>
#include <random>
#include <algorithm>

using namespace FlashForge;

int main() {
    SystemConfig config;
    config.pages_per_block = 64;
    config.total_blocks = 256;
    config.logical_pages = static_cast<uint32_t>(config.total_blocks * config.pages_per_block * 0.75);

    FtlEngine ftl(config);
    std::mt19937 gen(42);

    try {
        std::cout << "Starting FlashForge FTL Simulation. Please wait.\n";
        std::cout << "Logical Pages: " << config.logical_pages << "\n";
        std::cout << "Physical Capacity: " << (config.total_blocks * config.pages_per_block) << " pages\n";

        for (uint32_t lpn = 0; lpn < config.logical_pages; ++lpn) {
            ftl.write(lpn);
        }

        std::cout << "\n[+] Sequential Fill Complete. Initializing Hot/Cold Workload.\n";

        const uint32_t split = config.logical_pages / 5;

        uint32_t hot_lower = 0;
        uint32_t hot_upper = std::max(split, 1u) - 1;

        uint32_t cold_lower = split;
        uint32_t cold_upper = config.logical_pages - 1;

        std::cout << "Hot range: [" << hot_lower << ", " << hot_upper << "]\n";
        std::cout << "Cold range: [" << cold_lower << ", " << cold_upper << "]\n";

        std::uniform_real_distribution<> prob_dist(0.0, 1.0);
        std::uniform_real_distribution<> hot_or_cold(0.0, 1.0);

        std::uniform_int_distribution<uint32_t> hot_dist(hot_lower, hot_upper);
        std::uniform_int_distribution<uint32_t> cold_dist(cold_lower, cold_upper);

        const int TOTAL_OPERATIONS = 100000;

        for (int i = 0; i < TOTAL_OPERATIONS; ++i) {

            double op_type = prob_dist(gen);

            uint32_t target_lpn;

            if (hot_or_cold(gen) < 0.8) {
                target_lpn = hot_dist(gen);
            } 
            
            else {
                target_lpn = cold_dist(gen);
            }

            if (op_type < 0.7) {
                ftl.write(target_lpn);
            } 
            
            else if (op_type < 0.95) {
                ftl.read(target_lpn);
            } 
            
            else {
                ftl.trim(target_lpn);
            }

            if (i % 10000 == 0 && i > 0) {
                std::cout << "Progress: " << i << " ops\n";
            }
        }

        ftl.get_metrics().print_report(
            ftl.get_flash().get_all_erase_counts()
        );

        std::cout << "Simulation Finished Successfully!\n";
        return 0;

    } 
    
    catch (const std::exception& e) {
        std::cerr << "Fatal exception: " << e.what() << "\n";
        return 1;
    } 
    
    catch (...) {
        std::cerr << "Fatal unknown error\n";
        return 1;
    }
}