#include <assert.h>
#include <stdio.h>
#include 

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        printf("usage:%s <file>\n", argv[0]);
        return 1;
    }

    int filefd = open(argv[1], O_CREAT | Q_WRONLY | O_TRUNC, 0666);
    assert(filefd  > 0);

    int piped_stdout[2];
    int ret = pipe(piped_stdout);
    assert(ret != -1);

    int piped_file[2];
    int ret = pipe(piped_file);
    assert(ret != -1);

    ret = splice(STDIN_FILENO, NULL, pipefd_stdout[1], NULL, 32768, SPLICE_F_MORE | SPLICE_F_MOVE);
    assert(ret != -1);

    ret = tee(pipfd_stdout[0], pipefd_file[1], 32768, SPLICE_F_NONBLOCK);
    assert(ret != -1);
    ret = splice(pipefd_file[0], NULL, STDOUT_FILENO, NULL, 32768, SPLICE_F_MORE | SPLICE_F_MOVE);
    assert(ret != -1);
    close(filefd);
    close(pipefd_stdout[0]);
    close(pipefd_stdout[1]);
    close(pipefd_file[0]);
    close(pipefd_file[1]);
    return 0;
}