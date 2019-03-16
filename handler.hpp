#pragma once

#include <cstdint>
#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <tuple>
#include <vector>

namespace google
{
namespace ipmi
{

using VersionTuple =
    std::tuple<std::uint8_t, std::uint8_t, std::uint8_t, std::uint8_t>;

extern const std::string defaultConfigFile;

class HandlerInterface
{
  public:
    virtual ~HandlerInterface() = default;

    /**
     * Return ethernet details (hard-coded).
     *
     * @return tuple of ethernet details (channel, if name).
     */
    virtual std::tuple<std::uint8_t, std::string> getEthDetails() const = 0;

    /**
     * Return the value of rx_packets, given a if_name.
     *
     * @param[in] name, the interface name.
     * @return the number of packets received.
     * @throw IpmiException on failure.
     */
    virtual std::int64_t getRxPackets(const std::string& name) const = 0;

    /**
     * Return the values from a cpld version file.
     *
     * @param[in] id - the cpld id number.
     * @return the quad of numbers as a tuple (maj,min,pt,subpt)
     * @throw IpmiException on failure.
     */
    virtual VersionTuple getCpldVersion(unsigned int id) const = 0;

    /**
     * Set the PSU Reset delay.
     *
     * @param[in] delay - delay in seconds.
     * @throw IpmiException on failure.
     */
    virtual void psuResetDelay(std::uint32_t delay) const = 0;

    /**
     * Return the entity name.
     * On the first call to this method it'll build the list of entities.
     * @todo Consider moving the list building to construction time (and ignore
     * failures).
     *
     * @param[in] id - the entity id value
     * @param[in] instance - the entity instance
     * @return the entity's name
     * @throw IpmiException on failure.
     */
    virtual std::string getEntityName(std::uint8_t id,
                                      std::uint8_t instance) = 0;

    /**
     * Populate the i2c-pcie mapping vector.
     */
    virtual void buildI2cPcieMapping() = 0;

    /**
     * Return the size of the i2c-pcie mapping vector.
     *
     * @return the size of the vector holding the i2c-pcie mapping tuples.
     */
    virtual size_t getI2cPcieMappingSize() const = 0;

    /**
     * Return a copy of the entry in the vector.
     *
     * @param[in] entry - the index into the vector.
     * @return the tuple at that index.
     */
    virtual std::tuple<std::uint32_t, std::string>
        getI2cEntry(unsigned int entry) const = 0;
};

class Handler : public HandlerInterface
{
  public:
    explicit Handler(const std::string& entityConfigPath = defaultConfigFile) :
        _configFile(entityConfigPath){};
    ~Handler() = default;

    std::tuple<std::uint8_t, std::string> getEthDetails() const override;
    std::int64_t getRxPackets(const std::string& name) const override;
    VersionTuple getCpldVersion(unsigned int id) const override;
    void psuResetDelay(std::uint32_t delay) const override;
    std::string getEntityName(std::uint8_t id, std::uint8_t instance) override;
    void buildI2cPcieMapping() override;
    size_t getI2cPcieMappingSize() const override;
    std::tuple<std::uint32_t, std::string>
        getI2cEntry(unsigned int entry) const override;

  private:
    std::string _configFile;

    bool _entityConfigParsed = false;

    const std::map<uint8_t, std::string> _entityIdToName{
        {0x03, "cpu"},
        {0x04, "storage_device"},
        {0x06, "system_management_module"},
        {0x08, "memory_module"},
        {0x0B, "add_in_card"},
        {0x17, "system_chassis"},
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

extern Handler handlerImpl;

} // namespace ipmi
} // namespace google
