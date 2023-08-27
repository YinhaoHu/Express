#include "argparse/argparse.h"
#include "daemon/param.h"

#include <unistd.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <string>
#include <vector>
#include <format>
#include <filesystem>

static argparse::ArgumentParser *pProgram;

static void Init(int argc, char *argv[]);
static void Shutdown();
static void Syslog();
static void Proc();
static void Help();

static std::string_view shutdown_opt{"--shutdown"};
static std::string_view syslog_opt{"--syslog"};
static std::string_view proc_opt{"--proc"};

int main(int argc, char *argv[])
{
    Init(argc, argv);

    if (argc == 1)
        Help();

    if (pProgram->is_used(shutdown_opt))
        Shutdown();
    else if (pProgram->is_used(syslog_opt))
        Syslog();
    else if (pProgram->is_used(proc_opt))
        Proc();

    return 0;
}

static void Init(int argc, char *argv[])
{
    pProgram = new argparse::ArgumentParser("expressctl", "1.0");

    pProgram->add_argument(shutdown_opt.data())
        .help("Shutdown express server and exits.")
        .default_value(false)
        .implicit_value(true);

    pProgram->add_argument(syslog_opt.data())
        .help("Print the system log and exits. Usage : --syslog <num of pages>, like --syslog 15")
        .scan<'i', int>()
        .default_value(10);

    pProgram->add_argument(proc_opt.data())
        .help("Print the running processes related to express and exists.")
        .default_value(false)
        .implicit_value(true);

    try
    {
        pProgram->parse_args(argc, argv);
    }
    catch (const std::runtime_error &err)
    {
        std::cerr << err.what() << std::endl;
        std::exit(1);
    }
}

static void Help()
{
    std::cout << (*pProgram) << std::endl;
    exit(EXIT_SUCCESS);
}

static void Shutdown()
{
    auto FindProcess = [](std::string_view cmdlineToFind)
    {
        namespace fs = std::filesystem;

        for (const auto &entry : fs::directory_iterator("/proc"))
        {
            if (entry.is_directory())
            {
                std::string pidStr = entry.path().filename().string();

                if (std::all_of(pidStr.begin(), pidStr.end(), ::isdigit) && pidStr != "self")
                {
                    std::string cmdlinePath = entry.path() / "cmdline";
                    std::ifstream cmdlineFile(cmdlinePath);

                    if (cmdlineFile)
                    {
                        std::ostringstream cmdlineStream;
                        cmdlineStream << cmdlineFile.rdbuf();
                        std::string cmdline = cmdlineStream.str();
                        if (cmdline.substr(0, cmdlineToFind.size()) == cmdlineToFind)
                        {
                            // Parse the PID string into a pid_t
                            pid_t pid = std::stoi(pidStr);
                            return pid;
                        }
                    }
                }
            }
        }

        // Return -1 to indicate that the process was not found
        return -1;
    };
    

    if ((*pProgram)[shutdown_opt.data()] == true)
    {
        pid_t pid = FindProcess("./bin/expressd");
        if (pid < 0)
        {
            std::cout << "express server is not running." << std::endl;
            exit(EXIT_FAILURE);
        }

        if (kill(pid, express::daemon::kTerminateSignal) < 0)
        {
            std::cout << "error." << strerror(errno) << std::endl;
            exit(EXIT_FAILURE);
        }
        else
        {
            std::cout << "express server is shutdown." << std::endl;
            exit(EXIT_SUCCESS);
        }
    }
}

static void Syslog()
{
    system(std::format("cat /var/log/syslog | grep express | tail -{}", pProgram->get<int>("--syslog")).c_str());
    exit(EXIT_SUCCESS);
}

static void Proc()
{
    if ((*pProgram)[proc_opt.data()] == true)
    {
        system("ps -g 0 | grep express --color=never");
        exit(EXIT_SUCCESS);
    }
}