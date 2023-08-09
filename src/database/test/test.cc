#include "database/database.h"
#include "utility/test/tool.h"
#include "utility/misc.h"
#include <iostream>
#include <vector>
#include <barrier>
#include <filesystem>
#include <random>
#include <thread>
#include <unistd.h>

using namespace express::database;
using namespace express::utility::misc;
using namespace express::utility::test;
using namespace std;

static string dbname("/home/hoo/project/express/src/database/test/testdb"),
    tablename("multi-thread-db");

DataBase *db = new DataBase(dbname);
static size_t nitems;
static size_t data_unit_size, once_alloc_meta;
static size_t item_maxlen, item_minlen;
constexpr char item_min_ch = 'a', item_max_ch = 'z';

vector<string> write_bufs;

void test_init()
{
    db->MakeTable(tablename, data_unit_size, once_alloc_meta);
    write_bufs.resize(nitems);

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> size_dist(item_minlen, item_maxlen),
        ch_dist(item_min_ch, item_max_ch);

    size_t maxlen = 0;
    for (size_t i = 0; i < nitems; ++i)
    {
        size_t item_size = size_dist(gen);
        string curstr;
        for (size_t c = 0; c < item_size; ++c)
        {
            curstr.push_back(ch_dist(gen));
        }
        maxlen = max(maxlen, item_size);
        write_bufs[i] = (curstr);
    }
}

void test_create()
{
    bool result = true;

    for (int i = 0; i < nitems; ++i)
    {
        db->Create(tablename, write_bufs[i].c_str(), write_bufs[i].size() + 1);
    }

    for (int i = 0; i < nitems; ++i)
    {
        auto data = db->Retrieve(tablename, i);
        if (write_bufs[i].compare(data.Get()) != 0)
            result = false;
        delete data.Get();
    }

    test("create", "compare", [&result]() -> bool
         { return result == true; });
}

void test_update()
{
    if (nitems < 129)
    {
        printf("Test update: Nitems should at least be greaer than 128");
        return;
    }
    string update_data("123456789123456789123456789");
    vector<express::database::id_t> update_ids{0, 5, 10, 32, 63, 64, 65, 127, 128, 129};

    for (auto id : update_ids)
    {
        write_bufs[id] = update_data;
        db->Update(tablename, id, update_data.c_str(), update_data.size() + 1);
    }

    bool result = true;
    for (auto id : update_ids)
    {
        auto new_data = db->Retrieve(tablename, id);

        if (update_data.compare(new_data.Get()) != 0)
            result = false;
    }
    test("udpate", "compare", [result]() -> bool
         { return result; });
}

void test_retrieve()
{
    bool found_wrong = false;

    for (int i = 0; i < nitems; i += 3)
    {
        auto data = db->Retrieve(tablename, i);
        if (write_bufs[i].compare(data.Get()) != 0)
            found_wrong = true;
    }

    test("retrieve", "compare", [&]() -> bool
         { return !found_wrong; });
}

void test_delete()
{
    bool comp_found_wrong = false, reuse_found_wrong = false;
    uint64_t id;

    for (id = 0; id < nitems; id += 4)
    {
        db->Delete(tablename, id);
        auto data = db->Retrieve(tablename, id);
        if (data.Valid() == true)
            comp_found_wrong = true;
    }

    uint64_t newid = db->Create(tablename, write_bufs[0].c_str(), write_bufs[0].size());
    if (newid != id - 4)
        reuse_found_wrong = true;
    test("delete", "compare", [&]() -> bool
         { return comp_found_wrong == false; });
    test("delete", "reuse", [&]() -> bool
         { return reuse_found_wrong == false; });
}

static void single_thread_test()
{
    test_create();
    test_update();
    test_retrieve();
    test_delete();
}

