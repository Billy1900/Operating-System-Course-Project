#include "OSFile.h"

#define  MaxDisk 512*1024  //一个扇区512字节,假设有1024个扇区,模拟磁盘的大小

string ROOT_path = "/home/billyluo/Desktop/operate/ex5/5/";
string MFD_Filepath = ROOT_path+"MFD.txt";

int Max_User=10;
int Max_Open=5;
int Max_End=0;
UFD_ptr p_ufd=NULL;
MFD_ptr p_mfd=NULL;
AFD_ptr p_afd=NULL;
char User_name[30];   //存放当前用户的用户名
char flag='n';
char Disk_Size[MaxDisk];    //最后在打印输出的时候，用RWX表示即可

typedef struct Disk_Table
{
    int Max_length;      //最大长度
    int Start;          //开始位置
}Disk_Node;
Disk_Node Disk_Head;

void Init_Disk(){
    Disk_Head.Max_length=MaxDisk;
    Disk_Head.Start=0;
}

bool Init_MFD(){
    p_mfd=(MFD *)new MFD;   //带头节点的单向链表
    MFD *p = p_mfd;
    ifstream ifs(MFD_Filepath);  //文件的输入流对象
    if(!ifs){
        cerr<<"ERROR：can not open MFD file."<<endl;
        p->next=NULL;
        p->Ufd_next=NULL;
        return false;
    }
    while(!ifs.eof()){
        p->next=(MFD *)new MFD;
        ifs>>p->next->User_name>>p->next->User_passwd>>p->next->End;
        if(p->next->End > Max_End)
            Max_End=p->next->End;
        p=p->next;
        p->Ufd_next=NULL;
        p->next=NULL;
    }
    ifs.close();
    return true;
}

void Print_UFD(){
    UFD *p=p_ufd->next;
    if(!p)
    {
        cout<<"The user didn't create any file, please create a file first."<<endl;
        return ;
    }
    cout<<"filename\tMax file size\tfile permission\tInitail Position\tfile size"<<endl;
    while(p)
    {
        cout<<p->File_name<<"\t\t"<<p->Max_File_length;
        if(p->Protect==0)
            cout<<"\t\t"<<"---";
        else if(p->Protect==1)
            cout<<"\t\t"<<"r-x";
        else if(p->Protect==2)
            cout<<"\t\t"<<"rwx";
        cout<<"\t\t"<<p->Start;
        cout<<"\t\t"<<p->File_length<<endl;
        p=p->next;
    }
}

bool Init_UFD(string name){
    ifstream ifs(name);
    p_ufd=(UFD *)new UFD;
    UFD *p=p_ufd;
    char temp[MaxDisk];
    bool Index=false;
    if(!ifs){
        cerr<<"ERROR：can not open UFD file."<<endl;
        p->next=NULL;
        return false;
    }
    while(!ifs.eof()){
        memset(temp,'\0',sizeof(temp));
        p->next=(UFD *)new UFD;
        if(!Index){
            p_mfd->Ufd_next=p->next;
            Index=true;
        }
        ifs>>p->next->File_name>>p->next->Max_File_length>>p->next->Protect>>p->next->Start>>p->next->File_length;
        ifs>>temp;
        if(temp[0]=='#')
            temp[0]='\0';
        for(int i=p->next->Start,j=1;j<=p->next->Max_File_length-1;i++,j++){
            Disk_Size[i]=temp[j-1];
            if(!Disk_Size[i])
                break;
        }
        Disk_Head.Max_length-=p->next->Max_File_length;
        //	Disk_Head.Start+=p->next->Max_File_length;
        p=p->next;
        p->next=NULL;
    }
    ifs.close();
    return true;
}

void Rename_File()
{
    string File_name;
    UFD *temp=p_ufd;
    char Index;
    SD:
    cout<<"input the name of the file："<<endl;
    cin>>File_name;
    while(temp)
    {
        if(temp->File_name==File_name)
        {
            cout<<"input the new name of the file："<<endl;
            cin>>File_name;
            temp->File_name=File_name;
            break;
        }
        temp=temp->next;
    }
    if(!temp)
    {
        cout<<"the file did not exist,can not make the rename operation happen, still continue<y/n>?"<<endl;
        cin>>Index;
        if(Index=='y')
            goto SD;
        else
            return ;
    }
    else
        cout<<"Rename operation succeed!"<<endl;
}

