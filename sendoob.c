#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h>

int main(int argc, char *argv[])
{
    if(argc < 2)
    {
        printf("usaage:%s ip_address port_number\n", basename(argv[0]));
        return 1;
    }

    const char*ip = argv[1];
    int port = atoi(argv[2]);

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    if(inet_pton(AF_INET, ip, &server_addr.sin_addr) < 0)
    {
        perror("inet_pton");
        return 1;
    }

    server_addr.sin_port = htons(port);

    int sock = socket(PF_INET, SOCK_STREAM, 0);
    assert(sock >= 0);

    if(connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("connection failed!\n");
    }
    else
    {
        const char* oob_data = "abc";
        const char* normal_data = "123";
        send(sock, normal_data, strlen(normal_data), 0);
        send(sock, oob_data, strlen(oob_data), MSG_OOB);
        usleep(10);
        send(sock, normal_data, strlen(normal_data), 0);

    }

    close(sock);
    return 0;
}
