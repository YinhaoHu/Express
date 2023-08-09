#include "datalist.h"
#include "utility/misc.h"

#include <string>
#include <mutex>
#include <shared_mutex>
#include <memory>
#include <stdexcept>
#include <vector>

#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

using namespace std;

_START_EXPRESS_NAMESPACE_

namespace database
{
    template <typename ElementType>
    class FreeList
    {
    public:
        FreeList(string &name)
        {
            fd_ = open(name.c_str(), O_RDWR);
        }

        ~FreeList()
        {
            close(fd_);
        }

        void Add(ElementType &val)
        {
            mutex_.lock();

            lseek(fd_, 0, SEEK_END);
            write(fd_, &val, sizeof(ElementType));
            mutex_.unlock();
        }

        /**
         * @brief Take one element from the list.
         * @throw
         */
        ElementType Take()
        {
            ElementType ret;
            lock_guard<mutex> lock(mutex_);

            auto end = lseek(fd_, 0, SEEK_END);
            if (end < sizeof(ElementType))
                throw range_error("Free list is empty.");

            lseek(fd_, end - sizeof(ElementType), SEEK_SET);
            read(fd_, &ret, sizeof(ElementType));
            ftruncate(fd_, end - sizeof(ElementType));

            return ret;
        }

        bool IsEmpty()
        {
            struct stat buf;
            mutex_.lock();
            fstat(fd_, &buf);
            mutex_.unlock();
            return buf.st_size == 0;
        }

    private:
        mutex mutex_;
        int fd_;
    };

    class DataList::Impl
    {
    private:
        using FreeBlockList = FreeList<pos_t>;
        using FreeIDList = FreeList<id_t>;

        // A class controls the metadata region in the data file.
        // This is decoupled with free id list. Thus, any operations related to
        // free id list should be performed by FreeIDList object.
        class MetaData
        {
        private:
            shared_mutex region_rw_lock_;
            DataList::Impl *data_list_;
            pos_t start_pos_;
            size_t once_alloc_num_;

            // If one meta data is deleted, the pos field in that region should be
            // filled in this constant variable.
            static constexpr pos_t kDeletedMetaPos = UINT64_MAX;

        public:
            struct Unit
            {
                size_t size;
                pos_t pos;

                Unit() : size(0), pos(0) {}
                Unit(size_t data_size, pos_t data_start_pos)
                    : size(data_size), pos(data_start_pos) {}

                constexpr bool isDeleted() const noexcept
                {
                    return pos == kDeletedMetaPos && size == 0;
                }
                constexpr bool isUnused() const noexcept
                {
                    return pos == 0 && size == 0;
                }
            };

        private:
            /**
             * @return The start position of the specified meta data unit.
             * @note Thread safe.
             */
            pos_t Position(id_t id)
            {
                Unit unit;
                pos_t pos = start_pos_, next_pos;
                int fd = data_list_->fd_;
                auto pread_unit = [&](pos_t pos)
                {
                    pread(fd, &unit, sizeof(unit), pos);
                };
                auto pread_pos = [&](pos_t pos)
                {
                    pread(fd, &next_pos, sizeof(next_pos), pos);
                };

                for (id_t i = 0;;)
                {
                    if (i != 0 && i % once_alloc_num_ == 0)
                    {
                        pread_pos(pos);
                        if (next_pos == 0)
                            break;
                        else
                            pos = next_pos;
                        goto READ_UNIT;
                    }
                    else
                    {
                    READ_UNIT:
                        pread_unit(pos);
                        if (unit.isUnused())
                            break;
                        else if (i == id)
                            return pos;
                        ++i;
                        pos += sizeof(Unit);
                    }
                }

                return kNullPos;
            }

            /**
             * @brief Allocate one metadata block.
             */
            pos_t AllocateBlock()
            {
                size_t block_size = once_alloc_num_ * sizeof(Unit) + sizeof(pos_t);
                pos_t pos;
                {
                    lock_guard<mutex> lockg(data_list_->file_lock_);
                    pos = lseek(data_list_->fd_, 0, SEEK_END);
                    ftruncate(data_list_->fd_, pos + block_size);
                }
                return pos;
            }