bool User_Check()
{
    string User_passwd;
    int Count=0;
    while(1)
    {
        cout<<"input user name:";
        cin>>User_name;

        INIT:
        cout<<"input password:";
        cin>>User_passwd;
        MFD *p = p_mfd->next;
        string temp_str;
        temp_str = User_name;
        Count++;
        while(p)
        {
            if(User_name==p->User_name)
            {
                if(User_passwd==p->User_passwd)
                    cout<<"login in succeed."<<endl;
                else
                {
                    cout<<"sorry, password is wrong,login failed,input again!"<<endl;
                    if(Count==3)
                    {
                        cout<<"the wrong times you input passwd more than 3, system exit!"<<endl;
                        return false;
                    }
                    goto INIT;
                }
                temp_str += ".txt";
                temp_str = ROOT_path + temp_str;
                Init_UFD(temp_str);
                Print_UFD();
                Disk_Head.Start=Max_End;
                return true;
            }
            p=p->next;
        }
        cout<<"user did not exist,input again!"<<endl;
    }
//	return false;
}

void Init_AFD(){
    p_afd=(AFD *)new AFD;
    p_afd->next=NULL;
}

bool Create_MFD(){//创建文件命令
    string User_name;
    char User_passwd[30];
    MFD *p = p_mfd;
    cout<<"input the user you want to create: ";
    cin>>User_name;
    cout<<"input:"<<User_name<<"passwd: (default:admin)";
    getchar();
    cin.getline(User_passwd,30);
    if(User_passwd[0]=='\0')
        strcpy(User_passwd,"admin");
    while(p)
    {
        if(User_name==p->User_name)
        {
            cout<<"the name exists!"<<endl;
            return false;
        }
        if(!p->next)
            break;
        p= p->next;
    }
    p->next = (MFD *)new MFD;  //尾插法
    p=p->next;
    p->User_name=User_name;
    p->User_passwd=User_passwd;
    p->End=0;
    p->next = NULL;
    p->Ufd_next = NULL;
    Max_User--;
    return true;
}  //应该是 不同的用户下的UFD文件名是不一样的啊，怎么出问题了

bool Create_UFD(){//创建文件命令
    string File_name;
    UFD *p = p_ufd;
    unsigned int Protect;
    int Max_File_length;
//	AFD *pa = pafd;
    cout<<"input the file name you want to create: ";
    cin>>File_name;
    cout<<"input the type of protection of file you wanna create：(1-(read) / 2-(read & write))";
    cin>>Protect;
    cout<<"input the max size of the file："<<endl;
    cin>>Max_File_length;
    while(p){
        if(File_name==p->File_name)
        {
            cout<<"file name exists!"<<endl;
            return false;
        }
        if(!p->next)
            break;
        p= p->next;
    }
    p->next = (UFD *)new UFD;  //尾插法
    p=p->next;
    p->File_name=File_name;
    p->Max_File_length=Max_File_length;
    p->Start=Disk_Head.Start;
    p->Protect = Protect;
//	p->Time_info=      //暂定
    p->File_length = 0;     //刚创建文件时，文件是空的
    Disk_Head.Start+=Max_File_length;
    p->next = NULL;      //我觉得这部分代码有问题；用户创建了一个文件并不代表该文件就被打开了
    return true;
}

bool Delete_UFD(){ //关于主目录的用户文件夹的文件删除    //如果文件已经打开了，是不能删除的
    string File_name;
    cout<<"input the file name you want to delete：";
    cin>>File_name;
    UFD *p=p_ufd,*temp;
    AFD *pa=p_afd;
    while(pa->next)
    {
        if(File_name==pa->next->File_name)
        {
            cout<<"the file is open, please close it first, and then delete again!"<<endl;
            return false;
        }
    }
    while(p->next)
    {
        if(File_name==p->next->File_name)
        {
            temp=p->next;
            p->next=p->next->next;  //如果说要回收模拟的磁盘空间的话，应该是回收最大长度
            //	Disk_Head.Max_length+=temp->Max_File_length;    //链表中删除了，其他的文件内容的起始位置不变即可，又没事，还是照样可以访问数据的嘛
            delete temp;
            cout<<"file delete succeed!!!"<<endl;
            return true;
        }
        p=p->next;
    }
    cout<<"the file did not exist!!!"<<endl;
    return false;
}

