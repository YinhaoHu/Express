#ifndef _EXPRESS_UTILITY_DATA_RAWDATA_H
#define _EXPRESS_UTILITY_DATA_RAWDATA_H

#include "utility/macro.h"
#include <memory>
#include <cstddef>

_START_EXPRESS_NAMESPACE_

namespace utility
{
    namespace data
    {
        /*
            A class that takes the data and size together with ownership of the data memory.
        */
        class RawData
        {
        public:
            /**
             * @note If `nbytes` is zero, this class will not allocate memory and 
             * Valid() == false.
            */
            RawData(size_t nbytes);
            RawData(const RawData &other);
            RawData(RawData &&other);
            ~RawData();

            RawData &operator=(const RawData &rhs);
            RawData &operator=(RawData &&rhs);

            constexpr size_t Size() const noexcept
            {
                return size_;
            }
            std::shared_ptr<char[]> Get() const noexcept
            {
                return pdata_;
            }
            bool Valid() noexcept
            {
                return pdata_.get() != nullptr;
            }

        private:
            std::shared_ptr<char[]> pdata_;
            size_t size_;
        };
    }
}

_END_EXPRESS_NAMESPACE_

#endif