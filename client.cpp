#include "common.h"
#include "newport.h"
#include <iostream>
#include "math.h"
#include <fstream>		//needed for file checking
#include <stdlib.h>
#include <string>
#include <vector>		//needed for vectors
#include "newport2.h"		//needed for mySendingPort2 (no ARQ)
#include <sstream>
#include <algorithm> 		//needed to do the find command
#include <cstdlib>

#define advertisementInterval 10
#define lossPercent 0.05

using namespace std;

static const int maxLineLength=400;
static const int receivingPortNum = 6200;
static const int sendingPortNum = 6300;
static const int advPortNum = 6100;

struct hostnames
{
    string hostname_self;
    string hostname_bcast;
    string if_name;
};

int host_id = 1; //Host ID for 


void *receivedata(void *args)
{
    /*REQ Packet - Type 0
      RES Packet - Type 1   - to be implemented in client
      ADV Packet - Type 2*/

    struct res *sh2 = (struct res *)args;
    FILE *data;
    Packet *q;
    short size;
    while(1)
    {

        q = sh2->my_res_port->receivePacket();

        if (q!= NULL)
        {
            char type = q->accessHeader()->getOctet(0);
            //Receiving response
            if (type == '1')
            {
                sh2->my_req_port->setACKflag(true);
                sh2->my_req_port->timer_.stopTimer();
                char *outputstring[1];
                ofstream outputFile;
                int c_id = (int)q->accessHeader()->getOctet(1); //Get the content ID for which a file needs to be made.

                stringstream ss;
                ss << c_id;
                std::string str = ss.str();
                const char* chr = str.c_str();

                outputFile.open(chr,std::fstream::out |std::fstream::trunc); //create a file with the same name
                outputstring[1] = q->getPayload();//get the payload
                outputFile <<outputstring[1];//write the payload to the output file
                cout<<"Received response- content "<<c_id<<endl; //acknowledge to the user that we are done writing.

            }

        } //Closes if
    }//Closes while
    return NULL;

}//Closes void

struct hostnames SetupAddress(char *argv)
{
    string connectionsFilename(argv);
    fstream inputFile;
    char temp[maxLineLength];
    char temp1[maxLineLength];
    char temp2[maxLineLength];
    struct hostnames Hname;
    
    inputFile.open(connectionsFilename.c_str(), fstream::in);
    int i = 1;

    while(inputFile >> temp >> temp1 >> temp2)
    {
        if(i > 2)
        {
            Hname.hostname_self = string(temp1);
            Hname.hostname_bcast = string(temp2);
            Hname.if_name = string(temp);
        }
        i++;
    }

    inputFile.close();
    return Hname;    
}


