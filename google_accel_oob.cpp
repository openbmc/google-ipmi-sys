// Copyright 2022 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "google_accel_oob.hpp"

#include "commands.hpp"
#include "errors.hpp"
#include "handler.hpp"

#include <sdbusplus/bus.hpp>
#include <stdplus/print.hpp>

#include <cstdint>
#include <cstring>
#include <span>
#include <string>
#include <vector>

namespace google
{
namespace ipmi
{

#ifndef MAX_IPMI_BUFFER
#define MAX_IPMI_BUFFER 64
#endif

// token + address(8) + num_bytes + data(8) + len + NULL
constexpr size_t MAX_NAME_SIZE = MAX_IPMI_BUFFER - 1 - 8 - 1 - 8 - 1 - 1;

Resp accelOobDeviceCount(std::span<const uint8_t> data,
                         HandlerInterface* handler)
{
    struct Request
    {
    } __attribute__((packed));

    struct Reply
    {
        uint32_t count;
    } __attribute__((packed));

    if (data.size_bytes() < sizeof(Request))
    {
        stdplus::print(stderr, "AccelOob DeviceCount command too small: {}\n",
                       data.size_bytes());
        return ::ipmi::responseReqDataLenInvalid();
    }

    if (data.size_bytes() + sizeof(Reply) > MAX_IPMI_BUFFER)
    {
        stdplus::print(
            stderr,
            "AccelOob DeviceCount command too large for reply buffer: "
            "command={}B, payload={}B, max={}B\n",
            data.size_bytes(), sizeof(Reply), MAX_IPMI_BUFFER);
        return ::ipmi::responseReqDataLenExceeded();
    }

    uint32_t count = handler->accelOobDeviceCount();

    std::vector<uint8_t> replyBuf(sizeof(Reply));
    auto* reply = reinterpret_cast<Reply*>(replyBuf.data());
    reply->count = count;

    return ::ipmi::responseSuccess(SysOEMCommands::SysAccelOobDeviceCount,
                                   replyBuf);
}

Resp accelOobDeviceName(std::span<const uint8_t> data,
                        HandlerInterface* handler)
{
    struct Request
    {
        uint32_t index;
    } __attribute__((packed));

    struct Reply
    {
        uint8_t nameLength;
        char name[MAX_NAME_SIZE];
    } __attribute__((packed));

    if (data.size_bytes() < sizeof(Request))
    {
        stdplus::print(stderr, "AccelOob DeviceName command too small: {}\n",
                       data.size_bytes());
        return ::ipmi::responseReqDataLenInvalid();
    }

    if (data.size_bytes() + sizeof(Reply) > MAX_IPMI_BUFFER)
    {
        stdplus::print(
            stderr,
            "AccelOob DeviceName command too large for reply buffer: "
            "command={}B, payload={}B, max={}B\n",
            data.size_bytes(), sizeof(Reply), MAX_IPMI_BUFFER);
        return ::ipmi::responseReqDataLenExceeded();
    }

    auto* req = reinterpret_cast<const Request*>(data.data());
    std::string name = handler->accelOobDeviceName(req->index);

    if (name.size() > MAX_NAME_SIZE)
    {
        stdplus::print(stderr,
                       "AccelOob: name was too long. "
                       "'{}' len must be <= {}\n",
                       name, MAX_NAME_SIZE);
        return ::ipmi::responseReqDataTruncated();
    }

    std::vector<uint8_t> replyBuf(data.size_bytes() + sizeof(Reply));
    std::copy(data.begin(), data.end(), replyBuf.data());
    auto* reply = reinterpret_cast<Reply*>(replyBuf.data() + data.size_bytes());
    reply->nameLength = name.length();
    memcpy(reply->name, name.c_str(), reply->nameLength + 1);

    return ::ipmi::responseSuccess(SysOEMCommands::SysAccelOobDeviceName,
                                   replyBuf);
}

namespace
{

struct NameHeader
{
    uint8_t nameLength;
    char name[MAX_NAME_SIZE];
} __attribute__((packed));

// Reads the variable-length name from reqBuf and outputs the name and a pointer
// to the payload (next byte after name).
//
// Returns: =0: success.
//          >0: if dataLen is too small, returns the minimum buffers size.
//
// Params:
//    [in]  reqBuf      - the request buffer
//    [in]  dataLen     - the length of reqBuf, in bytes
//    [in]  payloadSize - the size of the expected payload
//    [out] name        - the name string
//    [out] payload     - pointer into reqBuf just after name
size_t ReadNameHeader(const uint8_t* reqBuf, size_t dataLen, size_t payloadSize,
                      std::string* name, const uint8_t** payload)
{
    constexpr size_t kNameHeaderSize = sizeof(NameHeader) - MAX_NAME_SIZE;

    auto* req_header = reinterpret_cast<const NameHeader*>(reqBuf);

    size_t minDataLen = kNameHeaderSize + payloadSize + req_header->nameLength;
    if (dataLen < minDataLen)
    {
        return minDataLen;
    }

    if (name)
    {
        *name = std::string(req_header->name, req_header->nameLength);
    }
    if (payload)
    {
        *payload = reqBuf + kNameHeaderSize + req_header->nameLength;
    }
    return 0;
}

} // namespace

Resp accelOobRead(std::span<const uint8_t> data, HandlerInterface* handler)
{
    struct Request
    {
        // Variable length header, handled by ReadNameHeader
        // uint8_t  nameLength;  // <= MAX_NAME_SIZE
        // char     name[nameLength];

        // Additional arguments
        uint8_t token;
        uint64_t address;
        uint8_t num_bytes;
    } __attribute__((packed));

    struct Reply
    {
        uint64_t data;
    } __attribute__((packed));

    stdplus::print(stderr,
                   "AccelOob Read command sizes: "
                   "command={}B, payload={}B, max={}B\n",
                   data.size_bytes(), sizeof(Reply), MAX_IPMI_BUFFER);

    std::string name;
    const uint8_t* payload;

    size_t min_size = ReadNameHeader(data.data(), data.size_bytes(),
                                     sizeof(Request), &name, &payload);
    if (min_size != 0)
    {
        stdplus::print(stderr, "AccelOob Read command too small: {}B < {}B\n",
                       data.size_bytes(), min_size);
        return ::ipmi::responseReqDataLenInvalid();
    }

    if (data.size_bytes() + sizeof(Reply) > MAX_IPMI_BUFFER)
    {
        stdplus::print(stderr,
                       "AccelOob Read command too large for reply buffer: "
                       "command={}B, payload={}B, max={}B\n",
                       data.size_bytes(), sizeof(Reply), MAX_IPMI_BUFFER);
        return ::ipmi::responseReqDataLenExceeded();
    }

    auto req = reinterpret_cast<const Request*>(payload);
    uint64_t r = handler->accelOobRead(name, req->address, req->num_bytes);

    std::vector<uint8_t> replyBuf(data.size_bytes() + sizeof(Reply));
    std::copy(data.begin(), data.end(), replyBuf.data());
    auto* reply = reinterpret_cast<Reply*>(replyBuf.data() + data.size_bytes());
    reply->data = r;

    return ::ipmi::responseSuccess(SysOEMCommands::SysAccelOobRead, replyBuf);
}

Resp accelOobWrite(std::span<const uint8_t> data, HandlerInterface* handler)
{
    struct Request
    {
        // Variable length header, handled by ReadNameHeader
        // uint8_t  nameLength;  // <= MAX_NAME_SIZE
        // char     name[nameLength];

        // Additional arguments
        uint8_t token;
        uint64_t address;
        uint8_t num_bytes;
        uint64_t data;
    } __attribute__((packed));

    struct Reply
    {
        // Empty
    } __attribute__((packed));

    std::string name{};
    const uint8_t* payload;

    size_t min_size = ReadNameHeader(data.data(), data.size_bytes(),
                                     sizeof(Request), &name, &payload);
    if (min_size != 0)
    {
        stdplus::print(stderr, "AccelOob Write command too small: {}B < {}B\n",
                       data.size_bytes(), min_size);
        return ::ipmi::responseReqDataLenInvalid();
    }

    if (data.size_bytes() + sizeof(Reply) > MAX_IPMI_BUFFER)
    {
        stdplus::print(stderr,
                       "AccelOob Write command too large for reply buffer: "
                       "command={}B, payload={}B, max={}B\n",
                       data.size_bytes(), sizeof(Reply), MAX_IPMI_BUFFER);
        return ::ipmi::responseReqDataLenExceeded();
    }

    auto req = reinterpret_cast<const Request*>(payload);
    handler->accelOobWrite(name, req->address, req->num_bytes, req->data);

    std::vector<uint8_t> replyBuf(data.size_bytes() + sizeof(Reply));
    std::copy(data.begin(), data.end(), replyBuf.data());

    return ::ipmi::responseSuccess(SysOEMCommands::SysAccelOobWrite, replyBuf);
}

Resp accelGetVrSettings(::ipmi::Context::ptr ctx, std::span<const uint8_t> data,
                        HandlerInterface* handler)
{
    uint16_t value;
    if (data.size_bytes() != 2)
    {
        stdplus::println(
            stderr,
            "accelGetVrSettings command has incorrect size: %zuB != %dB\n",
            data.size_bytes(), 2);
        return ::ipmi::responseReqDataLenInvalid();
    }

    try
    {
        value = handler->accelGetVrSettings(ctx, /*chip_id*/ data[0],
                                            /*settings_id*/ data[1]);
    }
    catch (const IpmiException& e)
    {
        return ::ipmi::response(e.getIpmiError());
    }
    return ::ipmi::responseSuccess(
        SysOEMCommands::SysGetAccelVrSettings,
        std::vector<uint8_t>{static_cast<uint8_t>(value),
                             static_cast<uint8_t>(value >> 8)});
}

Resp accelSetVrSettings(::ipmi::Context::ptr ctx, std::span<const uint8_t> data,
                        HandlerInterface* handler)
{
    if (data.size_bytes() != 4)
    {
        stdplus::println(
            stderr,
            "accelSetVrSettings command has incorrect size: %zuB != %dB\n",
            data.size_bytes(), 4);
        return ::ipmi::responseReqDataLenInvalid();
    }
    uint16_t value = static_cast<uint16_t>(data[2] | data[3] << 8);
    try
    {
        handler->accelSetVrSettings(ctx, /*chip_id*/ data[0],
                                    /*settings_id*/ data[1], /*value*/ value);
    }
    catch (const IpmiException& e)
    {
        return ::ipmi::response(e.getIpmiError());
    }
    return ::ipmi::responseSuccess(SysOEMCommands::SysSetAccelVrSettings,
                                   std::vector<uint8_t>{});
}
} // namespace ipmi
} // namespace google
