#ifndef _EXPRESS_UTILITY_DATA_RAWDATA_H
#define _EXPRESS_UTILITY_DATA_RAWDATA_H

#include "utility/macro.h"
#include <cstddef>

_START_EXPRESS_NAMESPACE_

namespace utility
{
    namespace data
    {
        /*
            A class that take data and size together without ownership.

            This class should be used very carefully, it just servers as a
            container without the obligation to reclaim the resource.
            And it will not be detected if the resource it shares has been
            reclaimed. It's just designed to delivery a pointer with
            size conveniently.
            The user can reclaim the data by calling Delete() member function
            or delete the pointer returned by Get().
        */
        class RawData
        {
        public:
            RawData();
            RawData(const char *bytes, size_t size);
            RawData(const RawData &other);
            RawData(RawData &&other) = delete;
            ~RawData();

            RawData &operator=(RawData &rhs);

            constexpr size_t Size() const noexcept
            {
                return size_;
            }

            constexpr const char *Get() const noexcept
            {
                return bytes_;
            }

            /**
             * @note If the this function return false, it means that Get() will return
             * nullptr.
             */
            constexpr bool Valid() const noexcept
            {
                return bytes_ != nullptr;
            }

            /**
             * @brief Delete the data, set data pointer to be nullptr and size to be 0.
             */
            void Delete()
            {
                delete[] bytes_;
                size_ = 0;
                bytes_ = nullptr;
            }

            void Set(const char *bytes, size_t size)
            {
                size_ = size;
                bytes_ = bytes;
            }

        private:
            const char *bytes_;
            size_t size_;
        };
    }
}

_END_EXPRESS_NAMESPACE_

#endif