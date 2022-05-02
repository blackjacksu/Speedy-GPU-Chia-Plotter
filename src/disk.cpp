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


    FileDisk::FileDisk(const fs::path &filename)
    {
        filename_ = filename;
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
                    "Could not open " + filename_.string() + ": " + ::strerror(errno) + ".";
                if (flags & retryOpenFlag) {
                    std::cout << error_message << " Retrying in five minutes." << std::endl;
                    std::this_thread::sleep_for(5min);
                } else {
                    throw InvalidValueException(error_message);
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

    FileDisk::~FileDisk() { Close(); }

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

    std::string FileDisk::GetFileName() { return filename_.string(); }

    uint64_t FileDisk::GetWriteMax() { return writeMax; }

    void FileDisk::Truncate(uint64_t new_size)
    {
        Close();
        fs::resize_file(filename_, new_size);
    }


BufferedDisk::BufferedDisk(FileDisk* disk, uint64_t file_size) : disk_(disk), file_size_(file_size) {}
uint8_t const* BufferedDisk::Read(uint64_t begin, uint64_t length)
{
    assert(length < read_ahead);
    NeedReadCache();
    // all allocations need 7 bytes head-room, since
    // SliceInt64FromBytes() may overrun by 7 bytes
    if (read_buffer_start_ <= begin
        && read_buffer_start_ + read_buffer_size_ >= begin + length
        && read_buffer_start_ + read_ahead >= begin + length + 7)
    {
        // if the read is entirely inside the buffer, just return it
        return read_buffer_.get() + (begin - read_buffer_start_);
    }
    else if (begin >= read_buffer_start_ || begin == 0 || read_buffer_start_ == std::uint64_t(-1)) {
        // if the read is beyond the current buffer (i.e.
        // forward-sequential) move the buffer forward and read the next
        // buffer-capacity number of bytes.
        // this is also the case we enter the first time we perform a read,
        // where we haven't read anything into the buffer yet. Note that
        // begin == 0 won't reliably detect that case, sinec we may have
        // discarded the first entry and start at some low offset but still
        // greater than 0
        read_buffer_start_ = begin;
        uint64_t const amount_to_read = std::min(file_size_ - read_buffer_start_, read_ahead);
        disk_->Read(begin, read_buffer_.get(), amount_to_read);
        read_buffer_size_ = amount_to_read;
        return read_buffer_.get();
    }
    else {
        // ideally this won't happen
        std::cout << "Disk read position regressed. It's optimized for forward scans. Performance may suffer\n"
            << "   read-offset: " << begin
            << " read-length: " << length
            << " file-size: " << file_size_
            << " read-buffer: [" << read_buffer_start_ << ", " << read_buffer_size_ << "]"
            << " file: " << disk_->GetFileName()
            << '\n';
        static uint8_t temp[128];
        // all allocations need 7 bytes head-room, since
        // SliceInt64FromBytes() may overrun by 7 bytes
        assert(length <= sizeof(temp) - 7);
        // if we're going backwards, don't wipe out the cache. We assume
        // forward sequential access
        disk_->Read(begin, temp, length);
        return temp;
    }
}
void BufferedDisk::Write(uint64_t const begin, const uint8_t *memcache, uint64_t const length)
{
    NeedWriteCache();
    if (begin == write_buffer_start_ + write_buffer_size_) {
        if (write_buffer_size_ + length <= write_cache) {
            ::memcpy(write_buffer_.get() + write_buffer_size_, memcache, length);
            write_buffer_size_ += length;
            return;
        }
        FlushCache();
    }
    if (write_buffer_size_ == 0 && write_cache >= length) {
        write_buffer_start_ = begin;
        ::memcpy(write_buffer_.get() + write_buffer_size_, memcache, length);
        write_buffer_size_ = length;
        return;
    }
    disk_->Write(begin, memcache, length);
}
void BufferedDisk::Truncate(uint64_t const new_size) 
{
    FlushCache();
    disk_->Truncate(new_size);
    file_size_ = new_size;
    FreeMemory();
}
std::string BufferedDisk::GetFileName()  { return disk_->GetFileName(); }
void BufferedDisk::FreeMemory() 
{
    FlushCache();
    read_buffer_.reset();
    write_buffer_.reset();
    read_buffer_size_ = 0;
    write_buffer_size_ = 0;
}
void BufferedDisk::FlushCache()
{
    if (write_buffer_size_ == 0) return;
    disk_->Write(write_buffer_start_, write_buffer_.get(), write_buffer_size_);
    write_buffer_size_ = 0;
}
void BufferedDisk::NeedReadCache()
{
    if (read_buffer_) return;
    read_buffer_.reset(new uint8_t[read_ahead]);
    read_buffer_start_ = -1;
    read_buffer_size_ = 0;
}
void BufferedDisk::NeedWriteCache()
{
    if (write_buffer_) return;
    write_buffer_.reset(new uint8_t[write_cache]);
    write_buffer_start_ = -1;
    write_buffer_size_ = 0;
}
