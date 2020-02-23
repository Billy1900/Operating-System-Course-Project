#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/vfs.h>
#include <sys/time.h>
#include <dirent.h>


char *txt_pid=NULL;
char *txt_pid2=NULL;

char* meminfo_read();	/*内存使用情况*/
char* stat_read();	/*cpu使用率*/
char* procsum_read();	/*进程数*/

gint mem_refresh(gpointer mem_label);	/*内存使用情况刷新*/
gint cpu_refresh(gpointer cpu_label);	/*cpu使用率刷新*/
gint process_refresh(gpointer process_label);	/*进程数刷新*/

gboolean cpu_record_callback (GtkWidget *widget,GdkEventExpose *event,gpointer data);

gboolean mem_record_callback (GtkWidget *widget,GdkEventExpose *event,gpointer data);

void cpu_record_draw(GtkWidget *widget);
void mem_record_draw(GtkWidget *widget);

static char temp_process[50];	/*进程数*/
static char temp_cpu[50];	/*cpu使用率*/
static char temp_mem[50];	/*内存使用*/
static long idle,total;		/*计算cpu时的数据*/
static int flag=0;		/*计算cpu使用率时启动程序的标志*/
static int flag1=0;		/*计算单个进程cpu使用率时使用的标志*/

static long mem_total;		/*内存总大小*/
static long mem_free;		/*空闲内存*/
static long long ustime[32768];	/*前一次记录的用户态和核心态的总时间*/
static long mtime[32768];	/*前一次记录的时刻*/
static float cpu_used_percent=0;	/*cpu使用率*/
static int cpu_start_position=15;	/*绘制cpu移动的线条*/
static float cpu_data[66];	/*cpu历史数据*/
static int flag2=0;		/*初始化cpu_data数组中数据的标志*/
static int cpu_first_data=0;	/*第一个数据，既最早的数据，下一个要淘汰的数据*/
static float mem_data[66];	/*cpu历史数据*/
static int flag3=0;		/*初始化cpu_data数组中数据的标志*/
static int mem_first_data=0;	/*第一个数据，既最早的数据，下一个要淘汰的数据*/
static int mem_start_position=15;	/*绘制内存移动的线条*/


static GtkWidget *cpu_record_drawing_area;

static GtkWidget *mem_record_drawing_area;

static GtkWidget *notebook;	/*笔记本*/
/////////////////////////////////////////////

void kill_proc(void)
{
        char buf[20];
	sprintf(buf,"kill -9 %s",txt_pid);
        system(buf);
}

gint delete_event( GtkWidget *widget,
             GdkEvent *event,
             gpointer   data )
{
    gtk_main_quit ();
    return FALSE;
}
  
char *get_cpu_name(char *_buf1)
{

	FILE * fp;
	int i=0;
	char *buf1=_buf1;

    	fp=fopen("/proc/cpuinfo","r");
	for(i=0;i<5;i++){
   	fgets(buf1,256,fp);
	}
	for(i=0;i<256;i++){
   	if(buf1[i]==':') break;
	}
	i+=2;
	buf1+=i;
	buf1[31]='\0';
	fclose(fp);
   	return buf1;
}

char *get_cpu_type(char *_buf2)
{

	FILE * fp;
	int i=0;
	char *buf2=_buf2;

    	fp=fopen("/proc/cpuinfo","r");
	for(i=0;i<2;i++){
   	fgets(buf2,256,fp);
	}
	for(i=0;i<256;i++){
   	if(buf2[i]==':') break;
	}
	i+=2;
	buf2+=i;
	buf2[12]='\0';
	fclose(fp);
   	return buf2;
}

char *get_cpu_f(char *_buf3)
{

	FILE * fp;
	int i=0;
	char *buf3=_buf3;

    	fp=fopen("/proc/cpuinfo","r");
	for(i=0;i<7;i++){
   	fgets(buf3,256,fp);
	}
	for(i=0;i<256;i++){
   	if(buf3[i]==':') break;
	}
	i+=2;
	buf3+=i;
	buf3[8]='\0';
	fclose(fp);
   	return buf3;
}

char *get_cache_size(char *_buf4)
{

	FILE * fp;
	int i=0;
	char *buf4=_buf4;

    	fp=fopen("/proc/cpuinfo","r");
	for(i=0;i<8;i++){
   	fgets(buf4,256,fp);
	}
	for(i=0;i<256;i++){
   	if(buf4[i]==':') break;
	}
	i+=2;
	buf4+=i;
	buf4[10]='\0';
	fclose(fp);
   	return buf4;
}