        public:
            MetaData(pos_t start_pos, size_t once_alloc_num,
                     DataList::Impl *data_list)
                : start_pos_(start_pos), once_alloc_num_(once_alloc_num),
                  data_list_(data_list)
            {
            }
            /**
             * @brief Create a new metadata unit.
             * @param new_unit The value of the new metadata unit
             * @return The new id.
             * @note This method does not check free id list.
             */
            id_t Create(Unit &new_unit)
            {
                Unit unit;
                id_t new_id;
                pos_t pos = start_pos_, next_pos;
                auto pread_unit = [this, &unit](pos_t pos)
                {
                    pread(this->data_list_->fd_, &unit, sizeof(unit), pos);
                };
                auto pread_pos = [this, &next_pos](pos_t pos)
                {
                    pread(this->data_list_->fd_, &next_pos, sizeof(next_pos), pos);
                };

                // Ensure read-and-write operation is atomic in this
                region_rw_lock_.lock();
                for (new_id = 0;;)
                {
                    if (new_id != 0 && new_id % once_alloc_num_ == 0)
                    {
                        pread_pos(pos);
                        if (next_pos == 0)
                        {
                            auto new_pos = AllocateBlock();
                            pwrite(data_list_->fd_, &new_pos, sizeof(pos_t), pos);
                            pos = new_pos;
                        }
                        else
                            pos = next_pos;
                        goto READ_UNIT;
                    }
                    else
                    {
                    READ_UNIT:
                        pread_unit(pos);
                        if (unit.isUnused())
                        {
                            pwrite(data_list_->fd_, &new_unit, sizeof(new_unit), pos);
                            break;
                        }
                        ++new_id;
                        pos += sizeof(Unit);
                    }
                }
                region_rw_lock_.unlock();

                return new_id;
            }

            /**
             * @brief Get the metadata specifed by id
             * @return Corresponding metadata unit if id is valid, otherwise,
             * zero unit(size = 0 && pos == 0) will be returend.
             */
            Unit Retrieve(id_t id)
            {
                auto pos = Position(id);
                Unit res;

                if (pos != kNullPos)
                    pread(data_list_->fd_, &res, sizeof(Unit), pos);
                return res;
            }

            /**
             * @brief Update the metadata specifed by id.
             * @throw std::range_error This error should be thrown if the
             * id is invalid.
             */
            void Update(id_t id, size_t new_size, pos_t new_pos)
            {
                auto pos = Position(id);

                if (pos == kNullPos)
                    throw range_error("Invalid id");
                else
                {
                    Unit unit;
                    pread(data_list_->fd_, &unit, sizeof(unit), pos);
                    unit.pos = new_pos;
                    unit.size = new_size;
                    pwrite(data_list_->fd_, &unit, sizeof(unit), pos);
                }
            }

            /**
             * @throw std::range_error will be thrown if id is invalid.
             */
            void Delete(id_t id)
            {
                Update(id, 0, kDeletedMetaPos);
            }
        };

        class RealData
        {
        private:
            shared_mutex region_rw_lock_;
            pos_t start_pos_;
            // The real size of one block which means it does not include the next pos field.
            size_t unit_size_;
            DataList::Impl *data_list_;

        private:
            /**
             * @param size The given size of one or more unit.
             * @return The desired unit size of the one or more units.
             */
            constexpr size_t RealUnitSize(size_t size) const noexcept
            {
                return (size % unit_size_ == 0) ? size : (size / unit_size_ + 1) * unit_size_;
            }

            /**
             * @note Thread-safe. Require `region_rw_lock`.
             */
            void SetNext(pos_t whose, pos_t next)
            {
                region_rw_lock_.lock();
                pwrite(data_list_->fd_, &next, sizeof(next), whose + unit_size_);
                region_rw_lock_.unlock();
            }
            /**
             * @note Thread-safe. Require `region_rw_lock_`.
             */
            pos_t Next(pos_t whose)
            {
                pos_t next;
                region_rw_lock_.lock_shared();
                pread(data_list_->fd_, &next, sizeof(pos_t), whose + unit_size_);
                region_rw_lock_.unlock_shared();
                return next;
            }

