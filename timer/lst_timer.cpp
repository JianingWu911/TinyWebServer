#include "lst_timer.h"
#include "../http/http_conn.h"

// sort_timer_lst::sort_timer_lst()
// {
//     head = NULL;
//     tail = NULL;
// }
// sort_timer_lst::~sort_timer_lst()
// {
//     util_timer *tmp = head;
//     while (tmp)
//     {
//         head = tmp->next;
//         delete tmp;
//         tmp = head;
//     }
// }
tw_timer::tw_timer(int rot, int ts) : prev(nullptr), \
    next(nullptr), rotation(rot), time_slot(ts){};

time_wheel::time_wheel() : cur_slot(0) {
    for (int i = 0; i < N; ++i) {
        slots[i] = nullptr;
    }
}

time_wheel::~time_wheel() {
    for (int i = 0; i < N; ++i) {
        tw_timer *tmp = slots[i]; // 将每个槽的所有节点删除掉
        while (tmp) {
            slots[i] = tmp->next;
            delete tmp;
            tmp = slots[i]->next;
        }
    }
}

tw_timer* time_wheel::add_timer(int timeout) {
    if(timeout < 0) return nullptr;
    int ticks = 0;
    if (timeout < SI) ticks = 1;
    else ticks = timeout / SI;
    int rotation = ticks / N; // 计算需要转多少圈
    int ts = (cur_slot + ticks % N) % N; // 计算应该插入哪个槽
    tw_timer *timer = new tw_timer(rotation, ts); 
    if (nullptr != slots[ts]) {
        timer->next = slots[ts];
        slots[ts]->prev = timer;
        slots[ts] = timer;
    }
    else {
        slots[ts] = timer;
    }
    return timer; // 为什么要返回这个指针呢？
}

void time_wheel::adjust_timer(tw_timer *timer, int time_slot) {
    if (nullptr == timer) return;
    int diff_rotation = (time_slot) / N; // rotation 的差值
    int new_slot = (cur_slot + (time_slot) % N) % N; //修改后的slot
    int old_slot = timer->time_slot; // 修改前的slot
    if (timer == slots[old_slot]) { // 如果是第一个元素
        slots[old_slot] = timer->next;
        if (nullptr != slots[old_slot]) slots[old_slot]->prev = nullptr;
    }
    else { // 不是第一个元素
        timer->prev->next = timer->next;
        if (nullptr != timer->next) timer->next->prev = timer->prev;
    }
    // 更新信息
    timer->rotation += diff_rotation;
    timer->time_slot = new_slot;
    timer->next = nullptr;
    timer->prev = nullptr;
    if (nullptr == slots[new_slot]) { // 为空，直接放入
        slots[new_slot] = timer;
    }
    else {// 非空，插入头部
        timer->next = slots[new_slot];
        slots[new_slot]->prev = timer;
        slots[new_slot] = timer;
    }
}

void time_wheel::del_timer(tw_timer *timer) {
    if (nullptr == timer) return;
    int ts = timer->time_slot;
    // 如果是第一个元素，需要处理slots[ts]
    if (timer == slots[ts]) {
        slots[ts] = timer->next;
        if (nullptr != slots[ts]) slots[ts]->prev = nullptr;
        delete timer;
        return;
    }
    // 不是第一个元素，不需要处理slots[ts];
    timer->prev->next = timer->next;
    if (nullptr != timer->next) timer->next->prev = timer->prev; // 如果非空，处理
    delete timer; 
}
void time_wheel::tick() {
    tw_timer *tmp = slots[cur_slot];
    while (nullptr != tmp) {
        if (tmp->rotation > 1) {
            --(tmp->rotation);
            tmp = tmp->next;
        }
        else {
            tmp->cb_func(tmp->user_data);
            // 是第一个元素
            if (slots[cur_slot] == tmp) {
                slots[cur_slot] = tmp->next;
                delete tmp;
                if(nullptr != slots[cur_slot]) slots[cur_slot]->prev = nullptr;
                tmp = slots[cur_slot];
            }
            // 不是第一个元素
            else {
                tw_timer *cur = tmp->next; // 用来记录下一个
                tmp->prev->next = tmp->next;
                if (nullptr != tmp->next) tmp->next->prev = tmp->prev;
                delete tmp;
                tmp = cur;
            }
        }
    }
    cur_slot = ++cur_slot % N;
}



