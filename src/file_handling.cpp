#include <iostream>
#include <fcntl.h>
#include <unistd.h>

int fd_;
size_t offset_;
using namespace std::string_literals;
std::system_error errnoException(const std::string& message)
{
    return std::system_error(errno, std::generic_category(), message);
}

void sysopen(const std::string& path, size_t offset)
{
    fd_ = open(path.c_str(), O_RDWR);
    offset_ = offset;
    if (fd_ < 0)
    {
        throw errnoException("Error opening file "s + path);
    }
}

void lseek(size_t pos)
{
    if (lseek(fd_, offset_ + pos, SEEK_SET) < 0)
    {
        throw errnoException("Cannot lseek to pos "s + std::to_string(pos));
    }
}

void readBin(void *ptr, size_t pos, size_t size)
{
    lseek(pos);
    size_t ret = read(fd_,ptr,size);
    if(ret < 0 )
    {
        throw errnoException("Error reading from file"s);
    }
}

void writeBin(void *ptr, size_t pos, size_t size)
{
    lseek(pos);
    ssize_t ret;
    ret = write(fd_,ptr,size);
    if (ret < 0)
    {
        throw errnoException("Error writing to file"s);
    }
    if (static_cast<size_t>(ret) != size)
    {
        throw std::runtime_error(
                "Tried to send data size "s + std::to_string(size) +
                " but could only send "s + std::to_string(ret));
    }
}
