/*!
  \file router.cpp
  \brief Implementation of router

  @author Aishwarya Babu
  @author Rakesh Ravuru
  @author Sudarshan Kandi
 */
#include<iostream>
#include"common.h"
#include"newport.h"
#include<vector>
#include<fstream>
#include<list>

/** Time to expire for Routing Table in seconds*/
#define rtTimeToExpire 27
/** Time to expire for Pending Request Table in seconds*/
#define prtTimeToExpire 25
/** Time before Timer wraps around*/
#define timerWrap 6000
/** Delay to check and update timers in Routing and Pending Request tables */
#define sleepDelay 1
/** Loss percentage for Lossy receiving port*/
#define lossPercent 0.2
using namespace std;

static int globalTimer=0; //!< Global timer - Clock for the Routing and Pending Request Tables
static vector<vector<string> >connectionsList; //!< Connections table- 2D vector mapping destination address to sending port+"receiving port"(interface) - MAY NOT BE REQUIRED WHEN WE WORK WITH BROADCAST ADDRESS 
static const int maxLineLength=400;
static const int receivingPortNum = 6200;
static const int sendingPortNum = 6300;

/*!
  Argument to be send to <NodeRecProc>(void *arg)
 */
struct cShared{
//    string receivingInterface;
    LossyReceivingPort* fwdRecvPort;
    mySendingPort* fwdSendPort;
    int position;
    //  int max;
};

struct routingTableElement{
    int contentId;
    string recInterface;
    int numHops;
    int timeToExp; 
};

struct pendingTableElement{
    int requestedContentId;
    int requestingHostId;
    string recInterface;
    int timeToExp;  
};

static vector<routingTableElement> routingTable; //!< Routing table- content id + receiving port + #hops + time to expire
static vector<pendingTableElement> pendingRequestTable; //!< Pending request table- Requested id + host id + destination port + time to expire
/*!
  \brief Displays 2dimensional vector/ table

  @param nameOfVector Vector to be printed
 */
void Display2DVector(vector<vector<int> > nameOfVector)
{
    for(unsigned int i = 0; i < nameOfVector.size(); i++)
    {
        for(unsigned int j = 0; j < nameOfVector[0].size(); j++)
            cout<<nameOfVector[i][j]<<" ";  
        cout<<endl;
    }
    cout<<endl;  
}

/*!
  \brief Creates a list of mapping between receiving, destination and sending ports

  A connection between 2 devices comprises 4 ports. This table maintains the corresponding port numbers for receiving, destination and sending ports that are used by these 2 devices. The port numbers are calculated by a formula. 

  @param argc Number of devices connected to + 1
  @param argv[] Source and the list of connected devices
 */
void CreateConnectionsList(char* argv)
{

    string connectionsFilename(argv); 
    //    vector<string> inputDataLines;
    fstream inputFile;
    char temp1[maxLineLength];
    char temp2[maxLineLength];

    int i = 1;
    inputFile.open(connectionsFilename.c_str(), fstream::in);

    while(inputFile >> temp1 >> temp2)
    {
        if(i > 2)
        {
            vector<string> listElement;
            listElement.push_back(string(temp1));
            listElement.push_back(string(temp2));
            connectionsList.push_back(listElement);
        }
        i++;
    }

    inputFile.close();
}

/*!
  \brief Adds a new routing table entry or edits the timer on an already existing entry

  If a new advertisement packet is received a new entry is made in the Routing table. If it is an update for an already existing entry the timer is updated.
 */
void AddRoutingTableEntry(int contentId, string recInterface, int numHops) //CHANGE RECPORTNUM to HOSTNAME - BROADCAST IP (all ports 6200 - receive)
{
    numHops+=1;
    //    int dstPortNum = SearchConnectionsTable(recPortNum);
    //Using config information build routing table
    if(routingTable.size()==0)
    {
        struct routingTableElement rtElem;
        rtElem.contentId = contentId;
        rtElem.recInterface = recInterface;
        rtElem.numHops = numHops;
        rtElem.timeToExp = globalTimer+rtTimeToExpire;

        routingTable.push_back(rtElem);
    }
    else
    {
        bool contentExists=false;
        for(unsigned int i = 0; i < routingTable.size(); i++)
        {
            if(routingTable[i].contentId==contentId)
            {
                contentExists=true;
                if(numHops <= routingTable[i].numHops)
                {
                    routingTable[i].recInterface=recInterface;
                    routingTable[i].numHops=numHops;
                    routingTable[i].timeToExp=globalTimer+rtTimeToExpire;      //Time to expire;
                }
                break;
            }
        }
        if(!contentExists)
        {
            struct routingTableElement rtElem;
            rtElem.contentId = contentId;
            rtElem.recInterface = recInterface;
            rtElem.numHops = numHops;
            rtElem.timeToExp = globalTimer+rtTimeToExpire;//Time to expire;
            routingTable.push_back(rtElem);
        }
    }

    cout<<"Ad received - Routing Table:"<<endl;   
    //    Display2DVector(routingTable);
}