bool Open()
{
    string File_name;
    unsigned int Protect;
    cout<<"input file name you want to open：";
    cin>>File_name;
    UFD *p=p_ufd->next;
    AFD *pa=p_afd->next;
    while(pa)
    {
        if(File_name==pa->File_name)
        {
            cout<<"file: "<<File_name<<"is open!"<<endl;
            return true;
        }
        if(!pa->next)
            break;
        pa=pa->next;
    }
    if(!pa)     //如果找不到，或者打开的文件目录链表为空表
        pa=p_afd;
    while(p)
    {
        if(File_name==p->File_name)
        {
            if(!Max_Open)
            {
                cout<<"the user only can open at best 5 files one time,please close other extra files!!!"<<endl;
                return false;
            }
            if(p->Protect==0)
            {
                cout<<"user ca not operate this file!"<<endl;
                return false;
            }
            cout<<"open the file with what type pf permission?<1-read only,2-read & write>：";
            cin>>Protect;
            pa->next=(AFD *)new AFD;
            pa=pa->next;
            pa->File_name=p->File_name;
            pa->Start=p->Start;
            pa->File_length=p->File_length;
            pa->Max_File_length=p->Max_File_length;
            pa->Protect=Protect;
            if(Protect==1)
                pa->Pointer=0;   //Poniter取0表示此时用户无法写数据(没有空间的含义)
            else
                pa->Pointer=p->File_length;
            pa->next=NULL;
            Max_Open--;
            cout<<"file "<<File_name<<"is open!!!"<<endl;
            return true;
        }
        p=p->next;
    }
    cout<<"the file did not exist!"<<endl;
    return false;
}

void Close()
{
    string file_name;
    UFD *pu=p_ufd->next;
    cout<<"input the file name you want to close：";
    cin>>file_name;
    AFD *p=p_afd,*temp;
    while(p&&p->next)
    {
        if(file_name==p->next->File_name)
        {
            temp=p->next;
            p->next=p->next->next;
            if(temp->Protect==2)      //可写的文件才有权把新写入的数据也保存起来嘛
            {
                while(pu)
                {
                    if(temp->File_name==pu->File_name)
                    {
                        pu->File_length=temp->Pointer;
                        break;
                    }
                    pu=pu->next;
                }
            }
            delete temp;
            cout<<"file"<<file_name<<"is close!"<<endl;
            return ;
        }
        p=p->next;
    }
    cout<<"the file is not open!"<<endl;
}

bool Read_File()  //因为读写都是通过修改运行文件目录的Pointer去模拟的嘛
{
    string File_name;       //你要读文件的话，你这文件肯定得在运行/打开文件目录里面，是吧
    unsigned int length;
    AFD *p=p_afd->next;
    cout<<"input the file name you want to read：";
    cin>>File_name;
    cout<<"input the size of the file：";  //读取的话，实际中文件的长度并不会改变
    cin>>length;          //即读取的长度并不能保存回去,为了方便（如果用户是读数据的话，pointer始终不变）
    while(p)
    {
        if(File_name==p->File_name)
        {
            for(int i=p->Start,j=1;j<=length;i++,j++)
                cout<<Disk_Size[i];
            cout<<endl;
            cout<<"file "<<File_name<<" read succeed!!!"<<endl;
            return true;
        }
        p=p->next;
    }
    cout<<"failed, file is not open!!!"<<endl;
    return false;
}

bool Write_File()  //写的话，自然是运行文件目录
{
    string File_name;
    unsigned int length;
    AFD *p=p_afd->next;
    char temp[MaxDisk]={'\0'};
    cout<<"input the filename you want to write：";
    cin>>File_name;
    while(p)
    {
        if(File_name==p->File_name)
        {
            if(p->Protect!=2)
            {
                cout<<"file "<<File_name<<"can not be written"<<endl;
                return false;
            }
            cout<<"input the size you want to write："<<endl;
            cin>>length;
            if(p->Pointer+length <= p->Max_File_length){
                cout<<"input the content of file：<End with Enter>"<<endl;
                getchar();
                gets(temp);
                for(int i=p->Start+p->File_length,j=1;j<=strlen(temp);i++,j++)
                    Disk_Size[i]=temp[j-1];
                p->Pointer += length;
                cout<<"file "<<File_name<<"write succeed!!!"<<endl;
            }
            else{
                cout<<"the data overflow,your data exceed the file max size,failed!!!"<<endl;
                return false;
            }
            return true;
        }
        p=p->next;
    }
    cout<<"failed,the file is not open!!"<<endl;
    return false;
}

void Destroy_Space()
{
    MFD *pm=p_mfd;
    while(pm)
    {
        p_mfd=p_mfd->next;
        delete pm;
        pm=p_mfd;
    }
    AFD *pa=p_afd;
    while(pa)
    {
        p_afd=p_afd->next;
        delete pa;
        pa=p_afd;
    }
    UFD *pu=p_ufd;
    while(pu)
    {
        p_ufd=p_ufd->next;
        delete pu;
        pu=p_ufd;
    }
}

void Save_MFD()
{
    ofstream ofs;        //文件的输出流对象
    ofs.open(MFD_Filepath);
    if(!ofs.is_open())
    {
        cout<<"The MFD can't open!!!"<<endl;
        if(flag=='y')
            cout<<"storing, Loading..."<<endl;
        else
            ofs.close();  //该语句待定
    }
    MFD *p=p_mfd->next;
    while(p){
        if(p->User_name==User_name)
            p->End=Disk_Head.Start;
        ofs<<p->User_name<<" "<<p->User_passwd<<" "<<p->End;
        if(p->next)
            ofs<<"\n";
        p=p->next;
    }
    ofs.close();
}