            /**
             * @brief Add pos into the free block list AND set its next to be kNullPos.
             * Be careful that this function does not clear the original data region.
             * The reuse of that region data is undefined behaviour.
             */
            void DeleteBlock(pos_t pos)
            {
                SetNext(pos, kNullPos);
                data_list_->free_block_list_->Add(pos);
            }

            unique_ptr<vector<pos_t>> AllocateBlock(size_t block_size, size_t block_num)
            {
                off_t first_pos;
                {
                    lock_guard<mutex> lockg(data_list_->file_lock_);
                    first_pos = lseek(data_list_->fd_, 0, SEEK_END);
                    ftruncate(data_list_->fd_, first_pos + block_size * block_num);
                }

                auto res = make_unique<vector<pos_t>>();
                for (size_t i = 0; i < block_num; ++i)
                    res->push_back(first_pos + block_size * i);
                return res;
            }

        public:
            RealData(size_t unit_size, pos_t start_pos, DataList::Impl *data_list)
                : unit_size_(unit_size), start_pos_(start_pos), data_list_(data_list)
            {
            }

            pos_t Create(size_t size, const char *data)
            {
                size_t block_num = RealUnitSize(size) / unit_size_;
                size_t block_size = unit_size_ + sizeof(pos_t);
                off_t data_off = 0;

                auto pos_vec_p = AllocateBlock(block_size, block_num);

                for (pos_t curpos : *pos_vec_p)
                {
                    auto write_bytes = min(unit_size_, size - data_off);

                    pwrite(data_list_->fd_, data + data_off, write_bytes, curpos);
                    data_off += write_bytes;

                    if (curpos != pos_vec_p->back())
                        SetNext(curpos, curpos + block_size);
                    else
                        SetNext(curpos, 0);
                }

                return pos_vec_p->front();
            }

            utility::data::RawData Retrieve(pos_t start_pos, size_t size)
            {
                size_t full_block_num, notfull_block_exist, notfull_unit_size;
                char *buf = new char[size]{0};
                off_t buf_off = 0;
                pos_t pos = start_pos;

                full_block_num = size / unit_size_;
                notfull_block_exist = ((full_block_num * unit_size_) != size);
                notfull_unit_size = size - full_block_num * unit_size_;

                for (int i = 0; i < full_block_num; ++i)
                {
                    pread(data_list_->fd_, buf + buf_off, unit_size_, pos);
                    buf_off += unit_size_;
                    pos = Next(pos);
                }

                if (notfull_block_exist)
                {
                    pread(data_list_->fd_, buf + buf_off, notfull_unit_size, pos);
                }

                return utility::data::RawData(buf, size);
            }
            /**
             * @brief Update the data of the related region to be new data specified by raw_data.
             * The size will be changed, too. But the metadata information will be remained. So,
             * the user neeeds to change the metadata manually.
             * @param start_pos Region start position
             * @param original_size Region original size.
             * @param raw_data New data set on the region.
             * @warning Be careful to change the metadata information manually.
             * @throw std::runtime_error will be thrown if the raw_data.size is zero.
             */
            void Update(pos_t start_pos, size_t original_size, utility::data::RawData &raw_data)
            {
                if (raw_data.Size() == 0)
                    throw runtime_error("Zero size update is not permitted");

                size_t old_total_unit_size = RealUnitSize(original_size);
                size_t new_total_unit_size = RealUnitSize(raw_data.Size());
                vector<pos_t> pos_vec;

                pos_t walk_pos = start_pos;
                while (walk_pos != kNullPos)
                {
                    pos_vec.push_back(walk_pos);
                    walk_pos = Next(walk_pos);
                }

                if (new_total_unit_size < old_total_unit_size)
                {
                    size_t spare_num = (old_total_unit_size - new_total_unit_size) / unit_size_;

                    for (auto iter = pos_vec.end() - 1; spare_num != 0; --iter)
                    {
                        DeleteBlock(*iter);
                        spare_num--;
                    }
                }
                else if (new_total_unit_size > old_total_unit_size)
                {
                    size_t lack_num = (new_total_unit_size - old_total_unit_size) / unit_size_;
                    auto lack_vec_p = AllocateBlock(unit_size_ + sizeof(pos_t), lack_num);
                    for (auto pos : *lack_vec_p)
                        pos_vec.push_back(pos);
                }

                off_t data_off = 0;
                for (auto pos : pos_vec)
                {
                    size_t nbytes = min(raw_data.Size() - data_off, unit_size_);
                    pwrite(data_list_->fd_, raw_data.Get() + data_off, nbytes, pos);
                    data_off += nbytes;
                }
            }

