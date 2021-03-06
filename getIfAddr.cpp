/*!
  \file getIfAddr.cpp
  \brief Obtains information about all the interfaces of the node

  Sample run:$ ./getIfAddr
Reference : http://man7.org/linux/man-pages/man3/getifaddrs.3.html

@author Aishwarya Babu
 */
#include <sys/types.h>
#include <ifaddrs.h>
#include <cstdlib>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <fstream>
#include <unistd.h>

# define NI_MAXHOST      1025
# define NI_MAXSERV      32

using namespace std;

int main(int argc, char *argv[])
{

    struct ifaddrs *ifaddr, *ifa; // ifa pointer iterator 
    char host[NI_MAXHOST];

    string connectionsFilename("connectionsList");
    fstream connectionsFile;

    if(getifaddrs(&ifaddr) == -1)
    {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    connectionsFile.open(connectionsFilename.c_str(), fstream::out);

    char hostname[1024];
    hostname[1023] = '\0';
    gethostname(hostname, 1023);
    //    cout<<hostname<<endl;
    connectionsFile<<hostname<<endl;

    int n;
    int family;
    //Walk through the linked list
    for(ifa=ifaddr, n=0; ifa!=NULL; ifa=ifa->ifa_next, n++)
    {
        if(ifa->ifa_addr == NULL)
            continue;

        family = ifa->ifa_addr->sa_family; //Address family : AF_XXX 
        string ifName(ifa->ifa_name);

        //prints interface name << address family << family
        string afFamily =  (family==AF_PACKET)?"AF_PACKET":(family==AF_INET)?"AF_INET":(family==AF_INET6)?"AF_INET6":"???";
        //              cout<<ifName<<" "<<afFamily<<" "<<family<<endl;

        if(family==2)
        {
            struct sockaddr_in *ip4addr = (struct sockaddr_in *)ifa->ifa_addr; //ifa_broadaddr;
            struct sockaddr_in *ip4Broadaddr = (struct sockaddr_in *)ifa->ifa_broadaddr; //ifa_broadaddr;
            char ipAddress[INET_ADDRSTRLEN];
            char ipBroadAddress[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(ip4addr->sin_addr), ipAddress, INET_ADDRSTRLEN);
            inet_ntop(AF_INET, &(ip4Broadaddr->sin_addr), ipBroadAddress, INET_ADDRSTRLEN);

            char *ifName = ifa->ifa_name;
            //                cout<<"interface name : "<<ifName<<endl;
            //              cout<<"broadcast address: "<<ipAddressStr<<endl;
            //                (if((ifName != "lo") && (ifName != "eth0")))
            connectionsFile<<ifName<<" "<<ipAddress<<" "<<ipBroadAddress<<endl;
            //              connectionsFile<<ifName<<" "<<afFamily<<" "<<ipAddress<<endl;

        }
    }

    connectionsFile.close();

    return 0;
}