/*!
  \brief If the timer wraps around the maximum value this functions resets it.
 */
void UpdateRoutingTableEntryTTL()
{
    for(unsigned int i=0; i<routingTable.size(); i++)
    {
        routingTable[i].timeToExp = routingTable[i].timeToExp - timerWrap;
    }

}

/*!
  \brief Delete a certain routing table entry

  @param row Row to be deleted
 */
void DeleteRoutingTableEntry(int row)
{
    routingTable.erase(routingTable.begin()+row);
    cout<<"Deleted entry - Routing Tab:"<<endl;
    //    Display2DVector(routingTable);

}

/*!
  \brief Check if any entries in the Routing table have expired

  If any entries in the Routing Table have expired they are deleted
 */
void CheckRoutingTableEntryExpired(int currentTime)
{
    for(unsigned int i=1; i<=routingTable.size(); i++)
    {
        if(routingTable[i-1].timeToExp == currentTime)
        {
            DeleteRoutingTableEntry(i-1);
            //routingTable.erase(routingTable.begin()+i-1);
            i--; // to ensure the deletion of 0th entry
        }
    }

}

/*!
  \brief Returns Receiving port of the connection for a given Content ID
 */
string getReceivingInterface(int requestedContentId)
{
    for(unsigned int i = 0; i < routingTable.size(); i++)
    {
        if(requestedContentId == routingTable[i].contentId)
        {
            return routingTable[i].recInterface; //Returns receiving port number   
        }
    }
//    return NULL;
    return ""; 

//return NULL gave terminate called after throwing an instance of 'std::logic_error'
//  what():  basic_string::_S_construct null not valid

}

/*!
  \brief Returns Number of Hops to a given Content ID as per the routing table
 */
int getNumberHops(int requestedContentId)
{
    for(unsigned int i = 0; i < routingTable.size(); i++)
    {
        if(requestedContentId == routingTable[i].contentId)
        {
            return routingTable[i].numHops; //Returns number of hops   
        }
    }
    return -1;
}

/*!
  \brief Checks if a specific Content ID already exists in the routing table
 */
bool contentIdExists(int requestedContentId)
{
    for(unsigned int i = 0; i < routingTable.size(); i++)
    {
        if(requestedContentId == routingTable[i].contentId)
            return true;
    }
    return false;
}

/*!
  \brief Make a new entry in the Pending request Table or update an already existing entry
 */
void UpdatePendingRequestTable(int requestedContentId, int requestingHostId, string recInterface) //USE HOSTNAME? BROADCAST IP INSTEAD
{
    struct pendingTableElement prtElem;
    prtElem.requestedContentId = requestedContentId;
    prtElem.requestingHostId = requestingHostId;

    //Maintain destination Port numbers at PRT  
    //    int destPort = SearchConnectionsTable(receivingPort);

    bool contentExists = false;
    for(unsigned int i = 0; i < pendingRequestTable.size(); i++)
    {   
        if((requestedContentId == pendingRequestTable[i].requestedContentId) && (requestingHostId == pendingRequestTable[i].requestingHostId))
        {
            contentExists = true;
            pendingRequestTable[i].timeToExp = globalTimer+ prtTimeToExpire;
            break;
        }
    }

    if(!contentExists)
    {
        prtElem.recInterface = recInterface;
        prtElem.timeToExp = globalTimer+prtTimeToExpire; // Time to expire
        pendingRequestTable.push_back(prtElem);

    }
    cout<<"Request recd - Pending Req Tab:"<<endl;
    //    Display2DVector(pendingRequestTable);
}

/*!
  \brief Update the timer in Pending Request table if timer wraps around
 */
void UpdatePendingRequestTableTTL()
{
    for(unsigned int i=0; i<pendingRequestTable.size(); i++)
    {
        pendingRequestTable[i].timeToExp = pendingRequestTable[i].timeToExp - timerWrap;
    }
    cout<<"Updated TTL - Pending Request Table"<<endl;
    //    Display2DVector(pendingRequestTable);
}

/*!
  \brief Delete a certain pending request table entry using unique combination of content ID and Host ID

  This function is essential in case a certain pending request times out.
 */
