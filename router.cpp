/*!
  \file router.cpp
  \brief Implementation of router

  Sample run:$ ./router connectionsList
  Router forwards the packets depending on the type of the packet to the appropriate next hop based on the routingtable and pending request table. In case of advertisements, it forwards them via all interfaces except the one it wa received on

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
#define lossPercent 0.05
using namespace std;

static int globalTimer=0; //!< Global timer - Clock for the Routing and Pending Request Tables
static vector<vector<string> >connectionsList; //!< Connections table- 2D vector keeping track of interface ip address + broadcast address + interface name 
static const int maxLineLength=400;
static const int receivingPortNum = 6200; //! Fixed receiving port number
static const int sendingPortNum = 6300; //! Fixed sending port number 

/*!
  \brief Structure containing interface/ port information for a thread 

  Argument to be sent to <NodeRecProc>(void *arg)
 */
struct cShared{
    LossyReceivingPort* fwdRecvPort;
    mySendingPort* fwdSendPort; 
    int position; //!Location(index) in connections List
};

/*!
  \brief Single row of the routing table

  A single row of the routing table comprises : content id, the interface on which this content can be obtained, number of hops to this content, time for expiration of this information (updated based on advertisements)
 */
struct routingTableElement{
    int contentId; //! Content Id available in the network
    string recInterface; //! Interface on which the content can be reached 
    int numHops; //! Least number of hops to this content 
    int timeToExp;  //! Time for expiration of this information (updated by advertisements)
};

/*!
  \brief Single row of the Pending request table

  A single row of the pending request table comprises: the content id that has been requested, the id of the client(device) requesting it, the interface on which it has been received, time for expiration of this request 
 */
struct pendingTableElement{
    int requestedContentId; //! Content Id that has been requested 
    int requestingHostId; //! Id of the client/ host requesting the content
    string recInterface; //! Interface on which the request is received 
    int timeToExp;  //! Time for expiration of the request
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
  \brief Displays contents of Connections list
 */
void DisplayConnectionsList()
{
    cout<<"Connections List: "<<endl;
    for(unsigned int i = 0; i < connectionsList.size(); i++)
    {
        cout<<connectionsList[i][0]<<" "<<connectionsList[i][1]<<endl;
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
    char temp[maxLineLength]; //Interface name
    char temp1[maxLineLength]; //Interface IP address 
    char temp2[maxLineLength]; //Interface Broadcast address

    int i = 1;
    inputFile.open(connectionsFilename.c_str(), fstream::in);

    string self;
    getline(inputFile, self);

    while(inputFile >> temp >> temp1 >> temp2)
    {
        i++;
        if(i > 3)
        {
            vector<string> listElement;
            listElement.push_back(string(temp1));
            listElement.push_back(string(temp2));
            listElement.push_back(string(temp));
            connectionsList.push_back(listElement);
        }
    }

    inputFile.close();
}

/*!
  \brief Displays contents of Routing table
 */
void DisplayRoutingTable()
{
    cout<<"Routing Table: "<<endl;
    for(unsigned int i = 0; i < routingTable.size(); i++)
    {
        cout<<routingTable[i].contentId<<" "<<routingTable[i].recInterface<<" "<<routingTable[i].numHops<<" "<<routingTable[i].timeToExp<<endl;
    }
}

/*!
  \brief Adds a new routing table entry or edits the timer on an already existing entry

  If a new advertisement packet is received a new entry is made in the Routing table. If it is an update for an already existing entry the timer is updated.
 */
void AddRoutingTableEntry(int contentId, string recInterface, int numHops) //CHANGE RECPORTNUM to HOSTNAME - BROADCAST IP (all ports 6200 - receive)
{
    numHops+=1;

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
  \brief Displays contents of Pending Request table
 */
void DisplayPendingRequestTable()
{
    cout<<"PRT Table: "<<endl;
    for(unsigned int i = 0; i < pendingRequestTable.size(); i++)
    {
        cout<<pendingRequestTable[i].requestedContentId<<" "<<pendingRequestTable[i].requestingHostId<<" "<<pendingRequestTable[i].recInterface<<" "<<pendingRequestTable[i].timeToExp<<endl;
    }
}


/*!
  \brief Make a new entry in the Pending request Table or update an already existing entry
 */
void UpdatePendingRequestTable(int requestedContentId, int requestingHostId, string recInterface)
{
    struct pendingTableElement prtElem;
    prtElem.requestedContentId = requestedContentId;
    prtElem.requestingHostId = requestingHostId;

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
    cout<<"Request Received: "<<endl;
    DisplayPendingRequestTable();
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
    //    return NULL;
    return "";
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
            //Packet type: Request
            if(recvPacket->accessHeader()->getOctet(0) == '0')
            {
                int requestedContentId = int(recvPacket->accessHeader()->getOctet(1));
                int requestingHostId = int(recvPacket->accessHeader()->getOctet(2)); 
                int position = sh->position;
                string receivingInterface = connectionsList[position][1];

                //Look up routing table based on content id and Forward to appropriate next hop
                string nextHopRecvInterface = getReceivingInterface(requestedContentId);

                if(nextHopRecvInterface != receivingInterface) //Broadcast causes re sending of request to self
                {
                    Address* dstAddr = new Address(nextHopRecvInterface.c_str(), receivingPortNum);
                    sh->fwdSendPort->setRemoteAddress(dstAddr);
                    sh->fwdSendPort->sendPacket(recvPacket);
                    delete(dstAddr);

                    //Make entry in pending request table                 
                    UpdatePendingRequestTable(requestedContentId, requestingHostId, receivingInterface); //PRT converts receiving port to dest port
                }
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
                string receivingInterface = connectionsList[position][1];

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
    Address* recvAddr;  //! Address of destination - Set to the broadcast address 
    Address* sendAddr; //! Address of the sending interface - Set to IP address of interface
    mySendingPort* sendPort; //! Sending port corresponding to sendAddr
    LossyReceivingPort* recvPort; //!Receiving port corresponding to recvAddr

    try{
        recvAddr = new Address(connectionsList[position][1].c_str(), receivingPortNum);  //CHANGE "localhost" to second argument and ports[0] to 6200
        recvAddr->setInterfaceName(connectionsList[position][2].c_str());
        sendAddr = new Address(connectionsList[position][0].c_str(), sendingPortNum);  //CHANGE "localhost" to second argument and ports[2] to 6300
        sendAddr->setInterfaceName(connectionsList[position][2].c_str());
        //        dstAddr =  new Address("localhost", ports[2]); //NEEDS TO GO and edit common.cpp line 380 to get rid of assertion

        recvPort = new LossyReceivingPort(lossPercent);
        recvPort->setAddress(recvAddr);
        recvPort->setBindtoDevice();

        sendPort = new mySendingPort();
        sendPort->setAddress(sendAddr);
        sendPort->setBroadcast();  //ADD THIS

        sendPort->init();
        recvPort->init();

    } catch(const char *reason ){
        cerr << "Exception:" << reason << endl;
        return;
    }

    //pthread_create() - with sender
    struct cShared *sh;
    sh = new struct cShared;
    sh->fwdRecvPort = recvPort;
    sh->fwdSendPort = sendPort;
    sh->position = position;
    pthread_create(thread, 0, &NodeRecProc, sh);
    //    pthread_join(thread, NULL);
}

/*!
  \brief Main function calls the separate threads for each receiving port
 */
int main(int argc, char* argv[])
{
    CreateConnectionsList(argv[1]);
    DisplayConnectionsList();

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
