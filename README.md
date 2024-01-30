### Sys Commands (0x32)

There are and will be a variety of sys specific commands.

#### Cablecheck - SubCommand 0x00

The cablecheck command checks whether the BMC is seeing traffic between itself
and the host's NIC. Sys specifies which if_name is expected to be connected. The
BMC presently only checks traffic on the interface specified. There are now
ethernet statistics available over IPMI, which can be checked directly in lieu
of this.

Request

| Byte(s)  | Value          | Data                                          |
| -------- | -------------- | --------------------------------------------- |
| 0x00     | 0x00           | Subcommand                                    |
| 0x01     | If_name length | Where you expect the cable, eth0 or eth1, etc |
| 0x02 ... | The name       | The string, not null-terminated               |

Response

| Byte(s) | Value     | Data                    |
| ------- | --------- | ----------------------- |
| 0x00    | 0x00      | Subcommand              |
| 0x01    | 0x00/0x01 | 0 for false, 1 for true |

#### CpldVersion - SubCommand 0x01

Any CPLD on the system that can only be read directly via the BMC can have its
version exported to Sys via the cpld version command.

Request

| Byte(s) | Value   | Data                                                            |
| ------- | ------- | --------------------------------------------------------------- |
| 0x00    | 0x01    | Subcommand                                                      |
| 0x01    | CPLD ID | A one-byte identifier for the CPLD file to read, unsigned byte. |

Response

| Byte(s) | Value | Data                  |
| ------- | ----- | --------------------- |
| 0x00    | 0x01  | Subcommand            |
| 0x01    | Major | Major version         |
| 0x02    | Minor | Minor Version         |
| 0x03    | Sub 1 | Third version number  |
| 0x04    | Sub 2 | Fourth version number |

**Per the above, if the version number doesn't fit in a byte it'll be cast to
size.**

#### GetEthDevice - SubCommand 0x02

The BMC itself must have hard-coded into the image, which ethernet device is
connected to the host NIC. This is true also in the mapping of ethernet device
to channel number. Alternatively, you can pass a specific interface name for
channel lookup. The channel number is used to configure the ethernet device over
IPMI, instead of the interface name. This is because we leverage the current
IPMI command set to read and write the networking configuration. Sys can be
programmed already to have this information in the board protobuf, however, this
information -- can be read from the BMC over IPMI.

Request

| Byte(s) | Value   | Data                                               |
| ------- | ------- | -------------------------------------------------- |
| 0x00    | 0x02    | Subcommand                                         |
| 0x01... | if_name | (optional) The interface name, not null-terminated |

Response

| Byte(s) | Value          | Data                                                                                                                        |
| ------- | -------------- | --------------------------------------------------------------------------------------------------------------------------- |
| 0x00    | 0x02           | Subcommand                                                                                                                  |
| 0x01    | Channel number | The IPMI channel number for use with the network configuration commands (such as reading the MAC or IP address of the BMC). |
| 0x02    | if_name length | The length of the if_name in bytes.                                                                                         |
| 0x03... | if_name        | The interface name, not null-terminated                                                                                     |

#### DelayedHardReset - SubCommand 0x03

Sys needs to be able to tell the BMC to reset the host but given a delay in
seconds.

Request

| Byte(s)    | Value | Data                      |
| ---------- | ----- | ------------------------- |
| 0x00       | 0x03  | Subcommand                |
| 0x01..0x04 |       | Seconds to delay (uint32) |

Response

| Byte(s) | Value | Data       |
| ------- | ----- | ---------- |
| 0x00    | 0x03  | Subcommand |

### GetPCIeSlotsCount - SubCommand 0x04

Sys can get the total number of PCIe slots from BMC using this command. When BMC
receives this command, BMC can enumerate over all the PCIe slots and create a
hashmap with all the available PCIe slot name - I2C bus number mappings. BMC can
then send the total number of PCIe slots as part of this command response.

Request

| Byte(s) | Value | Data       |
| ------- | ----- | ---------- |
| 0x00    | 0x04  | Subcommand |

Response

| Byte(s) | Value                      | Data                       |
| ------- | -------------------------- | -------------------------- |
| 0x00    | 0x04                       | Subcommand                 |
| 0x01    | Total number of PCIe slots | Total number of PCIe slots |

### GetPCIeSlotI2cBusMapping - SubCommand 0x05

