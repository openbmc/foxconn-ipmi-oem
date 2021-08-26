#include <iostream>
#include <string.h>
#include <boost/endian/arithmetic.hpp>
#include "sys_file_impl.hpp"

uint8_t op;
std::vector<uint8_t> boot_count;
int boot_check;
std::vector<uint8_t>clear_count;
uint32_t value = 0;

std::vector<uint8_t> boot_count_set(uint8_t x, uint8_t y)
{
    boot_count.clear();
    boot_count.push_back(x);
    boot_count.push_back(y);
    boot_count.push_back(y);
    boot_count.push_back(y);
    return boot_count;
}

auto file = std::make_unique<binstore::SysFileImpl>("/sys/bus/i2c/devices/4-0050/eeprom",
                                                    4096);
std::vector<uint8_t> read_boot_count()
{
   boost::endian::little_uint32_t size = 0;
   std::string readStr=file->readAsStr(0,sizeof(size));
   boot_count.clear();
   char readStrCh[readStr.length()];
   for (int i=0;i<sizeof(readStrCh);i++)
   {
       readStrCh[i] = readStr[i];
       readStrCh[i] = (uint8_t)readStrCh[i];
       boot_count.push_back(readStrCh[i]);
   }
   return boot_count;
}

std::string check_byte_set(uint8_t x)
{
    std::vector<uint8_t> set_byte;
    set_byte.push_back(x);
    std::string set_check_byte(set_byte.begin(),set_byte.end());
    return set_check_byte;
}
