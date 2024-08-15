#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

#define BUFFER_SIZE 1024
//请求行，请求头
enum CHECK_STATE {CHECK_STATE_REQUESTLINE = 0, CHECK_STATE_HEADER};

//行的读取状态， 完整的行， 行出错， 行数据不完整
enum LINE_STATUS {LINE_OK=0, LINE_BAD, LINE_OPEN};

//HTTP请求结果：请求不完整/完整的请求/请求出错/客户对资源没有访问权限/服务器内部错误/客户端已关闭连接
enum HTTTP_CODE {NO_REQUEST, GET_REQUEST, BAD_REQUEST, \
        FORBIDDEN_REQUEST, INTERNAL_ERROR, CLOSED_CONNECTION};

static const char* szret[] = {"I get a correct result\n", "Something wrong\n"};


//分析行
LINE_STATUS parse_line(char* buffer, int& checked_index, int& read_index)
{
    char temp;
    for(; checked_index < read_index; ++checked_index)
    {
        temp = buffer[checked_index];
        if(temp == '\r')
        {
            if((checked_index + 1) == read_index)
            {
                return LINE_OPEN;
            }
            else if(buffer[checked_index + 1] == '\n')
            {
                buffer[checked_index++] = '\0';
                buffer[checked_index++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
        else if( temp == '\n')
        {
            if((checked_index > 1) && buffer[checked_index -1] == '\r')
            {
                buffer[checked_index -1] = '\0';
                buffer[checked_index++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
    }

    return LINE_OPEN;
}

HTTTP_CODE parse_requestline(char* temp, CHECK_STATE& checkstate)
{
    char* url = strpbrk(temp, " \t");

    //没有空白字符或\t
    if(!url)
    {
        return BAD_REQUEST;
    }
    *url++ = '\0';

    char* method = temp;
    if(strcasecmp(method, "GET") == 0)   //仅支持GET
    {
        printf("The request method is GET\n");
    }
    else
    {
        return BAD_REQUEST;
    }
    
    url += strspn(url, " \t");

    char* version = strpbrk(url, " \t");
    if(!version)
    {
        return BAD_REQUEST;
    }
    *version++ = '\0';
    version += strspn(version, " \t");
    if(strcasecmp(version, "HTTP/1.1") != 0)
    {
        return BAD_REQUEST;
    }

    if(strncasecmp(url, "http://", 7) == 0)
    {
        url += 7;
        url = strchr(url,'/');
    }

    if(!url || url[0] != '/')
    {
        return BAD_REQUEST;
    }

    printf("The request URL is : %s\n", url);
    checkstate = CHECK_STATE_HEADER;
    return NO_REQUEST;
}

HTTTP_CODE parse_headers(char* temp)
{
    //空行
    if(temp[0] = '\0')
    {
        return GET_REQUEST;
    }
    else if(strncasecmp(temp, "HOST:", 5) == 0)
    {
        temp += 5;
        temp += strspn(temp, " \t");
        printf("the request host is : %s\n", temp);
    }
    else
    {
        printf("I can not handle this header\n");
    }

    return NO_REQUEST;
}

HTTTP_CODE parse_content(char* buffer, int& checked_index, CHECK_STATE& checkstate, int& read_index, int& start_line)
{
    LINE_STATUS linestatus = LINE_OK;
    HTTTP_CODE retcode = NO_REQUEST;
    while((linestatus = parse_line(buffer, checked_index, read_index)) == LINE_OK)
    {
        char* temp = buffer + start_line;
        start_line = checked_index;

        switch (checkstate)
        {
        case CHECK_STATE_REQUESTLINE:
        {
            retcode = parse_requestline(temp, checkstate);
            if(retcode == BAD_REQUEST)
            {
                return BAD_REQUEST;
            }
            break;
        }
        case CHECK_STATE_HEADER:
        {
            retcode = parse_headers(temp);
            if(retcode == BAD_REQUEST)
            {
                return BAD_REQUEST;
            }
            else if( retcode == GET_REQUEST)
            {
                return GET_REQUEST; 
            }
            break;
        }
        default:
            return INTERNAL_ERROR;
        }
    }

    if(linestatus == LINE_OPEN)
    {
        return NO_REQUEST;
    }
    else
    {
        return BAD_REQUEST;
    }
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

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    if(inet_pton(AF_INET, ip, &address.sin_addr) < 0)
    {
        perror("inet_pton");
        return 1;
    }
    address.sin_port = htons(port);

    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);

    int ret = bind(listenfd, (struct sockaddr*)&address, sizeof(address));
    assert(ret != -1);

    ret = listen(listenfd, 5);
    assert(ret != -1);

    struct sockaddr_in client;
    socklen_t client_len;

    int fd = accept(listenfd, (struct sockaddr*)&client, &client_len);
    if(fd < 0)
    {
        printf("errno is : %d\n", errno);
    }
    else
    {
        char buffer[BUFFER_SIZE];
        memset(buffer, '\0', BUFFER_SIZE);
        int data_read = 0;
        int read_index = 0;
        int checked_index = 0;
        int start_line = 0;
        CHECK_STATE checkstate = CHECK_STATE_REQUESTLINE;
        while(1)
        {
            data_read = recv(fd, buffer + read_index, BUFFER_SIZE-read_index, 0);
            if(data_read == -1)
            {
                printf("reading failed\n");
                break;
            }
            else if( data_read == 0)
            {
                printf("remote client has closed the connection\n");
                break;
            }

            read_index += data_read;

            HTTTP_CODE result = parse_content(buffer, checked_index, checkstate, read_index, start_line);
            if(result == NO_REQUEST)
            {
                continue;
            }
            else if(result == GET_REQUEST)
            {
                send(fd, szret[0], strlen(szret[0]), 0);
                break;
            }
            else
            {
                send(fd, szret[1], strlen(szret[1]), 0);
                break;
            }
        }
        close(fd);

    }
    close(listenfd);
    return 0;
}


