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
#include <ipmid/api.h>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <ipmid/iana.hpp>
#include <ipmid/oemrouter.hpp>

using namespace std::string_literals;

int fd_;
size_t offset_;
uint8_t op;
int boot_check;
uint32_t boot_count; 
std::string eeprom_path = "/sys/bus/i2c/devices/4-0050/eeprom";

/* Start of helper functions to read and write into eeprom /sys/bus/i2c/
devices/4-0050/eeprom */

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

/////////////////////////////////////////////////////////////////////////////
/* End of helper functions to read and write into eeprom /sys/bus/i2c/
devices/4-0050/eeprom */
////////////////////////////////////////////////////////////////////////////

namespace ipmi
{

/* Start of IPMI command which uses the Netfunction - 0x34 and command - 0x71 */

    static void registerBIOSFunctions() __attribute__((constructor));

    ipmi::RspType<uint32_t> FiiBIOSBootCount(boost::asio::yield_context yield, 
            std::vector<uint8_t> reqParams)
    {
        if (reqParams.empty())
        {
            phosphor::logging::log<phosphor::logging::level::ERR>(
                    " Fii bios cmd : command format error.");
            return ipmi::responseReqDataLenInvalid();
        }

        sysopen(eeprom_path,4096);
        op = reqParams[0] & 0b11;
        boost::endian::little_uint8_t checker = 0;
        readToBuf(4, sizeof(checker), reinterpret_cast<char*>(&checker));
        boot_check = (int)checker;

        if(op == OP_CODE_READ && boot_check == 255)
        {
            boot_count = 0x00000000;
            return ipmi::responseSuccess(boot_count);
        }

        else if (op == OP_CODE_READ && boot_check == 1)
        {
            boost::endian::little_uint32_t read_buffer = 0;
            readToBuf(0, sizeof(read_buffer), reinterpret_cast<char*>(&read_buffer));
            return ipmi::responseSuccess(read_buffer);
        }

        else if (op == OP_CODE_CLEAR)
        {
            uint32_t default_val = 0xffffffff;
            uint8_t clear_set = 0xff;
            writeBin(&default_val,0,sizeof(default_val));
            writeBin(&clear_set,4,sizeof(clear_set));
            return ipmi::responseSuccess(default_val);

        }

        else if (op == OP_CODE_WRITE && boot_check == 255)
        {
            uint8_t boot_set = 0x01; 
            if (reqParams.size() == 1)
            {
                boot_count = 0x01;
                writeBin(&boot_count,0,sizeof(boot_count));
                writeBin(&boot_set,4,sizeof(boot_set));
            }

            else if (reqParams.size() == FII_CMD_BIOS_BOOT_COUNT_LEN)
            {
                boot_count = reqParams[1] + + (reqParams[2] << 8) + (reqParams[3] << 16)
                    + (reqParams[4] << 24);
                writeBin(&boot_count,0,sizeof(boot_count));
                writeBin(&boot_set,4,sizeof(boot_set));
            }
        }

        else if (op == OP_CODE_WRITE && boot_check == 1)
        {
            boost::endian::little_uint32_t read_buffer = 0;
            readToBuf(0, sizeof(read_buffer), reinterpret_cast<char*>(&read_buffer)); 
            if (reqParams.size() == 1)
            {
                boot_count = read_buffer + 1;
            }

            else if (reqParams.size() == FII_CMD_BIOS_BOOT_COUNT_LEN)
            {
                boot_count = reqParams[1] + + (reqParams[2] << 8) + (reqParams[3] << 16)
                    + (reqParams[4] << 24);
            }
            writeBin(&boot_count,0,sizeof(boot_count));
        }

        else
        {
            return ipmi::responseInvalidCommand();
        }

        return ipmi::responseSuccess(boot_count);
    }

