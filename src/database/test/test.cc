#include "database/client.h"
#include "database/public.h"
#include "utility/test/tool.h"
#include "utility/ipc/unix_domain_socket.h"
#include <sys/wait.h>
#include <bits/stdc++.h>

using namespace std;
using namespace express;
using namespace database;
using namespace utility;

static string tablename("test_db_dir");

static size_t nitems = 256, data_unit_size = 64, once_alloc_meta = 64;
static size_t item_minlen = 1, item_maxlen = 512, nthreads = 1;
constexpr char item_min_ch = 'a', item_max_ch = 'z';

vector<string> write_bufs;
const string table_name("test-table");
static Client *db;

static void parse_argv(int argc, char *argv[]);
static void client_init();
static void client_create();
static void client_update();
static void client_delete();
 

int main(int argc, char *argv[])
{
    parse_argv(argc, argv);
    auto socket_pair = ipc::UnixDomainSocket::SocketPair();
    int server_sock = socket_pair.first;
    int client_sock = socket_pair.second;
    const char* db_dir_name = "bin/dbdir";
 
    int child_pid = fork();
    if (child_pid < 0)
    {
        perror("fork()\n");
        exit(EXIT_FAILURE);
    }
    else if (child_pid == 0)
    {
        close(client_sock);
        if (execl("./bin/db-server", "./bin/db-server",
                  db_dir_name, "log-mq-name",
                  to_string(server_sock).c_str(), to_string(nthreads).c_str(), nullptr) < 0)
        {
            perror("child process execl");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        close(server_sock);
        
        db = new Client(client_sock);

        client_init();
        client_create();
        client_update();
        client_delete();

        delete db;
    } 
    kill(child_pid, database::kTerminateSignal); 
    remove(db_dir_name);

    int stat;

    wait(&stat);
    cout << format("\033[0;36mServer process exits({}).\033[0m", stat) << endl;
    return 0;
}

static void parse_argv(int argc, char *argv[])
{
    unordered_map<char, string> opt_list = {
        {'h', "help"},
        {'n', "number of items"},
        {'t', "number of threads"},
        {'M', "item max len"},
        {'m', "item min len"},
        {'s', "real data unit size"},
        {'a', "once allocate number of meta data"}};

    int opt;

    while ((opt = getopt(argc, argv, "hn:t:M:m:s:a:")) != -1)
    {
        switch (opt)
        {
        case 'h':  
            for (const auto &option : opt_list)
                cout << format("-{}:{}\n",option.first, option.second.c_str());
            exit(EXIT_SUCCESS);

        case 'n':
            nitems = atoi(optarg);  
            break;
        case 't':
            nthreads = atoi(optarg);  
            break;
        case 'a':
            once_alloc_meta = atoi(optarg);  
            break;
        case 'm':
            item_minlen = atoi(optarg);  
            break;
        case 'M':
            item_maxlen = atoi(optarg);  
            break;
        case 's':
            data_unit_size = atoi(optarg);  
            break;
        default:
            cerr << format("invalid option: {}\n", opt);
            cerr << format("Try `{}` -h for more information.\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    cout << format( "Test Arguments(Try `{}` -h for more information.)\n"\
                    "nitems             = {}\n"\
                    "nthreads           = {}\n"\
                    "once_alloc_meta    = {}\n"\
                    "data_unit_size     = {}\n"\
                    "item_min_len       = {}\n"\
                    "item_max_len       = {}\n\n",
                    argv[0], nitems, nthreads, once_alloc_meta,
                    data_unit_size, item_minlen, item_maxlen);
}

static void client_init()
{
    db->MakeTable(tablename, data_unit_size, once_alloc_meta);
    sleep(1);
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

static void client_create()
{
    bool result = true;

    for (size_t i = 0; i < nitems; ++i) 
        db->Create(tablename, write_bufs[i].c_str(), write_bufs[i].size() + 1); 

    for (size_t i = 0; i < nitems; ++i)
    {
        auto data = db->Retrieve(tablename, i);
        if (write_bufs[i].compare(data.Get().get()) != 0)
            result = false;
    }

    test::test("create", "compare", [&result]() -> bool
               { return result == true; });
}

static void client_update()
{ 
    string update_data("123456789123456789123456789");
    vector<express::database::id_t> update_ids{0, 5, 10, 32, 63, 64, 65, 127, 128, 129};

    for (auto id : update_ids)
    {
        if(id >= nitems)
            break;
        write_bufs[id] = update_data;
        db->Update(tablename, id, update_data.c_str(), update_data.size() + 1);
    }

    bool result = true;
    for (auto id : update_ids)
    {
        if(id >= nitems) 
            break;
        auto new_data = db->Retrieve(tablename, id);

        if (update_data.compare(new_data.Get().get()) != 0)
            result = false;
    }
    test::test("udpate", "compare", [result]() -> bool
               { return result; });
}

static void client_delete()
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
    test::test("delete", "compare", [&]() -> bool
               { return comp_found_wrong == false; });
    test::test("delete", "reuse", [&]() -> bool
               { return reuse_found_wrong == false; });
}