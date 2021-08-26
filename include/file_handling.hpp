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
#include <iostream>
#include <unistd.h>

#define eeprom_offset 4096

std::string eeprom_path = "/sys/bus/i2c/devices/4-0050/eeprom";

std::system_error errnoException(const std::string& message);

void sysopen(const std::string& path, size_t offset);

void lseek(size_t pos);

void readBin(void *ptr, size_t pos, size_t size);

void writeBin(void *ptr, size_t pos, size_t size);
