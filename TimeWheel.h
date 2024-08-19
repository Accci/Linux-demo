#ifndef TIMEWHEEL
#define TIMEWHEEL
const int BUFFER_SIZE = 64;
struct client_data
{
    int sockfd;
    struct sockaddr_in address;
    char buf[BUFFER_SIZE];
    tw_timer* timer;
}

class tw_timer
{
public:
    tw_timer(int rot, int ts)
    :rotation(rot)
    ,time_slot(ts)
    ,next(nullptr)
    ,prev(nullptr)
public:
    int rotation;
    int time_slot;
    void (*cb_func)(client_data*);
    client_data* user_data;
    tw_timer* next;
    tw_timer* prev;
};

class time_wheel
{
public:
    time_wheel():cur_slot(0)
    {
        for(int i = 0; i < N; ++i)
        {
            slots[i] = nullptr;
        }
    }
    ~time_wheel()
    {
        for(int i = 0; i < N; ++i)
        {
            tw_timer * tmp = slots[i];
            while (tmp)
            {
                slots[i] = tmp->next;
                delete tmp;
                tmp = slots[i];
            }
            
        }
    }

public:
    tw_timer* add_timer(int timeout)
    {
        if(timeout < 0)
        {
            return nullptr;
        }
        int ticks = 0;

        if(timeout < SI)
        {
            ticks = 1;
        }
        else
        {
            ticks = timeout / SI;
        }
        int rot = ticks / N;

        int ts = (cur_slot + (ticks % N)) % N;

        tw_timer * timer = new tw_timer(rot, ts);

        if(slots[ts] == nullptr)
        {
            printf("add timer, rotation is %d, ts is %d, cur_slot is %d", rot, ts, cur_slot);
            slots[ts] = timer;
        }
        else
        {
            timer->next = slots[ts];
            slots[ts]->prev = timer;
            slots[ts] = timer;
        }

        return timer;
    }

    void del_timer(tw_timer* timer)
    {
        if(timer == nullptr)
        {
            return;
        }
        int ts = timer->time_slot;

        if(timer == slots[ts])
        {
            slots[ts] = slots[ts]->next;
            if(slots[ts])
            {
                slots[ts]->prev = nullptr;
            }
            
            delete timer;
        }
    }

    void tick()
    {
        tw_timer* tmp = slots[cur_slot];
        printf("current slot is %d\n", cur_slot);
        while(tmp)
        {
            printf("tick the timer once\n");
            if(tmp->rotation > 0)
            {
                tmp->rotation--;
                tmp = tmp->next;
            }
            else
            {
                tmp->cb_func(tmp->user_data);
                if(tmp == slots[cur_slot])
                {
                    printf("delete header in cur_slot\n");
                    slots[cur_slot] = tmp->next;
                    delete tmp;
                    if(slots[cur_slot])
                    {
                        slots[cur_slot]->prev = nullptr;
                    }
                    tmp = slots[cur_slot];
                }
                else
                {
                    tmp->prev->next = tmp->next;
                    if(tmp->next)
                    {
                        tmp->next->prev = tmp->prev;
                    }
                    tw_timer* tmp2 = tmp->next;
                    delete tmp;
                    tmp = tmp2;
                }
            }
        }
        cur_slot = ++cur_slot % N;
    }
private:
    static const int N = 60;
    static const int ST = 1;
    tw_timer* slots[N];
    int cur_slot;
};
#endif