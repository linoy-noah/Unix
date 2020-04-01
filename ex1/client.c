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
        perror("not enough arguments(2)");
        return 1;
    }
    int op;
    if (strcmp(argv[1], "get-new") == 0)
    {
        if (argc < 4)
        {
            perror("not enough arguments(4)");
            return 1;
        }
        op = 1;
    }
    else
    {
        if (strcmp(argv[1], "view-news") == 0)
        {
            op = 2;
        }
        else
        {
            perror("op not exist");
            return 1;
        }
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

    if (send(sock, (char *)&op, sizeof(int), 0) < 0)
    {
        perror("send");
        return 1;
    }

    int size;

    if (op == 1)
    { //get title outputfile
        char *token;
        char *rest = argv[2];

        token = strtok_r(rest, "“", &rest);
        size = strlen(token) + 1;
        if (send(sock, (char *)&size, sizeof(int), 0) < 0)
        {
            perror("send");
            return 1;
        } //send the size of the file's name
        if (send(sock, token, size, 0) < 0)
        {
            perror("send");
            return 1;
        }
        //send the file's name

        if ((nrecv = recv(sock, (char *)&size, sizeof(int), 0)) < 0)
        {
            perror("recv");
            return 1;
        }
        //recv the file size

        if (size == -14)
        {
            printf("Provided title does not exist.\n");
            return 1;
        }
        if (size == -1)
        {
            perror("\nserver fail to open file");
            return 1;
        }

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
        //recv file content

        //write the content to argv[3] file ;and print Successfully downloaded new to file argv3
        int fd;
        if ((fd = open(argv[3], O_WRONLY | O_CREAT | O_TRUNC, 0777)) < 0)
        {
            perror("open");
            return 1;
        }
        if (write(fd, buff, size) != size)
        {
            perror("write");
            return 1;
        }
        printf("Successfully downloaded new to file %s\n", argv[3]);

        free(buff);
    }

    if (op == 2)
    { //view
        int title_sz, classify;
        char *title_new;
        char *classy[] = {"", "-Economics", "-Science", "-Sports", "-Culture"};
        //recv title size
        if ((nrecv = recv(sock, (char *)&title_sz, sizeof(int), 0)) < 0)
        {
            perror("recv");
            return 1;
        }
        
        if (title_sz == -15)
        {

            printf("there is no news\n");
            return 1;
        }
        while (title_sz > 0)
        {
            title_new = (char *)malloc(title_sz);
            if (title_new == NULL)
            {
                perror("alloc");
                return 1;
            }
            //recv title
            if ((nrecv = recv(sock, title_new, title_sz, 0)) < 0)
            {
                perror("recv");
                return 1;
            }
            //recv class
            if ((nrecv = recv(sock, (char *)&classify, sizeof(int), 0)) < 0)
            {
                perror("recv");
                return 1;
            }
            //recv next title size
            if ((nrecv = recv(sock, (char *)&title_sz, sizeof(int), 0)) < 0)
            {
                perror("recv");
                return 1;
            }
            printf("%s%s\n", title_new, classy[classify]);
            free(title_new);
        }
        
    }
    return 0;
}
