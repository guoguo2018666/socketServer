#ifndef _CLIENT_SOCKET_hpp_
#define _CLIENT_SOCKET_hpp_


#define CELLSERVER_NUM 4
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

#include <iostream>
#include <vector>
#include "message.hpp"
#include <thread>
#include <mutex>
#include <atomic>
#include "CELLTimestemp.hpp"


#define RECV_BUFF_SIZE 1024*10
#define SEND_BUFF_SIZE 1024*10





class ClientSocket
{
public:
	ClientSocket(SOCKET sockfd = INVALID_SOCKET) {
		_sockfd = sockfd;
		_unDoSize = 0;
		memset(_szMsgBuf, 0, RECV_BUFF_SIZE);
		_unSendSize = 0;
		memset(_szSendBuf, 0, SEND_BUFF_SIZE);
	}
	virtual ~ClientSocket() {

	}

	SOCKET sockfd() {
		return _sockfd;
	}
	char * msgBuf() {
		return _szMsgBuf;
	}

	int getUnDoSize() {
		return _unDoSize;
	}

	void setUnDoSize(int size) {
		_unDoSize = size;
	}

	//发送数据
	int SendData(DataHeader *header) {
		int ret = SOCKET_ERROR;

		int nSendLen = header->dataLength;
		const char * pSendBuf = (const char*)header;
		//int nCanCopy = SEND_BUFF_SIZE - _unSendSize;


		while (true) {

			if (((_unSendSize + nSendLen) >= SEND_BUFF_SIZE)) {
				//可copy的字节数
				int nCanCopy = SEND_BUFF_SIZE - _unSendSize;
				memcpy(_szSendBuf + _unSendSize, pSendBuf, nCanCopy);

				ret = send(_sockfd, _szSendBuf, SEND_BUFF_SIZE, 0);
				if (ret == SOCKET_ERROR) {
					return ret;
				}

				memset(_szSendBuf, 0, SEND_BUFF_SIZE);
				_unSendSize = 0;

				nSendLen -= nCanCopy;
				pSendBuf = pSendBuf + nCanCopy;
			}
			else {
				memcpy(_szSendBuf + _unSendSize, pSendBuf, nSendLen);
				_unSendSize += nSendLen;
				if (_timestemp.getSecond() >= 1.0) {
					ret = send(_sockfd, _szSendBuf, _unSendSize, 0);
					if (ret == SOCKET_ERROR) {
						return ret;
					}
					memset(_szSendBuf, 0, SEND_BUFF_SIZE);
					_unSendSize = 0;
				}
				ret = 0;
				break;
			}
		}

		return ret;
	}

private:
	SOCKET _sockfd;
	//第二缓冲区--处理数据缓冲区
	char _szMsgBuf[RECV_BUFF_SIZE];
	//第二缓冲区中未处理数据的长度
	int _unDoSize;

	//发送缓冲区
	char _szSendBuf[SEND_BUFF_SIZE];
	int _unSendSize;
	CellTimestemp _timestemp;

};

#endif //