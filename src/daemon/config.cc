/**
 * @file config.cc
 * @author Hoo
 * @date 2023-8-18
 * @brief Implementation for config part in daemon component.
 * @note format in the config file and its explaination:
 * comment : # blahblah.
 * valid : KEY  = VALUE
 */
#include "config.h"

#include <syslog.h>
#include <unistd.h>
#include <fcntl.h>

#include <algorithm>
#include <utility>
#include <filesystem>
#include <fstream>
#include <format>
#include <string>

#include "utility/macro.h"

_START_EXPRESS_NAMESPACE_

namespace daemon
{
    Config *pConfig;

    const char *prog_name = "express-server";
    const char *lock_file_name = "/var/run/express-server.pid";

    void ParseArgs(int argc, char *argv[])
    {
    }

    void InitConfig()
    {
        pConfig = new Config;
    }

    Config::Config()
    {
        using namespace std::filesystem;

        if (!exists(file_name))
        {
            int fd = creat(file_name, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
            if (fd < 0)
            {
                syslog(LOG_ERR, "can not create configuration file");
                exit(EXIT_FAILURE);
            }
            close(fd);
            GenerateDefaulConfig();
        }
        else if (file_size(file_name) == 0)
        {
            GenerateDefaulConfig();
        }

        Load();
    }

    Config::~Config() = default;

    void Config::Reload() noexcept
    {
    }

    std::string Config::GetValue(Key key) noexcept
    {
        switch (key)
        {
        case Key::kDataBaseNThreads:
            return items["DataBaseNThreads"];
        case Key::kDataBaseDirName:
            return items["DataBaseDirName"];
        default:
            break;
        }

        return std::string{};
    }

    void Config::Load() noexcept
    {
        std::fstream file(file_name, std::ios::in);
        if (file.bad())
        {
            syslog(LOG_ERR, "error in open config file.\n");
            exit(EXIT_FAILURE);
        }

        std::string line;
        uint count = 0;
        while (getline(file, line))
        {
            if (!line.empty() && isalpha(static_cast<unsigned char>(line.front())))
            {
                auto key_first_iter = line.cbegin();
                auto key_last_iter = find_if_not(line.cbegin(), line.cend(), [](unsigned char ch)
                                                 { return isalpha(ch); });

                std::string key(key_first_iter, key_last_iter);

                auto value_first_iter = find_if(key_last_iter + 1, line.cend(), [](unsigned char ch)
                                                { return ch != ' ' && ch != '='; });

                auto value_last_iter = line.cend();

                std::string value(value_first_iter, value_last_iter);
                items[key] = value;
                count++;
            }
        }
        if (count != static_cast<uint>(Key::kCount))
        {
            syslog(LOG_ERR, "config file is incorrect");
            exit(EXIT_FAILURE);
        }
        file.close();
    }

    void Config::GenerateDefaulConfig() const noexcept
    {
        std::fstream file(file_name, std::ios_base::trunc | std::ios_base::out);

        file << std::format(
            "{:<25}= {}\n\n"
            "{:<25}= {}\n\n",
            "DataBaseNThreads", "2", "DataBaseDirName", "/var/expres-server/database");

        file.close();
    }
}

_END_EXPRESS_NAMESPACE_