#include "handler.h"
#include "public.h"
#include "database.h"
#include "utility/ipc/unix_domain_socket.h"
#include "utility/ipc/sent_message.h"
#include <unordered_map>
#include <functional>
#include <format>
#include <iostream>


using namespace std;
using namespace express::database;
using namespace express::utility::ipc;

using Message = ReceivedStreamMessage;
using MsgHdrField = Message::Header::Field;

extern DataBase *pDataBase;
extern UnixDomainSocket *pUnixDomainSocket; 
extern std::mutex uds_mutex;

static void handle_make_table(unique_ptr<Message> spMessage);
static void handle_create(unique_ptr<Message> spMessage);
static void handle_retrieve(unique_ptr<Message> spMessage);
static void handle_update(unique_ptr<Message> spMessage);
static void handle_delete(unique_ptr<Message> spMessage);

unordered_map<uint32_t, function<void(unique_ptr<Message>)>> handlers =
{
    {static_cast<uint32_t>(CommCode::kMakeTable)    ,   handle_make_table    },
    {static_cast<uint32_t>(CommCode::kCreate)       ,   handle_create        },
    {static_cast<uint32_t>(CommCode::kRetrieve)     ,   handle_retrieve      },
    {static_cast<uint32_t>(CommCode::kUpdate)       ,   handle_update        },
    {static_cast<uint32_t>(CommCode::kDelete)       ,   handle_delete        },
};

void express::database::handle(std::unique_ptr<ReceivedStreamMessage> msg)
{ 
    uint32_t comm_code = msg->GetHeaderField(MsgHdrField::kCommunicationCode);   

    handlers[comm_code](std::move(msg));
}

/**
 * @note RPC Arguments Explaination
 * From: <table_name>       <data_unit_size>        <once_alloc_meta>
 * To:      NO RESPONSE
 */
static void handle_make_table(unique_ptr<Message> spMessage)
{
    auto &body = *spMessage;

    string table_name(body[0].pData);
    size_t data_unit_size{*reinterpret_cast<const size_t *>(body[1].pData)};
    size_t once_alloc_meta{*reinterpret_cast<const size_t *>(body[2].pData)}; 
    pDataBase->MakeTable(table_name, data_unit_size, once_alloc_meta);
}

/**
 * @note RPC Arguments Explaination
 * From:    <table_name>    <data>
 * To:      <value>
 * @warning Table(table_name) is assumed existed.
 */
static void handle_create(unique_ptr<Message> spMessage)
{
    auto &body = *spMessage;

    string table_name(body[0].pData);
    const char *pData = body[1].pData;
    size_t data_size = body[1].size;

    express::database::id_t id = pDataBase->Create(table_name, pData, data_size);

    SentMessage response;
    response.SetCommunicationCode(static_cast<uint32_t>(CommCode::kCreate));
    response.Add(&id, sizeof(id));

    lock_guard lock(pUnixDomainSocket->GetWriteMutex());
    pUnixDomainSocket->SendMessage(response); 
}

/**
 * @note RPC Arguments Explaination
 * From:    <table_name>    <id>
 * To:      <status>        <*data>
 *      <status>: 0 mean fail. 1 means success.
 *      <*data> field might exist or not depend on sucess.
 * @warning Table(table_name)is assumed exist.
 */
static void handle_retrieve(unique_ptr<Message> spMessage)
{
    auto &body = *spMessage;

    string table_name(body[0].pData);
    express::database::id_t id = *reinterpret_cast<const express::database::id_t *>(body[1].pData);

    auto data = pDataBase->Retrieve(table_name, id);

    SentMessage response;
    response.SetCommunicationCode(static_cast<uint32_t>(CommCode::kRetrieve));

    char status = data.Valid();
    response.Add(&status, sizeof(status));
    if (data.Valid())
        response.Add(data.Get().get(), data.Size());

    lock_guard lock(pUnixDomainSocket->GetWriteMutex());
    pUnixDomainSocket->SendMessage(response);
}

/**
 * @note RPC Arguments Explaination
 * From:    <table_name>    <id>    <new_data>
 * To:      <staus>
 *          status: 0 means fail, 1 means success.
 * @warning Table(table_name)is assumed exist.
 */
static void handle_update(unique_ptr<Message> spMessage)
{
    auto &body = *spMessage;

    string table_name(body[0].pData);
    express::database::id_t id = *reinterpret_cast<const express::database::id_t *>(body[1].pData);

    SentMessage response;
    char status;
    response.SetCommunicationCode(static_cast<uint32_t>(CommCode::kUpdate));
    try
    {
        pDataBase->Update(table_name, id, body[2].pData, body[2].size);
        status = 1;
        response.Add(&status, sizeof(char));
    }
    catch (const std::runtime_error &e)
    {
        status = 0;
        response.Add(&status, sizeof(char));
    }
    lock_guard lock(pUnixDomainSocket->GetWriteMutex());
    pUnixDomainSocket->SendMessage(response);
}

/**
 * @note RPC Arguments Explaination
 * From:    <table_name>    <id>
 * To:      <staus>
 *          status: 0 means fail, 1 means success.
 * @warning Table(table_name)is assumed exist.
 */
static void handle_delete(unique_ptr<Message> spMessage)
{
    auto &body = *spMessage;

    string table_name(body[0].pData);
    express::database::id_t id =
        *reinterpret_cast<const express::database::id_t *>(body[1].pData);

    SentMessage response;
    char status;
    response.SetCommunicationCode(static_cast<uint32_t>(CommCode::kDelete));
    try
    {
        pDataBase->Delete(table_name, id);
        status = 1;
        response.Add(&status, sizeof(char));
    }
    catch (const std::runtime_error &e)
    {
        status = 0;
        response.Add(&status, sizeof(char));
    }
    lock_guard lock(pUnixDomainSocket->GetWriteMutex());
    pUnixDomainSocket->SendMessage(response);
}
