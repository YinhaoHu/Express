#include "spec.h"


static void process_message(ReceivedBoundaryMessage& msg, uint priority)
{
    auto field_pointer = msg[0].pData;  // New
    auto field_size = msg[0].size;      // New
 
    string str(field_pointer);

    print(FormatString("Receive: %s (nbytes = %ld length = %ld priority = %d)\n", 128,
         str.c_str(), field_size, str.size(), priority));

    if(str.compare("bye") == 0)
        throw std::runtime_error("Client wants to say bye!");
}



int main(int argc, char *argv[])
{ 
    MessageQueue mq(mqname, maxmsg, msgsize);

    cout << "Server is running now.\n";
    for( ; ;)
    {
        auto pmsg = mq.Receive();
        auto& msg = pmsg->message;
        auto prio = pmsg->priority;

        try
        {
            if(msg.GetHeaderField(
                    ReceivedBoundaryMessage::Header::Field::kCommunicationCode) == 0)
                process_message(msg,prio);
            else 
                continue;
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            break;
        }  
    }

    cout << "Server is cloed.\n";
    mq.Unlink();
    return 0;
}
 