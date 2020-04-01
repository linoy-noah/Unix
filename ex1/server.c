//server
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

#define DATA_PATH "./data.txt"
#define PORT 0x0da2 // 3490
#define IP_ADDR 0x7f000001
#define QUEUE_LEN 20
#define LINE_LEN 1024
typedef struct News
{
    char *title;
    char *path;
    int class; /*0="UNDEFINED",1="Economics",2="Science",3="Sports", 4="Culture"*/
    struct News *next;
    struct News *prev;

} News;

void handleClient(int fd)
{

    News *head = NULL;
    News *temp = NULL;
    int data_file;
    if ((data_file = open("./dataFile.txt", O_RDONLY)) < 0)
    {
        perror("open");
    }
    else
    {

        char *title = NULL;
        char *path = NULL;
        int classify = 0;

        int lenght_file = lseek(data_file, 0, SEEK_END);
        lseek(data_file, 0, SEEK_SET);

        if (lenght_file > 0)
        {
            char *data = (char *)malloc(lenght_file);
            if (data == NULL)
            {
                perror("alloc");
            }
            int n;
            if ((n = read(data_file, data, lenght_file)) < 0)
            {
                perror("read");
            }
            int i, j, t;

            for (i = 0; i < lenght_file - 1; i++)
            {
                while (data[i] == '\n')
                {
                    i++;
                }
                for (j = i; data[j] != '|'; j++)
                {
                }
                temp = (News *)malloc(sizeof(News));
                if (temp == NULL)
                {
                    perror("alloc");
                }
                temp->title = (char *)malloc(j);
                if (temp->title == NULL)
                {
                    perror("alloc");
                }
                for (t = 0, j = i; data[j] != '|'; t++, j++)
                {
                    temp->title[t] = data[j];
                }
                temp->title[t] = '\0';
                for (i = ++j; data[j] != '|'; j++)
                {
                }
                temp->path = (char *)malloc(j);
                if (temp->path == NULL)
                {
                    perror("alloc");
                }
                for (t = 0, j = i; data[j] != '|'; t++, j++)
                {
                    temp->path[t] = data[j];
                }
                temp->path[t] = '\0';
                if (data[++j] == '0')
                {
                    temp->class = 0;
                }
                if (data[j] == '1')
                {
                    temp->class = 1;
                }
                if (data[j] == '2')
                {
                    temp->class = 2;
                }
                if (data[j] == '3')
                {
                    temp->class = 3;
                }
                if (data[j] == '4')
                {
                    temp->class = 4;
                }

                i = j;
                temp->prev = NULL;
                temp->next = head;
                if (head != NULL)
                {
                    head->prev = temp;
                }
                head = temp;
            }
        }
    }

    int nrecv, op;
    if ((nrecv = recv(fd, (char *)&op, sizeof(int), 0)) < 0)
    {
        perror("recv");
        return;
    }
    //recv the op
    int size;
    char *buffer;
    

    if (op == 1)
    { //get
        if ((nrecv = recv(fd, (char *)&size, sizeof(int), 0)) < 0)
        {
            perror("recv");
            return;
        } //recv title size

        buffer = (char *)malloc(size);
        if ((nrecv = recv(fd, buffer, size, 0)) < 0)
        {
            perror("recv");
            return;
        } //recv title

        temp = head;
        News *temp2 = NULL;
        while (temp != NULL)
        {
            if ((strcmp(temp->title, buffer)) == 0)
            {
                temp2 = temp;
            }
            temp = temp->next;
        } //serch by title

        if (temp2 == NULL)
        {
            size = -14; //news not exsit;
            if (send(fd, (char *)&size, sizeof(int), 0) < 0)
            {
                perror("send");
                return;
            }
        }
        free(buffer);
        int fileptr;
        if ((fileptr = open(temp2->path, O_RDONLY)) < 0)
        {
            perror("\nBad command arguments");
            size = -1;
            if (send(fd, (char *)&size, sizeof(int), 0) < 0)
            {
                perror("send");
                return;
            }
        } //open file by path

        size = lseek(fileptr, 0, SEEK_END); //get file size
        lseek(fileptr, 0, SEEK_SET);
        if (send(fd, (char *)&size, sizeof(int), 0) < 0)
        {
            perror("send");
            return;
        } //send file size

        buffer = (char *)malloc(size);
        int n;
        while ((n = read(fileptr, buffer, size)) > 0)
        {
        } //read from file to buffer
        if (send(fd, buffer, size, 0) < 0)
        {
            perror("send");
            return;
        } //send file content
        free(buffer);
    }

    if (op == 2)
    { //view

        //send if there is news
        if (head == NULL)
        {
            size = -15;

            if (send(fd, (char *)&size, sizeof(int), 0) < 0)
            {
                perror("send");
                return;
            }
            //send int -15 (empty);
            return;
        }
        temp = head;

        while (temp != NULL)
        {
            size = strlen(temp->title) + 1;
            if (send(fd, (char *)&size, sizeof(int), 0) < 0)
            {
                perror("send");
                return;
            }
            //send int size title
            if (send(fd, temp->title, size, 0) < 0)
            {
                perror("send");
                return;
            }
            //send title
            if (send(fd, (char *)&temp->class, sizeof(int), 0) < 0)
            {
                perror("send");
                return;
            }
            //send int class
            temp = temp->next;
        }
        if (temp == NULL)
        {
            size = 0;
            if (send(fd, (char *)&size, sizeof(int), 0) < 0)
            {
                perror("send");
                return;
            }
            //send 0; indicate the end of the list
        }
    }
}

