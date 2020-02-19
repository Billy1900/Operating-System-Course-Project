#include<time.h>
#include<gtk/gtk.h>
#include <unistd.h>
#include<pthread.h>
#include <cstdio>

char s[1000];
GtkWidget *label;

void *thread(void * argc){
    int i,j,sum;
    for(i=1,sum=0,j=1;i<=100;i++){
        sleep(1);
        sum=sum+j;
        j+=1;
        sprintf(s,"%d",sum);
        gtk_label_set_text(GTK_LABEL(label),s);
    }
}


int main( int argc, char *argv[])
{   pthread_t id;
    int i,ret;
    ret=pthread_create(&id,NULL,&thread,NULL);
    GtkWidget *vbox;  		//定义一个组装盒;
    GtkWidget *window;
    /*初始化整个GTK+程序，是每一个GTK+程序必不可少的部分*/
    gtk_init(&argc, &argv);
    /*这里生成了一个窗口构件——GtkWindow，GTK_WINDOW_TOPLEVEL包含窗口的标题栏和边框，同意用窗口管理器来进行管理*/
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "sum");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 200);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    label = gtk_label_new (s);
    gtk_container_add (GTK_CONTAINER (window), label);
    gtk_widget_show (label);
    /*开始显示窗口*/
    gtk_widget_show(window);
    gtk_main();
    return 0;
}
