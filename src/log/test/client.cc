#include "spec.h"


int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        fprintf(stderr, "usage: %s <message>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    MessageQueue mq(mqname, maxmsg, msgsize);
    SentMessage msg;
    
    string data(argv[1]);

    msg.SetCommunicationCode(0); // New.
    msg.Add(data.c_str(), data.size() + 1);  
    mq.Send(msg, 0);

    return 0;
}