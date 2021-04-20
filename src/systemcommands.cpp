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

#include <stdio.h>

namespace ipmi
{
static void registerSystemFunctions() __attribute__((constructor));

ipmi::RspType<std::vector<uint8_t>> FiiSysPCIeInfo(boost::asio::yield_context yield)
{
	std::vector<uint8_t> rsp;
	char buffer[128], *token;
	uint32_t value;
	FILE *pipe = popen(PCIEINFO_COMMAND, "r");

	// Read pcie bifurcation information
	// it return two bytes, 1st byte bifurcation, 2nd byte present pin
	if (!pipe) throw std::runtime_error("popen() failed !!!");
	while (fgets(buffer, sizeof(buffer), pipe) != NULL)
	{
		std::cerr << " Command : " << buffer << std::endl;
	}
	pclose(pipe);

	token = std::strtok(buffer, " ");
	if (token == NULL)
	{
		phosphor::logging::log<phosphor::logging::level::ERR>(
			"Fii system cmd : Error geting PCIe Info came back null");
		ipmi::responseUnspecifiedError();
	}
	token = std::strtok(NULL, " ");
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

auto ipmiAppGetSystemIfCapabilities(uint8_t iface)
    -> ipmi::RspType<uint8_t, uint8_t, uint8_t, uint8_t>
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
	std::fprintf(stderr, "Registering OEM:[0x34], Cmd:[%#04X] for Fii System OEM Commands\n", FII_CMD_SYS_PCIE_INFO);
	ipmi::registerHandler(ipmi::prioOemBase, ipmi::netFnOemThree, FII_CMD_SYS_PCIE_INFO, ipmi::Privilege::User,
					FiiSysPCIeInfo);
	std::fprintf(stderr, "Registering APP:[0x06], Cmd:[%#04X] for Fii System OEM Commands\n", ipmi::app::cmdGetSystemIfCapabilities);
    ipmi::registerHandler(ipmi::prioOemBase, ipmi::netFnApp, ipmi::app::cmdGetSystemIfCapabilities, ipmi::Privilege::User,
                          ipmiAppGetSystemIfCapabilities);
	return;
}

}