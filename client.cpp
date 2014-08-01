/*!
  \file client.cpp
  \brief Implementation of Client

  Sample run:$ ./router connectionsList 1
  Client only makes requests for content

  @author Aishwarya Babu
  @author Rakesh Ravuru
  @author Sudarshan Kandi
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
#define lossPercent 0.05

using namespace std;

static const int maxLineLength=400;
static const int receivingPortNum = 6200; //! Fixed receiving port number
static const int sendingPortNum = 6300; //! Fixed sending port number

/*! 
  \brief Structure containing interface information of the client
 */
struct hostnames
{
    string hostname_self;
    string hostname_bcast;
    string if_name;
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
int host_id = 1; //Host ID for 

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
    Packet *q;
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
        i++;
    }
    inputFile.close();
    return Hname;    
}

/*
   \brief Request a certain content by id

   Function to send a packet requesting content and to resend the request if not received on the receiving thread
 */
void GetContent(string contentId, struct res *args)
{
    Packet *req_packet;
    req_packet = new Packet();
    req_packet->setPayloadSize(0); //No Payload

    PacketHdr *rqhdr = req_packet->accessHeader();

    rqhdr->setHeaderSize(3); //Need 3 bytes for the header
    rqhdr->setOctet('0',0); //Request packet = type 0
    rqhdr->setOctet((char)host_id,2); //Setting host id

    int contentId_int = atoi(contentId.c_str());

    rqhdr->setOctet((char)contentId_int,1); //Setting content request message

    args->my_req_port->sendPacket(req_packet);
    args->my_req_port->lastPkt_ = req_packet;
    //cout<<"First octet "<<rqhdr->getOctet(0)<<"Second Octet "<<rqhdr->getOctet(1)<<"Third Octet "<<rqhdr->getOctet(2)<<endl;
    cout<<"Sent Request"<<endl;
    args->my_req_port->setACKflag(false);
    args->my_req_port->timer_.startTimer(5);

    while(!args->my_req_port->isACKed())
    {
        sleep(1);
        if(!args->my_req_port->isACKed())
        {
            sleep(3);
            if(!args->my_req_port->isACKed())
            {
                sleep(5);
                if(!args->my_req_port->isACKed())
                {
                    sleep(7);
                    if(!args->my_req_port->isACKed())
                    {
                        sleep(9);
                        if(!args->my_req_port->isACKed())
                        {
                            cout<<"Giving up.."<<endl;							
                            args->my_req_port->setACKflag(true);
                        }	
                    }
                }
            }
        } 
        else{return;}
    }
    return;
}

// sample run:$ ./client connectionsList 1 
int main(int argc, char * argv[])
{
    pthread_t thread2; //for receiving all information

    string getContentId = argv[2];
    //create advertising sending port with the destination port included. Then we will asend it to the thread.
    Address *my_res_addr; //We receive information from here
    Address *my_req_addr; //We request content from here
    Address *router_addr; //We address the router from here
    mySendingPort *my_req_port; //Requesting port; note the change in type (2 vs no 2)
    LossyReceivingPort *my_res_port; //Receiving port information

    struct hostnames Hname; 
    Hname = SetupAddress(argv[1]);

    cout<<"hostnames: "<<Hname.hostname_self<<" "<<Hname.hostname_bcast<<endl;

    try{

        my_req_addr = new Address(Hname.hostname_self.c_str(), sendingPortNum);
        my_res_addr = new Address(Hname.hostname_bcast.c_str(), receivingPortNum); //Earlier was _self
        my_res_addr->setInterfaceName(Hname.if_name.c_str());
        router_addr = new Address(Hname.hostname_bcast.c_str(), receivingPortNum);  

        //Initialize requesting port (sending)
        my_req_port = new mySendingPort();
        my_req_port->setAddress(my_req_addr);
        my_req_port->setRemoteAddress(router_addr);
        //        my_req_port->setRemoteAddress(my_res_addr);
        my_req_port->setBroadcast();
        my_req_port->init();                               

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

    struct res *sh2;
    sh2 = (struct res*)malloc(sizeof(struct res));
    sh2->my_res_port = my_res_port;
    sh2->my_req_port = my_req_port;

    //creating thread to receive
    pthread_create(&(thread2),0,&receivedata,sh2);

    GetContent(getContentId, sh2);
    while(1)
    {
        //So that program does not end
    } //while close
    return 0;
}
