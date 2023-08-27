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

#include "param.h"
#include "utility/macro.h"

_START_EXPRESS_NAMESPACE_

namespace daemon
{
    Config *pConfig; 

    void InitConfig()
    {
        pConfig = new Config;
    }

    Config::Config()
    {
        using namespace std::filesystem;

        if (!exists(config_file_name))
        {
            int fd = creat(config_file_name.data(), S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
            if (fd < 0)
            {
                syslog(LOG_ERR, "can not create configuration file");
                exit(EXIT_FAILURE);
            }
            close(fd);
            GenerateDefaulConfig();
        }
        else if (file_size(config_file_name) == 0)
        {
            GenerateDefaulConfig();
        }

        Load();
    }

    Config::~Config() = default;

    void Config::Reload() noexcept
    {
        // TODO: To be implemented for signal requesting reload config.
    }

    std::string Config::GetValue(Key key) noexcept
    {
        switch (key)
        {
        case Key::kDataBaseNThreads:
            return items["DataBaseNThreads"];
        case Key::kDataBaseDirName:
            return items["DataBaseDirName"];
        case Key::kCoreNWorkers:
            return items["CoreNWorkers"];
        case Key::kCorePort:
            return items["CorePort"];
        case Key::kCoreWorkerHeartBeatRate:
            return items["CoreWorkerHeartBeatRate"];
        default:
            break;
        }

        return std::string{};
    }

    void Config::Load() noexcept
    {
        _EXPRESS_DEBUG_INSTRUCTION(RETRY:)
        std::fstream file(config_file_name.data(), std::ios::in);
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
            _EXPRESS_DEBUG_INSTRUCTION(file.close();)
            _EXPRESS_DEBUG_INSTRUCTION(GenerateDefaulConfig();)
            _EXPRESS_DEBUG_INSTRUCTION(goto RETRY;)
            exit(EXIT_FAILURE);
        }
        file.close();
    }

    void Config::GenerateDefaulConfig() const noexcept
    {
        std::fstream file(config_file_name.data(), std::ios_base::trunc | std::ios_base::out);

        GenerateOneConfigItem(file, "DataBaseNThreads", "2", "Number of threads of data base.");
        GenerateOneConfigItem(file, "DataBaseDirName", "/var/expres-server/database", "Working directory of data base.");
        GenerateOneConfigItem(file, "CoreNWorkers", "2", "Number of worker processes");
        GenerateOneConfigItem(file, "CorePort", "32993", "Server listening port");
        GenerateOneConfigItem(file, "CoreWorkerHeartBeatRate", "180", "Specify the worker to heart beat every # msec.");

        file.close();
    }

    void Config::GenerateOneConfigItem(std::ostream &out, std::string key,
                                       std::string value, std::string comment) const noexcept
    {
        constexpr const size_t comment_one_line_max = 60;
        size_t printted_comment_line_num = 0;

        for (unsigned char ch : comment)
        {
            if (printted_comment_line_num == 0)
                out << "# ";

            out << ch;
            ++printted_comment_line_num;

            if (printted_comment_line_num > comment_one_line_max && (isalpha(ch) || isdigit(ch)))
            {
                printted_comment_line_num = 0;
                out << "\n";
            }
        }
        out << std::format("\n{:<25}= {}\n\n\n", key, value);
    }
}

_END_EXPRESS_NAMESPACE_