void handleAdminInput()
{
    News *head = NULL;
    int op = 0;

    char buff[LINE_LEN * 3];
    char *token;
    char part1[15] = "";
    char part2[LINE_LEN] = "";
    char part3[15] = "";
    char *parts[3] = {part1, part2, part3};

    char *ops[] = {"add-new", "classify-new", "view-news"};
    char *class_p3[] = {"Economics", "Science", "Sports", "Culture"};

    struct News *temp = NULL;
    char title[LINE_LEN];
    while (1)
    {
        op = 0;
        printf("> ");
        gets(buff);

        char *rest = buff;

        for (int i = 0; (token = strtok_r(rest, " ", &rest)); i++)
        {
            strcpy(parts[i], token);
        }
        /*divde the string to parts ->(char*) parts[3] = {part1,part2,part3};*/
        for (int i = 0; i < 3; i++)
        {
            if (strcmp(part1, ops[i]) == 0)
            {
                op = i + 1;
            }
        } /*ops[]={"add-new","classify-new","view-news"}; switch by index in the op array*/

        /////

        /////

        switch (op)
        {
        case 1:
        { /*add new news and add to list*/
            int fd;

            if ((fd = open(part2, O_RDONLY)) < 0)
            {
                printf("Bad command arguments\n");
                break;
            }

            temp = (News *)malloc(sizeof(News));
            if (NULL == temp)
            {
                perror("memory alloc");
            }
            temp->path = (char *)malloc(strlen(part2) + 1);
            if (NULL == temp->path)
            {
                perror("memory alloc");
            }
            strcpy(temp->path, part2);

            int n;
            char *temp_T;
            char buffer[LINE_LEN * 2] = "";
            while ((n = read(fd, buffer, LINE_LEN * 2)) > 0)
            {
            }
            temp_T = strtok(buffer, "\n");
            strcpy(title, temp_T);
            /*assume that every line in the new is max 1024 charcters*/

            temp->title = (char *)malloc(strlen(title) + 1);
            if (NULL == temp->title)
            {
                perror("memory alloc");
            }
            strcpy(temp->title, title);

            close(fd);

            temp->class = 0; /*claasify as undefined*/

            temp->next = head;
            temp->prev = NULL;
            if (head != NULL)
            {
                head->prev = temp;
            }
            head = temp;
            printf("Successfully added new %s\n", part2);
            break;
        }

        case 2:
        { /*classfy news*/

            int classfy_index = 0;
            for (int i = 0; i < 4; i++)
            {
                if (strcmp(part3, class_p3[i]) == 0)
                {
                    classfy_index = i + 1;
                }
            }                    /*class_p3[]={"Economics","Science","Sports", "Culture"}; */
            int found_flag = -1; /*if the news title doesnt exist*/
            for (temp = head; temp != NULL; temp = temp->next)
            {
                if (strcmp(temp->title, part2) == 0)
                { ///
                    temp->class = classfy_index;
                    found_flag = 1;
                }
            }
            if (found_flag == -1)
            {
                perror("Provided title does not exist.");
                break;
            }
            else if (classfy_index != 0)
            {
                printf("Successfully classified %s as %s\n", part2, part3);
            }
            else
            {
                perror("Provided classification does not exist.");
                break;
            }

            break;
        }

        case 3:
        { /*print news title*/
            for (temp = head; temp != NULL; temp = temp->next)
            {
                printf("%s\n", temp->title);
            }
            break;
        }
        }
        /*print list*/
        if (op == 1 || op == 2)
        {
            int data_file;
            if ((data_file = open("./dataFile.txt", O_WRONLY | O_CREAT | O_TRUNC, 0777)) < 0)
            {
                perror("open");
            }
            else
            {
                for (temp = head; temp != NULL; temp = temp->next)
                {

                    if (write(data_file, temp->title, strlen(temp->title)) != strlen(temp->title))
                    {
                        perror("write");
                    }
                    if (write(data_file, "|", 1) != 1)
                    {
                        perror("write");
                    }
                    if (write(data_file, temp->path, strlen(temp->path)) != strlen(temp->path))
                    {
                        perror("write");
                    }
                    if (write(data_file, "|", 1) != 1)
                    {
                        perror("write");
                    }
                    switch (temp->class)
                    {
                    case 1:
                    {
                        if (write(data_file, "1", 1) != 1)
                        {
                            perror("write");
                        }
                        break;
                    }
                    case 2:
                    {
                        if (write(data_file, "2", 1) != 1)
                        {
                            perror("write");
                        }
                        break;
                    }
                    case 3:
                    {
                        if (write(data_file, "3", 1) != 1)
                        {
                            perror("write");
                        }
                        break;
                    }
                    case 4:
                    {
                        if (write(data_file, "4", 1) != 1)
                        {
                            perror("write");
                        }
                        break;
                    }
                    default:
                        if (write(data_file, "0", 1) != 1)
                        {
                            perror("write");
                        }
                        break;
                    }

                    if (write(data_file, "\n", strlen("\n")) != strlen("\n"))
                    {
                        perror("write");
                    }
                }

                close(data_file);
            }
        }
    }
    //free all memory allocated
    while (head != NULL)
    {
        temp = head->next;
        free(head->title);
        free(head->path);
        free(head);
        head = temp;
    }
}

