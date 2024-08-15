#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <pthread.h>

#define MAX_EVENT_NUMBER 1024
#define BUFFER_SIZE 10

int setNonBlocking(int fd)
{
    int oldOption = fcntl(fd, F_GETFL);
    int newOption = oldOption | O_NONBLOCK;
    fcntl(fd, F_SETFL, newOption);
    return oldOption;
}

void addfd(int epollfd, int fd, bool enable_et)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN;
    if(enable_et)
    {
        event.events |= EPOLLET;
    }

    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setNonBlocking(fd);
}

//LT（水平触发）
void lt(epoll_event* events, int number, int epollfd, int listenfd)
{
    char buf[BUFFER_SIZE];
    for(int i = 0; i < number; ++i)
    {
        int sockfd = events[i].data.fd;
        if(sockfd == listenfd)
        {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            int connfd = accept(listenfd, (struct sockaddr*)&client_addr, &client_len);
            addfd(epollfd, connfd, false);
        }
        else if( events[i].events & EPOLLIN)
        {
            //只要socket度缓存中还有未读出的数据，这段代码就会触发
            printf("event trigger once\n");
            memset(buf, '\0', BUFFER_SIZE);
            int ret = recv(sockfd, buf, BUFFER_SIZE-1, 0);
            if(ret <= 0)
            {
                close(sockfd);
                continue;;
            }
            printf("get %d bytes of content: %s\n", ret, buf);
        }
        else
        {
            printf("something else happened \n");
        }
    }
}

//ET模式的工作流程
void et(epoll_event* events, int number, int epollfd, int listenfd)
{
    char buf[BUFFER_SIZE];
    for(int i = 0;i < number; ++i)
    {
        int sockfd = events[i].data.fd;
        if(sockfd == listenfd)
        {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            int connfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_len);
            
            addfd(epollfd, connfd, true);
        }
        else if(events[i].events & EPOLLIN)
        {
            printf("event trigger once\n");
            while (1)
            {
                memset(buf, '\0', BUFFER_SIZE);
                int ret = recv(sockfd, buf, BUFFER_SIZE-1, 0);
                if(ret < 0)
                {
                    if((errno == EAGAIN) || (errno == EWOULDBLOCK))
                    {
                        printf("read later\n");
                        break;
                    }
                    close(sockfd);
                    break;
                }else if(ret == 0)
                {
                    close(sockfd);
                }
                else
                {
                    printf("get %d bytes of content:%s\n",ret , buf);
                }
            }
            
        }
        else
        {
            printf("something else happended\n");
        }
    }
}

int main(int argc, char *argv[])
{
    if(argc <= 2)
    {
        printf("usage:%s ip_address port_number\n", basename(argv[0]));
        return 1;
    }

    const char* ip = argv[1];
    int port = atoi(argv[2]);
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    
    address.sin_family = AF_INET;
    if(inet_pton(AF_INET, ip, &address.sin_addr) < 0)
    {
        perror("inet_pton");
        return 1;
    }

    address.sin_port = htons(port);
    
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    assert(sock >= 0);

    int ret = bind(sock, (struct sockaddr*)&address, sizeof(address));
    assert(ret != -1);

    ret = listen(sock, 5);
    assert(ret != -1);

    epoll_event events[MAX_EVENT_NUMBER];
    int epollfd = epoll_create(5);
    assert(epollfd != -1);
    addfd(epollfd, sock, true);
    while(1)
    {
        ret - epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
        if(ret < 0)
        {
            printf("epoll failure\n");
            break;
        }

        lt(events, ret, epollfd, sock);
    }

    close(sock);
    return 0;
}