char *get_system_type(char *_buf1)
{
	FILE * fp;
        int i=0;
	char *buf1=_buf1;

        //fp=fopen("/proc/version","r");
        fp=fopen("/etc/issue","r");
        fgets(buf1,256,fp);
        for(i=0;i<256;i++){
   	if(buf1[i]=='\\') break;
	}
        buf1[i]='\0';
        fclose(fp);
        return buf1;
}

char *get_system_version(char *_buf2)
{
	FILE * fp;
        int i=0;
        int j=0;
	char *buf2=_buf2;

        fp=fopen("/proc/version","r");
        fgets(buf2,256,fp);
        for(i=0,j=0;i<256&&j<2;i++){
   	if(buf2[i]==' ') j++;
	}
        buf2+=i;
        for(i=0;i<256;i++){
   	if(buf2[i]==')') break;
	}
        buf2[i+1]='\0';
        fclose(fp);
        return buf2;
}

char *get_gcc_version(char *_buf3)
{
	FILE * fp;
        int i=0;
        int j=0;
	char *buf3=_buf3;

        fp=fopen("/proc/version","r");
        fgets(buf3,256,fp);
        for(i=0,j=0;i<256&&j<6;i++){
   	if(buf3[i]==' ') j++;
	}
        buf3+=i;
        for(i=0;i<256;i++){
   	if(buf3[i]==')') break;
	}
        buf3[i+1]='\0';
        fclose(fp);
        return buf3;
}

void get_proc_info(GtkWidget *clist,int *p,int *q,int *r,int *s)
{
        DIR *dir;
        struct dirent *ptr;
        int i,j;
        FILE *fp;
        char buf[1024];
        char _buffer[1024];
        char *buffer=_buffer;
        char *buffer2;
        char proc_pid[1024];
        char proc_name[1024];
        char proc_stat[1024];
        char proc_pri[1024];
        char proc_takeup[1024];
        char text[5][1024];
        gchar *txt[5];

        gtk_clist_set_column_title(GTK_CLIST(clist),0,"PID");
        gtk_clist_set_column_title(GTK_CLIST(clist),1,"名称");
	gtk_clist_set_column_title(GTK_CLIST(clist),2,"状态"); 
	gtk_clist_set_column_title(GTK_CLIST(clist),3,"优先级"); 
	gtk_clist_set_column_title(GTK_CLIST(clist),4,"占用内存"); 
        gtk_clist_set_column_width(GTK_CLIST(clist),0,50);
        gtk_clist_set_column_width(GTK_CLIST(clist),1,100);
        gtk_clist_set_column_width(GTK_CLIST(clist),2,50);
        gtk_clist_set_column_width(GTK_CLIST(clist),3,50);
        gtk_clist_set_column_width(GTK_CLIST(clist),4,55);
        gtk_clist_column_titles_show(GTK_CLIST(clist)); 
        dir=opendir("/proc");

        while(ptr=readdir(dir)){
                if((ptr->d_name)[0]>=48&&(ptr->d_name)[0]<=57){
                        (*p)++;
                        sprintf(buf,"/proc/%s/stat",ptr->d_name);
				
                	fp=fopen(buf,"r");
                        fgets(buffer,1024,fp);
                        fclose(fp);

                        for(i=0;i<1024;i++){
                        	if(buffer[i]==' ') break;
                        }
                        buffer[i]='\0';
                        strcpy(proc_pid,buffer);
                        i+=2;
                        buffer+=i;
                        for(i=0;i<1024;i++){
                        	if(buffer[i]==')') break;
                        }
                        buffer[i]='\0';
                        strcpy(proc_name,buffer);
                        i+=2;
                        buffer2=buffer+i;
                        buffer2[1]='\0';
                        strcpy(proc_stat,buffer2);
                        for(i=0,j=0;i<1024&&j<15;i++){
                        	if(buffer2[i]==' ') j++;
                        }
                        buffer2+=i;
                        for(i=0;i<1024;i++){
                        	if(buffer2[i]==' ') break;
                        }
                        buffer2[i]='\0';
                        strcpy(proc_pri,buffer2);
                        for(j=0;i<1024&&j<4;i++){
                        	if(buffer2[i]==' ') j++;
                        }
                        buffer2+=i;
                        for(i=0;i<1024;i++){
                        	if(buffer2[i]==' ') break;
                        }
                        buffer2[i]='\0';
                        strcpy(proc_takeup,buffer2);
                        
                        if(!strcmp(proc_stat,"R")) (*q)++;
                        if(!strcmp(proc_stat,"S")) (*r)++;
			if(!strcmp(proc_stat,"Z")) (*s)++;
                                
                        sprintf(text[0],"%s",proc_pid);
                        sprintf(text[1],"%s",proc_name);
                        sprintf(text[2],"%s",proc_stat);
                        sprintf(text[3],"%s",proc_pri);
			sprintf(text[4],"%s",proc_takeup);

			txt[0]=text[0];
			txt[1]=text[1];
			txt[2]=text[2];
			txt[3]=text[3];
			txt[4]=text[4];

                        gtk_clist_append(GTK_CLIST(clist),txt);
                }
        } 

        closedir(dir);
}