void thread_entry(int tid, mutex *iomutex, barrier<> *delete_barrier)
{
    vector<uint64_t> id_vec;
    bool size_found_wrong = false, data_found_wrong = false;
    bool delete_found_error = false, update_found_error = false;
    string new_write_buf_1("first_string");
    string new_write_buf_2("second string");

    // Create
    for (int i = 0; i < nitems; ++i)
    {
        uint64_t newid = db->Create(tablename, write_bufs[i].c_str(), write_bufs[i].size() + 1);
        id_vec.push_back(newid);
    }

    // Retrieve
    int i = 0;
    for (auto id : id_vec)
    {
        auto data = db->Retrieve(tablename, id);
        if (write_bufs[i].compare(data.Get()) != 0)
            data_found_wrong = true;
        if ((write_bufs[i].size() + 1) != data.Size())
        {
            lock_guard<mutex> lk(*iomutex);
            printf("Error size: %ld (expect %ld)\n", data.Size(), write_bufs[i].size() + 1);
            size_found_wrong = true;
        }
        i++;
    }

    // Update
    for (auto id : id_vec)
    {
        if (tid == 1)
            db->Update(tablename, id, new_write_buf_1.c_str(), new_write_buf_1.size() + 1);

        else
            db->Update(tablename, id, new_write_buf_2.c_str(), new_write_buf_2.size() + 1);
    }

    // Check Update
    for (auto id : id_vec)
    {
        auto data = db->Retrieve(tablename, id);
        if (tid == 1 && new_write_buf_1.compare(data.Get()))
            update_found_error = true;
        else if (tid != 1 && new_write_buf_2.compare(data.Get()))
            update_found_error = true;
        delete[] data.Get();
    }

    // Delete and check
    delete_barrier->arrive_and_wait();
    int j = 0;
    for (auto id : id_vec)
    {
        db->Delete(tablename, id);
        auto data = db->Retrieve(tablename, id);
        if (data.Valid())
            delete_found_error = true;
        ++j;
    }

    iomutex->lock();
    printf("[Thread %d] "
           "data: %s\033[0m\t"
           "size: %s\033[0m\t"
           "update: %s\033[0m\t"
           "delete: %s\033[0m\n",
           tid,
           data_found_wrong ? "\033[0;31mFAIL" : "\033[0;33mPASS",
           size_found_wrong ? "\033[0;31mFAIL" : "\033[0;33mPASS",
           update_found_error ? "\033[0;31mFAIL" : "\033[0;33mPASS",
           delete_found_error ? "\033[0;31mFAIL" : "\033[0;33mPASS");
    iomutex->unlock();
}

static void multi_thread_test(int nthreads)
{
    vector<thread *> threads;
    barrier delete_barrier(nthreads);
    mutex iomutex;
    for (int i = 0; i < nthreads; ++i)
    {
        threads.push_back(new thread(thread_entry, i, &iomutex, &delete_barrier));
    }

    for (auto th : threads)
        th->join();
}

int main(int argc, char *argv[])
{
    vector<pair<char, string>> opt_list{
        pair<char, string>{'h', "help"},
        pair<char, string>{'n', "number of items"},
        pair<char, string>{'t', "number of threads"},
        pair<char, string>{'M', "item max len"},
        pair<char, string>{'m', "item min len"},
        pair<char, string>{'s', "real data unit size"},
        pair<char, string>{'a', "once allocate number of meta data"}};

    int opt, opt_count = 0, nthreads;
    opterr = 0;

    while ((opt = getopt(argc, argv, "hn:t:M:m:s:a:")) != -1)
    {
        switch (opt)
        {
        case 'h':
            printf("Help\n");
            for (const auto &option : opt_list)
                printf("%c : %s\n", option.first, option.second.c_str());
            exit(EXIT_SUCCESS);

        case 'n':
            nitems = atoi(optarg);
            opt_count++;
            break;
        case 't':
            nthreads = atoi(optarg);
            opt_count++;
            break;
        case 'a':
            once_alloc_meta = atoi(optarg);
            opt_count++;
            break;
        case 'm':
            item_minlen = atoi(optarg);
            opt_count++;
            break;
        case 'M':
            item_maxlen = atoi(optarg);
            opt_count++;
            break;
        case 's':
            data_unit_size = atoi(optarg);
            opt_count++;
            break;
        default:
            fprintf(stderr, "Invalid input:%c %s\n", opt, optarg);
            exit(EXIT_FAILURE);
        }
    }

    if (opt_count != 6)
    {
        fprintf(stderr, "missing arguments.\n");
        exit(EXIT_FAILURE);
    }

    printf("\033[0;37mThe performance in testing should not be considered to compare.\033[0m\n");
    test_init();
    if (nthreads == 1)
        single_thread_test();
    else
        multi_thread_test(nthreads);

    delete db;

    return 0;
}
