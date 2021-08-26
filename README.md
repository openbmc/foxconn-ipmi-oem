### Fii IPMI OEM Commands (0x34)

There are and will be a variety of sys specific commands.

### System Commands (0x h)

#### PCIe Bifurcation Command 0x03

The PCIe bifurcation command checks the type of pcei adapter instailled in
each slot.

Request

|Byte(s) |Value  |Data
|--------|-------|----
|0x00|0x03|Subcommand

Response

|Byte(s) |Value  |Data
|--------|-------|----
|0x00|0x03|Subcommand
|0x01|0x--|Bifurcation each bit identified the slot support x16(1b) or x8(0b)
|0x02|0x--|Present bit each bit identified PCIe adapter installed(0b) or not(1b)

Example
|# |Command |Example
|--------|-------|----
|1|Read the information|ipmitool 0x34 0x03

### BIOS Related Commands (0x7X)

#### BIOS Boot Count 0x71

This is a 32 bits register that provides bios pushs the boot counter to BMC, or 
get the boot counter from BMC.

Request
|Byte(s) |Value|Data
|--------|-----|----
|0x00|0x71|Subcommand
|0x01|0x--|Operation to be performed

|Byte1 |Operation
|------|---------
|0x00|Read boot count value
|0x01|Increment boot count value
|0x02|Clear boot count value to its default value
|0x03|Set the boot count with given 4 byte value

Byte(s) 2-5 - 32-bit Count value for 0x03 operation

Response
|Byte(s) |Value  |Data
|--------|-------|----
|0x00|--|Byte 0 of boot count
|0x01|--|Byte 1 of boot count
|0x02|--|Byte 2 of boot count
|0x03|--|Byte 3 of boot count

Example
|# |Command |Example
|--------|-------|----
|1|Read the boot count|ipmitool raw 0x34 0x71 0x00
|2|Increment the boot count by 1|ipmitool raw 0x34 0x71 0x01
|3|Clear the boot count to its default value|ipmitool raw 0x34 0x71 0x02
|4|Set the boot count to given value|ipmitool raw 0x34 0x71 0x03 byte0 byte1 byte2 byte3