If Sys gets N total slots as part of the above command, then Sys can send this
command N times with Entry IDs ranging from 0 to N - 1. Say, Sys sends this
command with Entry ID as 1, BMC can go and fetch the first PCIe slot name - I2C
bus number mapping from the hashmap created above and then send the PCIe slot
name and I2C bus number as part of the command response.

Request

| Byte(s) | Value    | Data                             |
| ------- | -------- | -------------------------------- |
| 0x00    | 0x05     | Subcommand                       |
| 0x01    | Entry ID | Entry ID ranging from 0 to N - 1 |

Response

| Byte(s) | Value                 | Data                                                     |
| ------- | --------------------- | -------------------------------------------------------- |
| 0x00    | 0x05                  | Subcommand                                               |
| 0x01    | I2C bus number        | The I2C bus number which is input to the above PCIe slot |
| 0x02    | PCIe slot name length | The PCIe slot name length                                |
| 0x03... | PCIe slot name        | The PCIe slot name without null terminator               |

### GetEntityName - SubCommand 0x06

Gsys can get the "Entity ID:Entity Instance" to Entity name mapping from BMC
using this command. When BMC receives this command, BMC can check the related
JSON file and then send the name for that particular entity as this command
response.

Request

| Byte(s) | Value           | Data            |
| ------- | --------------- | --------------- |
| 0x00    | 0x06            | Subcommand      |
| 0x01    | Entity ID       | Entity ID       |
| 0x02    | Entity Instance | Entity Instance |

Response

| Byte(s)             | Value                      | Data                                |
| ------------------- | -------------------------- | ----------------------------------- |
| 0x00                | 0x06                       | Subcommand                          |
| 0x01                | Entity name length (say N) | Entity name length                  |
| 0x02...0x02 + N - 1 | Entity name                | Entity name without null terminator |

### GetMachineName - SubCommand 0x07

The BMC parses /etc/os-release for the OPENBMC_TARGET_MACHINE field and returns
its value.

Request

| Byte(s) | Value | Data       |
| ------- | ----- | ---------- |
| 0x00    | 0x06  | Subcommand |

Response

| Byte(s)             | Value                     | Data                               |
| ------------------- | ------------------------- | ---------------------------------- |
| 0x00                | 0x06                      | Subcommand                         |
| 0x01                | Model name length (say N) | Model name length                  |
| 0x02...0x02 + N - 1 | Model name                | Model name without null terminator |

### HardResetOnShutdown - SubCommand 0x08

Tells the BMC to powercycle the next time the host shuts down.

Request

| Byte(s) | Value | Data       |
| ------- | ----- | ---------- |
| 0x00    | 0x08  | Subcommand |

Response

| Byte(s) | Value | Data       |
| ------- | ----- | ---------- |
| 0x00    | 0x08  | Subcommand |

### GetFlashSize - SubCommand 0x09

Request the physical size of the BMC flash.

Request

| Byte(s) | Value | Data       |
| ------- | ----- | ---------- |
| 0x00    | 0x09  | Subcommand |

Response

| Byte(s)     | Value      | Data       |
| ----------- | ---------- | ---------- |
| 0x00        | 0x09       | Subcommand |
| 0x01...0x04 | Flash size | Flash size |

### HostPowerOff - SubCommand 0x0A

Sys command needs to be able to let the BMC knows host attempt S5 shutdown, it
need power-off the Host gracefully and disable the watchdog with given time
delay.

Request

| Byte(s)    | Value | Data                      |
| ---------- | ----- | ------------------------- |
| 0x00       | 0x0A  | Subcommand                |
| 0x01..0x04 |       | Seconds to delay (uint32) |

Response

| Byte(s) | Value | Data       |
| ------- | ----- | ---------- |
| 0x00    | 0x0A  | Subcommand |

### AccelOobDeviceCount - SubCommand 0x0B

Query the number of available devices from the google-accel-oob service.

If not enough data is proveded, `IPMI_CC_REQ_DATA_LEN_INVALID` is returned.

Request

| Byte(s) | Value | Data       |
| ------- | ----- | ---------- |
| 0x00    | 0x0B  | Subcommand |

Response

| Byte(s)    | Value | Data                        |
| ---------- | ----- | --------------------------- |
| 0x00       | 0x0B  | Subcommand                  |
| 0x01..0x04 |       | Number of devices available |

### AccelOobDeviceName - SubCommand 0x0C

Query the name of a single device from the google-accel-oob service.

This name is used as the identifier for the AccelOobRead and AccelOobWrite
commands.

