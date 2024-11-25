#ifndef FILE_H
#define FILE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64) || defined(__MINGW32__) || defined(__CYGWIN__)
    // Для Windows подключить библиотеки и определить типы
    #include <windows.h>
    #include <winerror.h>

    typedef HANDLE MyDescriptor;

    #define INVALID_MYDESCRIPTOR_VALUE INVALID_HANDLE_VALUE
    #define INVALID_MYFILE_SIZE INVALID_FILE_SIZE
    #define INVALID_MYSET_FILE_POINTER INVALID_SET_FILE_POINTER
#elif defined(__linux__)
    // Для Linux подключить библиотеки и определить типы
    #include <unistd.h>
    #include <fcntl.h>
    #include <errno.h>
    #include <limits.h>

    typedef int MyDescriptor;

    #define INVALID_MYDESCRIPTOR_VALUE -1
    #define INVALID_MYFILE_SIZE SSIZE_MAX
    #define INVALID_MYSET_FILE_POINTER SSIZE_MAX
#endif

//---------------------------------------------------------------------------

// Коды ошибок

#define LAB_OK 0
#define LAB_ERR -1
#define LAB_BAD_BUF -2
#define LAB_NOT_FOUND -3
#define LAB_STOP -4
#define LAB_SERVER_DISCONNECTED_OK -5
#define LAB_BAD_FILE -6

// Флаги для работы функции о выводе ошибки

#define LAB_FLAG_WSA 0
#define LAB_FLAG_GET 1
#define LAB_FLAG_ERRNO 2
#define LAB_FLAG_MY 3
#define LAB_FLAG_RESULT 4

// Константы

#define LAB_MAX_BUF_SIZE 128
#define LAB_MIN_BUF_LEN 1
#define LAB_HOST_NAME_SIZE 64

//---------------------------------------------------------------------------

// Прототипы функций:
#if defined(_WIN32) || defined(_WIN64) || defined(__MINGW32__) || defined(__CYGWIN__)
// Вывод сообщения об ошибке Windows
void ErrorOutputWindows(
        const char *chFuncFailMessage, // Заголовок сообщения
        int iErr, // Номер ошибки, возвращенный функцией, где произошла ошибка
        int iFlag // Флаг, как получить сообщение об ошибке
        );
#elif defined(__linux__)
// Вывод сообщения об ошибке Linux
void ErrorOutputLinux(
        const char *chFuncFailMessage, // Заголовок сообщения
        int iErr, // Номер ошибки, возвращенный функцией, где произошла ошибка
        int iFlag // Флаг, как получить сообщение об ошибке
        );
#endif

// Вывод сообщения об ошибке
void ErrorOutput(
        const char *chFuncFailMessage, // Заголовок сообщения
        int iErr, // Номер ошибки, возвращенный функцией, где произошла ошибка
        int iFlag // Флаг, как получить сообщение об ошибке
        );
        
//---------------------------------------------------------------------------

int ReleaseHandle(
        MyDescriptor *h // Указатель на дескриптор 
        );

//---------------------------------------------------------------------------

void ReleaseVoidMemory(
        void **vMemory // Указатель на указатель на память любого типа
        );

//---------------------------------------------------------------------------

int LabOpenFile(
        const char *chFileName, // Имя файла
        MyDescriptor *hFile, // Указатель на дескриптор
        unsigned long *dwFileSize // Указатель на переменную с размером файла   
        );

//---------------------------------------------------------------------------

int LabReadFile(
        MyDescriptor hFile, // Дескриптор
        char **chFileContent, // Указатель на указатель на контент файла
        unsigned long *dwFileSize // Указатель на переменную с размером файла
        );

//---------------------------------------------------------------------------

int LabWriteFile(
        MyDescriptor hFile, // Дескриптор
        const char *chFileContent, // Указатель на контент файла
        unsigned long *dwFileSize // Указатель на переменную с размером файла
        );

//---------------------------------------------------------------------------

int LabCloseFile(
        MyDescriptor *hFile // Указатель на дескриптор
        );

//---------------------------------------------------------------------------

#endif // FILE_H