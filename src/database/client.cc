#include "client.h"
#include "public.h"
#include "utility/ipc/unix_domain_socket.h"
#include <stdexcept>

_START_EXPRESS_NAMESPACE_

using namespace std;
using namespace utility::ipc;

namespace database
{
    class Client::Impl
    {
    private:
        UnixDomainSocket uds_;

    public:
        Impl(Socket sockfd) : uds_(sockfd) {}
        ~Impl() {}

        void MakeTable(const std::string &name, size_t data_unit_size, size_t meta_once_alloc_num)
        {
            SentMessage request;

            request.SetCommunicationCode(static_cast<uint32_t>(CommCode::kMakeTable));
            request.Add(name.c_str(), name.size() + 1);
            request.Add(&data_unit_size, sizeof(data_unit_size));
            request.Add(&meta_once_alloc_num, sizeof(meta_once_alloc_num));
            uds_.SendMessage(request);
        }

        /**
         * @note table_name is assumed to be existed.
         */
        id_t Create(const std::string &table_name, const char *data, size_t size) noexcept(true)
        {
            SentMessage request;

            request.SetCommunicationCode(static_cast<uint32_t>(CommCode::kCreate));
            request.Add(table_name.c_str(), table_name.size() + 1);
            request.Add(data, size);
            uds_.SendMessage(request);

            auto response = uds_.ReceiveMessage();

            return *reinterpret_cast<const id_t *>((*response)[0].pData);
        }

        utility::data::RawData Retrieve(const std::string &table_name, id_t id) noexcept(true)
        {
            using namespace utility::data;
            SentMessage request;

            request.SetCommunicationCode(static_cast<uint32_t>(CommCode::kRetrieve));
            request.Add(table_name.c_str(), table_name.size() + 1);
            request.Add(&id, sizeof(id));

            uds_.SendMessage(request);
            auto response = uds_.ReceiveMessage();
            auto& body = (*response);

            char status = *(body[0].pData);
            if(status == 0)
            {
                return RawData(0);
            }
            else 
            { 
                RawData res{body[1].size};
                memcpy(res.Get().get(), body[1].pData, body[1].size);
                return res;
            }

            // This expression will never be executed.
            return RawData{0};
        }

        void Update(const std::string &table_name, id_t id, const char *new_data, size_t new_size)
        {
            SentMessage request;

            request.SetCommunicationCode(static_cast<uint32_t>(CommCode::kUpdate));
            request.Add(table_name.c_str(), table_name.size() + 1);
            request.Add(&id, sizeof(id));
            request.Add(new_data, new_size);

            uds_.SendMessage(request);
            auto response = uds_.ReceiveMessage();
            auto& body = (*response);
            char status = *(body[0].pData);
            if(status == 0)
            {
                throw runtime_error("Invalid id");
            }
        }

        void Delete(const std::string &table_name, id_t id)
        { 
            SentMessage request;

            request.SetCommunicationCode(static_cast<uint32_t>(CommCode::kDelete));
            request.Add(table_name.c_str(), table_name.size() + 1);
            request.Add(&id, sizeof(id)); 
            uds_.SendMessage(request);
            
            auto response = uds_.ReceiveMessage();
            auto& body = (*response);
            char status = *(body[0].pData);
            if(status == 0)
            {
                throw runtime_error("Invalid id");
            }
        }
    };

    Client::Client(Socket sockfd) : pImpl(new Impl(sockfd)) {}
    Client::~Client() { delete pImpl; }

    void Client::MakeTable(const std::string &name, size_t data_unit_size, size_t meta_once_alloc_num)
    {
        pImpl->MakeTable(name, data_unit_size, meta_once_alloc_num);
    }

    id_t Client::Create(const std::string &table_name, const char *data, size_t size) noexcept(true)
    {
        return pImpl->Create(table_name, data, size);
    }

    utility::data::RawData Client::Retrieve(const std::string &table_name, id_t id) noexcept(true)
    {
        return pImpl->Retrieve(table_name, id);
    }

    void Client::Update(const std::string &table_name, id_t id, const char *new_data, size_t new_size)
    {
        pImpl->Update(table_name, id, new_data, new_size);
    }

    void Client::Delete(const std::string &table_name, id_t id)
    {
        pImpl->Delete(table_name, id);
    }
}

_END_EXPRESS_NAMESPACE_