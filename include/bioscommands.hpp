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

#pragma once
#define BOOT_COUNT_READ    0x00
#define BOOT_COUNT_WRITE   0x01
#define BOOT_COUNT_CLEAR   0x02
#define FII_CMD_BIOS_BOOT_COUNT 0x71
#define FII_CMD_BIOS_BOOT_COUNT_LEN 5
#define INITIAL_BOOT_COUNT_HEADER 0x00000001
#define DEFAULT_VALUE 0xffffffff
#define EEPROM_OFFSET 4096

std::string EEPROM_PATH = "/sys/bus/i2c/devices/4-0050/eeprom";
