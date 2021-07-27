/********************************************************************************
*                       HON HAI Precision IND.Co., LTD.                         *
*            Personal Computer & Enterprise Product Business Group              *
*                      Enterprise Product Business Group                        *
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


#include <systemcommands.hpp>
#include <ipmid/api.hpp>
#include <phosphor-logging/log.hpp>
#include <sdbusplus/message/types.hpp>

namespace ipmi
{
static void registerSystemFunctions() __attribute__((constructor));

ipmi::RspType<uint8_t, uint8_t> FiiSysPCIeInfo(boost::asio::yield_context yield)
{
    uint8_t bifurcation, presence;

    std::ifstream infile;
    try {
        infile.open(PCIEINFO_REG);
    }
    catch (const std::ifstream::failure& e) {
        std::cerr << "Error opening/reading file" << std::endl;
    }
    std::string str;

    std::getline(infile, str);

    infile.close();

    if (str.empty()) {
        phosphor::logging::log<phosphor::logging::level::ERR>(
            "Fii system cmd : Error geting PCIe Info came back null");
        ipmi::responseUnspecifiedError();
    }

    std::sscanf(str.c_str(), "%x %x", &bifurcation, &presence);

    return ipmi::responseSuccess(bifurcation, presence);
}

ipmi::RspType<uint8_t, uint8_t, uint8_t, uint8_t> ipmiAppGetSystemIfCapabilities(uint8_t iface)
{

    // Per IPMI 2.0 spec, the input and output buffer size must be the max
    // buffer size minus one byte to allocate space for the length byte.
    constexpr uint8_t reserved = 0x00;
    constexpr uint8_t support = 0b10000000;
    constexpr uint8_t inMsgSize = 240;
    constexpr uint8_t outMsgSize = 240;

    return ipmi::responseSuccess(reserved, support,
                                inMsgSize, outMsgSize);
}

void registerSystemFunctions()
{
    std::fprintf(stderr, "Registering OEM:[0x34], Cmd:[%#04X] for Fii System OEM Commands\n",
        FII_CMD_SYS_PCIE_INFO);
    ipmi::registerHandler(ipmi::prioOemBase, ipmi::netFnOemThree, FII_CMD_SYS_PCIE_INFO,
        ipmi::Privilege::User, FiiSysPCIeInfo);

    std::fprintf(stderr, "Registering APP:[0x06], Cmd:[%#04X] for Fii System OEM Commands\n",
        ipmi::app::cmdGetSystemIfCapabilities);
    ipmi::registerHandler(ipmi::prioOemBase, ipmi::netFnApp, ipmi::app::cmdGetSystemIfCapabilities,
        ipmi::Privilege::User, ipmiAppGetSystemIfCapabilities);

}
}
