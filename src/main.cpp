// main.cpp
#include "config.h"
#include "ftl.h"
#include <iostream>
#include <random>

using namespace FlashForge;

int main() {
    SystemConfig config;
    config.pages_per_block = 64; 
    config.total_blocks = 256;   
    config.logical_pages = (config.total_blocks * config.pages_per_block) * 0.75;

    FtlEngine ftl(config);
    std::mt19937 gen(42);

    std::cout << "Starting FlashForge FTL Simulation. Please wait.\n";
    std::cout << "Logical Pages: " << config.logical_pages << "\n";
    std::cout << "Physical Capacity: " << (config.total_blocks * config.pages_per_block) << " pages\n";

    for (uint32_t lpn = 0; lpn < config.logical_pages; ++lpn) {
        ftl.write(lpn);
    }

    std::cout << "\n[+] Sequential Fill Complete. Initializing Hot/Cold Workload. Please wait.\n";

    std::uniform_real_distribution<> prob_dist(0.0, 1.0);
    std::uniform_int_distribution<> hot_dist(0, (config.logical_pages * 0.2) - 1);
    std::uniform_int_distribution<> cold_dist(config.logical_pages * 0.2, config.logical_pages - 1);

    const int TOTAL_OPERATIONS = 100'000;
    
    for (int i = 0; i < TOTAL_OPERATIONS; ++i) {
        double op_type = prob_dist(gen);
        
        uint32_t target_lpn;
        if (prob_dist(gen) < 0.8) {
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
    }

    ftl.get_metrics().print_report(ftl.get_flash().get_all_erase_counts());

    return 0;
}