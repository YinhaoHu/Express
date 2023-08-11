#ifndef _EXPRESS_UTILITY_IPC_ABSTRACT_MESSAGE_H
#define _EXPRESS_UTILITY_IPC_ABSTRACT_MESSAGE_H

#include "utility/macro.h"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <vector>

_START_EXPRESS_NAMESPACE_

namespace utility
{
    namespace ipc
    {
        class AbstractMessage
        {
        public:
            struct Field
            {
                const char* pData;
                size_t size;
 
                Field(const char* p = nullptr, size_t nbytes  = 0)
                    : pData(p), size(nbytes){}
            };
            struct Header
            {
                enum class Field
                {
                    kCommunicationCode,
                    kNumOfFields,
                    kBodySize
                };

                uint8_t num_of_fields;
                char reserved[11];
                uint32_t communication_code;
                uint64_t body_size;

                Header() : num_of_fields(0), communication_code(0),
                           body_size(0) {}

                Header(const Header &other) : num_of_fields(other.num_of_fields),
                                              communication_code(other.communication_code),
                                              body_size(other.body_size)
                {
                }

                Header &operator=(const Header &rhs)
                {
                    if (&rhs != this)
                    {
                        this->num_of_fields = rhs.num_of_fields;
                        this->communication_code = rhs.communication_code;
                        this->body_size = rhs.body_size;
                    }
                    return *this;
                } 

                char* GetData() 
                {
                    return reinterpret_cast<char*>(this);
                }

                static constexpr size_t Size() noexcept
                {
                    return sizeof(Header);
                }
            };

        protected:
            std::shared_ptr<Header> spHeader_;

        public:
            AbstractMessage(std::shared_ptr<Header> pheader = std::make_shared<Header>())
                : spHeader_(pheader){};
            AbstractMessage(const AbstractMessage &other)
            :spHeader_(other.spHeader_){}
            
            AbstractMessage(AbstractMessage &&other) = delete;
            AbstractMessage &operator=(const AbstractMessage &rhs) = delete;

            virtual ~AbstractMessage() = default;

            uint64_t GetHeaderField(Header::Field field) const noexcept
            {
                using F = Header::Field;
                uint64_t res;

                switch (field)
                {
                case F::kBodySize:
                    res = spHeader_->body_size;
                    break;
                case F::kCommunicationCode:
                    res = spHeader_->communication_code;
                    break;
                case F::kNumOfFields:
                    res = spHeader_->num_of_fields;
                    break;
                default:
                    break;
                }
                return res;
            }

            size_t Size() const noexcept
            {
                return spHeader_->Size() + spHeader_->body_size;
            }
        };
    }
}

_END_EXPRESS_NAMESPACE_

#endif