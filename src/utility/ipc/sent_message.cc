#include "sent_message.h"

_START_EXPRESS_NAMESPACE_

namespace utility::ipc
{
    SentMessage::SentMessage() : spData(new std::vector<Field>)
    {
        spData->emplace_back(reinterpret_cast<const char *>(spHeader_.get()), spHeader_->Size());
    };
   SentMessage:: ~SentMessage() = default;

    void SentMessage::Add(const void *pData, size_t size)
    {
        spHeader_->body_size += size + sizeof(size_t);
        spHeader_->num_of_fields++;

        spData->emplace_back(reinterpret_cast<const char *>(pData), size);
    }

    void SentMessage::SetCommunicationCode(uint32_t comm_code)
    {
        spHeader_->communication_code = comm_code;
    }

    /**
     * @note Used for stream IPC channels.
     * @warning It is a typical error that forget to send Field.size before
     * Field.pData. To correctly use this function for sending Message,
     * you can learn how TCPClient::Send() do this.
     */
    std::shared_ptr<std::vector<SentMessage::Field>> SentMessage::StreamData() const
    {
        return spData;
    }

    /**
     * @note Used for bounded IPC channels.
     */
    std::unique_ptr<char[]> SentMessage::MessageData() const
    {
        std::unique_ptr<char[]> res(new char[this->Size()]);

        auto p = res.get();
        size_t offset = 0;

        memcpy(p, spHeader_.get(), spHeader_->Size());
        offset += spHeader_->Size();
        for (size_t i = 0; i < spHeader_->num_of_fields; ++i)
        {
            auto &item = spData->at(i + 1);

            memcpy(p + offset, &item.size, sizeof(item.size));
            offset += sizeof(item.size);
            memcpy(p + offset, item.pData, item.size);
            offset += item.size;
        }

        return res;
    }

}

_END_EXPRESS_NAMESPACE_