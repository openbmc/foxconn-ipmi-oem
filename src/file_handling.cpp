#include <iostream>
#include <fcntl.h>
#include <unistd.h>

//size_t offset_;
using namespace std::string_literals;
std::system_error errnoException(const std::string& message)
{
    return std::system_error(errno, std::generic_category(), message);
}

int sysopen(const std::string& path)
{
    int fd_ = open(path.c_str(), O_RDWR);
    if (fd_ < 0)
    {
        throw errnoException("Error opening file "s + path);
    }
    return fd_;
}

void sysclose(int fd_)
{
    close(fd_);
}

void lseeker(int fd_, size_t offset, size_t pos)
{
    if (lseek(fd_, offset + pos, SEEK_SET) < 0)
    {
        throw errnoException("Cannot lseek to pos "s + std::to_string(pos));
    }
}

void readBin(int fd_, size_t offset, void *ptr, size_t pos, size_t size)
{
    lseeker(fd_, offset, pos);
    size_t ret = read(fd_, ptr, size);
    if(ret < 0 )
    {
        throw errnoException("Error reading from file"s);
    }
}

void writeBin(int fd_, size_t offset, void *ptr, size_t pos, size_t size)
{
    lseeker(fd_, offset, pos);
    ssize_t ret;
    ret = write(fd_, ptr, size);
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