void get_module_info(GtkWidget *clist2)
{
	FILE *fp;
        int i;
        char _buffer[1024];
        char *buffer=_buffer; 
        char *buffer2; 
        char mod_name[1024];
        char mod_takeup[1024];
        char mod_times[1024]; 
	char text[3][1024];
        gchar *txt[3];     

        gtk_clist_set_column_title(GTK_CLIST(clist2),0,"模块名称");
	gtk_clist_set_column_title(GTK_CLIST(clist2),1,"占用内存");
	gtk_clist_set_column_title(GTK_CLIST(clist2),2,"使用次数");
	gtk_clist_set_column_width(GTK_CLIST(clist2),0,150);
	gtk_clist_set_column_width(GTK_CLIST(clist2),1,150);
	gtk_clist_set_column_width(GTK_CLIST(clist2),2,150);
	gtk_clist_column_titles_show(GTK_CLIST(clist2));

	fp=fopen("/proc/modules","r");

        fgets(buffer,1024,fp);

        while(!feof(fp)){
        	for(i=0;i<1024;i++){
        		if(buffer[i]==' ') break;
        	}
        	buffer[i]='\0';
        	strcpy(mod_name,buffer);
        	i++;
        	buffer+=i;
        	for(i=0;i<1024;i++){
        		if(buffer[i]==' ') break;
        	}
        	buffer[i]='\0';
        	strcpy(mod_takeup,buffer);
        	i++;
        	buffer2=buffer+i;
        	for(i=0;i<1024;i++){
        		if(buffer2[i]==' ') break;
        	}
        	buffer2[i]='\0';
		strcpy(mod_times,buffer2);
	
        	sprintf(text[0],"%s",mod_name);
        	sprintf(text[1],"%s",mod_takeup);
        	sprintf(text[2],"%s",mod_times);
	
		txt[0]=text[0];
		txt[1]=text[1];
		txt[2]=text[2];
	
		gtk_clist_append(GTK_CLIST(clist2),txt);
		fgets(buffer,1024,fp);
	}
	fclose(fp);
}	

/*设置select_row信号的回调函数*/
void select_row_callback(GtkWidget *clist,gint row,gint column,GdkEventButton *event,gpointer data)
{
	
	gtk_clist_get_text(GTK_CLIST(clist),row,column,&txt_pid);
	printf("%s\n",txt_pid);
}

void select_row_callback2(GtkWidget *clist,gint row,gint column,GdkEventButton *event,gpointer data)
{
	
	gtk_clist_get_text(GTK_CLIST(clist),row,column,&txt_pid2);
	printf("%s\n",txt_pid2);
}

void refresh(GtkWidget *clist)
{
        DIR *dir;
        struct dirent *ptr;
        int i,j;
        FILE *fp;
        char buf[1024];
        char _buffer[1024];
        char *buffer=_buffer;
        char *buffer2;
        char proc_pid[1024];
        char proc_name[1024];
        char proc_stat[1024];
        char proc_pri[1024];
        char proc_takeup[1024];
        char text[5][1024];
        gchar *txt[5];

	gtk_clist_clear(GTK_CLIST(clist));
        gtk_clist_set_column_title(GTK_CLIST(clist),0,"PID");
        gtk_clist_set_column_title(GTK_CLIST(clist),1,"名称");
	gtk_clist_set_column_title(GTK_CLIST(clist),2,"状态"); 
	gtk_clist_set_column_title(GTK_CLIST(clist),3,"优先级"); 
	gtk_clist_set_column_title(GTK_CLIST(clist),4,"占用内存"); 
        gtk_clist_set_column_width(GTK_CLIST(clist),0,50);
        gtk_clist_set_column_width(GTK_CLIST(clist),1,100);
        gtk_clist_set_column_width(GTK_CLIST(clist),2,50);
        gtk_clist_set_column_width(GTK_CLIST(clist),3,50);
        gtk_clist_set_column_width(GTK_CLIST(clist),4,55);
        gtk_clist_column_titles_show(GTK_CLIST(clist)); 
        dir=opendir("/proc");

        while(ptr=readdir(dir)){
                if((ptr->d_name)[0]>=48&&(ptr->d_name)[0]<=57){
                        sprintf(buf,"/proc/%s/stat",ptr->d_name);
				
                	fp=fopen(buf,"r");
                        fgets(buffer,1024,fp);
                        fclose(fp);

                        for(i=0;i<1024;i++){
                        	if(buffer[i]==' ') break;
                        }
                        buffer[i]='\0';
                        strcpy(proc_pid,buffer);
                        i+=2;
                        buffer+=i;
                        for(i=0;i<1024;i++){
                        	if(buffer[i]==')') break;
                        }
                        buffer[i]='\0';
                        strcpy(proc_name,buffer);
                        i+=2;
                        buffer2=buffer+i;
                        buffer2[1]='\0';
                        strcpy(proc_stat,buffer2);
                        for(i=0,j=0;i<1024&&j<15;i++){
                        	if(buffer2[i]==' ') j++;
                        }
                        buffer2+=i;
                        for(i=0;i<1024;i++){
                        	if(buffer2[i]==' ') break;
                        }
                        buffer2[i]='\0';
                        strcpy(proc_pri,buffer2);
                        for(j=0;i<1024&&j<4;i++){
                        	if(buffer2[i]==' ') j++;
                        }
                        buffer2+=i;
                        for(i=0;i<1024;i++){
                        	if(buffer2[i]==' ') break;
                        }
                        buffer2[i]='\0';
                        strcpy(proc_takeup,buffer2);
                                
                        sprintf(text[0],"%s",proc_pid);
                        sprintf(text[1],"%s",proc_name);
                        sprintf(text[2],"%s",proc_stat);
                        sprintf(text[3],"%s",proc_pri);
			sprintf(text[4],"%s",proc_takeup);

			txt[0]=text[0];
			txt[1]=text[1];
			txt[2]=text[2];
			txt[3]=text[3];
			txt[4]=text[4];

                        gtk_clist_append(GTK_CLIST(clist),txt);
                }
        } 

        closedir(dir);
}

