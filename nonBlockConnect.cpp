#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>


#define BUFFER_SIZE 1023

int setNonBlocking(int fd)
{
    int oldOption = fcntl(fd, F_GETFL);
    int newOption = oldOption | O_NONBLOCK;
    fcntl(fd, F_SETFL, newOption);
    return oldOption;
}

int unblock_connect(const char* ip, int port, int time)
{
    int ret = 0;
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    if(inet_pton(AF_INET,ip, &address.sin_addr) < 0)
    {
        perror("inet_pton");
        return 1;
    }
    address.sin_port = htons(port);

    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    int fdopt  = setNonBlocking(sockfd);
    ret = connect(sockfd, (struct sockaddr*)&address, sizeof(address));
    if(ret == 0)
    {
        //如果连接成功，则恢复sockfd的属性
        printf("connect with server immediately\n");
        fcntl(sockfd, F_SETFL, fdopt);
        return fdopt;
    }
    else if (errno != EINPROGRESS)
    {
        //如果连接没有立即建立
        printf("unblock connect not support\n");
        return 1;
    }

    fd_set readfds;
    fd_set writefds;
    struct timeval timeout;

    FD_ZERO(&readfds);
    FD_SET(sockfd, &writefds);

    timeout.tv_sec = time;
    timeout.tv_usec = 0;

    ret = select(sockfd + 1, NULL, &writefds, NULL, &timeout);

    if(ret <= 0)
    {
        printf("connection time out\n");
        close(sockfd);
        return -1;
    }

    if(!FD_ISSET(sockfd, &writefds))
    {
        printf("no events no sockfd found\n");
        close(sockfd);
        return -1;
    }

    int error = 0;
    socklen_t len = sizeof(error);
    //调用getsockopt 来获取并清除sockfd上的错误
    if(getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
    {
        printf("get socket option failed\n");
        close(sockfd);
        return -1;
    }

    if(error != 0)
    {
        printf("connection failed after select with the error");
        close(sockfd);
        return -1;
    }

    printf("connection ready after select with the socket:%d", sockfd);
    fcntl(sockfd, F_SETFL, fdopt);
    return sockfd;
}

int main(int argc, char* argv[])
{
    if(argc <= 2)
    {
        printf("usage:%s ip_address port_number\n", basename(argv[0]));
        return 1;
    }

    const char* ip = argv[1];
    int port = atoi(argv[2]);

    int sockfd = unblock_connect(ip, port, 10);
    if(sockfd < 0)
    {
        return 1;
    }
    close(sockfd);
    return 0;

}




