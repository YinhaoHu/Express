#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <cstdio>
#include <cstdlib>

using namespace std;

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        printf("usage: <file> <off> <nbytes>\n");
        exit(EXIT_FAILURE);
    }

    string filename(argv[1]);
    off_t off = atol(argv[2]);
    size_t size = atol(argv[3]);
    char *buf = new char[size];
    int fd = open(filename.c_str(), O_RDONLY);

    if (fd < 0)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }

    printf("Bytes in file : %s\n", filename.c_str());
    pread(fd, buf, size, off);

    printf("Text: %s\nBytes:\n", buf);
    for (int i = 0; i < size; ++i)
    {
        if (i % 8 == 0)
            printf("%6d ~ %-6d byte:\t ", i, i + 7);
        printf("%x ", buf[i]);
        if (i != 0 && i % 8 == 7)
        {
            putchar('\n');
        }
    }
    printf("\n");

    close(fd);

    delete[] buf;
    return 0;
}