// void sort_timer_lst::add_timer(util_timer *timer)
// {
//     if (!timer)
//     {
//         return;
//     }
//     if (!head)
//     {
//         head = tail = timer;
//         return;
//     }
//     if (timer->expire < head->expire)
//     {
//         timer->next = head;
//         head->prev = timer;
//         head = timer;
//         return;
//     }
//     add_timer(timer, head);
// }
// void sort_timer_lst::adjust_timer(util_timer *timer)
// {
//     if (!timer)
//     {
//         return;
//     }
//     util_timer *tmp = timer->next;
//     if (!tmp || (timer->expire < tmp->expire))
//     {
//         return;
//     }
//     if (timer == head)
//     {
//         head = head->next;
//         head->prev = NULL;
//         timer->next = NULL;
//         add_timer(timer, head);
//     }
//     else
//     {
//         timer->prev->next = timer->next;
//         timer->next->prev = timer->prev;
//         add_timer(timer, timer->next);
//     }
// }
// void sort_timer_lst::del_timer(util_timer *timer)
// {
//     if (!timer)
//     {
//         return;
//     }
//     if ((timer == head) && (timer == tail))
//     {
//         delete timer;
//         head = NULL;
//         tail = NULL;
//         return;
//     }
//     if (timer == head)
//     {
//         head = head->next;
//         head->prev = NULL;
//         delete timer;
//         return;
//     }
//     if (timer == tail)
//     {
//         tail = tail->prev;
//         tail->next = NULL;
//         delete timer;
//         return;
//     }
//     timer->prev->next = timer->next;
//     timer->next->prev = timer->prev;
//     delete timer;
// }
// void sort_timer_lst::tick()
// {
//     if (!head)
//     {
//         return;
//     }
    
//     time_t cur = time(NULL);
//     util_timer *tmp = head;
//     while (tmp)
//     {
//         if (cur < tmp->expire)
//         {
//             break;
//         }
//         tmp->cb_func(tmp->user_data);
//         head = tmp->next;
//         if (head)
//         {
//             head->prev = NULL;
//         }
//         delete tmp;
//         tmp = head;
//     }
// }

// void sort_timer_lst::add_timer(util_timer *timer, util_timer *lst_head)
// {
//     util_timer *prev = lst_head;
//     util_timer *tmp = prev->next;
//     while (tmp)
//     {
//         if (timer->expire < tmp->expire)
//         {
//             prev->next = timer;
//             timer->next = tmp;
//             tmp->prev = timer;
//             timer->prev = prev;
//             break;
//         }
//         prev = tmp;
//         tmp = tmp->next;
//     }
//     if (!tmp)
//     {
//         prev->next = timer;
//         timer->prev = prev;
//         timer->next = NULL;
//         tail = timer;
//     }
// }

void Utils::init(int timeslot)
{
    m_TIMESLOT = timeslot;
}

//对文件描述符设置非阻塞
int Utils::setnonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

//将内核事件表注册读事件，ET模式，选择开启EPOLLONESHOT
void Utils::addfd(int epollfd, int fd, bool one_shot, int TRIGMode)
{
    epoll_event event;
    event.data.fd = fd;

    if (1 == TRIGMode)
        event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    else
        event.events = EPOLLIN | EPOLLRDHUP;

    if (one_shot)
        event.events |= EPOLLONESHOT;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}

//信号处理函数
void Utils::sig_handler(int sig)
{
    //为保证函数的可重入性，保留原来的errno
    int save_errno = errno;
    int msg = sig;
    send(u_pipefd[1], (char *)&msg, 1, 0);
    errno = save_errno;
}

//设置信号函数
void Utils::addsig(int sig, void(handler)(int), bool restart)
{
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = handler;
    if (restart)
        sa.sa_flags |= SA_RESTART;
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, NULL) != -1);
}

//定时处理任务，重新定时以不断触发SIGALRM信号
void Utils::timer_handler()
{
    t_wheel.tick();
    alarm(m_TIMESLOT);
}

void Utils::show_error(int connfd, const char *info)
{
    send(connfd, info, strlen(info), 0);
    close(connfd);
}

int *Utils::u_pipefd = 0;
int Utils::u_epollfd = 0;

class Utils;
void cb_func(client_data *user_data)
{
    epoll_ctl(Utils::u_epollfd, EPOLL_CTL_DEL, user_data->sockfd, 0);
    assert(user_data);
    close(user_data->sockfd);
    http_conn::m_user_count--;
}
