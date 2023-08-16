#ifndef _EXPRESS_DATABASE_DATALIST_H
#define _EXPRESS_DATABASE_DATALIST_H

#include "utility/macro.h"
#include "utility/data/rawdata.h"
#include <cstdint>

_START_EXPRESS_NAMESPACE_

namespace database
{
    using pos_t = uint64_t;
    using id_t = uint64_t;
    inline constexpr pos_t kNullPos = 0l;
    //! Data List class
    class DataList
    {
    public:
        /**
         * @param name The name of the specified table.
         * @throw std::system_error will be thrown if errno is set .
         */
        DataList(const char *name);
        ~DataList();

        id_t Create(const char *data, size_t size) noexcept(true);

        /**
         * @brief Find the specified data related to id.
         * @return Return the data desiered if id is valid. Otherwise, invalid raw_data will
         * be returned.
         * @warning The data in the rawdata should be delete manually.
         */
        utility::data::RawData Retrieve(id_t id) noexcept(true);
        void Update(id_t id, const char *new_data, size_t new_size);
        void Delete(id_t id);

    private:
        class Impl;
        Impl *pImpl_;
    };
}

_END_EXPRESS_NAMESPACE_

#endif