int main(void)
{
    int data_file;
    if ((data_file = open("./dataFile.txt", O_WRONLY | O_CREAT | O_TRUNC, 0777)) < 0)
            {
                perror("open");
            }
    close(data_file);/*to clear the previous data*/
    int listenS = socket(AF_INET, SOCK_STREAM, 0);
    if (listenS < 0)
    {
        perror("socket");
        return 1;
    }
    struct sockaddr_in s = {0};
    s.sin_family = AF_INET;
    s.sin_port = htons(PORT);
    s.sin_addr.s_addr = htonl(IP_ADDR);
    if (bind(listenS, (struct sockaddr *)&s, sizeof(s)) < 0)
    {
        perror("bind");
        return 1;
    }
    if (listen(listenS, QUEUE_LEN) < 0)
    {
        perror("listen");
        return 1;
    }

    if (fork() == 0)
    {
        handleAdminInput();
        exit(0);
    }

    while (1)
    {
        struct sockaddr_in clientIn;
        int clientInSize = sizeof clientIn;
        int newfd = accept(listenS, (struct sockaddr *)&clientIn, (socklen_t *)&clientInSize);
        if (newfd < 0)
        {
            perror("accept");
            return 1;
        }
        int result = fork();
        if (result == 0)
        {
            close(listenS);
            handleClient(newfd);
            return 0;
        }
        close(newfd);
    }
    close(listenS);
    return 0;
}