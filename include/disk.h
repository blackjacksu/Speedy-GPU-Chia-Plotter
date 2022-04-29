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

#ifndef SRC_CPP_DISK_HPP_
#define SRC_CPP_DISK_HPP_

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>

// enables disk I/O logging to disk.log
// use tools/disk.gnuplot to generate a plot
#define ENABLE_LOGGING 0

using namespace std::chrono_literals; // for operator""min;

// #include "chia_filesystem.hpp"

// #include "./bits.hpp"
#include "util.h"
// #include "bitfield.hpp"

constexpr uint64_t write_cache = 1024 * 1024;
constexpr uint64_t read_ahead = 1024 * 1024;

struct Disk {
    virtual uint8_t const* Read(uint64_t begin, uint64_t length) = 0;
    virtual void Write(uint64_t begin, const uint8_t *memcache, uint64_t length) = 0;
    virtual void Truncate(uint64_t new_size) = 0;
    virtual std::string GetFileName() = 0;
    virtual void FreeMemory() = 0;
    virtual ~Disk() = default;
};

#if ENABLE_LOGGING
// logging is currently unix / bsd only: use <fstream> or update
// calls to ::open and ::write to port to windows
#include <fcntl.h>
#include <unistd.h>
#include <mutex>
#include <unordered_map>
#include <cinttypes>

enum class op_t : int { read, write};

void disk_log(fs::path const& filename, op_t const op, uint64_t offset, uint64_t length)
{
    static std::mutex m;
    static std::unordered_map<std::string, int> file_index;
    static auto const start_time = std::chrono::steady_clock::now();
    static int next_file = 0;

    auto const timestamp = std::chrono::steady_clock::now() - start_time;

    int fd = ::open("disk.log", O_WRONLY | O_CREAT | O_APPEND, 0755);

    std::unique_lock<std::mutex> l(m);

    char buffer[512];

    int const index = [&] {
        auto it = file_index.find(filename.string());
        if (it != file_index.end()) return it->second;
        file_index[filename.string()] = next_file;

        int const len = std::snprintf(buffer, sizeof(buffer)
            , "# %d %s\n", next_file, filename.string().c_str());
        ::write(fd, buffer, len);
        return next_file++;
    }();

    // timestamp (ms), start-offset, end-offset, operation (0 = read, 1 = write), file_index
    int const len = std::snprintf(buffer, sizeof(buffer)
        , "%" PRId64 "\t%" PRIu64 "\t%" PRIu64 "\t%d\t%d\n"
        , std::chrono::duration_cast<std::chrono::milliseconds>(timestamp).count()
        , offset
        , offset + length
        , int(op)
        , index);
    ::write(fd, buffer, len);
    ::close(fd);
}
#endif

struct FileDisk {
    explicit FileDisk(const std::string &filename);

    void Open(uint8_t flags = 0);

    FileDisk(FileDisk &&fd);

    // FileDisk(const FileDisk &) = delete;
    // FileDisk &operator=(const FileDisk &) = delete;

    void Close();

    ~FileDisk();

    void Read(uint64_t begin, uint8_t *memcache, uint64_t length);

    void Write(uint64_t begin, const uint8_t *memcache, uint64_t length);

    std::string GetFileName();

    uint64_t GetWriteMax();

    void Truncate(uint64_t new_size);

private:

    uint64_t readPos = 0;
    uint64_t writePos = 0;
    uint64_t writeMax = 0;
    bool bReading = true;

    std::string filename_;
    FILE *f_ = nullptr;

    static const uint8_t writeFlag = 0b01;
    static const uint8_t retryOpenFlag = 0b10;
};

#endif