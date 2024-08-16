#ifndef TIME_MIN_HEAP
#define TIME_MIN_HEAP


#include <iostream>
#include <netinet/in.h>
#include <time.h>

const int BUFFER_SIZE = 64;

class heap_timer;

struct client_data
{
    int sockfd;
    sockaddr_in address;
    char buf[BUFFER_SIZE];
    heap_timer* timer;
};

class heap_timer
{
public:
    heap_timer(int delay)
    {
        expire = time(NULL) + delay;
    }
public:
    time_t expire;
    void  (*cb_func)(client_data*);
    client_data* user_data;
};

class time_heap
{
public:
    time_heap(int cap = 20)
    :capacity(cap),cur_size(0)
    {
        array = new heap_timer*[capacity];
        if(array == nullptr)
        {
            throw std::exception();
        }

        for(int i = 0; i < capacity; ++i)
        {
            array[i] = nullptr;
        }
    }

    time_heap(heap_timer** init_arr, int size, int cap)
    :capacity(cap)
    ,cur_size(size)
    {
        if(cap < size)
        {
            throw std::exception();
        }

        array = new heap_timer*[capacity];

        if(!array)
        {
            throw std::exception();
        }

        for(int i = 0; i < capacity; ++i)
        {
            array[i] = nullptr;
        }

        if(size != 0)
        {
            for(int i = 0; i < size; ++i)
            {
                array[i] = init_arr[i];
            }

            for(int i = (cur_size-1)/2; i >= 0; --i)
            {
                //0~(cur_size-1)/2 执行下滤操作
                percolate_down(i);
            }
        }
    }

    ~time_heap()
    {
        for(int i = 0; i < cur_size; ++i)
        {
            delete array[i];
        }
        delete [] array;
    }
public:
    void add_timer(heap_timer* timer)
    {
        if(timer == nullptr)
        {
            return;
        }

        int hole = cur_size++;
        int parent = 0;
        for(; hole > 0; hole = parent)
        {
            parent = (hole-1)/2;
            if(array[parent]->expire <= timer->expire)
            {
                break;
            }
            array[hole] = array[parent];
        }

        array[hole] = timer;
    }

    void del_timer(heap_timer* timer)
    {
        if(timer == nullptr)
        {
            return;
        }
        //延迟删除，仅仅将目标定时器的回调函数置空
        timer->cb_func = nullptr;
    }

    heap_timer* top() const
    {
        if(empty())
        {
            return nullptr;
        }
        return array[0];
    }

    //删除堆顶
    void pop_timer()
    {
        if(empty())
        {
            return;
        }
        if(array[0])
        {
            delete array[0];
            array[0] = array[--cur_size];
            percolate_down(0);
        }
    }

    void tick()
    {
        heap_timer* tmp = array[0];
        time_t cur = time(NULL);

        while(!empty())
        {
            if(!tmp)
            {
                break;
            }

            if(tmp->expire > cur)
            {
                break;
            }

            if(array[0]->cb_func)
            {
                array[0]->cb_func(array[0]->user_data);
            }
            pop_timer();
            tmp = array[0];
        }
    }

    bool empty()const
    {
        return cur_size == 0;
    }
private:
    void perlocate_down(int hole)
    {
        heap_timer* tmp = array[hole];
        int child = 0;
        for(; ((hole*2+1) <= (cur_size-1)); hole = child)
        {
            if(child < (cur_size-1) && array[child+1]->expire < array[child]->expire)
            {
                child++;
            }
            if(array[child]->expire < tmp->expire)
            {
                array[hole] = array[child];
            }
            else
            {
                break;
            }
        }
        array[hole] = tmp;
    }

    void resize()
    {
        heap_timer** tmp = new heap_timer* [2*capacity];
        for(int i = 0; i < 2 * capacity; ++i)
        {
            temp[i] = nullptr;
        }

        if(tmp == nullptr)
        {
            throw std::exception();
        }
        capacity *= 2;
        for(int i = 0; i < cur_size; ++i)
        {
            tmp[i] = array[i];
        }

        delete[] array;
        array = tmp;
    }
private:
    heap_timer** array;
    int capacity;
    int cur_size;

};
#endif