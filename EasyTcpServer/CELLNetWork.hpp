#ifndef _CELL_NETWORK_HPP_
#define _CELL_NETWORK_HPP_
#ifdef _WIN32

#define FD_SETSIZE 10240
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <Windows.h>
#pragma comment(lib,"ws2_32.lib")
#else
#include <unistd.h>
#include<arpa/inet.h>
#include<string.h>
#define SOCKET int
#define INVALID_SOCKET 0
#define SOCKET_ERROR -1
#endif // _WIN32


class CELLNetWork
{
public:


	static void Instance() {
		static CELLNetWork cellNetWork;
		//return cellNetWork;
	}

private:
	CELLNetWork() {
#ifdef _WIN32
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif
	}

	~CELLNetWork() {
#ifdef _WIN32	
		WSACleanup();
#endif

	}

};



#endif 