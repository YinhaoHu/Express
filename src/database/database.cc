#include "database.h"
#include "datalist.h"

#include <unordered_map>
#include <filesystem>
#include <fstream>
#include <vector>

#include <fcntl.h>
#include <unistd.h>

_START_EXPRESS_NAMESPACE_

using namespace std;

namespace database
{
    class DataBase::Impl
    {
    private:
        constexpr static string meta_file_name_{"meta"};
        fstream meta_stream_;
        string base_path_name_, meta_path_name_;
        unordered_map<string, DataList *> tables_;

        void LoadMeta()
        {
            string buf;

            while (getline(meta_stream_, buf).eof() == false)
            {
                string name;
                name.append(base_path_name_);
                name.push_back('/');
                name.append(buf);
                tables_[buf] = new DataList(name.c_str());
                buf.clear();
            }
            meta_stream_.clear();
            meta_stream_.seekp(0, ios_base::end);
        }

        void WriteMetaItem(const string &name)
        {
            meta_stream_ << name << "\n";
        }

    public:
        Impl(const string &absolute_path_name)
            : base_path_name_(absolute_path_name), meta_path_name_(absolute_path_name)
        {
            using namespace filesystem;
            meta_path_name_.push_back('/');
            meta_path_name_.append(meta_file_name_);

            if (!exists(absolute_path_name))
            {
                create_directory(absolute_path_name);
                close(creat(meta_path_name_.c_str(), S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH));
            }

            meta_stream_.open(meta_path_name_, ios::out | ios::in | ios::app);
            LoadMeta();
        }

        ~Impl()
        {
            meta_stream_.close();
            for (auto &table : tables_)
                delete table.second;
        }

        void MakeTable(const string &name, size_t data_unit_size, size_t meta_once_alloc_num)
        {
            WriteMetaItem(name);

            vector<string> names;

            for (int i = 0; i < 3; i++)
            {
                names.push_back(base_path_name_);
                names[i].push_back('/');
                names[i].append(name);
                if (i == 1)
                    names[i].push_back('1');
                else if (i == 2)
                    names[i].push_back('2');
                close(creat(names[i].c_str(), S_IRUSR | S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH));
            }

            int fd = open(names[0].c_str(), O_RDWR);
            size_t real_data_start = 4 * sizeof(size_t) +
                                     meta_once_alloc_num * (sizeof(size_t) + sizeof(pos_t)) + sizeof(pos_t);

            ftruncate(fd, real_data_start);
            write(fd, &data_unit_size, sizeof(size_t));
            write(fd, &meta_once_alloc_num, sizeof(size_t));
            write(fd, &real_data_start, sizeof(size_t));
            close(fd);

            tables_[name] = new DataList(names[0].c_str());
        }

        id_t Create(const string &table_name, const char *data, size_t size) noexcept(true)
        {
            return tables_[table_name]->Create(data, size);
        }

        utility::data::RawData Retrieve(const string &table_name, id_t id)
        {
            return tables_[table_name]->Retrieve(id);
        }

        void Update(const string &table_name, id_t id, const char *new_data, size_t new_size)
        {
            tables_[table_name]->Update(id, new_data, new_size);
        }

        void Delete(const string &table_name, id_t id)
        {
            tables_[table_name]->Delete(id);
        }
    };

    DataBase::DataBase(const std::string &dir_name)
        : pimpl_(new Impl(dir_name))
    {
    }

    DataBase::~DataBase()
    {
        delete pimpl_;
    }

    void DataBase::MakeTable(const std::string &name, size_t data_unit_size, size_t meta_once_alloc_num)
    {
        pimpl_->MakeTable(name, data_unit_size, meta_once_alloc_num);
    }

    id_t DataBase::Create(const std::string &table_name, const char *data, size_t size) noexcept(true)
    {
        return pimpl_->Create(table_name, data, size);
    }

    utility::data::RawData DataBase::Retrieve(const std::string &table_name, id_t id)
    {
        return pimpl_->Retrieve(table_name, id);
    }

    void DataBase::Update(const std::string &table_name, id_t id, const char *new_data, size_t new_size)
    {
        pimpl_->Update(table_name, id, new_data, new_size);
    }

    void DataBase::Delete(const std::string &table_name, id_t id)
    {
        pimpl_->Delete(table_name, id);
    }
}

_END_EXPRESS_NAMESPACE_