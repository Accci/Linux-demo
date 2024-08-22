#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <wait.h>

pthread_mutex_t mutex;

void* another(void* arg)
{
    printf("in child thread, lock the mutex\n");
    pthread_mutex_lock(&mutex);
    sleep(5);
    pthread_mutex_unlock(&mutex);
}

int main()
{
    pthread_mutex_init(&mutex, NULL);
    pthread_t id;

    pthread_create(id, NULL, another, NULL);

    sleep(1);
    int pid = fork();
    if(pid < 0)
    {
        pthread_join(id, NULL);
        pthread_mutex_destroy(&mutex);
        return 1;
    }else if(pid == 0)
    {
        printf("I am in the child, want to get the lock\n");
        /*子进程从父进程继承了木特性的状态， 该互斥锁处于锁住的状态，这是由父进程中的子
        线程执行pthread_mutex_lock引起的，因此下面的语句回一直阻塞
        */

       pthread_mutex_lock(&mutex);
       printf("I can not run to here\n");
       pthread_mutex_unlock(&mutex);
       exit(0);
    }
    else
    {
        wait(NULL);
    }
    pthread_join(id, NULL);
    pthread_mutex_destroy(&mutex);
    return 0;


    /*
        pthread_atfork(void (*prepare)(void), void(*parent)(void), void (*child)(void));
        :确保fork调用后父进程和子进程都由一个清除的锁状态
    */


}
