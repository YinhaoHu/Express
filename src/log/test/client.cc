#include "spec.h"


int main(int argc, char *argv[])
{
    MessageQueue mq(mqname, maxmsg, msgsize);
    
    if(argc != 2)
    {
        fprintf(stderr, "usage: %s <message>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    string str(argv[1]);

    Message msg(1); // Any communication code here is okay, just testing.
    msg.Add(str.c_str(), str.size() + 1); 
    // +1 means the termination character of a string, which is '\0'

    mq.Send(msg, 0); // Any priority here is okay, just testing,

    return 0;
}