#include "rawdata.h"
#include <cstring>

_START_EXPRESS_NAMESPACE_

namespace utility::data
{
    RawData::RawData() : bytes_(nullptr), size_(0)
    {
    }

    RawData::RawData(const char *bytes, size_t size)
        : bytes_(bytes), size_(size)
    {
    }

    RawData::RawData(const RawData &other)
        : bytes_(other.bytes_), size_(other.size_)
    {
    }

    RawData::~RawData()
    {
        bytes_ = nullptr;
        size_ = 0;
    }

    RawData &RawData::operator=(RawData &rhs)
    {
        size_ = rhs.size_;
        bytes_ = rhs.bytes_;
        return *this;
    }

}

_END_EXPRESS_NAMESPACE_