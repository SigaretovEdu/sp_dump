// Подключить нашу библиотеку
#include "file.h"

#if defined(_WIN32) || defined(_WIN64) || defined(__MINGW32__) || defined(__CYGWIN__)
void ErrorOutputWindows(const char *chFuncFailMessage, int iErr, int iFlag)
{
    DWORD n = 0; // Код ошибки
    const DWORD size = LAB_MAX_BUF_SIZE; // Максимальный размер сообщения об ошибке
    char buf[LAB_MAX_BUF_SIZE]; // Буфер сообщения

    switch ( iFlag ) // Обработка флага
    {
        case LAB_FLAG_GET:
            n = GetLastError();
            FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM,
                           NULL,
                           n,
                           MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT),
                           buf,
                           size - 1,
                           NULL);
        break;

        case LAB_FLAG_ERRNO:
            n = errno;
            FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM,
                           NULL,
                           n,
                           MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT),
                           buf,
                           size - 1,
                           NULL);
        break;

        case LAB_FLAG_RESULT:
            FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM,
                           NULL,
                           iErr,
                           MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT),
                           buf,
                           size - 1,
                           NULL);
            break;

        default :
        sprintf(buf, "Error in programmer's function code is %d", iErr);
        break;
    }

    MessageBoxA(0, buf, chFuncFailMessage, MB_ICONERROR | MB_OK); // Вывод сообщения об ошибке
}
#endif

#if defined(__linux__)
void ErrorOutputLinux(const char *chFuncFailMessage, int iErr, int iFlag)
{
    const unsigned long size = LAB_MAX_BUF_SIZE; // Максимальный размер сообщения об ошибке
    char buf[LAB_MAX_BUF_SIZE]; // Буфер сообщения

    switch ( iFlag ) // Обработка флага
    {
        case LAB_FLAG_GET:
            printf("%s: error code: %d, error msg: %s\n", buf, errno, strerror(errno));
        break;

        case LAB_FLAG_ERRNO:
            printf("%s: error code: %d, error msg: %s\n", buf, errno, strerror(errno));
        break;

        case LAB_FLAG_RESULT:
            printf("%s: error code: %d\n", buf, iErr);
            break;

        default :
        sprintf(buf, "Error in programmer's function code is %d", iErr);
        break;
    }
}
#endif

void ErrorOutput(const char *chFuncFailMessage, int iErr, int iFlag)
{
    #if defined(_WIN32) || defined(_WIN64) || defined(__MINGW32__) || defined(__CYGWIN__)
    ErrorOutputWindows(chFuncFailMessage, iErr, iFlag);
    #elif defined(__linux__)
    ErrorOutputLinux(chFuncFailMessage, iErr, iFlag);
    #endif
}

//---------------------------------------------------------------------------

int ReleaseHandle(MyDescriptor *h)
{
    int bRet = 0;

    //  Если хендл валидный
    if ( *h != INVALID_MYDESCRIPTOR_VALUE )
    {
        #if defined(_WIN32) || defined(_WIN64) || defined(__MINGW32__) || defined(__CYGWIN__)
        bRet = CloseHandle(*h); // Закрыть
        #elif defined(__linux__)
        bRet = close(*h);
        #endif

        *h = INVALID_MYDESCRIPTOR_VALUE; // Сделать невалидным
    }

    return bRet;
}

//---------------------------------------------------------------------------

void ReleaseVoidMemory(void **vMemory)
{
    // Если указатель на область памяти валидный
    if ( *vMemory != NULL )
    {
        free(*vMemory); // Освободить
        *vMemory = NULL; // Сделать невалидным
    }
}

//---------------------------------------------------------------------------