void Save_UFD(string name)
{
    ofstream ofs(name);        //文件的输出流对象
    char temp[MaxDisk]={'\0'};

    if(!ofs.is_open()){
        cout<<"The "<<name<<"file can't open!!!"<<endl;
        ofs.close();  //该语句待定，这语句真的用的很妙，因为我并没有马上退出这次函数调用了
    }
    UFD *p=p_ufd->next;
    while(p){
        memset(temp,'\0',sizeof(temp));
        for(int i=p->Start,j=1;j<=p->Max_File_length;i++,j++)
        {
            if(Disk_Size[i])
                temp[j-1]=Disk_Size[i];
            else
                break;
        }
        if(!temp[0])        //如果文件没有数据的话，就用空格表示此文件没有任何内容
            temp[0]='#';
        ofs<<p->File_name<<" "<<p->Max_File_length<<" "<<p->Protect<<" "<<p->Start<<" "<<p->File_length<<" "<<temp;
        if(p->next)
            ofs<<"\n";
        p=p->next;
    }
    ofs.close();
}

void Quit_System()
{
    AFD *pa=p_afd->next;
    UFD *pu=p_ufd->next;
    while(pa)
    {
        if(pa->Protect==2)  //2表示可写   //1表示只读  //0表示不可操作
        {
            while(pu)
            {
                if(pa->File_name==pu->File_name)
                {
                    pu->File_length=pa->Pointer;
                    break;
                }
                pu=pu->next;
            }
        }
        pa=pa->next;
    }
    string temp_str;
    temp_str = User_name;
    temp_str += ".txt";
    temp_str = ROOT_path+temp_str;
    Save_MFD();
    Save_UFD(temp_str);
    Print_UFD();
    Destroy_Space();
}

void Print_Help()
{
    cout<<"************************2-class file system*************************"<<endl;
    cout<<"*\t\torder\t\tinstructions"<<endl;
    cout<<"*\t\tlogin\t\tlogin"<<endl;
    cout<<"*\t\tcreate\t\tcreate file"<<endl;
    cout<<"*\t\tdelete\t\tdelete file"<<endl;
    cout<<"*\t\topen\t\topen file"<<endl;
    cout<<"*\t\tclose\t\tclose file"<<endl;
    cout<<"*\t\tread\t\tread file"<<endl;
    cout<<"*\t\twrite\t\twrite file"<<endl;
    cout<<"*\t\tls  \t\tlist content"<<endl;
    cout<<"*\t\trename\t\trename file"<<endl;
    cout<<"*\t\thelp\t\thelp"<<endl;
    cout<<"*\t\tcls \t\tclear the window"<<endl;
    cout<<"*\t\tlogout\t\tuser logout"<<endl;
    cout<<"*\t\tquit\t\tquit"<<endl;
    cout<<"*****************************************************************"<<endl;
}

void System_Init()
{
    Start:
    Print_Help();
    Init_Disk();
    if(!Init_MFD()){
        int num;
        cout<<"menu is not created yet, create?<y/n>"<<endl;
        cin>>flag;
        cout<<"input the number of the content(1-10):"<<endl;
        cin>>num;
        if(flag=='y')
        {
            while(num--)
            {
                Create_MFD();
                if(!Max_User)
                {
                    cout<<"there are at most 10 users, you can not create other users!"<<endl;
                    return ;
                }
            }
            Save_MFD();
            cout<<"created succeed,continue?<y/n>!"<<endl;
            cin>>flag;
            if(flag=='y')
                goto Start;
        }
    }
    return;
}

void File_System()
{
    while(1)
    {
        string Command;
        cout<<"order:";
        cin>>Command;
        if(Command=="create")
            Create_UFD();
        else if(Command=="delete")
            Delete_UFD();
        else if(Command=="open")
            Open();
        else if(Command=="close")
            Close();
        else if(Command=="read")
            Read_File();
        else if(Command=="write")
            Write_File();
        else if(Command=="quit")
        {
            Quit_System();
            break;
        }
        else if(Command=="ls")
            Print_UFD();
        else if(Command=="cls")
            system("clear");
        else if(Command=="login")
        {
            if(!User_Check())
                break;
            Init_AFD();
        }
        else if(Command=="rename")
            Rename_File();
        else if(Command=="logout")
        {
            cout<<"logout succeed!"<<endl;
            Quit_System();
            exit(0);
        }
        else
            Print_Help();
    }
}

int main(){
    System_Init();
    File_System();
    return 0;
}
