#ifndef LST_TIMER
#define LST_TIMER

#include <time.h>
#define BUFFER_SIZE 1024
class util_timer;
struct client_data
{
    sockaddr_in address;
    int sockfd;
    char buf[BUFFER_SIZE];
    util_timer* timer;
};

class util_timer
{
public:
    util_timer():next(nullptr),prev(nullptr)
    {}
    time_t expire;                      //过期时间
    void (*cb_func)(client_data*);      
    client_data* user_data;
    
    util_timer *next;
    util_timer *prev;
};

class Sort_Timer_Lst
{
public:
    Sort_Timer_Lst():head(nullptr), tail(nullptr){}
    ~Sort_Timer_Lst()
    {
        util_timer* tmp = head;
        while(tmp)
        {
            head = tmp->next;
            delete tmp;
            tmp = head;
        }
    }

    void add_timer(util_timer* timer)
    {
        if(timer == nullptr)
        {
            return;
        }

        if(head == nullptr)
        {
            head = tail = timer;
            return;
        }

        if(timer->expire < head->expire)
        {
            timer->next = head;
            head->prev = timer;
            head = timer;
            return;
        }

        add_timer(timer, head);
    }

    void adjust_timer(util_timer* timer)
    {
        if(timer == nullptr)
        {
            return;
        }
        util_timer* tmp = timer->next;
        if((tmp == nullptr) || timer->expire < tmp->expire)
        {
            return;
        }
        if(timer == head)
        {
            head = head->next;
            head->prev = nullptr;
            timer->next = nullptr;
            add_timer(timer, head);
        }
        else
        {
            timer->next->prev = timer->prev;
            timer->prev->next = timer->next;
            add_timer(timer, timer->next);    
        }
    }

    void del_timer(util_timer* timer)
    {
        if(timer == nullptr)
        {
            return;
        }
        if((timer == head) && (timer == tail))
        {
            delete timer;
            head = tail = nullptr;
            return;
        }
        if(timer == head)
        {
            head = head->next;
            head->prev = nullptr;
            delete timer;
            return;
        }
        if(timer == tail)
        {
            tail = tail->prev;
            tail->next = nullptr;
            delete timer;
            return;
        }
        timer->next->prev = timer->prev;
        timer->prev->next = timer->next;
        delete timer;
    }

    void tick()
    {
        if(head == nullptr)
        {
            return;
        }
        printf("timer tick\n");
        time_t cur = time(NULL);    //获取当前时间
        util_timer* tmp = head;
        while(tmp)
        {
            if(cur < tmp->expire)
            {
                break;
            }
            tmp->cb_func(tmp->user_data);
            head = tmp->next;
            if(head != nullptr)
            {
                head->prev = nullptr;
            }
            delete tmp;
            tmp = head;
        }
    }


private:

    void add_timer(util_timer* timer, util_timer* head)
    {
        util_timer* prev = head;
        util_timer* tmp  = prev->next;
        while(tmp)
        {
            if(timer->expire < tmp->expire)
            {
                timer->next = tmp;
                tmp->prev = timer;
                prev->next = timer;
                timer->prev = prev;
                break;
            }
            prev = tmp;
            tmp = tmp->next;
        }

        if(tmp == nullptr)
        {
            prev->next = timer;
            timer->prev = prev;
            timer->next = nullptr;
            tail = timer;
        }
    }

    util_timer *head;
    util_timer *tail;
};

#endif