int main(int argc, char * argv[])
{
    cout<<"I am "<<argv[1]<<endl;

    pthread_t thread; //for advertising
    pthread_t thread2; //for receiving all information

    //create advertising sending port with the destination port included. Then we will asend it to the thread.
    Address *my_adv_addr; //We advertise from here
    Address *my_res_addr; //We receive information from here
    Address *my_req_addr; //We request content from here
    Address *router_addr; //We address the router from here
    mySendingPort *my_req_port; //Requesting port; note the change in type (2 vs no 2)
    mySendingPort2 *my_adv_port;//Sending port for advertisements
    LossyReceivingPort *my_res_port; //Receiving port information

    struct hostnames Hname; 
    Hname = SetupAddress(argv[1]);

    cout<<"hostnames: "<<Hname.hostname_self<<" "<<Hname.hostname_bcast<<endl;

    try{

        my_req_addr = new Address(Hname.hostname_self.c_str(), sendingPortNum);
        my_res_addr = new Address(Hname.hostname_bcast.c_str(), receivingPortNum); //Earlier was _self
        my_res_addr->setInterfaceName(Hname.if_name.c_str());
        my_adv_addr = new Address(Hname.hostname_self.c_str(), advPortNum);
        router_addr = new Address(Hname.hostname_bcast.c_str(), receivingPortNum);  

        //Initialize requesting port (sending)
        my_req_port = new mySendingPort();
        my_req_port->setAddress(my_req_addr);
        my_req_port->setRemoteAddress(router_addr);
//        my_req_port->setRemoteAddress(my_res_addr);
        my_req_port->setBroadcast();
        my_req_port->init();                               

        //Initialize advertising port (sending)
        my_adv_port = new mySendingPort2();
        my_adv_port->setAddress(my_adv_addr);
        my_adv_port->setRemoteAddress(router_addr);
//        my_adv_port->setRemoteAddress(my_res_addr); //Shouldnt matter that it is the same as the receiving address since the we broadcast
        my_adv_port->setBroadcast();
        my_adv_port->init();

        //Initialize receiving port (receiving)
        my_res_port = new LossyReceivingPort(lossPercent);
        my_res_port->setAddress(my_res_addr);
        my_res_port->setBindtoDevice();
        my_res_port->init();
    }

    catch(const char *reason)
    {
        cerr << "Exception:" << reason << endl;
        exit(-1);
    }

    struct adv *sh;
    sh = (struct adv*)malloc(sizeof(struct adv));
    sh->my_adv_port = my_adv_port;

    //creating thread to advertise

    pthread_create(&(thread), 0,&advertisement,sh);

    struct res *sh2;
    sh2 = (struct res*)malloc(sizeof(struct res));
    sh2->my_res_port = my_res_port;
    sh2->my_req_port = my_req_port;

    //creating thread to receive
    pthread_create(&(thread2),0,&receivedata,sh2);

    while(1)
    {
        string input;
        string input2;
        //int hdrSize = 256;
        //get user input on
        cout << "Prompt> ";
        cin >> input >> input2;      

        if(input.compare("add")==0)
        {
//NEEDS GENERALIZING
            string src_path = "../";
            string path = "cp " + src_path + input2 + " " + input2;
            const char* content_add = path.c_str();
            system(content_add); //Makes a system call to physically copy file from parent directory to child directory

            //Code checks to see if the file actually exists; else no reason to add to the content table.
            FILE *pfile;
            pfile = fopen(input2.c_str(),"r");
            if (pfile == NULL)
            {
                continue;
            }

            else
            {
                fclose(pfile); //need to close the open file!
                int input2_int = atoi(input2.c_str());

                //Add value to content table if its unique.

                if(std::find(content.begin(),content.end(),input2_int) == content.end())
                {
                    content.push_back(input2_int);
                }

                cout << "This host has "<< content.size() <<" content(s) in its library" <<endl;

                for(unsigned int n=0;n<content.size();n++)
                {
                    cout<<content[n]<<", ";
                }

                printf("\n");
            } //closes else

        } //closes if compare

        if(input.compare("get")==0)
        {
            Packet *req_packet;
            req_packet = new Packet();
            req_packet->setPayloadSize(0); //No Payload

            PacketHdr *rqhdr = req_packet->accessHeader();

            rqhdr->setHeaderSize(3); //Need 3 bytes for the header
            rqhdr->setOctet('0',0); //Request packet = type 0
            rqhdr->setOctet((char)host_id,2); //Setting host id

            int input2_int = atoi(input2.c_str());

            rqhdr->setOctet((char)input2_int,1); //Setting content request message

            my_req_port->sendPacket(req_packet);
            my_req_port->lastPkt_ = req_packet;
            //cout<<"First octet "<<rqhdr->getOctet(0)<<"Second Octet "<<rqhdr->getOctet(1)<<"Third Octet "<<rqhdr->getOctet(2)<<endl;
            cout<<"Sent Request"<<endl;
            my_req_port->setACKflag(false);
            my_req_port->timer_.startTimer(5);

            while(!my_req_port->isACKed())
            {
                sleep(1);
                if(!my_req_port->isACKed())
                {
                    sleep(3);
                    if(!my_req_port->isACKed())
                    {
                        sleep(5);
                        if(!my_req_port->isACKed())
                        {
                            sleep(7);
                            if(!my_req_port->isACKed())
                            {
                                sleep(9);
                                if(!my_req_port->isACKed())
                                {
                                    cout<<"Giving up.."<<endl;							
                                    my_req_port->setACKflag(true);
                                }	
                            }
                        }
                    }
                } 
                else{continue;}
            }
        }

        if(input.compare("delete")==0)
        {
            string path_r = "rm " + input2;
            const char* content_remove = path_r.c_str();
            system(content_remove);

            for( std::vector<int>::iterator iter = content.begin(); iter !=content.end();++iter)
            {

                if(*iter == atoi(input2.c_str()))
                {
                    content.erase(iter);
                    cout<< "Content "<<input2<<" deleted"<<endl;
                    break;
                }
            }

            for(unsigned int n=0;n<content.size();n++)
            {
                cout<<content[n]<<" ";
            }

            printf("\n");


        }

        if(input.compare("exit")==0)
        {
            cout <<"shutting down host" <<endl;
            return 0;
        }


    } //while close


    return 0;
}
