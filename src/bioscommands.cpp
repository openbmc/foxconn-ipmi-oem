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
struct bios_boot_count
{
    uint32_t header;
    uint32_t count;
};

namespace ipmi
{
static void registerBIOSFunctions() __attribute__((constructor));
ipmi::RspType<uint32_t> FiiBIOSBootCount(boost::asio::yield_context yield,
        std::vector<uint8_t> reqParams)
{
    bios_boot_count boot;
    int fd = sysopen(EEPROM_PATH,EEPROM_OFFSET);
    readBin(fd,&boot,0,sizeof(boot));

    if (reqParams.empty())
    {
        phosphor::logging::log<phosphor::logging::level::ERR>(
                " Fii bios cmd : command format error.");
        return ipmi::responseReqDataLenInvalid();
    }

    boot_count_operation = reqParams[0];

    switch(boot_count_operation)
    {
        case BOOT_COUNT_READ:
        case BOOT_COUNT_INCREMENT:
        case BOOT_COUNT_CLEAR:
             if(reqParams.size() != OPERATION_BYTE_LENGTH)
                return ipmi::responseReqDataLenInvalid();
	     break;
        case BOOT_COUNT_SET:
             if(reqParams.size() != SET_BYTE_LENGTH)
                return ipmi::responseReqDataLenInvalid();
             break;
        default:
             return ipmi::responseInvalidFieldRequest();
    }

    switch(boot_count_operation)
    {
        case BOOT_COUNT_READ:
            if(boot.header != BOOT_COUNT_HEADER)
            {
                boot.count = INITIAL_VALUE;
            }
            break;
        case BOOT_COUNT_INCREMENT:
            if(boot.header != BOOT_COUNT_HEADER)
            {
                boot.header = BOOT_COUNT_HEADER;
                boot.count = START_BOOT_COUNT_VALUE;
                writeBin(fd,&boot,0,sizeof(boot));
            }
            else
            {
                boot.count = boot.count + 1;
                writeBin(fd,&boot.count,4,sizeof(boot.count));
            }
            break;
        case BOOT_COUNT_CLEAR:
            boot.header = DEFAULT_VALUE;
            boot.count = DEFAULT_VALUE;
            writeBin(fd,&boot,0,sizeof(boot));
            break;
        case BOOT_COUNT_SET:
	    memcpy(&boot.count,&reqParams[1],sizeof(boot.count));
            if(boot.header != BOOT_COUNT_HEADER)
            {
                boot.header = BOOT_COUNT_HEADER;
                writeBin(fd,&boot,0,sizeof(boot));
            }
            else
            {
                writeBin(fd,&boot.count,4,sizeof(boot.count));
            }
            break;
    }

    sysclose(fd);
    return ipmi::responseSuccess(boot.count);
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
