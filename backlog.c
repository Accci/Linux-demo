#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <libgen.h>


static bool stop = false;

static void handle_term(int sig)
{
	stop = true;
}

int main(int argc, char*argv[])
{
	signal(SIGTERM, handle_term);

    if(argc <= 3)
    {
        printf("usage: %s ip_address port_number backlog\n", basename(argv[0]));
        return 1;
    }

    const char *ip= argv[1];
    int port = atoi(argv[2]);
    int backlog = atoi(argv[3]);

    int sock = socket(PF_INET, SOCK_STREAM, 0);
    assert(sock >= 0);
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    if(inet_pton(AF_INET, ip, &address.sin_addr) <= 0)
    {
        perror("inet_pton");
        close(sock);
        return 1;
    }
    address.sin_port = htons(port);

    int ret = bind(sock, (struct sockaddr*)&address, sizeof(address));
    if(ret < 0)
    {
        perror("bind");
        close(sock);
        return 1;
    }

    ret = listen(sock, backlog);
    if(ret < 0)
    {
        perror("listen");
        close(sock);
        return 1;
    }

    while(!stop)
    {
        sleep(1);
    }

    close(sock);
    return 0;
}

