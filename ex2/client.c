//client
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <fcntl.h>

#define PORT 0x0da2
#define IP_ADDR 0x7f000001

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        perror("not enough arguments");
        return 1;
    }
   

    int sock = socket(AF_INET, SOCK_STREAM, 0), nrecv;
    struct sockaddr_in s = {0};
    s.sin_family = AF_INET;
    s.sin_port = htons(PORT);
    s.sin_addr.s_addr = htonl(IP_ADDR);
    if (connect(sock, (struct sockaddr *)&s, sizeof(s)) < 0)
    {
        perror("connect");
        return 1;
    }

   

    int size=strlen(argv[1])+1;
     if (send(sock,(char*)&size, sizeof(int), 0) < 0)
    {
        perror("send");
        return 1;
    }//send the lenght of the file name

     if (send(sock, argv[1], size, 0) < 0)
    {
        perror("send");
        return 1;
    } // send file name

        if ((nrecv = recv(sock, (char *)&size, sizeof(int), 0)) < 0)
        {
            perror("recv");
            return 1;
        }
        //recv the lenght of the path

    

        char *buff = (char *)malloc(size);
        if (buff == NULL)
        {
            perror("alloc");
            return 1;
        }

        if ((nrecv = recv(sock, buff, size, 0)) < 0)
        {
            perror("recv");
            return 1;
        }
        //recv path

      
        printf("%s, %s\n",argv[1], buff); //print path

        free(buff);
    

   
    return 0;
}
