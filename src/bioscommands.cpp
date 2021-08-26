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
#include <common.hpp>
#include <unistd.h>
#include <boost/endian/arithmetic.hpp>
#include <file_handling.hpp>
#include <string.h>
#include <fcntl.h>
#include <ipmid/api.h>
#include <cstdint>
#include <cstdio>
#include <functional>

uint8_t op;
int boot_check;
uint32_t boot_count;
uint32_t readVal;

namespace ipmi
{

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

    sysopen(eeprom_path,eeprom_offset);
    op = reqParams[0];
    uint8_t checker;
    readBin(&checker,4,sizeof(checker));
    boot_check = (int)checker;

    if(op == OP_CODE_READ && boot_check != 1)
    {
        boot_count = 0x00000000;
        return ipmi::responseSuccess(boot_count);
    }

    else if (op == OP_CODE_READ && boot_check == 1)
    {
        readBin(&readVal,0,sizeof(readVal));
        return ipmi::responseSuccess(readVal);
    }

    else if (op == OP_CODE_CLEAR)
    {
        uint32_t default_val = 0xffffffff;
        uint8_t clear_set = 0xff;
        writeBin(&default_val,0,sizeof(default_val));
        writeBin(&clear_set,4,sizeof(clear_set));
        return ipmi::responseSuccess(default_val);

    }

    else if (op == OP_CODE_WRITE && boot_check != 1)
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
            memcpy(&boot_count,&reqParams[1],sizeof(uint32_t));
            writeBin(&boot_count,0,sizeof(boot_count));
            writeBin(&boot_set,4,sizeof(boot_set));
        }
    }

    else if (op == OP_CODE_WRITE && boot_check == 1)
    {
        readBin(&readVal,0,sizeof(readVal));
        if (reqParams.size() == 1)
        {
            boot_count = readVal + 1;
        }

        else if (reqParams.size() == FII_CMD_BIOS_BOOT_COUNT_LEN)
        {
            memcpy(&boot_count,&reqParams[1],sizeof(uint32_t));
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
}

