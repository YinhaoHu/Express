#ifndef _EXPRESS_DATABASE_CLIENT_H
#define _EXPRESS_DATABASE_CLIENT_H

#include "utility/macro.h"
#include "utility/ipc/socket.h"
#include "utility/data/rawdata.h"
#include <string>

_START_EXPRESS_NAMESPACE_

namespace database
{
    using id_t = uint64_t;

    class Client
    {
    public:
        Client(Socket sockfd);
        Client(const Client &&) = delete;
        ~Client();

        /**
         * @brief Make a new table with the given parameters.
         * @param name Table name
         * @param data_unit_size Size of each unit data
         * @param meta_once_alloc_num Specify how many meta data units will be allocated every
         * meta data allocation.
         * @warning This function is NOT thread-safe.
         */
        void MakeTable(const std::string &name, size_t data_unit_size, size_t meta_once_alloc_num);

        /**
         * @brief Create an item in the specified table.
         */
        id_t Create(const std::string &table_name, const char *data, size_t size) noexcept(true);
        
        /**
         * @brief Retrieve the specified data in the specified table.
         * @return The data desiered if id is valid. Otherwise, invalid raw_data will
         * be returned.
         */
        utility::data::RawData Retrieve(const std::string &table_name, id_t id) noexcept(true);
        
        /**
         * @throw std::runtime_error will be thrown if the id is invalid.
         */
        void Update(const std::string &table_name, id_t id, const char *new_data, size_t new_size);
        
        /**
         * @throw std::runtime_error will be thrown if the id is invalid.
         */
        void Delete(const std::string &table_name, id_t id);

    private:
        class Impl;
        Impl *pImpl;
    };
}

_END_EXPRESS_NAMESPACE_

#endif