    void registerBIOSFunctions()
    {
        std::fprintf(stderr, "Registering OEM:[0x34], Cmd:[%#04X] for Fii BIOS OEM Commands\n",
                FII_CMD_BIOS_BOOT_COUNT);
        ipmi::registerHandler(ipmi::prioOemBase, ipmi::netFnOemThree, FII_CMD_BIOS_BOOT_COUNT,
                ipmi::Privilege::User, FiiBIOSBootCount);

        return;
    }
/////////////////////////////////////////////////////////////////////////////////////
/*End of IPMI command which uses the Netfunction - 0x34 and command - 0x71 */
/////////////////////////////////////////////////////////////////////////////////////

/*Start of exteded IPMI OEM command which uses the oem, IANA number format */

    struct biosrequest
    {
        uint8_t subcommand;
        uint8_t op;
    };

    struct biossetrequest
    {
        uint8_t subcommand;
        uint8_t op;
        uint8_t set_byte1;
        uint8_t set_byte2;
        uint8_t set_byte3;
        uint8_t set_byte4;
    };

    struct biosreply
    {
        uint8_t subcommand;
        uint8_t count1;
        uint8_t count2;
        uint8_t count3;
        uint8_t count4;
    };

    ipmi_ret_t FiiOemCount(const uint8_t* reqBuf, 
            uint8_t* replyBuf, size_t* dataLen)
    {
        struct biosrequest request;
        struct biossetrequest setrequest;
        struct biosreply reply;
        uint8_t boot_set;
        sysopen(eeprom_path,4096);
        boost::endian::little_uint8_t checker = 0;
        readToBuf(4, sizeof(checker), reinterpret_cast<char*>(&checker));
        boot_check = (int)checker;
        
        if ((*dataLen) < sizeof(request))
        {
            phosphor::logging::log<phosphor::logging::level::ERR>(
                    " Fii bios cmd : command format error.");
            return IPMI_CC_REQ_DATA_LEN_INVALID;
        }
        
        if((*dataLen) == sizeof(request))
        {
            std::memcpy(&request, &reqBuf[0], sizeof(request));
            if(request.op == 0 && boot_check == 1)
            {
                boost::endian::little_uint32_t read_buffer = 0;
                readToBuf(0, sizeof(read_buffer), reinterpret_cast<char*>(&read_buffer));
                reply.subcommand = 0x00;
                reply.count1 = static_cast<uint8_t>(read_buffer);
                reply.count2 = static_cast<uint8_t>(read_buffer >> 8);
                reply.count3 = static_cast<uint8_t>(read_buffer >> 16);
                reply.count4 = static_cast<uint8_t>(read_buffer >> 24);
                std::memcpy(&replyBuf[0], &reply, sizeof(reply));
                (*dataLen) = sizeof(reply);
                return IPMI_CC_OK;
            }
            
            else if(request.op == 0 && boot_check == 255)
            {
                boost::endian::little_uint32_t read_buffer = 0;
                readToBuf(0, sizeof(read_buffer), reinterpret_cast<char*>(&read_buffer));
                reply.subcommand = 0x00;
                reply.count1 = 0x00;
                reply.count2 = 0x00;
                reply.count3 = 0x00;
                reply.count4 = 0x00;
                std::memcpy(&replyBuf[0], &reply, sizeof(reply));
                (*dataLen) = sizeof(reply);
                return IPMI_CC_OK;
            }
            
            else if(request.op == 2)
            {
                boot_count = 0xffffffff;
                boot_set = 0xff;
                reply.subcommand = 0x00;
                reply.count1 = 0xff;
                reply.count2 = 0xff;
                reply.count3 = 0xff;
                reply.count4 = 0xff;
                writeBin(&boot_count,0,sizeof(boot_count));
                writeBin(&boot_set,4,sizeof(boot_count));
                std::memcpy(&replyBuf[0], &reply, sizeof(reply));
                (*dataLen) = sizeof(reply);
                return IPMI_CC_OK;
            }
            
            else if(request.op == 1 && boot_check == 1)
            {
                boost::endian::little_uint32_t read_buffer = 0;
                readToBuf(0, sizeof(read_buffer), reinterpret_cast<char*>(&read_buffer));
                boot_count = read_buffer + 1;
                reply.subcommand = 0x00;
                reply.count1 = static_cast<uint8_t>(boot_count);
                reply.count2 = static_cast<uint8_t>(boot_count >> 8);
                reply.count3 = static_cast<uint8_t>(boot_count >> 16);
                reply.count4 = static_cast<uint8_t>(boot_count >> 24);
                writeBin(&boot_count,0,sizeof(boot_count));
                std::memcpy(&replyBuf[0], &reply, sizeof(reply));
                (*dataLen) = sizeof(reply);
                return IPMI_CC_OK;
            }

            else if(request.op == 1 && boot_check == 255)
            {
                boot_count = 0x00000001;
                boot_set = 0x01;
                reply.subcommand = 0x00;
                reply.count1 = 0x01;
                reply.count2 = 0x00;
                reply.count3 = 0x00;
                reply.count4 = 0x00;
                writeBin(&boot_count,0,sizeof(boot_count));
                writeBin(&boot_set,4,sizeof(boot_set));
                std::memcpy(&replyBuf[0], &reply, sizeof(reply));
                (*dataLen) = sizeof(reply);
                return IPMI_CC_OK;
            }
        }

        else if((*dataLen) == sizeof(setrequest))
        {
            std::memcpy(&setrequest, &reqBuf[0], sizeof(setrequest));
            if(setrequest.op == 1 && boot_check == 1)
            {
                reply.subcommand = 0x00;
                reply.count1 = setrequest.set_byte1;
                reply.count2 = setrequest.set_byte2;
                reply.count3 = setrequest.set_byte3;
                reply.count4 = setrequest.set_byte4;
                boot_count = reply.count1 + + (reply.count2 << 8) + (reply.count3 << 16)
                    + (reply.count4 << 24);
                writeBin(&boot_count,0,sizeof(boot_count));
                std::memcpy(&replyBuf[0], &reply, sizeof(reply));
                (*dataLen) = sizeof(reply);
                return IPMI_CC_OK;
            } 
            else if(setrequest.op == 1 && boot_check == 255)
            {
                boot_set = 0x01;
                reply.subcommand = 0x00;
                reply.count1 = setrequest.set_byte1;
                reply.count2 = setrequest.set_byte2;
                reply.count3 = setrequest.set_byte3;
                reply.count4 = setrequest.set_byte4;
                boot_count = reply.count1 + + (reply.count2 << 8) + (reply.count3 << 16)
                    + (reply.count4 << 24);
                writeBin(&boot_count,0,sizeof(boot_count));
                writeBin(&boot_set,4,sizeof(boot_set));
                std::memcpy(&replyBuf[0], &reply, sizeof(reply));
                (*dataLen) = sizeof(reply);
                return IPMI_CC_OK;
            }
        }
        else
            return IPMI_CC_REQ_DATA_LEN_INVALID;
    }

    ipmi_ret_t handlesyscommand(ipmi_cmd_t, const uint8_t* reqBuf, uint8_t* replyBuf,
            size_t* dataLen)
    {
        if ((*dataLen) < 1)
        {
            std::fprintf(stderr, "*dataLen too small: %u\n",
                    static_cast<uint32_t>(*dataLen));
            return IPMI_CC_REQ_DATA_LEN_INVALID;
        }
        if (reqBuf[0] == 0)
        {
            return FiiOemCount(reqBuf, replyBuf, dataLen);
        }
    }

    void setupfoxconnOemSysCommands() __attribute__((constructor));    
    void setupfoxconnOemSysCommands()
    {
        static Handler handlerImpl;
        oem::Router* oemRouter = oem::mutableRouter();
        std::fprintf(stderr,
                    "Registering OEM:[%#08X], Cmd:[%#04X] for Sys Commands\n",
                    oem::foxOemNumber, sysCmd);
        using namespace std::placeholders;
        oemRouter->registerHandler(
            oem::foxOemNumber, sysCmd,
            handlesyscommand);       
    }

/////////////////////////////////////////////////////////////////////////////
/* End of exteded IPMI OEM command which uses the oem, IANA number format */
/////////////////////////////////////////////////////////////////////////////

}