Index values start at zero and go up to (but don't include) the device count.

The name of the device is exactly as it appears in DBus, except for the common
"/com/google/customAccel/" prefix. This prefix is removed to reduce the size of
the IPMI packet.

DBus requires all element names to be non-empty strings of ASCII characters
"[A-Z][a-z][0-9]\_", seperated by ASCII '/'. Therefore, all device names will be
valid ASCII strings (1 byte/character).

For convenience, the name string is followed by a single 0x00 (NULL terminator)
byte which is not included as part of the length.

The length field (byte 5) is the number of bytes in the name string (not
including the trailing NULL terminator byte).

The maximum length for any name is 43 bytes (not including the trailing NULL).

If a name is longer than 43 bytes, `IPMI_CC_REQ_DATA_TRUNCATED` is returned.
These names will not be usable in the rest of the API. Changing the name
requires code changes to the `managed_acceld` service binary.

If not enough data is proveded, `IPMI_CC_REQ_DATA_LEN_INVALID` is returned.

If a name does not begin with the expected "/com/google/customAccel/" prefix,
`IPMI_CC_INVALID` is returned. This indicates a change in the DBus API for the
google-accel-oob service that requires a matching code change in the handler.

Request

| Byte(s) | Value | Data               |
| ------- | ----- | ------------------ |
| 0x00    | 0x0C  | Subcommand         |
| 0x05    |       | Length of the name |
| 0x06..n |       | Name of the device |

Response

| Byte(s)    | Value | Data                |
| ---------- | ----- | ------------------- |
| 0x00       | 0x0C  | Subcommand          |
| 0x01..0x04 |       | Index of the device |
| 0x05       |       | Length of the name  |
| 0x06..n    |       | Name of the device  |

### AccelOobRead - SubCommand 0x0D

Read a PCIe CSR from a device.

Length is the length of the name, in bytes.

The device name gets prepended with "/com/google/customAccel/" and sent to DBus.
This string must **NOT** have a trailing NULL terminator.

The token is an arbitrary byte that gets echoed back in the reply; it is not
interpreted by the service at all. This is used to disambiguate identical
requests so clients can check for lost transactions.

Address is the 64b PCIe address to read from.

Number of bytes is the size of the read, in bytes (max 8). The value is subject
to hardware limitations (both PCIe and ASIC), so it will generally be 1, 2, 4,
or 8.

The register data is always returned in 8 bytes (uint64) in little Endian order.
If fewer than than 8 bytes are read, the MSBs are padded with 0s.

On success, the response ends with the data read as a single uint64.

If the number of bytes requested would not fit in a single IPMI payload,
`IPMI_CC_REQUESTED_TOO_MANY_BYTES` is returned.

If not enough data is proveded, `IPMI_CC_REQ_DATA_LEN_INVALID` is returned.

Request

| Byte(s)   | Value | Data                                           |
| --------- | ----- | ---------------------------------------------- |
| 0x00      | 0x0D  | Subcommand                                     |
| 0x01      |       | Number of bytes in the device name             |
| 0x02..n   |       | Name of the device (from `AccelOobDeviceName`) |
| n+1       |       | Token                                          |
| n+2..n+10 |       | Address                                        |
| n+11      |       | Number of bytes                                |

Response

| Byte(s)    | Value | Data                                  |
| ---------- | ----- | ------------------------------------- |
| 0x00       | 0x0D  | Subcommand                            |
| 0x01       |       | Number of bytes in the device name    |
| 0x02..n    |       | Name of the device (no trailing NULL) |
| n+1        |       | Token                                 |
| n+2..n+10  |       | Address                               |
| n+11       |       | Number of bytes                       |
| n+12..n+20 |       | Data                                  |

### AccelOobWrite - SubCommand 0x0E

Write a PCIe CSR from a device.

All parameters are identical to AccelOobRead (above). The only difference is the
register data becomes an input parameter (in the Request) instead of an output
value (in the Response).

As with read, the register data must be 8 bytes (uint64) in little Endian order.
If fewer than 8 bytes will be written, only the LSBs will be read and the the
MSBs will be ignored.

All fields returned in the Response are simply a copy of the Request.

On success, `IPMI_CC_OK` is returned.

If not enough data is proveded, `IPMI_CC_REQ_DATA_LEN_INVALID` is returned.

Request

| Byte(s)    | Value | Data                                           |
| ---------- | ----- | ---------------------------------------------- |
| 0x00       | 0x0D  | Subcommand                                     |
| 0x01       |       | Number of bytes in the device name             |
| 0x02..n    |       | Name of the device (from `AccelOobDeviceName`) |
| n+1        |       | Token                                          |
| n+2..n+10  |       | Address                                        |
| n+11       |       | Number of bytes                                |
| n+12..n+20 |       | Data                                           |

Response

| Byte(s)    | Value | Data                                  |
| ---------- | ----- | ------------------------------------- |
| 0x00       | 0x0D  | Subcommand                            |
| 0x01       |       | Number of bytes in the device name    |
| 0x02..n    |       | Name of the device (no trailing NULL) |
| n+1        |       | Token                                 |
| n+2..n+10  |       | Address                               |
| n+11       |       | Number of bytes                       |
| n+12..n+20 |       | Data                                  |

### PCIe Bifurcation - SubCommand 0x0F

Sys command to return the highest level of bifurcation for the target PCIe Slot.

Request

| Byte(s) | Value          | Data                 |
| ------- | -------------- | -------------------- |
| 0x00    | 0x0F           | Subcommand           |
| 0x01    | PE slot number | Index of the PE slot |

Response

| Byte(s)            | Value             | Data                                                                              |
| ------------------ | ----------------- | --------------------------------------------------------------------------------- |
| 0x00               | 0x0F              | Subcommand                                                                        |
| 0x01               | Config length (N) | Number of bytes needed for the bifurcation config                                 |
| 0x02..0x02 + N - 1 | Lanes per device  | Each byte represents the number of lanes bonded together for each endpoint device |

### GetBmcMode - SubCommand 0x10

Request the operational mode of the BMC.

Request

| Byte(s) | Value | Data       |
| ------- | ----- | ---------- |
| 0x00    | 0x10  | Subcommand |

Response

| Byte(s) | Value            | Data                                                                                                          |
| ------- | ---------------- | ------------------------------------------------------------------------------------------------------------- |
| 0x00    | 0x10             | Subcommand                                                                                                    |
| 0x01    | Current BMC MODE | <ul><li>0 -> Non Bare Metal Mode</li><li>1 -> Bare Metal Mode</li><li>2 -> Bare Metal Cleaning Mode</li></ul> |

### LinuxBootDone - SubCommand 0x11

Notify the BMC that LinuxBoot is finished and will kexec into the OS
momentarily.

If in bare metal mode, the BMC will disable IPMI upon receiving this command, to
protect against a malicious OS. For this reason, the BMC may not respond to this
command.

If not in bare metal mode, this command has no effect.

Request

| Byte(s) | Value | Data       |
| ------- | ----- | ---------- |
| 0x00    | 0x11  | Subcommand |

Response (if applicable)

| Byte(s) | Value | Data       |
| ------- | ----- | ---------- |
| 0x00    | 0x11  | Subcommand |

### SysGetAccelVrSettings - SubCommand 0x12

Get the accel's VR setting value for the given chip and settings ID

Currently 3 settings are supported.
[0] IdleMode
[1] PowerBreak
[2] Loadline

On success, the response contains 2 bytes containing the setting value.

If not enough data is proveded, `IPMI_CC_REQ_DATA_LEN_INVALID` is returned.

Request

| Byte(s) | Value | Data       |
| ------- | ----- | ---------- |
| 0x00    | 0x12  | Subcommand |
| 0x01    |       | Chip ID    |
| 0x02    |       | SettingsID |

Response (if applicable)

| Byte(s)    | Value | Data           |
| ---------- | ----- | -------------- |
| 0x00       | 0x12  | Subcommand     |
| 0x01..0x02 | 0x12  | Settings Value |

### SysSetAccelVrSettings - SubCommand 0x13

Update the VR settings of a given accel device for a specific settings id.

Currently 3 settings are supported.
[0] IdleMode
[1] PowerBreak
[2] Loadline

The settings value parameter is a 2 byte value and is expected in
little endian format

On success, `IPMI_CC_OK` is returned.

If not enough data is proveded, `IPMI_CC_REQ_DATA_LEN_INVALID` is returned.

Request

| Byte(s)    | Value | Data           |
| ---------- | ----- | -------------- |
| 0x00       | 0x13  | Subcommand     |
| 0x01       |       | Chip ID        |
| 0x02       |       | Settings ID    |
| 0x03..0x04 | 0x13  | Settings Value |

Response (if applicable)

| Byte(s) | Value | Data       |
| ------- | ----- | ---------- |
| 0x00    | 0x13  | Subcommand |