void refresh2(GtkWidget *clist2)
{
	FILE *fp;
        int i;
        char _buffer[1024];
        char *buffer=_buffer; 
        char *buffer2; 
        char mod_name[1024];
        char mod_takeup[1024];
        char mod_times[1024]; 
	char text[3][1024];
        gchar *txt[3];  
  
	gtk_clist_clear(GTK_CLIST(clist2));
        gtk_clist_set_column_title(GTK_CLIST(clist2),0,"模块名称");
	gtk_clist_set_column_title(GTK_CLIST(clist2),1,"占用内存");
	gtk_clist_set_column_title(GTK_CLIST(clist2),2,"使用次数");
	gtk_clist_set_column_width(GTK_CLIST(clist2),0,150);
	gtk_clist_set_column_width(GTK_CLIST(clist2),1,150);
	gtk_clist_set_column_width(GTK_CLIST(clist2),2,150);
	gtk_clist_column_titles_show(GTK_CLIST(clist2));

	fp=fopen("/proc/modules","r");

        fgets(buffer,1024,fp);

        while(!feof(fp)){
        	for(i=0;i<1024;i++){
        		if(buffer[i]==' ') break;
        	}
        	buffer[i]='\0';
        	strcpy(mod_name,buffer);
        	i++;
        	buffer+=i;
        	for(i=0;i<1024;i++){
        		if(buffer[i]==' ') break;
        	}
        	buffer[i]='\0';
        	strcpy(mod_takeup,buffer);
        	i++;
        	buffer2=buffer+i;
        	for(i=0;i<1024;i++){
        		if(buffer2[i]==' ') break;
        	}
        	buffer2[i]='\0';
		strcpy(mod_times,buffer2);
	
        	sprintf(text[0],"%s",mod_name);
        	sprintf(text[1],"%s",mod_takeup);
        	sprintf(text[2],"%s",mod_times);
	
		txt[0]=text[0];
		txt[1]=text[1];
		txt[2]=text[2];
	
		gtk_clist_append(GTK_CLIST(clist2),txt);
		fgets(buffer,1024,fp);
	}
	fclose(fp);
}