            /**
             * @brief Delete the data list from the start_pos. It does not clean the data in the
             * original region. So, reuse it will be undefined.
             */
            void Delete(pos_t start_pos)
            {
                pos_t pos = start_pos;

                while (pos != kNullPos)
                {
                    pos_t tmp = Next(pos);
                    DeleteBlock(pos);
                    pos = tmp;
                }
            }
        };

    public:
        Impl(const char *name)
        {
            string data_file_name(name);
            fd_ = open(data_file_name.c_str(), O_RDWR);
            if (fd_ < 0)
                throw runtime_error(utility::misc::FormatString("open file %s: %s\n", 128,
                                                                name, strerror(errno)));

            string free_id_name(name), free_blk_name(name);
            size_t once_alloc_num_metadata, unitsize_realdata;
            pos_t start_pos_metadata, start_pos_realdata;

            free_id_name.push_back('1');
            free_blk_name.push_back('2');

            start_pos_metadata = info_region_size;
            auto read_entry = [&](void *buf)
            {
                read(fd_, buf, sizeof(size_t));
            };
            read_entry(&unitsize_realdata);
            read_entry(&once_alloc_num_metadata);
            read_entry(&start_pos_realdata);

            free_id_list_ = new FreeIDList(free_id_name);
            free_block_list_ = new FreeBlockList(free_blk_name);
            meta_data_ = new MetaData(start_pos_metadata, once_alloc_num_metadata, this);
            real_data_ = new RealData(unitsize_realdata, start_pos_realdata, this);
        }

        ~Impl()
        {
            delete free_id_list_;
            delete free_block_list_;
            delete meta_data_;
            delete real_data_;
            close(fd_);
        }

        id_t Create(const char *data, size_t size) noexcept(true)
        {
            id_t id;
            MetaData::Unit unit;
            unit.pos = real_data_->Create(size, data);
            unit.size = size;

            if (free_id_list_->IsEmpty() == false)
            {
                id = free_id_list_->Take();
                meta_data_->Update(id, unit.size, unit.pos);
            }
            else
            {
                id = meta_data_->Create(unit);
            }
            return id;
        }

        utility::data::RawData Retrieve(id_t id)
        {
            auto meta = meta_data_->Retrieve(id);
            if (meta.pos == 0 || meta.size == 0)
                return utility::data::RawData(nullptr, 0);
            else
                return real_data_->Retrieve(meta.pos, meta.size);
        }

        void Update(id_t id, const char *new_data, size_t new_size)
        {
            auto unit = meta_data_->Retrieve(id);
            utility::data::RawData raw_data(new_data, new_size);
            real_data_->Update(unit.pos, unit.size, raw_data);
            meta_data_->Update(id, new_size, unit.pos);
        }

        void Delete(id_t id)
        {
            auto meta = meta_data_->Retrieve(id);
            real_data_->Delete(meta.pos);
            meta_data_->Delete(id);
            free_id_list_->Add(id);
        }

    private:
        FreeBlockList *free_block_list_;
        FreeIDList *free_id_list_;
        MetaData *meta_data_;
        RealData *real_data_;
        mutex file_lock_;
        int fd_;

        // space for 4 long
        static constexpr size_t info_region_size = 32;
    };

    DataList::DataList(const char *name)
        : pImpl_(new Impl(name))
    {
    }

    DataList::~DataList()
    {
        delete pImpl_;
    }

    id_t DataList::Create(const char *data, size_t size) noexcept(true)
    {
        return pImpl_->Create(data, size);
    }

    utility::data::RawData DataList::Retrieve(id_t id)
    {
        return pImpl_->Retrieve(id);
    }

    void DataList::Update(id_t id, const char *new_data, size_t new_size)
    {
        pImpl_->Update(id, new_data, new_size);
    }

    void DataList::Delete(id_t id)
    {
        pImpl_->Delete(id);
    }
}

_END_EXPRESS_NAMESPACE_