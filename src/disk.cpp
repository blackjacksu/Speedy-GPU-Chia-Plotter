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


#include "../include/disk.h"

FileDisk::FileDisk(/*const std::string &filepath,*/ const std::string &filename)
{
    // filepath_ = filepath;
    filename_ = /*filepath +*/ filename;
    std::cout << "f name: " << filename_ << std::endl;
    Open(writeFlag);
}

void FileDisk::Open(uint8_t flags)
{
    // if the file is already open, don't do anything
    if (f_) return;
    // Opens the file for reading and writing
    do {
#ifdef _WIN32
        f_ = ::_wfopen(filename_.c_str(), (flags & writeFlag) ? L"w+b" : L"r+b");
#else
        f_ = ::fopen(filename_.c_str(), (flags & writeFlag) ? "w+b" : "r+b");
#endif
        if (f_ == nullptr) {
            std::string error_message =
                "Could not open " + filename_ + ": " + ::strerror(errno) + ".";
            if (flags & retryOpenFlag) {
                std::cout << error_message << " Retrying in five minutes." << std::endl;
                std::this_thread::sleep_for(5min);
            } else {
                // throw InvalidValueException(error_message);
                std::cout << "Exeception: " + error_message << std::endl;
            }
        }
    } while (f_ == nullptr);
}

FileDisk::FileDisk(FileDisk &&fd)
{
    filename_ = std::move(fd.filename_);
    f_ = fd.f_;
    fd.f_ = nullptr;
}

// FileDisk(const FileDisk &) = delete;
// FileDisk &operator=(const FileDisk &) = delete;
void FileDisk::Close()
{
    if (f_ == nullptr) return;
    ::fclose(f_);
    f_ = nullptr;
    readPos = 0;
    writePos = 0;
}
FileDisk::~FileDisk() 
{ 
    Close(); 
}

void FileDisk::Read(uint64_t begin, uint8_t *memcache, uint64_t length)
{
    Open(retryOpenFlag);
#if ENABLE_LOGGING
    disk_log(filename_, op_t::read, begin, length);
#endif
    // Seek, read, and replace into memcache
    uint64_t amtread;
    do {
        if ((!bReading) || (begin != readPos)) {
#ifdef _WIN32
            _fseeki64(f_, begin, SEEK_SET);
#else
            // fseek() takes a long as offset, make sure it's wide enough
            static_assert(sizeof(long) >= sizeof(begin));
            ::fseek(f_, begin, SEEK_SET);
#endif
            bReading = true;
        }
        amtread = ::fread(reinterpret_cast<char *>(memcache), sizeof(uint8_t), length, f_);
        readPos = begin + amtread;
        if (amtread != length) {
            std::cout << "Only read " << amtread << " of " << length << " bytes at offset "
                      << begin << " from " << filename_ << " with length " << writeMax
                      << ". Error " << ferror(f_) << ". Retrying in five minutes." << std::endl;
            // Close, Reopen, and re-seek the file to recover in case the filesystem
            // has been remounted.
            Close();
            bReading = false;
            std::this_thread::sleep_for(5min);
            Open(retryOpenFlag);
        }
    } while (amtread != length);
}
void FileDisk::Write(uint64_t begin, const uint8_t *memcache, uint64_t length)
{
    Open(writeFlag | retryOpenFlag);
#if ENABLE_LOGGING
    disk_log(filename_, op_t::write, begin, length);
#endif
    // Seek and write from memcache
    uint64_t amtwritten;
    do {
        if ((bReading) || (begin != writePos)) {
#ifdef _WIN32
            _fseeki64(f_, begin, SEEK_SET);
#else
            // fseek() takes a long as offset, make sure it's wide enough
            static_assert(sizeof(long) >= sizeof(begin));
            ::fseek(f_, begin, SEEK_SET);
#endif
            bReading = false;
        }
        amtwritten =
            ::fwrite(reinterpret_cast<const char *>(memcache), sizeof(uint8_t), length, f_);
        writePos = begin + amtwritten;
        if (writePos > writeMax)
            writeMax = writePos;
        if (amtwritten != length) {
            // If an error occurs, the resulting value of the file-position indicator for the stream is unspecified.
            // https://pubs.opengroup.org/onlinepubs/007904975/functions/fwrite.html
            //
            // And in the code above if error occurs with 0 bytes written (full disk) it will not reset the pointer
            // (writePos will still be equal to begin), however it need to be reseted.
            //
            // Otherwise this causes #234 - in phase3, when this bucket is read, it goes into endless loop.
            //
            // Thanks tinodj!
            writePos = UINT64_MAX;
            std::cout << "Only wrote " << amtwritten << " of " << length << " bytes at offset "
                      << begin << " to " << filename_ << " with length " << writeMax
                      << ". Error " << ferror(f_) << ". Retrying in five minutes." << std::endl;
            // Close, Reopen, and re-seek the file to recover in case the filesystem
            // has been remounted.
            Close();
            bReading = false;
            std::this_thread::sleep_for(5min);
            Open(writeFlag | retryOpenFlag);
        }
    } while (amtwritten != length);
}

std::string FileDisk::GetFileName() { return filename_; }

std::string FileDisk::GetPathName() { return filepath_; }

uint64_t FileDisk::GetWriteMax()
{ return writeMax; }
// void FileDisk::Truncate(uint64_t new_size)
// {
//     Close();
//     fs::resize_file(filename_, new_size);
// }