void DeletePendingRequestTableEntry(int requestedContentId, int requestingHostId)
{
    for(unsigned int i=0; i<pendingRequestTable.size(); i++)
    {
        if((pendingRequestTable[i].requestedContentId == requestedContentId)&&(pendingRequestTable[i].requestingHostId == requestingHostId))
        {
            pendingRequestTable.erase(pendingRequestTable.begin()+i);
            break;
        }
    }
    cout<<"Deleted entry - PRT:"<<endl;
    if(pendingRequestTable.size()==0)
        cout<<"No pending req"<<endl;
    //    else
    //        Display2DVector(pendingRequestTable);

}

/*!
  \brief Check if any entries in the Pending Request table have expired

  If any entries in the Pending Request Table have expired they are deleted
 */
void CheckPendingRequestTableExpired(int currentTime)
{
    for(unsigned int i=1; i<=pendingRequestTable.size(); i++)
    {
        if(pendingRequestTable[i-1].timeToExp == currentTime)
        {
            int contentID = pendingRequestTable[i-1].requestedContentId;
            int hostID = pendingRequestTable[i-1].requestingHostId;
            DeletePendingRequestTableEntry(contentID, hostID);
            //            pendingRequestTable.erase(pendingRequestTable.begin()+i-1);
            i--; // to ensure the deletion of 0th entry
        }
    }
}

/*!
  \brief Return Destination port number associated with a given unique combination of content ID and host ID
 */
string SearchPendingRequestTable(int contentId, int hostId)
{
    for(unsigned int i = 0; i < pendingRequestTable.size(); i++)
    {
        if(contentId==pendingRequestTable[i].requestedContentId && hostId==pendingRequestTable[i].requestingHostId)
            return pendingRequestTable[i].recInterface;
    }
    return NULL;
}

/*!
  \brief Keeps track of which entries in the Routing and Pending request tables need to be deleted
 */
void ExpiryTimer()
{
    while(1)
    {
        sleep(sleepDelay);
        if(globalTimer>=timerWrap)
        {
            globalTimer=globalTimer-timerWrap;
            UpdatePendingRequestTableTTL();
            UpdateRoutingTableEntryTTL();
        }
        globalTimer++;
        CheckPendingRequestTableExpired(globalTimer);
        CheckRoutingTableEntryExpired(globalTimer);
    }
}

/*!
  \brief Receiver thread function which is constantly listening on the receiving port for a given connection

  Checks whether the packet is of type request, response or advertisement and accordingly services it 
 */
void* NodeRecProc(void* arg)
{
    cout<<"Created thread"<<endl;
    struct cShared *sh = (struct cShared *)arg;
    Packet* recvPacket;

    //Packet received : needs to be checked for appropriate forwarding and editing of table
    while(true)
    {
        recvPacket = sh->fwdRecvPort->receivePacket();
        if(recvPacket != NULL)
        {
            //            sh->fwdSendPort->sendPacket(recvPacket);
            //Request Packet
            if(recvPacket->accessHeader()->getOctet(0) == '0')
            {
                int requestedContentId = int(recvPacket->accessHeader()->getOctet(1));
                int requestingHostId = int(recvPacket->accessHeader()->getOctet(2)); 
//                string receivingInterface = sh->receivingInterface;
                int position = sh->position;
                string receivingInterface = connectionsList[position][0];


                //Look up routing table based on content id and Forward to appropriate next hop

                string nextHopRecvInterface = getReceivingInterface(requestedContentId);
                //                int nextHopDestPortNum = SearchConnectionsTable(nextHopRecvPortNum);
                Address* dstAddr = new Address(nextHopRecvInterface.c_str(), receivingPortNum);
                sh->fwdSendPort->setRemoteAddress(dstAddr);
                sh->fwdSendPort->sendPacket(recvPacket);
                delete(dstAddr);

                //Make entry in pending request table                 
                UpdatePendingRequestTable(requestedContentId, requestingHostId, receivingInterface); //PRT converts receiving port to dest port
            }
            //Response Packet
            else if(recvPacket->accessHeader()->getOctet(0) == '1')
            {
                int requestedContentId = int(recvPacket->accessHeader()->getOctet(1));
                int requestingHostId = int(recvPacket->accessHeader()->getOctet(2)); 
                string dstInterface = SearchPendingRequestTable(requestedContentId, requestingHostId);
                //Search for appropriate destination address in connections table
                if(dstInterface != "127.0.0.1") //FGURE OUT HOW TO CHANGE
                {
                    Address* dstAddr = new Address(dstInterface.c_str(), receivingPortNum);
                    sh->fwdSendPort->setRemoteAddress(dstAddr);
                    sh->fwdSendPort->sendPacket(recvPacket);
                    delete(dstAddr);

                    //Delete from pending request table entry
                    DeletePendingRequestTableEntry(requestedContentId, requestingHostId);
                }

            }
            //Announcement
            else if(recvPacket->accessHeader()->getOctet(0) == '2')
            {
                int receivedContentId = int(recvPacket->accessHeader()->getOctet(1));

//                string receivingInterface = sh->receivingInterface;
                int position = sh->position;
                string receivingInterface = connectionsList[position][0];

                int numHops = int(recvPacket->accessHeader()->getOctet(2));

                int currentHops = getNumberHops(receivedContentId);
                string currentReceivingInterface = getReceivingInterface(receivedContentId);

                bool noEntry = !(contentIdExists(receivedContentId));
                //                if(currentReceivingPort==-1)
                //                    noEntry = true;

                bool forwardFlag = false;
                if((receivingInterface == currentReceivingInterface))
                {
                    if(numHops <= (currentHops-1))
                    {
                        forwardFlag = true;
                    }
                }
                else
                { 
                    if(numHops < (currentHops-1))
                    {
                        forwardFlag = true;
                    }
                }


                if(forwardFlag || noEntry)
                {
                    AddRoutingTableEntry(receivedContentId, receivingInterface, numHops); //Takes care of updating timer and comparing num Hops
                    //Routing table converts receiving port num to appropriate dest port num
                    //Increment number of hops
                    numHops++;
                    recvPacket->accessHeader()->setOctet(char(numHops), 2);

                    //Forward to all other ports
                    string destInterfaceToSkip = receivingInterface;
                    //BroadcastPacket(destPortNumToSkip, recvPacket);
                    for(unsigned int i = 0; i < connectionsList.size(); i++)
                    {
                        string destInterface = connectionsList[i][1];
                        if(destInterfaceToSkip != destInterface)
                        {
                            //                        cout<<destPort<<endl;
                            Address* dstAddr = new Address(destInterface.c_str(), receivingPortNum);
                            sh->fwdSendPort->setRemoteAddress(dstAddr);
                            sh->fwdSendPort->sendPacket(recvPacket);
                            delete(dstAddr);
                        }
                    }
                }
            }
        }
    }
    return NULL;
}

