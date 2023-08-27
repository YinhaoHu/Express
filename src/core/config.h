#ifndef _EXPRESS_CORE_BOOT_H
#define _EXPRESS_CORE_BOOT_H

#include <unordered_map>
#include <variant>
#include <string>

#include "utility/macro.h"

_START_EXPRESS_NAMESPACE_

namespace core
{
    class Config
    {
    public:
        enum class Key
        {
            kDBWorkersChannel,
            kNWorkers,
            kPort,
            kHearBeatRate
        };

        using ValueType = std::variant<int, std::string>;

    public:
        Config(char *argv[]);
        Config(const Config &) = delete;
        ~Config() = default;

        ValueType GetConfig(Key key);

    private:
        std::unordered_map<Key, ValueType>  config_items_;
    };
}

_END_EXPRESS_NAMESPACE_

#endif