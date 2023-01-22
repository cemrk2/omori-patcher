#include <fcntl.h>
#include <cstdio>
#include <iostream>
#include "utils.h"
#include "pch.h"
#include "io.h"
#include "consts.h"

namespace Utils
{
    // https://stackoverflow.com/questions/311955/redirecting-cout-to-a-console-in-windows
    void BindCrtHandlesToStdHandles(bool bindStdIn, bool bindStdOut, bool bindStdErr)
    {
        // Re-initialize the C runtime "FILE" handles with clean handles bound to "nul". We do this because it has been
        // observed that the file number of our standard handle file objects can be assigned internally to a value of -2
        // when not bound to a valid target, which represents some kind of unknown internal invalid state. In this state our
        // call to "_dup2" fails, as it specifically tests to ensure that the target file number isn't equal to this value
        // before allowing the operation to continue. We can resolve this issue by first "re-opening" the target files to
        // use the "nul" device, which will place them into a valid state, after which we can redirect them to our target
        // using the "_dup2" function.
        if (bindStdIn)
        {
            FILE* dummyFile;
            freopen_s(&dummyFile, "nul", "r", stdin);
        }
        if (bindStdOut)
        {
            FILE* dummyFile;
            freopen_s(&dummyFile, "nul", "w", stdout);
        }
        if (bindStdErr)
        {
            FILE* dummyFile;
            freopen_s(&dummyFile, "nul", "w", stderr);
        }

        // Redirect unbuffered stdin from the current standard input handle
        if (bindStdIn)
        {
            HANDLE stdHandle = GetStdHandle(STD_INPUT_HANDLE);
            if (stdHandle != INVALID_HANDLE_VALUE)
            {
                int fileDescriptor = _open_osfhandle((intptr_t)stdHandle, _O_TEXT);
                if (fileDescriptor != -1)
                {
                    FILE* file = _fdopen(fileDescriptor, "r");
                    if (file != NULL)
                    {
                        int dup2Result = _dup2(_fileno(file), _fileno(stdin));
                        if (dup2Result == 0)
                        {
                            setvbuf(stdin, NULL, _IONBF, 0);
                        }
                    }
                }
            }
        }

        // Redirect unbuffered stdout to the current standard output handle
        if (bindStdOut)
        {
            HANDLE stdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
            if (stdHandle != INVALID_HANDLE_VALUE)
            {
                int fileDescriptor = _open_osfhandle((intptr_t)stdHandle, _O_TEXT);
                if (fileDescriptor != -1)
                {
                    FILE* file = _fdopen(fileDescriptor, "w");
                    if (file != NULL)
                    {
                        int dup2Result = _dup2(_fileno(file), _fileno(stdout));
                        if (dup2Result == 0)
                        {
                            setvbuf(stdout, NULL, _IONBF, 0);
                        }
                    }
                }
            }
        }

        // Redirect unbuffered stderr to the current standard error handle
        if (bindStdErr)
        {
            HANDLE stdHandle = GetStdHandle(STD_ERROR_HANDLE);
            if (stdHandle != INVALID_HANDLE_VALUE)
            {
                int fileDescriptor = _open_osfhandle((intptr_t)stdHandle, _O_TEXT);
                if (fileDescriptor != -1)
                {
                    FILE* file = _fdopen(fileDescriptor, "w");
                    if (file != NULL)
                    {
                        int dup2Result = _dup2(_fileno(file), _fileno(stderr));
                        if (dup2Result == 0)
                        {
                            setvbuf(stderr, NULL, _IONBF, 0);
                        }
                    }
                }
            }
        }

        // Clear the error state for each of the C++ standard stream objects. We need to do this, as attempts to access the
        // standard streams before they refer to a valid target will cause the iostream objects to enter an error state. In
        // versions of Visual Studio after 2005, this seems to always occur during startup regardless of whether anything
        // has been read from or written to the targets or not.
        if (bindStdIn)
        {
            std::wcin.clear();
            std::cin.clear();
        }
        if (bindStdOut)
        {
            std::wcout.clear();
            std::cout.clear();
        }
        if (bindStdErr)
        {
            std::wcerr.clear();
            std::cerr.clear();
        }
    }

    void Info(const char* msg)
    {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, Consts::INFO);
        std::cout << "[INFO] ";
        SetConsoleTextAttribute(hConsole, Consts::RESET);
        std::cout << msg << std::endl;
    }

    void Infof(const char* format, ...)
    {
        va_list argptr;
        va_start(argptr, format);

        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, Consts::INFO);
        std::cout << "[INFO] ";
        SetConsoleTextAttribute(hConsole, Consts::RESET);

        vprintf(format, argptr);
        va_end(argptr);
        std::cout << std::endl;
    }

    void Success(const char* msg)
    {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, Consts::SUCCESS);
        std::cout << "[SUCCESS] ";
        SetConsoleTextAttribute(hConsole, Consts::RESET);
        std::cout << msg << std::endl;
    }

    void Successf(const char* format, ...)
    {
        va_list argptr;
        va_start(argptr, format);

        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, Consts::SUCCESS);
        std::cout << "[SUCCESS] ";
        SetConsoleTextAttribute(hConsole, Consts::RESET);

        vprintf(format, argptr);
        va_end(argptr);
        std::cout << std::endl;
    }

    void Warn(const char* msg)
    {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, Consts::WARN);
        std::cout << "[WARN] ";
        SetConsoleTextAttribute(hConsole, Consts::RESET);
        std::cout << msg << std::endl;
    }

    void Warnf(const char* format, ...)
    {
        va_list argptr;
        va_start(argptr, format);

        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, Consts::WARN);
        std::cout << "[WARN] ";
        SetConsoleTextAttribute(hConsole, Consts::RESET);

        vprintf(format, argptr);
        va_end(argptr);
        std::cout << std::endl;
    }

    void Error(const char* msg)
    {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, Consts::ERR);
        std::cout << "[ERROR] ";
        SetConsoleTextAttribute(hConsole, Consts::RESET);
        std::cout << msg << std::endl;
    }

    void Errorf(const char* format, ...)
    {
        va_list argptr;
        va_start(argptr, format);

        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, Consts::ERR);
        std::cout << "[ERROR] ";
        SetConsoleTextAttribute(hConsole, Consts::RESET);

        vprintf(format, argptr);
        va_end(argptr);
        std::cout << std::endl;
    }

    void Debug(DWORD_PTR addr, size_t len)
    {
        Utils::Infof("====[%p]====", addr);
        for (size_t i = 0; i < len; i++)
        {
            Utils::Infof("[%p] %X", addr, (*(int*)addr) & 0xFF);
            addr++;
        }
        Utils::Info("========");
    }
}