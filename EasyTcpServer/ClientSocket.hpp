#ifndef _CLIENT_SOCKET_hpp_
#define _CLIENT_SOCKET_hpp_

//#include "objectPool.hpp"
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
#include "CELLBuffer.hpp"

#define RECV_BUFF_SIZE 1024*10
#define SEND_BUFF_SIZE 1024*10

//客戶端心跳檢測死亡計時時間
#define CLIENT_HEART_DEAD_TIME 60000 

//客户端定时发送数据时间
#define CLIENT_SEND_BUFF_TIME 200

//class ClientSocket:public ObjectPoolBase<ClientSocket,100>
class ClientSocket
{
public:
	ClientSocket(SOCKET sockfd = INVALID_SOCKET):_recvBuffer(RECV_BUFF_SIZE),_sendBuffer(SEND_BUFF_SIZE) {
		_sockfd = sockfd;
		//_unDoSize = 0;
		//memset(_szMsgBuf, 0, RECV_BUFF_SIZE);
		//_unSendSize = 0;
		//memset(_szSendBuf, 0, SEND_BUFF_SIZE);

		resetDtHeart();
		resetLastSendTime();
		_oldHeartTime = CELLTime::getNowTimeInMillsec();
		_oldSendTime = CELLTime::getNowTimeInMillsec();;

	}
	virtual ~ClientSocket() {

	}

	SOCKET sockfd() {
		return _sockfd;
	}
	/*char * msgBuf() {
		return _szMsgBuf;
	}

	int getUnDoSize() {
		return _unDoSize;
	}

	void setUnDoSize(int size) {
		_unDoSize = size;
	}*/

	/*int SendData(DataHeader *header) {
	}*/
	bool hasMsg() {
		return _recvBuffer.hasMsg();
	}

	DataHeader* frontMsg() {
		return (DataHeader*)_recvBuffer.getData();
	}

	int podFrontMsg(int nLength) {
		
		if (hasMsg()) {
			return _recvBuffer.pop(nLength);
		}

		return -1;
	}

	int RecvData() {
		return _recvBuffer.Read4Socket(_sockfd);
	}

	//立即将缓冲区发送给客户端
	int SendDataReal() {
		int ret = _sendBuffer.Write2Socket(_sockfd);
		resetLastSendTime();
		return ret;
	}

	//缓冲区的控制根据业务需求的差异而调整
	//
	//发送数据
	int SendData(DataHeader *header) {
		int ret = SOCKET_ERROR;

		int nSendLen = header->dataLength;
		const char * pSendBuf = (const char*)header;
		//int nCanCopy = SEND_BUFF_SIZE - _unSendSize;

		if (_sendBuffer.push(pSendBuf, header->dataLength)) {
			ret = 0;
		}
		return ret;
	}

	

	bool checkHeart(time_t dt) {
		//_dtHeart += dt;
		if (dt >= CLIENT_HEART_DEAD_TIME) {
			std::cout << "xxxxxxxxxxxxxxxxxxxx超時[" << _sockfd << "]--time["<<dt<<"]" << std::endl;
			return true;
		}
		return false;
	}

	void resetDtHeart() {
		_dtHeart = 0;
	}

	time_t getOldTime() {
		return _oldHeartTime;
	}
	void setOldTime(time_t oldHeartTime) {
		_oldHeartTime = oldHeartTime;
	}

	//-------------
	void resetLastSendTime() {
		//_dtSend = 0;
		time_t nowTime = CELLTime::getNowTimeInMillsec();
		setOldSendTime(nowTime);
	}

	time_t getOldSendTime() {
		return _oldSendTime;
	}
	void setOldSendTime(time_t oldSendTime) {
		_oldSendTime = oldSendTime;
	}


	bool checkSend(time_t dt) {
		//_dtSend += dt;
		if (dt >= CLIENT_SEND_BUFF_TIME) {
			std::cout << "超時发送[" << _sockfd << "]--[" << dt << "]" << std::endl;
			//立刻将发送缓冲区的数据发送出去
			SendDataReal();
			////重制发送计数
			resetLastSendTime();
			return true;
		}
		return false;
	}

private:
	SOCKET _sockfd;
	//第二缓冲区--处理数据缓冲区
	//char _szMsgBuf[RECV_BUFF_SIZE];
	//第二缓冲区中未处理数据的长度
	//int _unDoSize;

	CELLBuffer _recvBuffer;
	CELLBuffer _sendBuffer;

	//发送缓冲区
	//char _szSendBuf[SEND_BUFF_SIZE];
	//int _unSendSize;
	CellTimestemp _timestemp;

	//心跳計時
	time_t _dtHeart;
	time_t _oldHeartTime ;

	//上次发送消息数据的时间
	//time_t _dtSend;
	time_t _oldSendTime;

	//发送缓冲区遇到写满情况
	int _sendBuffFullCount = 0;
	

};

#endif //