#include <sys/types.h>
#include <ifaddrs.h>
#include <cstdlib>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <fstream>

# define NI_MAXHOST      1025
# define NI_MAXSERV      32

using namespace std;


int main(int argc, char *argv[])
{

        struct ifaddrs *ifaddr, *ifa; //ifa pointer iterator 
        char host[NI_MAXHOST];

        string connectionsFilename("connectionsList");
        fstream connectionsFile;

        if(getifaddrs(&ifaddr) == -1)
        {
                perror("getifaddrs");
                exit(EXIT_FAILURE);
        }

        connectionsFile.open(connectionsFilename.c_str(), fstream::out);

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
                char ipAddress[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &(ip4addr->sin_addr), ipAddress, INET_ADDRSTRLEN);
                string ipAddressStr(ipAddress);
//              cout<<"broadcast address: "<<ipAddressStr<<endl;
//                (if((ifName != "lo") && (ifName != "eth0")))
                connectionsFile<<ipAddress<<endl;
//              connectionsFile<<ifName<<" "<<afFamily<<" "<<ipAddress<<endl;
                }
        }

        connectionsFile.close();

        return 0;
}

