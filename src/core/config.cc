#include "config.h"

#include <cstdlib>

_START_EXPRESS_NAMESPACE_

namespace core
{
    Config::Config(char *argv[])
    {
        config_items_[Key::kDBWorkersChannel] = atoi(argv[1]);
        config_items_[Key::kNWorkers] = atoi(argv[2]);
        config_items_[Key::kPort] = argv[3];
        config_items_[Key::kHearBeatRate] = atoi(argv[4]);
    }

    Config::ValueType Config::GetConfig(Key key)
    { 
        return config_items_[key];
    }
}

_END_EXPRESS_NAMESPACE_