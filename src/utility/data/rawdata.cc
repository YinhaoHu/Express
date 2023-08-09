#include "rawdata.h"

_START_EXPRESS_NAMESPACE_
using namespace std;

namespace utility::data
{
    RawData::RawData(size_t nbytes)
        : pdata_( (nbytes == 0) ? (nullptr) : (new char[nbytes]{0}) ), size_(nbytes)
    {
    }
    RawData::RawData(const RawData &other)
        : pdata_(other.pdata_), size_(other.size_)
    {
    }
    RawData::RawData(RawData &&other)
        : pdata_(std::move(other.pdata_)), size_(other.size_)
    {
    }
    RawData::~RawData() {}

    RawData &RawData::operator=(const RawData &rhs)
    {
        if (this != (&rhs))
        {
            this->pdata_ = rhs.pdata_;
            this->size_ = rhs.size_;
        }
        return *this;
    }
    RawData &RawData::operator=(RawData &&rhs)
    {
        if (this != (&rhs))
        {
            this->pdata_ = std::move(rhs.pdata_);
            this->size_ = rhs.size_;
        }
        return *this;
    }
}

_END_EXPRESS_NAMESPACE_