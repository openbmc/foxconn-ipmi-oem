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

#include <memory>
#include <sys/stat.h>
#include <iomanip>
#include <ipmid/api.hpp>
#include <ipmid/utils.hpp>
#include <ipmid/message.hpp>
#include <phosphor-logging/log.hpp>
#include <sdbusplus/message/types.hpp>
#include <bioscommands.hpp>
#include <unistd.h>
#include <boost/endian/arithmetic.hpp>
#include <file_handling.hpp>
#include <string.h>
#include <fcntl.h>
#include <ipmid/api.h>
#include <cstdint>
#include <cstdio>
#include <functional>

int boot_count_operation;
struct bios_boot
{
    uint32_t magic_head;
    uint32_t boot_count;
};

namespace ipmi
{
static void registerBIOSFunctions() __attribute__((constructor));
ipmi::RspType<uint32_t> FiiBIOSBootCount(boost::asio::yield_context yield,
        std::vector<uint8_t> reqParams)
{
    bios_boot boot;
    sysopen(EEPROM_PATH,EEPROM_OFFSET);
    readBin(&boot,0,sizeof(boot));
    
    if (reqParams.empty())
    {
        phosphor::logging::log<phosphor::logging::level::ERR>(
                " Fii bios cmd : command format error.");
        return ipmi::responseReqDataLenInvalid();
    }

    else if(reqParams.size() == 1)
    {
        boot_count_operation = reqParams[0];
        if(boot_count_operation == BOOT_COUNT_READ &&
                boot.magic_head != INITIAL_BOOT_COUNT_HEADER)
        {
            boot.boot_count = 0x00000000;
        }
        else if(boot_count_operation == BOOT_COUNT_READ &&
                boot.magic_head == INITIAL_BOOT_COUNT_HEADER)
        {
        } 
        else if(boot_count_operation == BOOT_COUNT_WRITE &&
                boot.magic_head != INITIAL_BOOT_COUNT_HEADER)
        {
            boot.magic_head = INITIAL_BOOT_COUNT_HEADER;
            boot.boot_count = 0x00000001;
            writeBin(&boot,0,sizeof(boot));
        }
        else if(boot_count_operation == BOOT_COUNT_WRITE &&
                boot.magic_head == INITIAL_BOOT_COUNT_HEADER)
        {
            boot.boot_count = boot.boot_count + 1;
            writeBin(&boot.boot_count,4,sizeof(boot.boot_count));
        }
        else if (boot_count_operation == BOOT_COUNT_CLEAR)
        {
            boot.magic_head = DEFAULT_VALUE;
            boot.boot_count = DEFAULT_VALUE;
            writeBin(&boot,0,sizeof(boot));
        }
        else
            return ipmi::responseParmOutOfRange();
    } 
 
    else if (reqParams.size() == FII_CMD_BIOS_BOOT_COUNT_LEN)
    {
        boot_count_operation = reqParams[0];
        if (boot_count_operation == BOOT_COUNT_WRITE &&
                boot.magic_head != INITIAL_BOOT_COUNT_HEADER)
        {
            memcpy(&boot.boot_count,&reqParams[1],sizeof(boot.boot_count));
            boot.magic_head = INITIAL_BOOT_COUNT_HEADER;
            writeBin(&boot,0,sizeof(boot));
        }
        else if (boot_count_operation == BOOT_COUNT_WRITE &&
                boot.magic_head == INITIAL_BOOT_COUNT_HEADER)
        {
            memcpy(&boot.boot_count,&reqParams[1],sizeof(boot.boot_count));
            writeBin(&boot.boot_count,4,sizeof(boot.boot_count));
        }
        else
            return ipmi::responseParmOutOfRange();
    }
    else
        return ipmi::responseReqDataLenInvalid();
    return ipmi::responseSuccess(boot.boot_count);
}

void registerBIOSFunctions()
{
    std::fprintf(stderr, "Registering OEM:[0x34], Cmd:[%#04X] for Fii BIOS OEM Commands\n",
            FII_CMD_BIOS_BOOT_COUNT);
    ipmi::registerHandler(ipmi::prioOemBase, ipmi::netFnOemThree, FII_CMD_BIOS_BOOT_COUNT,
            ipmi::Privilege::User, FiiBIOSBootCount);

    return;
}
}