int main( int argc,char *argv[] )
{
    GtkWidget *window;
    GtkWidget *button1;
    GtkWidget *button2;
    GtkWidget *button3;
    GtkWidget *table;
    GtkWidget *notebook;
    GtkWidget *frame;
    GtkWidget *frame2;
    GtkWidget *frame3;
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *label;
    GtkWidget *clist,*clist2;
    GtkWidget *scrolled_window;
       
    GtkWidget *capability;	/*内存资源*/
    GtkWidget *cpu_record;	/*cpu曲线图*/
    GtkWidget *mem_record;	/*内存曲线图*/
    GtkWidget *cpu_hbox;	/*容纳cpu两个图*/
    GtkWidget *mem_hbox;	/*容纳mem两个图*/
    GtkWidget *process_label;	/*进程数*/
    GtkWidget *cpu_label;	/*cpu使用率*/
    GtkWidget *mem_label;	/*内存使用情况*/

    char bufferf1[1000];
    char bufferf2[1000];
    char bufferf3[1000];
    char bufferl[20];
    char buf1[256],buf2[256],buf3[256],buf4[256];
    float temp[3]={0.0,0.0,0.0};  
    int p=0,q=0,r=0,s=0;  

    gtk_init (&argc, &argv);
    
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (window), "sysmanage");  
    gtk_widget_set_size_request (window, 550, 600);  

    g_signal_connect (G_OBJECT (window), "delete_event",
	              G_CALLBACK (delete_event), NULL);
    
    gtk_container_set_border_width (GTK_CONTAINER (window), 10);

    table = gtk_table_new (3, 6, FALSE);
    gtk_container_add (GTK_CONTAINER (window), table);    

    /* 创建一个新的笔记本，将标签页放在顶部 */
    notebook = gtk_notebook_new ();
    gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_TOP);
    gtk_table_attach_defaults (GTK_TABLE (table), notebook, 0, 6, 0, 1);
    gtk_widget_show (notebook);
    
    
    /*1.建立系统信息选项卡*/
    sprintf(bufferf1, "处理器信息");
    sprintf(bufferf2,"操作系统信息");
    
    sprintf(bufferl, "系统信息");

    vbox = gtk_vbox_new(FALSE,0);

    frame = gtk_frame_new (bufferf1);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
    gtk_widget_set_size_request (frame, 500, 150);
    sprintf(bufferf1, "    CPU名称：%s\n    CPU类型：%s\n   Cache大小：%s\n",get_cpu_name(buf1),get_cpu_type(buf2),get_cache_size(buf4));
    label = gtk_label_new (bufferf1);
    gtk_container_add (GTK_CONTAINER (frame), label);
    gtk_widget_show (label);
    gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,FALSE,5);
    gtk_widget_show (frame);

    frame2 = gtk_frame_new (bufferf2);
    gtk_container_set_border_width (GTK_CONTAINER (frame2), 10);
    gtk_widget_set_size_request (frame2, 500, 150);
    sprintf(bufferf1, "    操作系统类型：%s\n    操作系统版本：%s\nGCC编译器版本：%s\n",get_system_type(buf1),get_system_version(buf2),get_gcc_version(buf3));
    label = gtk_label_new (bufferf1);
    gtk_container_add (GTK_CONTAINER (frame2), label);
    gtk_widget_show (label);
    gtk_box_pack_start(GTK_BOX(vbox),frame2,FALSE,FALSE,5);
    gtk_widget_show (frame2);

    
    
    
    gtk_widget_show (vbox);

    label = gtk_label_new (bufferl);
    gtk_notebook_append_page (GTK_NOTEBOOK (notebook), vbox, label);	
	
    /*2.建立进程信息选项卡*/
    sprintf(bufferl, "进程信息");

    hbox=gtk_vbox_new(FALSE, 5);
    
    scrolled_window = gtk_scrolled_window_new (NULL, NULL);
    gtk_widget_set_size_request (scrolled_window, 300, 300);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);   
    clist=gtk_clist_new(5);	//5 lists	
    
    get_proc_info(clist,&p,&q,&r,&s);	//
    gtk_signal_connect(GTK_OBJECT(clist),"select_row",GTK_SIGNAL_FUNC(select_row_callback),NULL);
    gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrolled_window),clist); 
    gtk_box_pack_start(GTK_BOX(hbox), scrolled_window, TRUE, TRUE, 5);

    vbox=gtk_vbox_new(FALSE, 5); 
    frame = gtk_frame_new (bufferl);
    gtk_widget_set_size_request (frame, 500, 100);
    sprintf(bufferf1,"All process：%d Running：%d Sleeping：%d Zombied：%d",p,q,r,s);
    label = gtk_label_new (bufferf1);
    gtk_container_add (GTK_CONTAINER (frame), label);
    gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 10);
    button1 = gtk_button_new_with_label("结束进程");
    g_signal_connect (G_OBJECT (button1), "clicked",G_CALLBACK (kill_proc), "结束进程");
    gtk_box_pack_start(GTK_BOX(vbox), button1, FALSE, FALSE, 10);
    
    button2 = gtk_button_new_with_label("刷新");
    g_signal_connect_swapped (G_OBJECT (button2), "clicked",G_CALLBACK (refresh), clist);  
    
    gtk_box_pack_start(GTK_BOX(vbox), button2, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 5);

    gtk_widget_show_all(hbox);

    label = gtk_label_new (bufferl);
    gtk_notebook_append_page (GTK_NOTEBOOK (notebook), hbox, label);

    /*3.建立模块信息选项卡*/
    sprintf(bufferl, "模块信息");
    
    vbox = gtk_vbox_new(FALSE,0);
    
    scrolled_window = gtk_scrolled_window_new (NULL, NULL);
    gtk_widget_set_size_request (scrolled_window, 500, 250);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);   
    clist2=gtk_clist_new(3);
    get_module_info(clist2);
    gtk_signal_connect(GTK_OBJECT(clist2),"select_row",GTK_SIGNAL_FUNC(select_row_callback2),NULL);
    gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrolled_window),clist2); 
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 5);

    hbox=gtk_hbox_new(FALSE, 0);
    button3 = gtk_button_new_with_label("刷新");
    g_signal_connect_swapped (G_OBJECT (button3), "clicked",G_CALLBACK (refresh2), clist2);
    gtk_box_pack_end(GTK_BOX(hbox), button3, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);

    gtk_widget_show_all(vbox);	

    label = gtk_label_new (bufferl);
    gtk_notebook_append_page (GTK_NOTEBOOK (notebook), vbox, label);

    
    /* 4.建立内存资源选项卡 */
    capability = gtk_vbox_new(FALSE,0);	/*内存资源*/
    gtk_container_set_border_width(GTK_CONTAINER(capability),5);
    gtk_widget_set_size_request(capability,200,320);
    gtk_widget_show(capability);
    label = gtk_label_new(" 内存资源 ");	
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),capability,label);
    gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook),3);	/*把此页设为当前显示页*/
    cpu_hbox = gtk_hbox_new(FALSE,0);		/*cpu横向box*/
    gtk_box_pack_start(GTK_BOX(capability),cpu_hbox,TRUE,TRUE,2);
    gtk_widget_show(cpu_hbox);
    mem_hbox = gtk_hbox_new(FALSE,0);		/*mem横向box*/
    gtk_box_pack_start(GTK_BOX(capability),mem_hbox,TRUE,TRUE,2);
    gtk_widget_show(mem_hbox);
    
    /*******************************************/

    cpu_record = gtk_frame_new("cpu使用记录");	/*cpu使用记录窗口*/
    gtk_container_set_border_width(GTK_CONTAINER(cpu_record),5);
    gtk_widget_set_size_request(cpu_record,1000,130);
    gtk_widget_show(cpu_record);
    gtk_box_pack_start(GTK_BOX(cpu_hbox),cpu_record,TRUE,TRUE,2);

    cpu_record_drawing_area = gtk_drawing_area_new ();
    gtk_widget_set_size_request (cpu_record_drawing_area, 50,50);
    g_signal_connect (G_OBJECT(cpu_record_drawing_area), "expose_event",G_CALLBACK(cpu_record_callback),NULL);
    gtk_container_add (GTK_CONTAINER(cpu_record), cpu_record_drawing_area);
    gtk_widget_show (cpu_record_drawing_area);

    /*******************************************/

    mem_record = gtk_frame_new("内存使用记录");	/*内存使用记录窗口*/
    gtk_container_set_border_width(GTK_CONTAINER(mem_record),5);
    gtk_widget_set_size_request(mem_record,1000,130);
    gtk_widget_show(mem_record);
    gtk_box_pack_start(GTK_BOX(mem_hbox),mem_record,TRUE,TRUE,2);

    mem_record_drawing_area = gtk_drawing_area_new ();
    gtk_widget_set_size_request (mem_record_drawing_area, 50,50);
    g_signal_connect (G_OBJECT(mem_record_drawing_area), "expose_event",G_CALLBACK(mem_record_callback),NULL);
    gtk_container_add (GTK_CONTAINER(mem_record), mem_record_drawing_area);
    gtk_widget_show (mem_record_drawing_area);

    /*************************************************************/

    hbox = gtk_hbox_new(FALSE,0);		/*最下面的横向容器*/
    gtk_box_pack_start(GTK_BOX(capability),hbox,FALSE,FALSE,2);
    gtk_widget_show(hbox);

    process_label = gtk_label_new("");
    cpu_label = gtk_label_new("");
    mem_label = gtk_label_new("");	

    gtk_timeout_add(1000,(GtkFunction)process_refresh,(gpointer)process_label);	/*进程数刷新*/
    gtk_timeout_add(1000,(GtkFunction)cpu_refresh,(gpointer)cpu_label);	/*cpu使用率刷新*/
    gtk_timeout_add(1000,(GtkFunction)mem_refresh,(gpointer)mem_label);	/*内存使用刷新*/


    gtk_label_set_justify(GTK_LABEL(process_label),GTK_JUSTIFY_RIGHT);
    gtk_label_set_justify(GTK_LABEL(cpu_label),GTK_JUSTIFY_RIGHT);
    gtk_label_set_justify(GTK_LABEL(mem_label),GTK_JUSTIFY_RIGHT);
    gtk_box_pack_start(GTK_BOX(hbox),process_label,FALSE,FALSE,10);
    gtk_box_pack_start(GTK_BOX(hbox),cpu_label,FALSE,FALSE,10);
    gtk_box_pack_start(GTK_BOX(hbox),mem_label,FALSE,FALSE,10);
    gtk_widget_show(process_label);
    gtk_widget_show(cpu_label);
    gtk_widget_show(mem_label);    
    
    gtk_widget_show (table);
    gtk_widget_show (window);
    gtk_main ();
    
    return 0;
}

	/*******************************************************************/
