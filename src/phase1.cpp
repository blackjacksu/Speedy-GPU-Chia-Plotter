#include "../include/phase1.h"



GlobalData globals;

void* F1thread(int const index, uint8_t const k, const uint8_t* id, std::mutex* smm, bool gpu_boost)
{
    uint32_t const entry_size_bytes = 16;
    uint64_t const max_value = ((uint64_t)1 << (k));
    uint64_t const right_buf_entries = 1 << (kBatchSizes);
    uint8_t gpu_subgroup_idx = 0;
    uint64_t gpu_x[GPU_GROUP_SIZE];
    uint64_t gpu_loopcount[GPU_GROUP_SIZE];
    uint64_t right_writer_count = 0;

    std::unique_ptr<uint64_t[]> f1_entries(new uint64_t[(1U << kBatchSizes)]);

    F1Calculator f1(k, id, gpu_boost);
    // std::string file_name = "plot_disk_k";
    // file_name += std::to_string(k);
    // file_name += "_";
    // std::cout << "File: " << file_name << std::endl;
    // FileDisk plot_file(/*file_path,*/ file_name);

    // std::cout << plot_file.GetFileName() << std::endl;

    std::unique_ptr<uint8_t[]> right_writer_buf(new uint8_t[right_buf_entries * entry_size_bytes]);

    std::cout << "Thread Id: " << index << std::endl;
    std::cout << "End: lp = " << (((uint64_t)1) << (k - kBatchSizes)) << std::endl;
    std::cout << "Inc: +globals.num_threads" << globals.num_threads << std::endl;

    if (gpu_boost)
    {
        // Using GPU plotting CalculateBuckets_Boost
        // Using CPU plotting
        // Instead of computing f1(1), f1(2), etc, for each x, we compute them in batches
        // to increase CPU efficency.
        for (uint64_t lp = index; lp <= (((uint64_t)1) << (k - kBatchSizes));
             lp = lp + globals.num_threads)
        {
            gpu_x[gpu_subgroup_idx] = lp * (1 << (kBatchSizes)); // KBatchSizes = 8 lp = 0,4,8,12 x = 0,1024,2048,3072
            gpu_loopcount[gpu_subgroup_idx] = std::min(max_value - gpu_x[gpu_subgroup_idx], (uint64_t)1 << (kBatchSizes)); // loopcount = 256 most of the time
            gpu_subgroup_idx++;
            if (gpu_subgroup_idx >= GPU_GROUP_SIZE)
            {
                // Send the group to GPU
                gpu_subgroup_idx = 0;
                // For each pair x, y in the batch
                // Instead of computing f1(1), f1(2), etc, for each x, we compute them in batches
                // to increase CPU efficency.
                f1.CalculateBuckets_Boost(gpu_x, gpu_loopcount, f1_entries.get());
                
                right_writer_count = 0;
                for (uint8_t j = 0; j < GPU_GROUP_SIZE ; j++)
                {
                    for (uint32_t i = 0; i < gpu_loopcount[j]; i++) {
                        uint128_t entry;
                        // To-Do: the pointer right here might be incorrect in the second iteration: j = 1, should use f1_entries[i + shift]
                        entry = (uint128_t)f1_entries[i] << (128 - kExtraBits - k);
                        entry |= (uint128_t)gpu_x[j] << (128 - kExtraBits - 2 * k);
                        IntTo16Bytes(&right_writer_buf[i * entry_size_bytes], entry);
                        right_writer_count++;
                        gpu_x[j]++;
                    }   
                    // To-Do: shift += gpu_loopcount[j];

                    std::lock_guard<std::mutex> l(*smm);

                    // Write it out
                    for (uint32_t i = 0; i < right_writer_count; i++) {
                        globals.L_sort_manager->AddToCache(&(right_writer_buf[i * entry_size_bytes]));
                    }
                }

            }

        }
    }
    else
    {
        // Using CPU plotting
        // Instead of computing f1(1), f1(2), etc, for each x, we compute them in batches
        // to increase CPU efficency.
        for (uint64_t lp = index; lp <= (((uint64_t)1) << (k - kBatchSizes));
             lp = lp + globals.num_threads)
        {
            // For each pair x, y in the batch

            right_writer_count = 0;
            uint64_t x = lp * (1 << (kBatchSizes)); // KBatchSizes = 8 lp = 0,4,8,12 x = 0,1024,2048,3072

            uint64_t const loopcount = std::min(max_value - x, (uint64_t)1 << (kBatchSizes)); // loopcount = 256 most of the time

            // Instead of computing f1(1), f1(2), etc, for each x, we compute them in batches
            // to increase CPU efficency.
            f1.CalculateBuckets(x, loopcount, f1_entries.get());
            for (uint32_t i = 0; i < loopcount; i++) {
                uint128_t entry;

                entry = (uint128_t)f1_entries[i] << (128 - kExtraBits - k);
                entry |= (uint128_t)x << (128 - kExtraBits - 2 * k);
                IntTo16Bytes(&right_writer_buf[i * entry_size_bytes], entry);
                right_writer_count++;
                x++;
            }

            std::lock_guard<std::mutex> l(*smm);

            // Write it out
            for (uint32_t i = 0; i < right_writer_count; i++) {
                globals.L_sort_manager->AddToCache(&(right_writer_buf[i * entry_size_bytes]));
            }
        }
    }

    

    std::cout << "Thread " << index << " completed" << std::endl;

    return 0;
}