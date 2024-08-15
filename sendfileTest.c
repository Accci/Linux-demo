#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <libgen.h>


int main(int argc, char* argv[])
{
    if(argc < 3)
    {
        printf("usage: %sip_address, port_number filename\n", basename(argv[0]));
        return 1;
    }
    const char* ip = argv[1];
    int port = atoi(argv[2]);
    const char* filename = argv[3];

    int filefd = open(filename, O_RDONLY);
    assert(filefd > 0);
    struct stat stat_buf;
    fstat(filefd, &stat_buf);

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

    int  ret = bind(sock, (struct sockaddr*)&address, sizeof(address));
    assert(ret != -1);

    ret = listen(sock,5);

    struct sockaddr_in client;
    socklen_t client_len;
    int connfd = accept(sock, (struct sockaddr*)&client, &client_len);
    if(connfd < 0)
    {
        printf("errno is : %d \n", errno);
    }
    else
    {
        sendfile(connfd, filefd, NULL, stat_buf.st_size);
        close(connfd);
    }

    close(sock);
    return 0;
}