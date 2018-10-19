### Sys Commands (0x32)

There are and will be a variety of sys specific commands.

#### Cablecheck - SubCommand 0x00

The cablecheck command checks whether the BMC is seeing traffic between itself
and the host's NIC.  Sys specifies which if_name is expected to be connected.
The BMC presently only checks traffic on the interface specified.  There are
now ethernet statistics available over IPMI, which can be checked directly in
lieu of this.

Request

|Byte(s) |Value  |Data
|--------|-------|----
|0x00|0x00|Subcommand
|0x01|If_name length|Where you expect the cable, eth0 or eth1, etc
|0x02 ... |The name|The string, not null-terminated

Response

|Byte(s) |Value  |Data
|--------|-------|----
|0x00|0x00|Subcommand
|0x01|0x00/0x01|0 for false, 1 for true

#### CpldVersion - SubCommand 0x01

Any CPLD on the system that can only be read directly via the BMC can have its
version exported to Sys via the cpld version command.

Request

|Byte(s) |Value  |Data
|--------|-------|----
|0x00|0x01|Subcommand
|0x01|CPLD ID|A one-byte identifier for the CPLD file to read, unsigned byte.


Response

|Byte(s) |Value  |Data
|--------|-------|----
|0x00|0x01|Subcommand
|0x01|Major|Major version
|0x02|Minor|Minor Version
|0x03|Sub 1|Third version number
|0x04|Sub 2|Fourth version number

**Per the above, if the version number doesn't fit in a byte it'll be cast to
size.**

#### GetNcsiEthDevice - SubCommand 0x02

The BMC itself must have hard-coded into the image, which ethernet device is
connected to the host NIC.  This is true also in the mapping of ethernet device
to channel number.  The channel number is used to configure the ethernet device
over IPMI, instead of the interface name.  This is because we leverage the
current IPMI command set to read and write the networking configuration.  Sys
can be programmed already to have this information in the board protobuf,
however, this information -- can be read from the BMC over IPMI.

Request

|Byte(s) |Value  |Data
|--------|-------|----
|0x00|0x02|Subcommand

Response

|Byte(s) |Value  |Data
|--------|-------|----
|0x00|0x02|Subcommand
|0x01|Channel number|The IPMI channel number for use with the network configuration commands (such as reading the MAC or IP address of the BMC).
|0x02|if_name length|The length of the if_name in bytes.
|0x03... |if_name|The interface name, not null-terminated

#### DelayedHardReset - SubCommand 0x03

Sys needs to be able to tell the BMC to reset the host but given a delay in
seconds.

Request

|Byte(s) |Value  |Data
|--------|-------|----
|0x00|0x03|Subcommand
|0x01..0x04| |Seconds to delay (uint32)

Response

|Byte(s) |Value  |Data
|--------|-------|----
|0x00|0x03|Subcommand