gboolean cpu_record_callback(GtkWidget *widget,GdkEventExpose *event,gpointer data)/*cpu使用记录回调函数*/
{
	gtk_timeout_add(1000,(GtkFunction)cpu_record_draw,(gpointer)widget);
	return TRUE;
}

void cpu_record_draw(GtkWidget *widget)/*cpu使用记录绘图函数*/
{
	int i;
	int my_first_data;
	GdkColor color;
	GdkDrawable *canvas;
	GdkGC *gc;
	GdkFont *font;
	canvas = widget->window; 
	gc = widget->style->fg_gc[GTK_WIDGET_STATE(widget)];
 
	gdk_draw_rectangle(canvas, gc, TRUE, 10, 5, 450, 105);
	color.red = 0;
	color.green = 20000;
	color.blue = 0;
	gdk_gc_set_rgb_fg_color(gc, &color);	
	for(i=20;i<110;i+=15)	/*绘制横线*/
	{
		gdk_draw_line(canvas, gc, 10, i, 450, i);
	}

	for(i=10;i<450;i+=15)
	{
		gdk_draw_line(canvas, gc, i+cpu_start_position,5, i+cpu_start_position,110);			
	}
	cpu_start_position-=3;
	if(cpu_start_position==0)  cpu_start_position=15;

	if(flag2==0)	/*第一次清空数据*/
	{
		for(i=0;i<66;i++)
			cpu_data[i]=0;
		flag2=1;
		cpu_first_data=0;
	}

	cpu_data[cpu_first_data]=cpu_used_percent/100;
	cpu_first_data++;	
	if(cpu_first_data==66) cpu_first_data=0;

	color.red = 0;
	color.green = 65535;
	color.blue = 0;
	gdk_gc_set_rgb_fg_color(gc, &color);
	
	my_first_data=cpu_first_data;
	for(i=0;i<65;i++)
	{
		gdk_draw_line(canvas,gc,250+i*3,110-104*cpu_data[my_first_data%66],250+(i+1)*3,110-104*cpu_data[(my_first_data+1)%66]);
		my_first_data++;
		if(my_first_data==66)	my_first_data=0;
	}
 	color.red = 0;
	color.green = 0;
	color.blue = 0;
	gdk_gc_set_rgb_fg_color(gc, &color);
}

