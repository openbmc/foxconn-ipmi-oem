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
#include "sys_file_impl.hpp"
#include <memory>
#include <unistd.h>
#include <boost/endian/arithmetic.hpp>
#include <string.h>

namespace ipmi
{
    static void registerBIOSFunctions() __attribute__((constructor));

    ipmi::RspType<std::vector<uint8_t>> FiiBIOSBootCount(boost::asio::yield_context yield, std::vector<uint8_t> reqParams)
    {
        bool op;
        std::vector<uint8_t> boot_count;
        uint32_t counter = 0, ret;

        if (reqParams.empty())
        {
            phosphor::logging::log<phosphor::logging::level::ERR>(" Fii bios cmd : command format error.");

            return ipmi::responseReqDataLenInvalid();
        }

        op = reqParams[0] & 0b11;
        //create a object for the SysFileImpl with its path and offset
        auto file = std::make_unique<binstore::SysFileImpl>("/sys/bus/i2c/devices/4-0050/eeprom",
                                                            1000);
        boost::endian::little_uint32_t size = 0;
        std::string readStr=file->readAsStr(0,sizeof(size));

        char readStrCh[readStr.length()];
        for (int i=0;i<sizeof(readStrCh);i++)
        {
            readStrCh[i] = readStr[i];
            readStrCh[i] = (uint8_t)readStrCh[i];
            boot_count.push_back(readStrCh[i]);
        }

        if (op == OP_CODE_READ)
        {
            return ipmi::responseSuccess(boot_count);
        }
        else if (op == OP_CODE_WRITE)
        {
            uint32_t value = 0;
            if (reqParams.size() == 1)
            {
                value = boot_count[0] + (boot_count[1] << 8) + (boot_count[2] << 16) + (boot_count[3] << 24);
                value += 1;
                boot_count.clear();
                boot_count.push_back(static_cast<uint8_t>(value));
                boot_count.push_back(static_cast<uint8_t>(value >> 8));
                boot_count.push_back(static_cast<uint8_t>(value >> 16));
                boot_count.push_back(static_cast<uint8_t>(value >> 24));
            }
            else if (reqParams.size() == FII_CMD_BIOS_BOOT_COUNT_LEN)
            {
                value = reqParams[1] + + (reqParams[2] << 8) + (reqParams[3] << 16) + (reqParams[4] << 24);
                boot_count.clear();
                boot_count.insert(boot_count.begin(), reqParams.begin()+1, reqParams.end());
            }
            
            //convert the boot_count vector from uint8 to string
            std::string s(boot_count.begin(),boot_count.end());
            //write the data into EEPROM
            file->writeStr(s,0);
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
