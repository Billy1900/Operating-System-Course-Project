#ifndef TEST_OSFILE_H
#define TEST_OSFILE_H

#include <cmath>
#include<ctime>
#include<string>
#include<fstream>
#include<cstdlib>
#include<iostream>
#include <cstring>
using namespace std;

typedef struct UFD{
    string File_name;//file name
    int Start; //Initial address
    int Protect;//Protection type
    int File_length;//file length
    int Max_File_length;//max size of file
    struct UFD *next;
}UFD, *UFD_ptr;

typedef struct MFD
{
    string User_name;
    string User_passwd;
    UFD *Ufd_next;
    int End;
    struct MFD *next;
}MFD,*MFD_ptr;

typedef struct AFD
{
    string File_name;
    int File_length;
    int Start;
    int Protect;
    int Pointer;    //根据这个指针的值去完成用户的要求  读写指针
    int Max_File_length;
    struct AFD *next;
}AFD,*AFD_ptr;

#endif //TEST_OSFILE_H

