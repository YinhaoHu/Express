#ifndef _EXPRESS_DAEMON_CONFIG_H
#define _EXPRESS_DAEMON_CONFIG_H

#include "utility/macro.h"
#include <string>
#include <unordered_map>

_START_EXPRESS_NAMESPACE_

namespace daemon
{
    /**
     * Config module class.
     */
    class Config
    {
    public:
        /**
         * @note `kCount` is used to get the size of Key. It MUST be 
         * in the last position and DON'T set the value manually.
         */
        enum class Key : unsigned int
        {
            kDataBaseNThreads,
            kDataBaseDirName,
            kCoreNWorkers,
            kCorePort,
            kCoreWorkerHeartBeatRate,
            kCount
        };

    public:
        Config();
        ~Config();

        Config(const Config &) = delete;

        void Reload() noexcept;
        std::string GetValue(Key item) noexcept;

    private:
        void Load() noexcept;
        void GenerateDefaulConfig() const noexcept;
        void GenerateOneConfigItem(std::ostream& out, std::string key, 
            std::string value, std::string comment)const noexcept;

        std::unordered_map<std::string, std::string> items;
    };
    extern Config *pConfig;
 
    extern void InitConfig(); 
} // End daemon namespace

_END_EXPRESS_NAMESPACE_

#endif