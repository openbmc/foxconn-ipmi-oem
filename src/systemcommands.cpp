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
#include <systemcommands.hpp>
#include <string>
#include <iostream>
#include <fstream>

namespace ipmi
    {
    static void registerSystemFunctions() __attribute__((constructor));

    ipmi::RspType<std::vector<uint8_t>> FiiSysPCIeInfo(boost::asio::yield_context yield)
    {
        std::vector<uint8_t> rsp;
        char *token;
        uint32_t value;

        std::ifstream infile;
        try {
            infile.open(PCIEINFO_REG);
        }
        catch (const std::ifstream::failure& e) {
            std::cerr << "Error opening/reading file" << std::endl;
        }
        std::stringstream strStream;

        strStream << infile.rdbuf();
        char *buffer = (char *) strStream.str().c_str();

        infile.close();

        while (1)   //Remove trailing white spaces
        {
            int len = std::strlen(buffer);
            if(buffer[len - 1] == ' ' || buffer[len - 1] == '\n')
                buffer[len - 1] = '\0';
            else
                break;
        }

        token = std::strtok(buffer, " ");
        if (token == NULL)
        {
            phosphor::logging::log<phosphor::logging::level::ERR>(
                "Fii system cmd : Error geting PCIe Info came back null");
            ipmi::responseUnspecifiedError();
        }
        while (token != NULL)
        {
            //std::cerr << " Command token: " << token << std::endl;
            value = std::stoul(token, nullptr, 16);
            //std::cerr << " Command value: " << value << ":" << std::hex << value << std::endl;
            rsp.push_back(static_cast<uint8_t>(value & 0xFF));
            token = std::strtok(NULL, " ");
        }

        return ipmi::responseSuccess(rsp);
    }

    void registerSystemFunctions()
    {
        std::fprintf(stderr, "Registering OEM:[0x34], Cmd:[%#04X] for Fii System OEM Commands\n", FII_CMD_SYS_PCIE_INFO);
        ipmi::registerHandler(ipmi::prioOemBase, ipmi::netFnOemThree, FII_CMD_SYS_PCIE_INFO, ipmi::Privilege::User,
                FiiSysPCIeInfo);

        return;
    }
}
