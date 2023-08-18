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

        std::unordered_map<std::string, std::string> items;
        static constexpr const char *file_name = "/etc/express-server.conf";
    };

    extern const char *prog_name;
    extern const char *lock_file_name;
    extern Config *pConfig;

    extern void ParseArgs(int argc, char *argv[]);
    extern void InitConfig();

} // End daemon namespace

_END_EXPRESS_NAMESPACE_

#endif