#ifndef LST_TIMER
#define LST_TIMER

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/uio.h>

#include <time.h>
#include "../log/log.h"

class tw_timer;

struct client_data
{
    sockaddr_in address;
    int sockfd;
    tw_timer *timer;
};

// class util_timer
// {
// public:
//     util_timer() : prev(NULL), next(NULL) {}

// public:
//     time_t expire;
    
//     void (* cb_func)(client_data *);
//     client_data *user_data;
//     util_timer *prev;
//     util_timer *next;
// };

class tw_timer {
public:
    tw_timer(int rot, int ts);
    int rotation; // 记录多少圈(相对圈数）后生效
    int time_slot; // 记录属于哪个slot ？？？
    void (* cb_func)(client_data *);
    client_data *user_data;
    tw_timer* prev;
    tw_timer* next;
};

class time_wheel {
private:
    static const int N = 100; // 共有10个槽
    static const int SI = 1; //时间间隔为 1s, 每间隔100s转一圈
    tw_timer* slots[N]; // 每个槽都是一个链表
    int cur_slot; // 当前槽
public:
    time_wheel();
    ~time_wheel();
    tw_timer* add_timer(int timeout); // timeout单位是秒
    void del_timer(tw_timer *timer);
    void adjust_timer(tw_timer *timer, int time_slot); // 向后延迟time_slot毫秒
    void tick();
};
// class sort_timer_lst
// {
// public:
//     sort_timer_lst();
//     ~sort_timer_lst();

//     void add_timer(util_timer *timer);
//     void adjust_timer(util_timer *timer);
//     void del_timer(util_timer *timer);
//     void tick();

// private:
//     void add_timer(util_timer *timer, util_timer *lst_head);

//     util_timer *head;
//     util_timer *tail;
// };

class Utils
{
public:
    Utils() {}
    ~Utils() {}

    void init(int timeslot);

    //对文件描述符设置非阻塞
    int setnonblocking(int fd);

    //将内核事件表注册读事件，ET模式，选择开启EPOLLONESHOT
    void addfd(int epollfd, int fd, bool one_shot, int TRIGMode);

    //信号处理函数
    static void sig_handler(int sig);

    //设置信号函数
    void addsig(int sig, void(handler)(int), bool restart = true);

    //定时处理任务，重新定时以不断触发SIGALRM信号
    void timer_handler();

    void show_error(int connfd, const char *info);

public:
    static int *u_pipefd;
    time_wheel t_wheel; 
    static int u_epollfd;
    int m_TIMESLOT;
};

void cb_func(client_data *user_data);

#endif
