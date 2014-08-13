/*!
  \file host.cpp
  \brief Implementation of host

  Sample run:$ ./host connectionsList 1 3 5
  Host contains certain content and services requests that it receives. At regular intervals the host sends advertisements to announce the content ids that it is hosting

  @author Sudarshan Kandi
  @author Rakesh Ravuru
  @author Aishwarya Babu
 */

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
#define lossPercent 0.0

using namespace std;

static const int maxLineLength=400;
static const int receivingPortNum = 6200; //! Fixed receiving port number 
static const int sendingPortNum = 6300; //! Fixed sending port number 
static const int advPortNum = 6100; //! Fixed port number to send advertisements 

/*! 
  \brief Structure containing interface information of the host
 */
struct hostnames
{
    string hostname_self; //! IP address of the interface
    string hostname_bcast; //! Broadcast IP address of the interface
    string if_name; //! Interface name
};

/*!
  \brief Structure containing advertisement port to send to advertisements thread

  This structure is sent as argument to the function: void *advertisement(void *args)  
 */
struct adv
{
    SendingPort *my_adv_port;
};

/*!
  \brief Structure containing interface(port) information to send to receiver thread

  This structure is sent as argument to the funciton: void *receivedata(void *args)
 */
struct res
{
    LossyReceivingPort *my_res_port;
    mySendingPort *my_req_port;
};

std::vector<int> content; //! Vector of int type to hold content IDs hosted
int host_id = 1; //setting host ID = 0

/*!
  \brief Thread function which sends advertisements at regular intervals

  The thread created runs a while loop sending advertisements at regular intervals on the  interface specified by the argument.
  @param args The interface is specified by the arguments *args. 
 */
void *advertisement(void *args)
{
    struct adv *sh = (struct adv *)args;
    while(1)
    {
        int hops = 0; //setting default hop count from host.

        Packet *adv_packet;
        adv_packet = new Packet();
        adv_packet->setPayloadSize(0); //No payload

        PacketHdr *hdr = adv_packet->accessHeader();

        hdr->setHeaderSize(3); //Need 3 bytes for the header
        hdr->setOctet('2',0); //Advertisement Packet type = 2
        hdr->setOctet((char)hops,2); //# of hops to content = 0

        for (unsigned int i=0;i!=content.size();i++)
        {
            hdr->setOctet((char)content[i],1);
            sh->my_adv_port->sendPacket(adv_packet);
        } 
        sleep(advertisementInterval); //do this every 10 seconds.
    }
    return NULL;    
}

/*!
  \brief Thread function which is waiting for packets on the interface specified

  The thread created runs a while loop waiting to receive packets on the interface specified by the argument.
  @param args The interface is specified by the arguments *args. 
 */
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
            //Servicing a request
            if (type == '0')
            {
                int c_id = (int)q->accessHeader()->getOctet(1);
                cout<<"Received request for content "<<c_id<<endl;
                //Creating Response Packet
                //char* str;
                //itoa (c_id,str, 10);

                bool contentExists = false;
                for(unsigned int i = 0; i < content.size(); i++)
                {
                    if(content[i] == c_id)
                    {
                        contentExists= true;    
                        break;
                    }
                }

                stringstream ss;
                ss << c_id;
                std::string str = ss.str();
                const char* chr = str.c_str();
                data = fopen(chr,"r"); //open file to be sent

                if (!contentExists)
                {
                    cout<<"This content is not hosted here"<<endl;
                    continue;
                }

                else
                {
                    fseek(data,0,SEEK_END);
                    size = ftell(data);
                    rewind(data);

                    char k[size];
                    fread(k,1,size,data); //Reading data into an array.
                    char *o = &k[0]; //Address for the beginning of data

                    Packet *res_packet;
                    res_packet = new Packet();
                    res_packet->setPayloadSize(size);

                    PacketHdr *reshdr = res_packet->accessHeader();

                    reshdr->setHeaderSize(5);
                    reshdr->setOctet('1',0); //Type 1 for response packet
                    reshdr->setOctet((char)c_id,1); //Setting Content ID that is being sent
                    char foreign_host = q->accessHeader()->getOctet(2);
                    reshdr->setOctet(foreign_host,2);//Setting
                    reshdr->setShortIntegerInfo(size,3);//Setting the size of the file

                    res_packet->fillPayload(size,o); //Adding the payload.
                    sh2->my_req_port->sendPacket(res_packet); //Sending the packet to the destination! BOOYA!
                    cout<<"Sent Response- content "<<c_id<<endl;
                }
            }
        }
    }
    return NULL;
}

/*!
  \brief Sets up the interface information into the structure #hostnames
 */
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

    string self;
    getline(inputFile, self);
    host_id = atoi(self.substr(1).c_str());
    cout<<"Selfid is: "<<host_id<<endl;

    while(inputFile >> temp >> temp1 >> temp2)
    {
        i++;
        if(i > 3)
        {
            Hname.hostname_self = string(temp1);
            Hname.hostname_bcast = string(temp2);
            Hname.if_name = string(temp);
        }
    }

    inputFile.close();

    return Hname;   
}

void AddContent(string contentId)
{
    //NEEDS GENERALIZING
    string src_path = "../";
    string path = "cp " + src_path + contentId + " " + contentId;
    const char* content_add = path.c_str();
    system(content_add); //Makes a system call to physically copy file from parent directory to child directory

    //Code checks to see if the file actually exists; else no reason to add to the content table.
    FILE *pfile;
    pfile = fopen(contentId.c_str(),"r");
    if (pfile == NULL)
    {
        return;
    }

    else
    {
        fclose(pfile); //need to close the open file!
        int contentId_int = atoi(contentId.c_str());

        //Add value to content table if its unique.

        if(std::find(content.begin(),content.end(),contentId_int) == content.end())
        {
            content.push_back(contentId_int);
        }

        cout << "This host has "<< content.size() <<" content(s) in its library" <<endl;

        for(unsigned int n=0;n<content.size();n++)
        {
            cout<<content[n]<<", ";
        }
        cout<<endl;
    }
}

/*!
  \brief Delete a specific content id

  The content id is removed from the directory and from the Contents list
  @param input2 Content Id (string) to be deleted
 */
void DeleteContent(string input2)
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
    cout<<endl;
}

/*!
  \brief Main function

  Defines the required interfaces (ports) and creates threads for advertisements and receiving packets
 */
int main(int argc, char * argv[])
{
    //    cout<<"I am "<<argv[1]<<endl;
    //ADD specific content to the host's directory
    for(int i = 2; i < argc; i++)
        AddContent(argv[i]);

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
            AddContent(input2);
        } //closes if compare

        if(input.compare("delete")==0)
        {
            DeleteContent(input2);
        }

        if(input.compare("exit")==0)
        {
            cout <<"shutting down host" <<endl;
            return 0;
        }

    } //while close


    return 0;
}