gboolean mem_record_callback(GtkWidget *widget,GdkEventExpose *event,gpointer data)	/*内存记录回调函数*/
{
	gtk_timeout_add(1000,(GtkFunction)mem_record_draw,(gpointer)widget);
	return TRUE;
}

void mem_record_draw(GtkWidget *widget)	/*内存记录绘图函数*/
{
	int i;
	int my_first_data;
	GdkColor color;
	GdkDrawable *canvas;
	GdkGC *gc;
	GdkFont *font;
	canvas = widget->window; 
	gc = widget->style->fg_gc[GTK_WIDGET_STATE(widget)];
 
	gdk_draw_rectangle(canvas, gc, TRUE, 10, 5, 450, 105);
	color.red = 0;
	color.green = 20000;
	color.blue = 0;
	gdk_gc_set_rgb_fg_color(gc, &color);	
	for(i=20;i<110;i+=15)	/*绘制横线*/
	{
		gdk_draw_line(canvas, gc, 10, i, 450, i);
	}

	for(i=10;i<450;i+=15)
	{
		gdk_draw_line(canvas, gc, i+mem_start_position,5, i+mem_start_position,110);			
	}
	mem_start_position-=3;
	if(mem_start_position==0)  mem_start_position=15;

	if(flag3==0)	/*第一次清空数据*/
	{
		for(i=0;i<66;i++)
			mem_data[i]=0;
		flag3=1;
		mem_first_data=0;
	}

	mem_data[mem_first_data]=(float)(mem_total-mem_free)/mem_total;
	mem_first_data++;	
	if(mem_first_data==66) mem_first_data=0;

	color.red = 0;
	color.green = 65535;
	color.blue = 0;
	gdk_gc_set_rgb_fg_color(gc, &color);
	
	my_first_data=mem_first_data;
	for(i=0;i<65;i++)
	{
		gdk_draw_line(canvas,gc,10+i*3,109-104*mem_data[my_first_data%66],10+(i+1)*3,109-104*mem_data[(my_first_data+1)%66]);
		my_first_data++;
		if(my_first_data==66)	my_first_data=0;
	}
 	color.red = 0;
	color.green = 0;
	color.blue = 0;
	gdk_gc_set_rgb_fg_color(gc, &color);
}

