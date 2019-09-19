#include "mainwindow.h"
#include <QApplication>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>

void *deal_fun(void *arg)
{
    int fd = (int)arg;
    while(1)
    {
        char buf[128] = "";
        read(fd,buf,sizeof(buf));
        //printf("buf = %s\n",buf);
        char cmd[128] = "";
        float num = 0.0f;
        sscanf(buf,"%[^=]=%f",cmd,&num);
        if(strcmp(cmd,"ANS_PERCENT_POSITION") == 0)
        {
            printf("\r已播放%05.2f%%\t",num);
        }
        else if(strcmp(cmd,"ANS_TIME_POSITION") == 0)
        {
            printf("播放时间 %02d:%02d",(int)(num+0.5)/60,(int)(num+0.5)%60);
        }
        fflush(stdout);
    }
}
void *deal_fun2(void *arg)
{
    int fifo_fd = (int)arg;
    while(1)
        {
            usleep(300*1000);
            write(fifo_fd,"get_percent_pos\n",strlen("get_percent_pos\n"));//获得百分比
            usleep(200*1000);
            write(fifo_fd,"get_time_pos\n",strlen("get_time_pos\n"));//获得时间
        }
        //write(fifo_fd,"loadfile 2.mp3\n",strlen("loadfile 2.mp3\n"));//切割
        close(fifo_fd);
}

int main(int argc, char *argv[])
{
    //创建一个无名管道 获取mplayer回应
        int fd[2];
        pipe(fd);
        pid_t pid = fork();
        if(pid == 0)//子进程
        {
            //创建一个管道s
            mkfifo("fifo_cmd",0666);
            //将1号设备重定向到fd[1]
            dup2(fd[1],1);
            //使用execlp启动mplayer
            execlp("mplayer","mplayer","-idle","-slave","-quiet","-input","file=./fifo_cmd","3.mp3",NULL);
        }
        else//父进程
        {
            //创建一个接收mplayer回应的线程
            pthread_t tid;
            pthread_create(&tid,NULL,deal_fun,(void *)(fd[0]));
            pthread_detach(tid);
            //创建一个管道
            mkfifo("fifo_cmd",0666);
            int fifo_fd = open("fifo_cmd",O_WRONLY);
            pthread_t fifo_tid;
            pthread_create(&fifo_tid,NULL,deal_fun2,(void *)(fifo_fd));
            pthread_detach(fifo_tid);
            //sleep(10);

            QApplication a(argc, argv);
            MainWindow w;
            w.show();

            return a.exec();

        }
}
