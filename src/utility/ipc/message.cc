#include "message.h"
#include <type_traits>

_START_EXPRESS_NAMESPACE_

using namespace std;

namespace utility::ipc
{
    Message::Message(uint32_t communication_code)
        : delete_mode_(DeleteMode::kOneByOne), 
          header_(new Header(communication_code)),
          fields_(new remove_pointer_t<decltype(fields_)>)
    {
    }

    Message::Message(const Header &header)
        : delete_mode_(DeleteMode::kOneByOne), 
          header_(new Header(header)),
          fields_(new remove_pointer_t<decltype(fields_)>)
    {
    }

    Message::Message(const void *rawdata)
        : delete_mode_(DeleteMode::kIntegrally), 
            header_(new Header), 
            fields_(new remove_pointer_t<decltype(fields_)>)
          
    {
        memcpy(static_cast<void*>(header_), rawdata, Header::Size());
        SetFields(static_cast<const char *>(rawdata) + Header::Size());
    }

    Message::~Message()
    {
        delete header_;
        delete fields_;
    }

    void Message::Add(const void *buf, size_t size) noexcept
    {
        header_->num_of_fields++;
        header_->body_size += (size + sizeof(size_t));
        fields_->emplace_back(buf, size);
    }

    std::unique_ptr<vector<Message::Field>> Message::Body() const noexcept
    {
        using namespace std;
        using return_value_type = vector<Field>;

        auto res = make_unique<return_value_type>();

        for (auto iter = fields_->cbegin(); iter != fields_->cend(); ++iter)
            res->emplace_back(*iter);
        return res;
    }

    uint64_t Message::GetHeaderField(HeaderField field) const noexcept
    {
        switch (field)
        {
        case HeaderField::KBodySize:
            return header_->body_size;
        case HeaderField::kCommunicationCode:
            return header_->communication_code;
        case HeaderField::kNumOfFields:
            return header_->num_of_fields;
        }
        // This statement will never be executed in normal case.
        return 0;
    }

    void Message::Delete()
    {
        switch (delete_mode_)
        {
        case DeleteMode::kIntegrally:
            delete[] (static_cast<const char *>(fields_->front().pointer) - sizeof(size_t) - Header::Size());
            break;
        case DeleteMode::kOneByOne:
            for (const auto &f : *fields_)
                delete[] static_cast<const char *>(f.pointer);
            break;
        }
    }

    std::unique_ptr<char[]> Message::Rawdata() const noexcept
    {
        using namespace std;
        unique_ptr<char[]> res(new char[Size()]);

        size_t offset = 0;
        memcpy(res.get() + offset, header_, header_->Size());
        offset += header_->Size();
        for (const auto &f : (*fields_))
        {
            memcpy(res.get() + offset, &f.size, sizeof(f.size));
            offset += sizeof(f.size);
            memcpy(res.get() + offset, f.pointer, f.size);
            offset += f.size;
        }

        return res;
    }

    void Message::SetFields(const void *fields_rawdata) noexcept
    {
        size_t rawdata_offset = 0;

        for (auto count = header_->num_of_fields; count != 0; --count)
        {
            size_t field_size;

            memcpy(&field_size, static_cast<const char *>(fields_rawdata) + rawdata_offset, sizeof(field_size));
            rawdata_offset += sizeof(field_size);
            fields_->emplace_back(static_cast<const char *>(fields_rawdata) + rawdata_offset, field_size);
            rawdata_offset += field_size;
        }
    }
} // namespace ipc

_END_EXPRESS_NAMESPACE_