gint process_refresh(gpointer process_label)
{
	gtk_label_set_text(GTK_LABEL(process_label),procsum_read());	
	gtk_widget_show(process_label);
	return TRUE;
}


gint cpu_refresh(gpointer cpu_label)
{
	gtk_label_set_text(GTK_LABEL(cpu_label),stat_read());
	gtk_widget_show(cpu_label);
	return TRUE;
}


gint mem_refresh(gpointer mem_label)
{
	gtk_label_set_text(GTK_LABEL(mem_label),meminfo_read());	
	gtk_widget_show(mem_label);
	return TRUE;
}


char* procsum_read()	/*进程数*/
{
	int i,sum=0;	/*进程总数*/ 
	int fd;
	char path[30];	

	for(i=1;i<32768;i++)
	{
		sprintf(path,"/proc/%d/statm",i);
		if( !((fd=open(path,O_RDONLY))<0))
		{
			sum++; 		
			close(fd);
		}			
	}
	sprintf(temp_process,"进程数：%d",sum);
	//puts(temp_process);
	return temp_process;
}

char* meminfo_read()
{
	char buffer[100+1];
	char data[20];	
	long total=0,free=0;	/*总内存和用户内存*/
	int i=0,j=0,counter=0;
	int fd;
	fd=open("/proc/meminfo",O_RDONLY);
	read(fd,buffer,100);	
	
	for(i=0,j=0;i<100;i++,j++)
	{
		if (buffer[i]==':') counter++;
		if (buffer[i]==':'&&counter==1)	/*MemTotal总内存*/
		{
			while(buffer[++i]==' ');
			for(j=0;j<20;j++,i++)
			{
				if(buffer[i]=='k')  break;
				data[j]=buffer[i];
			}
			data[--j]='\0';
			total=atol(data)/1024;
		}

		if (buffer[i]==':'&&counter==2)	/*MemFree空闲内存*/
		{
			while(buffer[++i]==' ');
			for(j=0;j<20;j++,i++)
			{
				if(buffer[i]=='k')  break;
				data[j]=buffer[i];
			}
			data[--j]='\0';
			free+=atol(data)/1024;
		}

		if (buffer[i]==':'&&counter==3)	/*Buffers设备缓冲*/
		{
			while(buffer[++i]==' ');
			for(j=0;j<20;j++,i++)
			{
				if(buffer[i]=='k')  break;
				data[j]=buffer[i];
			}
			data[--j]='\0';
			//free+=atol(data)/1024;
		}

		if (buffer[i]==':'&&counter==4)	/*Cached高速缓冲*/
		{
			while(buffer[++i]==' ');
			for(j=0;j<20;j++,i++)
			{
				if(buffer[i]=='k')  break;
				data[j]=buffer[i];
			}
			data[--j]='\0';
			//free+=atol(data)/1024;
		}
	}
	mem_total=total;
	mem_free=free;
	sprintf(temp_mem,"内存:%ldM/%ldM",total-free,total);
	close(fd);	
	return temp_mem;
}


char* stat_read()
{
	long  user_t, nice_t, system_t, idle_t,total_t;	/*此次读取的数据*/
	long total_c,idle_c;	/*此次数据与上次数据的差*/
	char cpu_t[10],buffer[70+1];		
	int fd;
	fd=open("/proc/stat",O_RDONLY);
	read(fd,buffer,70);
	sscanf(buffer, "%s %ld %ld %ld %ld", cpu_t, &user_t, &nice_t, &system_t, &idle_t);

	if(flag==0)	
	{
		flag=1;	
		idle=idle_t;
		total=user_t+nice_t+system_t+idle_t;
		cpu_used_percent=0;	
	}
	else
	{
		total_t=user_t+nice_t+system_t+idle_t;
		total_c=total_t-total;
		idle_c=idle_t-idle;	
		cpu_used_percent=100*(total_c-idle_c)/total_c;
		total=total_t;	/*此次数据保存*/
		idle=idle_t;
	}
	//sprintf(temp_cpu,"%s %ld %ld %ld %ld %ld", cpu_t, user_t, nice_t, system_t, idle_t,total_t);
	//puts(temp_cpu);
	close(fd);	
		
	sprintf(temp_cpu,"cpu使用率：%0.1f%%",cpu_used_percent);
	//puts(temp_cpu);
	return temp_cpu;
}