/*!
  \brief Sets up the receiving and sending port numbers for a given connection and calls the thread function
 */
void StartNodeThread(pthread_t* thread, int position)
{

    //setup ports numbers
    Address* recvAddr;  //receive from port corresponding to node2 
    Address* sendAddr; // sending port corresponding to node1
    //    Address* dstAddr;  //address of node1 //NEEDS TO GO
    mySendingPort* sendPort; //sending port corr to send_addr
    LossyReceivingPort* recvPort; //receiving port corr to recvAddr;

    try{
        recvAddr = new Address(connectionsList[position][0].c_str(), receivingPortNum);  //CHANGE "localhost" to second argument and ports[0] to 6200
        sendAddr = new Address(connectionsList[position][0].c_str(), sendingPortNum);  //CHANGE "localhost" to second argument and ports[2] to 6300
        //        dstAddr =  new Address("localhost", ports[2]); //NEEDS TO GO and edit common.cpp line 380 to get rid of assertion

        recvPort = new LossyReceivingPort(lossPercent);
        recvPort->setAddress(recvAddr);

        sendPort = new mySendingPort();
        sendPort->setAddress(sendAddr);
        sendPort->setBroadcast();  //ADD THIS
        //        sendPort->setRemoteAddress(dstAddr); //NEEDS TO GO 

        sendPort->init();
        recvPort->init();

    } catch(const char *reason ){
        cerr << "Exception:" << reason << endl;
        return;
    }

    //pthread_create() - with sender
    struct cShared *sh;
    sh = (struct cShared*)malloc(sizeof(struct cShared));
    sh->fwdRecvPort = recvPort;
    sh->fwdSendPort = sendPort;
//    sh->receivingInterface = hostnames[0];
    sh->position = position;
    //    sh->max = n;
    //    pthread_t thread;
    pthread_create(thread, 0, &NodeRecProc, sh);
    //    pthread_join(thread, NULL);
}

/*!
  \brief Main function calls the separate threads for each receiving port
 */
int main(int argc, char* argv[])
{

    cout<<"I am router"<<endl;
    //sender 4000
    //receiver localhost 4001 

    //CREATE CONNECTIONS LIST: COPY BROADCAST ADDRESSES INTO VECTOR
    CreateConnectionsList(argv[1]);
    //    Display2DVector(connectionsList);

    int N = connectionsList.size();

    pthread_t threads[N];

    for(int i = 0; i < N; i++)
    {
        StartNodeThread(&(threads[i]), i);
    }
    ExpiryTimer();
    for(int i = 0; i < N; i++)
    {
        pthread_join(threads[i], NULL);
    } 
    return 0;
}
