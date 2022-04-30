// Copyright 2018 Chia Network Inc

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//    http://www.apache.org/licenses/LICENSE-2.0

// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SRC_CPP_PHASE1_HPP_
#define SRC_CPP_PHASE1_HPP_


#include <math.h>
#include <stdio.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <thread>
#include <memory>
#include <mutex>

// #include "chia_filesystem.hpp"

#include "calculate_bucket.h"
// #include "entry_sizes.hpp"
// #include "exceptions.hpp"
// #include "pos_constants.hpp"
// #include "sort_manager.hpp"
// #include "threading.hpp"
#include "util.h"
#include "disk.h"
// #include "progress.hpp"

struct GlobalData {
    uint64_t left_writer_count;
    uint64_t right_writer_count;
    uint64_t matches;
    // std::unique_ptr<SortManager> L_sort_manager;
    // std::unique_ptr<SortManager> R_sort_manager;
    uint64_t left_writer_buf_entries;
    uint64_t left_writer;
    uint64_t right_writer;
    uint64_t stripe_size;
    uint8_t num_threads;
};


void* F1thread(int const index, uint8_t const k, const uint8_t* id, std::mutex* smm, std::string file_path, std::string start_time);

#endif