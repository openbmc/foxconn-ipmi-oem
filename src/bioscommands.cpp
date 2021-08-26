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
#include "bioshelper.hpp"
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
        uint32_t counter = 0, ret;

        if (reqParams.empty())
        {
            phosphor::logging::log<phosphor::logging::level::ERR>(" Fii bios cmd : command format error.");

            return ipmi::responseReqDataLenInvalid();
        }

        op = reqParams[0] & 0b11;
        boost::endian::little_uint8_t checker = 0;
        file->readToBuf(4, sizeof(checker), reinterpret_cast<char*>(&checker));
        boot_check = (int)checker; 
        
        if(boot_check == 255 && op == OP_CODE_READ)
        {
            boot_count = boot_count_set(0,0);
            return ipmi::responseSuccess(boot_count);
        }

        else if (op == OP_CODE_READ && boot_check == 1)
        {
            boot_count = read_boot_count();
            return ipmi::responseSuccess(boot_count);
        }
        else if (op == OP_CODE_CLEAR)
        {
            clear_count = boot_count_set(255,255);
            std::string clear_string = check_byte_set(255); 
            std::string default_set(clear_count.begin(),clear_count.end());
            file->writeStr(default_set,0);
            file->writeStr(clear_string,4);
            return ipmi::responseSuccess(clear_count);

        }
        else if (op == OP_CODE_WRITE && boot_check == 255)
        {
            boot_count = read_boot_count();
            std::string set_check = check_byte_set(1);
            if (reqParams.size() == 1)
            {
                boot_count = boot_count_set(1,0);
                std::string write_check(boot_count.begin(),boot_count.end());
                file->writeStr(write_check,0);
                file->writeStr(set_check,4);
            }
            else if (reqParams.size() == FII_CMD_BIOS_BOOT_COUNT_LEN)
            {
                value = reqParams[1] + + (reqParams[2] << 8) + (reqParams[3] << 16) + (reqParams[4] << 24);
                boot_count.clear();
                boot_count.insert(boot_count.begin(), reqParams.begin()+1, reqParams.end());
                std::string s(boot_count.begin(),boot_count.end());
                file->writeStr(s,0);
                file->writeStr(set_check,4);
            }
        }
        else if (op == OP_CODE_WRITE && boot_check == 1)
        {
            boot_count = read_boot_count();
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

