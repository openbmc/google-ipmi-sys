#pragma once

#include "handler.hpp"

#include <cstdint>
#include <map>
#include <nlohmann/json.hpp>
#include <sdbusplus/bus.hpp>
#include <string>
#include <tuple>
#include <vector>

namespace google
{
namespace ipmi
{

constexpr char defaultConfigFile[] =
    "/usr/share/ipmi-entity-association/entity_association_map.json";

class Handler : public HandlerInterface
{
  public:
    explicit Handler(const std::string& entityConfigPath = defaultConfigFile) :
        _configFile(entityConfigPath){};
    ~Handler() = default;

    std::tuple<std::uint8_t, std::string>
        getEthDetails(std::string intf) const override;
    std::int64_t getRxPackets(const std::string& name) const override;
    VersionTuple getCpldVersion(unsigned int id) const override;
    void psuResetDelay(std::uint32_t delay) const override;
    void psuResetOnShutdown() const override;
    std::string getEntityName(std::uint8_t id, std::uint8_t instance) override;
    uint32_t getFlashSize() override;
    std::string getMachineName() override;
    void buildI2cPcieMapping() override;
    size_t getI2cPcieMappingSize() const override;
    void hostPowerOffDelay(std::uint32_t delay) const override;
    std::tuple<std::uint32_t, std::string>
        getI2cEntry(unsigned int entry) const override;

    uint32_t accelOobDeviceCount() const override;
    std::string accelOobDeviceName(size_t i) const override;
    uint64_t accelOobRead(const std::string& name, uint64_t address,
                          uint8_t num_bytes) const override;
    void accelOobWrite(const std::string& name, uint64_t address,
                       uint8_t num_bytes, uint64_t data) const override;

  protected:
    // Exposed for dependency injection
    virtual sdbusplus::bus::bus accelOobGetDbus() const;

  private:
    std::string _configFile;

    bool _entityConfigParsed = false;

    const std::map<uint8_t, std::string> _entityIdToName{
        {0x03, "cpu"},
        {0x04, "storage_device"},
        {0x06, "system_management_module"},
        {0x07, "system_board"},
        {0x08, "memory_module"},
        {0x0B, "add_in_card"},
        {0x0E, "power_system_board"},
        {0x10, "system_internal_expansion_board"},
        {0x11, "other_system_board"},
        {0x17, "system_chassis"},
        {0x1D, "fan"},
        {0x1E, "cooling_unit"},
        {0x20, "memory_device"}};

    nlohmann::json _entityConfig{};

    std::vector<std::tuple<uint32_t, std::string>> _pcie_i2c_map;
};

/**
 * Given a type, entity instance, and a configuration, return the name.
 *
 * @param[in] type - the entity type
 * @param[in] instance - the entity instance
 * @param[in] config - the json object holding the entity mapping
 * @return the name of the entity from the map
 */
std::string readNameFromConfig(const std::string& type, uint8_t instance,
                               const nlohmann::json& config);

} // namespace ipmi
} // namespace google
