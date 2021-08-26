/********************************************************************************
*                       HON HAI Precision IND.Co., LTD.                         *
*            Personal Computer & Enterprise Product Business Group              *
*                      Enterprise Product Business Gro:qup                      *
*                                                                               *
*     Copyright (c) 2010 by FOXCONN/CESBG/CABG/SRD. All rights reserved.        *
*     All data and information contained in this document is confidential       *
*     and proprietary information of FOXCONN/CESBG/CABG/SRD and all rights      *
*     are reserved. By accepting this material the recipient agrees that        *
*     the information contained therein is held in confidence and in trust      *
*     and will not be used, copied, reproduced in whole or in part, nor its     *
*     contents revealed in any manner to others without the express written     *
*     permission of FOXCONN/CESBG/CABG/SRD.                                     *
*                                                                               *
********************************************************************************/

#include <common.hpp>
#include <bioscommands.hpp>
#include <memory>
#include <unistd.h>
#include <boost/endian/arithmetic.hpp>
#include <string.h>
#include <fcntl.h>

using namespace std::string_literals;

int fd_;
size_t offset_;
uint8_t op;
int boot_check;
uint32_t value = 0;
uint32_t boot_count; 
std::string eeprom_path = "/sys/bus/i2c/devices/4-0050/eeprom";

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

size_t readToBuf(size_t pos, size_t count, char* buf)
{
    lseek(pos);
    size_t bytesRead = 0;
    do
    {
        auto ret = read(fd_, &buf[bytesRead], count - bytesRead);
        if (ret < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }

            throw errnoException("Error reading from file"s);
        }
        else if (ret > 0)
        {
            bytesRead += ret;
        }
        else // ret == 0
        {
            break;
        }
    } while (bytesRead < count);
    return bytesRead;
}

void write32Bin(void *ptr, size_t pos)
{
    lseek(pos);
    ssize_t ret;
    ret = write(fd_,ptr,sizeof(*(uint32_t *)ptr));
    if (ret < 0)
    {
        throw errnoException("Error writing to file"s);
    }
    if (static_cast<size_t>(ret) != sizeof(*(uint32_t *)ptr))
    {
        throw std::runtime_error(
                "Tried to send data size "s + std::to_string(sizeof(*(uint32_t *)ptr)) +
                " but could only send "s + std::to_string(ret));
    }
}

void write8Bin(void *ptr, size_t pos)
{
    lseek(pos);
    ssize_t ret;
    ret = write(fd_,ptr,sizeof(*(uint8_t *)ptr));
    if (ret < 0)
    {
        throw errnoException("Error writing to file"s);
    }
    if (static_cast<size_t>(ret) != sizeof(*(uint8_t *)ptr))
    {
        throw std::runtime_error(
                "Tried to send data size "s + std::to_string(sizeof(*(uint8_t *)ptr)) +
                " but could only send "s + std::to_string(ret));
    }
}

namespace ipmi
{
    static void registerBIOSFunctions() __attribute__((constructor));

    ipmi::RspType<uint32_t> FiiBIOSBootCount(boost::asio::yield_context yield, std::vector<uint8_t> reqParams)
    {
        uint32_t counter = 0, ret;

        if (reqParams.empty())
        {
            phosphor::logging::log<phosphor::logging::level::ERR>(" Fii bios cmd : command format error.");

            return ipmi::responseReqDataLenInvalid();
        }

        sysopen(eeprom_path,4096);
        op = reqParams[0] & 0b11;
        boost::endian::little_uint8_t checker = 0;
        readToBuf(4, sizeof(checker), reinterpret_cast<char*>(&checker));
        boot_check = (int)checker;

        if(boot_check != 1 && op == OP_CODE_READ)
        {
            boot_count = 0x00000000;
            return ipmi::responseSuccess(boot_count);
        }

        else if (op == OP_CODE_READ && boot_check == 1)
        {
            boost::endian::little_uint32_t checker = 0;
            readToBuf(0, sizeof(checker), reinterpret_cast<char*>(&checker));
            return ipmi::responseSuccess(checker);
        }

        else if (op == OP_CODE_CLEAR)
        {
            uint32_t default_val = 0xffffffff;
            uint8_t clear_set = 0xff;
            write32Bin(&default_val,0);
            write8Bin(&clear_set,4);
            return ipmi::responseSuccess(default_val);

        }
        else if (op == OP_CODE_WRITE && boot_check != 1)
        {
            boost::endian::little_uint32_t checker = 0;
            readToBuf(0, sizeof(checker), reinterpret_cast<char*>(&checker));
            uint8_t boot_set = 0x01;
            std::cout << "Araara " << "boot set is  " << boot_set << std::endl;
            if (reqParams.size() == 1)
            {
                boot_count = 0x01;
                write32Bin(&boot_count,0);
                write8Bin(&boot_set,4);
            }
            else if (reqParams.size() == FII_CMD_BIOS_BOOT_COUNT_LEN)
            {
                value = reqParams[1] + + (reqParams[2] << 8) + (reqParams[3] << 16) + (reqParams[4] << 24);
                boot_count = value;
                write32Bin(&boot_count,0);
                write8Bin(&boot_set,4);
            }
        }
        else if (op == OP_CODE_WRITE && boot_check == 1)
        {
            boost::endian::little_uint32_t checker = 0;
            readToBuf(0, sizeof(checker), reinterpret_cast<char*>(&checker));
            if (reqParams.size() == 1)
            {
                value = checker + 1;
                boot_count = value;
            }
            else if (reqParams.size() == FII_CMD_BIOS_BOOT_COUNT_LEN)
            {
                value = reqParams[1] + + (reqParams[2] << 8) + (reqParams[3] << 16) + (reqParams[4] << 24);
                boot_count = value;
            }
            write32Bin(&boot_count,0);
        }
        else
        {
            return ipmi::responseInvalidCommand();
        }
        return ipmi::responseSuccess(boot_count);
    }

    void registerBIOSFunctions()
    {
        std::fprintf(stderr, "Registering OEM:[0x34], Cmd:[%#04X] for Fii BIOS OEM Commands\n", FII_CMD_BIOS_BOOT_COUNT);
        ipmi::registerHandler(ipmi::prioOemBase, ipmi::netFnOemThree, FII_CMD_BIOS_BOOT_COUNT, ipmi::Privilege::User,
                FiiBIOSBootCount);

        return;
    }
}