int LabOpenFile(const char *chFileName, MyDescriptor *hFile, unsigned long *dwFileSize)
{
    int bRet = 0; // Возвращаемое функцией значение

    if ( chFileName == NULL ) // Если плохой буфер
    {
        ErrorOutput("Error in input buffer", LAB_BAD_BUF, LAB_FLAG_MY); // Сообщение
        return LAB_BAD_BUF; // Вернуть ошибку
    }

    // Открыть файл на чтение и запись
    #if defined(_WIN32) || defined(_WIN64) || defined(__MINGW32__) || defined(__CYGWIN__)
    *hFile = CreateFileA(chFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    #elif defined(__linux__)
    *hFile = open(chFileName, O_CREAT | O_RDWR | O_APPEND);
    #endif

    if ( *hFile == INVALID_MYDESCRIPTOR_VALUE ) // Если ошибка
    {
        ErrorOutput("Error in creating file function", LAB_ERR, LAB_FLAG_GET); // Вывести сообщение
        return LAB_ERR; // Вернуть -1
    }

    // Получение размера файла
    #if defined(_WIN32) || defined(_WIN64) || defined(__MINGW32__) || defined(__CYGWIN__)
    *dwFileSize = GetFileSize(*hFile, NULL);
    if ( *dwFileSize == INVALID_MYFILE_SIZE )
    {
        bRet = LAB_ERR;
    }
    #elif defined(__linux__)
    bRet = lseek(*hFile, 0, SEEK_END);
    if ( bRet >= 0 )
    {
        *dwFileSize = (unsigned long)bRet;
    }
    #endif

    if ( bRet == LAB_ERR ) // Если ошибка
    {
        ErrorOutput("Error in getting file size function", LAB_ERR, LAB_FLAG_GET); // Вывести сообщение
        return LAB_ERR; // Вернуть -1
    }

    return LAB_OK;
}

//---------------------------------------------------------------------------

int LabReadFile(MyDescriptor hFile, char **chFileContent, unsigned long *dwFileSize)
{
    int bRet = 0; // Возвращаемое функцией значение
    unsigned long dwNumberOfBytesRead = 0; // Количество считанных байт
    unsigned long dwSum = 0; // Суммарное количество считанных байт
    unsigned long dwFilePointer = INVALID_MYSET_FILE_POINTER; // Смещение в файле
    #if defined(_WIN32) || defined(_WIN64) || defined(__MINGW32__) || defined(__CYGWIN__)
    BOOL bRetWin = FALSE; // Возвращаемое функцией значение
    #endif

    if ( hFile == INVALID_MYDESCRIPTOR_VALUE ) // Если плохой хендл
    {
        ErrorOutput("Error in file", LAB_BAD_FILE, LAB_FLAG_MY); // Сообщение
        return LAB_BAD_FILE; // Вернуть ошибку
    }

    #if defined(_WIN32) || defined(_WIN64) || defined(__MINGW32__) || defined(__CYGWIN__)
    dwFilePointer = SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
    if ( dwFilePointer == INVALID_MYSET_FILE_POINTER )
    {
        bRet = LAB_ERR;
    }
    #elif defined(__linux__)
    bRet = lseek(hFile, 0, SEEK_SET);
    if ( bRet >= 0 )
    {
        dwFilePointer = (unsigned long)bRet;
    }
    #endif

    if ( bRet == LAB_ERR ) // Если ошибка
    {
        ErrorOutput("Error in setting file pointer function", LAB_ERR, LAB_FLAG_GET); // Вывести сообщение
        return LAB_ERR; // Вернуть -1
    }

    // Получение размера файла
    #if defined(_WIN32) || defined(_WIN64) || defined(__MINGW32__) || defined(__CYGWIN__)
    *dwFileSize = GetFileSize(hFile, NULL);
    if ( *dwFileSize == INVALID_MYFILE_SIZE )
    {
        bRet = LAB_ERR;
    }
    #elif defined(__linux__)
    bRet = lseek(hFile, 0, SEEK_END);
    if ( bRet >= 0 )
    {
        *dwFileSize = (unsigned long)bRet;
    }
    #endif

    if ( bRet == LAB_ERR ) // Если ошибка
    {
        ErrorOutput("Error in getting file size function", LAB_ERR, LAB_FLAG_GET); // Вывести сообщение
        return LAB_ERR; // Вернуть -1
    }

    // Выделение памяти под содержимое файла
    *chFileContent = (char *)malloc(*dwFileSize + 1);

    if ( *chFileContent == NULL ) // Если память не выделилась
    {
        ErrorOutput("Error in malloc function", LAB_ERR, LAB_FLAG_ERRNO); // Вывести сообщение
        return LAB_ERR; // Вернуть -1
    }

    // Чтение файла
    while ( dwSum < *dwFileSize )
    {
        #if defined(_WIN32) || defined(_WIN64) || defined(__MINGW32__) || defined(__CYGWIN__)
        bRetWin = ReadFile(hFile, (LPVOID)(*chFileContent + dwSum), *dwFileSize - dwSum, &dwNumberOfBytesRead, NULL);
        if ( bRetWin == FALSE )
        {
            bRet = LAB_ERR;
        }
        #elif defined(__linux__)
        bRet = lseek(hFile, dwSum, SEEK_SET);
        if ( bRet == LAB_ERR ) // Если ошибка
        {
            ErrorOutput("Error in setting file pointer function", LAB_ERR, LAB_FLAG_GET); // Вывести сообщение
            return LAB_ERR; // Вернуть -1
        }

        bRet = read(hFile, (void *)(*chFileContent + dwSum), *dwFileSize - dwSum);
        if ( bRet >= 0 )
        {
            dwNumberOfBytesRead = (unsigned long)bRet;
        }
        #endif

        if ( bRet == LAB_ERR ) // Если ошибка
        {
            ErrorOutput("Error in reading file function", LAB_ERR, LAB_FLAG_GET); // Вывести сообщение
            return LAB_ERR; // Вернуть -1
        }
        else if ( dwNumberOfBytesRead == 0 )
        {
            ErrorOutput("Error in reading file function: dwNumberOfBytesRead is 0", LAB_BAD_FILE, LAB_FLAG_MY); // Вывести сообщение
            return LAB_BAD_FILE; // Вернуть -1
        }

        dwSum += dwNumberOfBytesRead;
    }

    *dwFileSize = dwSum;

    (*chFileContent)[*dwFileSize] = '\0'; // Нуль-терм

    return LAB_OK;
}

//---------------------------------------------------------------------------

int LabWriteFile(MyDescriptor hFile, const char *chFileContent, unsigned long *dwFileSize)
{
    int bRet = 0; // Возвращаемое функцией значение
    unsigned long dwFilePointer = INVALID_MYSET_FILE_POINTER; // Смещение в файле
    #if defined(_WIN32) || defined(_WIN64) || defined(__MINGW32__) || defined(__CYGWIN__)
    BOOL bRetWin = FALSE; // Возвращаемое функцией значение
    #endif

    if ( hFile == INVALID_MYDESCRIPTOR_VALUE ) // Если плохой хендл
    {
        ErrorOutput("Error in file", LAB_BAD_FILE, LAB_FLAG_MY); // Сообщение
        return LAB_BAD_FILE; // Вернуть ошибку
    }

    #if defined(_WIN32) || defined(_WIN64) || defined(__MINGW32__) || defined(__CYGWIN__)
    dwFilePointer = SetFilePointer(hFile, 0, NULL, FILE_END);
    if ( dwFilePointer == INVALID_MYSET_FILE_POINTER )
    {
        bRet = LAB_ERR;
    }
    #elif defined(__linux__)
    bRet = lseek(hFile, 0, SEEK_END);
    if ( bRet >= 0 )
    {
        dwFilePointer = (unsigned long)bRet;
    }
    #endif

    if ( bRet == LAB_ERR ) // Если ошибка
    {
        ErrorOutput("Error in setting file pointer function", LAB_ERR, LAB_FLAG_GET); // Вывести сообщение
        return LAB_ERR; // Вернуть -1
    }

    // Запись в файл
    #if defined(_WIN32) || defined(_WIN64) || defined(__MINGW32__) || defined(__CYGWIN__)
    bRetWin = WriteFile(hFile, (LPCVOID)(chFileContent), strlen(chFileContent), dwFileSize, NULL);
    if ( bRetWin == FALSE )
    {
        bRet = LAB_ERR;
    }
    #elif defined(__linux__)
    bRet = write(hFile, (const void *)(chFileContent), strlen(chFileContent));
    #endif

    if ( bRet == LAB_ERR ) // Если ошибка
    {
        ErrorOutput("Error in writing file function", LAB_ERR, LAB_FLAG_GET); // Вывести сообщение
        return LAB_ERR; // Вернуть -1
    }

    return LAB_OK;
}

//---------------------------------------------------------------------------

int LabCloseFile(MyDescriptor *hFile)
{
    int bRet = 0;

    bRet = ReleaseHandle(hFile);

    return bRet;
}

//---------------------------------------------------------------------------

int main()
{
    unsigned long len = 0;
    int ret = LAB_OK;
    char *buf = NULL;
    char *command = NULL;
    char *chFileRead = NULL;
    char chFileName[] = {"EgorPortnyagin.txt"};
    char chFileToWrite[] = {"Hello world\n"};
    unsigned long i = 0;
    MyDescriptor hFile = INVALID_MYDESCRIPTOR_VALUE;

    // Выделение памяти под "сырой" буфер с командой
    buf = (char *)malloc(LAB_MAX_BUF_SIZE + 1);

    // Выделение памяти под буфер с командой
    command = (char *)malloc(LAB_MAX_BUF_SIZE + 1);

    while ( 1 )
    {
        printf("Waiting for user input...\n");

        // Считать команду без двух последних символов
        fgets(buf, LAB_MAX_BUF_SIZE - 2, stdin);

        printf("buf is %s\n", buf);

        // Вычисление позиции первого символа-разделителя
        for ( i = 0; (i < strlen(buf)) && (buf[i] != ' ') && (buf[i] != '\t') && (buf[i] != '\n') ; i++ ) ;

        strncpy(command, buf, i);
        command[i] = '\0';

        printf("command is %s\n", command);

        if ( strlen(command) == 0 )
        {
            fprintf(stderr, "command %s is empty\n", command);
        }
        else if ( strncasecmp(command, "open", strlen(command)) == 0 )
        {
            ret = LabOpenFile(chFileName, &hFile, &len);
        }
        else if ( strncasecmp(command, "write", strlen(command)) == 0 )
        {
            ret = LabWriteFile(hFile, chFileToWrite, &len);
        }
        else if ( strncasecmp(command, "read", strlen(command)) == 0 )
        {
            ret = LabReadFile(hFile, &chFileRead, &len);

            // Вывести на экран содержимое файла
            printf("Content of the file is:\n%s\n", chFileRead);
            ReleaseVoidMemory((void **)&chFileRead);
        }
        else if ( strncasecmp(command, "close", strlen(command)) == 0 )
        {
            ret = LabCloseFile(&hFile);
        }
        else if ( strncasecmp(command, "end", strlen(command)) == 0 )
        {
            break;
        }
        else
        {
            fprintf(stderr, "command %s error\n", command);
        }
    }

    // Освободить память, если не освобождена

    ReleaseVoidMemory((void **)&buf);
    ReleaseVoidMemory((void **)&command);
    ReleaseVoidMemory((void **)&chFileRead);

    // Закрыть файл, если не закрыт

    ret = ReleaseHandle(&hFile);

    return ret;
}
