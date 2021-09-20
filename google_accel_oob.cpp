#include "google_accel_oob.hpp"

#include "commands.hpp"

#include <cstdint>
#include <cstring>
#include <sdbusplus/bus.hpp>
#include <string>
#include <tuple>
#include <vector>

namespace google
{
namespace ipmi
{

#ifndef MAX_IPMI_BUFFER
#define MAX_IPMI_BUFFER 64
#endif

// subcommand + token + address(8) + num_bytes + data(8) + len + NULL
constexpr size_t MAX_NAME_SIZE = MAX_IPMI_BUFFER - 1 - 1 - 8 - 1 - 8 - 1 - 1;

ipmi_ret_t accelOobDeviceCount(const uint8_t* reqBuf, uint8_t* replyBuf,
                               size_t* dataLen, HandlerInterface* handler)
{
    struct Request
    {
        uint8_t subcommand;
    } __attribute__((packed));

    struct Reply
    {
        uint32_t count;
    } __attribute__((packed));

    if (*dataLen < sizeof(Request))
    {
        std::fprintf(stderr, "AccelOob DeviceCount command too small: %zu\n",
                     *dataLen);
        return IPMI_CC_REQ_DATA_LEN_INVALID;
    }

    if (*dataLen + sizeof(Reply) > MAX_IPMI_BUFFER)
    {
        std::fprintf(stderr,
                     "AccelOob DeviceCount command too large for reply buffer: "
                     "command=%zuB, payload=%zuB, max=%dB\n",
                     *dataLen, sizeof(Reply), MAX_IPMI_BUFFER);
        return IPMI_CC_REQUESTED_TOO_MANY_BYTES;
    }

    uint32_t count = handler->accelOobDeviceCount();

    memcpy(replyBuf, reqBuf, *dataLen);
    auto *reply = reinterpret_cast<Reply*>(replyBuf + *dataLen);
    reply->count = count;

    *dataLen += sizeof(Reply);
    return IPMI_CC_OK;
}

ipmi_ret_t accelOobDeviceName(const uint8_t* reqBuf, uint8_t* replyBuf,
                              size_t* dataLen, HandlerInterface* handler)
{
    struct Request
    {
        uint8_t subcommand;
        uint32_t index;
    } __attribute__((packed));

    struct Reply
    {
        uint8_t nameLength;
        char    name[MAX_NAME_SIZE];
    } __attribute__((packed));

    if (*dataLen < sizeof(Request))
    {
        std::fprintf(stderr, "AccelOob DeviceName command too small: %zu\n",
                     *dataLen);
        return IPMI_CC_REQ_DATA_LEN_INVALID;
    }

    if (*dataLen + sizeof(Reply) > MAX_IPMI_BUFFER)
    {
        std::fprintf(stderr,
                     "AccelOob DeviceName command too large for reply buffer: "
                     "command=%zuB, payload=%zuB, max=%dB\n",
                     *dataLen, sizeof(Reply), MAX_IPMI_BUFFER);
        return IPMI_CC_REQUESTED_TOO_MANY_BYTES;
    }

    auto *req = reinterpret_cast<const Request*>(reqBuf);
    std::string name = handler->accelOobDeviceName(req->index);

    if (name.size() > MAX_NAME_SIZE) {
        std::fprintf(stderr, "AccelOob: name was too long. "
                             "'%s' len must be <= %zu\n",
                     name.c_str(), MAX_NAME_SIZE);
        return IPMI_CC_REQ_DATA_TRUNCATED;
    }

    memcpy(replyBuf, reqBuf, *dataLen);
    auto *reply = reinterpret_cast<Reply*>(replyBuf + *dataLen);
    reply->nameLength = name.size();
    memcpy(reply->name, name.c_str(), reply->nameLength + 1);

    *dataLen += sizeof(Reply) - MAX_NAME_SIZE + reply->nameLength + 1;
    return IPMI_CC_OK;
}

namespace {

struct NameHeader
{
    uint8_t  subcommand;
    uint8_t  nameLength;
    char     name[MAX_NAME_SIZE];
} __attribute__((packed));


// Reads the variable-length name from reqBuf and outputs the name and a pointer
// to the payload (next byte after name).
//
// Returns: =0: success.
//          >0: if dataLen is too small, returns the minimum buffers size
//
// Params:
//    [in]  reqBuf      - the request buffer
//    [in]  dataLen     - the length of reqBuf, in bytes
//    [in]  payloadSize - the size of the expected payload
//    [out] name        - the name string
//    [out] payload     - pointer into reqBuf just after name
size_t ReadNameHeader(const uint8_t *reqBuf, size_t dataLen, size_t payloadSize,
                      std::string *name, const uint8_t **payload) {
    constexpr size_t kNameHeaderSize = sizeof(NameHeader) - MAX_NAME_SIZE;

    size_t minDataLen = kNameHeaderSize + payloadSize;
    if (dataLen < minDataLen) { return minDataLen; }

    auto *req_header = reinterpret_cast<const NameHeader*>(reqBuf);
    minDataLen += req_header->nameLength;

    if (dataLen < minDataLen) { return minDataLen; }

    if (name) {
        *name = std::string(req_header->name, req_header->nameLength);
    }
    if (payload) {
        *payload = reqBuf + kNameHeaderSize + req_header->nameLength;
    }
    return 0;
}

}  // namespace

ipmi_ret_t accelOobRead(const uint8_t* reqBuf, uint8_t* replyBuf,
                        size_t* dataLen, HandlerInterface* handler)
{
    struct Request
    {
        // uint8_t  subcommand;
        // uint8_t  nameLength;
        // char     name[MAX_NAME_SIZE];
        uint8_t  token;
        uint64_t address;
        uint8_t  num_bytes;
    } __attribute__((packed));

    struct Reply
    {
        uint64_t data;
    } __attribute__((packed));

    std::fprintf(stderr,
        "AccelOob Read command sizes: "
        "command=%zuB, payload=%zuB, max=%dB\n",
        *dataLen, sizeof(Reply), MAX_IPMI_BUFFER);

    std::string name{};
    const uint8_t *payload;

    size_t min_size =
        ReadNameHeader(reqBuf, *dataLen, sizeof(Request), &name, &payload);
    if (min_size != 0) {
        std::fprintf(stderr, "AccelOob Read command too small: %zuB < %zuB\n",
                     *dataLen, min_size);
        return IPMI_CC_REQ_DATA_LEN_INVALID;
    }

    if (*dataLen + sizeof(Reply) > MAX_IPMI_BUFFER)
    {
        std::fprintf(stderr,
                     "AccelOob Read command too large for reply buffer: "
                     "command=%zuB, payload=%zuB, max=%dB\n",
                     *dataLen, sizeof(Reply), MAX_IPMI_BUFFER);
        return IPMI_CC_REQUESTED_TOO_MANY_BYTES;
    }

    auto req = reinterpret_cast<const Request *>(payload);

    std::fprintf(stderr,
                "AccelOob Read: "
                "name=%s, token=%02x, addr=%016lx, size=%02x\n",
                name.c_str(), req->token, req->address, req->num_bytes);

    uint64_t data =
        handler->accelOobRead(name, req->address, req->num_bytes);

    memcpy(replyBuf, reqBuf, *dataLen);
    auto *reply = reinterpret_cast<Reply*>(replyBuf + *dataLen);
    reply->data = data;

    *dataLen += sizeof(Reply);
    return IPMI_CC_OK;
}


ipmi_ret_t accelOobWrite(const uint8_t* reqBuf, uint8_t* replyBuf,
                        size_t* dataLen, HandlerInterface* handler)
{
    struct Request
    {
        // uint8_t  subcommand;
        // uint8_t  nameLength;
        // char     name[MAX_NAME_SIZE];
        uint8_t  token;
        uint64_t address;
        uint8_t  num_bytes;
        uint64_t data;
    } __attribute__((packed));

    struct Reply
    {
        // Empty
    } __attribute__((packed));

    std::string name{};
    const uint8_t *payload;

    size_t min_size =
        ReadNameHeader(reqBuf, *dataLen, sizeof(Request), &name, &payload);
    if (min_size != 0) {
        std::fprintf(stderr, "AccelOob Write command too small: %zuB < %zuB\n",
                     *dataLen, min_size);
        return IPMI_CC_REQ_DATA_LEN_INVALID;
    }

    if (*dataLen + sizeof(Reply) > MAX_IPMI_BUFFER)
    {
        std::fprintf(stderr,
                     "AccelOob Write command too large for reply buffer: "
                     "command=%zuB, payload=%zuB, max=%dB\n",
                     *dataLen, sizeof(Reply), MAX_IPMI_BUFFER);
        return IPMI_CC_REQUESTED_TOO_MANY_BYTES;
    }

    auto req = reinterpret_cast<const Request *>(payload);
    handler->accelOobWrite(name, req->address, req->num_bytes, req->data);

    memcpy(replyBuf, reqBuf, *dataLen);
    return IPMI_CC_OK;
}

} // namespace ipmi
} // namespace google
