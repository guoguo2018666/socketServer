#ifndef _CELL_SERVER_hpp_
#define _CELL_SERVER_hpp_
#define CELLSERVER_NUM 4

#include "alloc.h"
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
#include "CellTask.hpp"
#include "INetEvent.h"
#include "CellSendMsgTask.hpp"


class CellServer
{

public:
	CellServer(SOCKET socket) {
		_sock = socket;
		_pThread = nullptr;
		_pEvent = nullptr;
	}
	virtual ~CellServer() {
		CloseSocket();
		_sock = INVALID_SOCKET;
	}

	void setNetEventObj(INetEvent* pEvent) {
		_pEvent = pEvent;
	}
	void CloseSocket() {
		for (size_t n = 0; n < _clients.size(); n++) {
#ifdef _WIN32	
			closesocket(_clients[n]->sockfd());
#else
			close(g_clients[n]->sockfd());
#endif
		}
		if (_sock != INVALID_SOCKET) {
#ifdef _WIN32	
			//4关闭套接字
			closesocket(_sock);
			WSACleanup();
#else
			close(_sock);
#endif
		}
	}

	std::shared_ptr<ClientSocket> findClient(SOCKET cSocket) {
		auto iter = _clients.begin();
		for (; iter != _clients.end(); iter++) {
			if ((*iter)->sockfd() == cSocket) {
				break;
			}
		}

		if (iter != _clients.end()) {
			return *iter;
		}
		else {
			return nullptr;
		}

	}

	//发送数据
	int SendData(SOCKET cSock, DataHeader *header) {
		if (header != nullptr) {
			return send(cSock, (const char*)header, header->dataLength, 0);
		}
		return 0;
	}

	int SendDataToAll(DataHeader *header) {
		if (header != nullptr) {
			for (int n = 0; n < (int)_clients.size(); n++) {
				SendData(_clients[n]->sockfd(), header);
			}
		}
		return 0;
	}

	int RecvData(std::shared_ptr<ClientSocket>pClient) {

		//接受客户端数据
		char * szRecv = pClient->msgBuf() + pClient->getUnDoSize();
		//缓冲区
		//int nLen = recv(pClient->sockfd(), _szRecv, RECV_BUFF_SIZE*10, 0);
		int nLen = recv(pClient->sockfd(), szRecv, RECV_BUFF_SIZE - pClient->getUnDoSize(), 0);
		_pEvent->OnNetRecv(pClient);
		_recvCount++;
		//timePrintf(pClient->sockfd());
		//int nLen = recv(cSock, szRecv, sizeof(header), 0);
		if (nLen <= 0) {
			std::cout << "与服务器断开连接，任务结束" << std::endl;
			return -1;
		}

		//std::cout << "接受客户端的数据长度[" << nLen << "]" << std::endl;

		//memcpy(pClient->msgBuf() + pClient->getUnDoSize(), _szRecv, nLen);
		pClient->setUnDoSize(pClient->getUnDoSize() + nLen);

		while (pClient->getUnDoSize() >= sizeof(DataHeader)) {
			DataHeader *header = (DataHeader *)(pClient->msgBuf());
			if (pClient->getUnDoSize() >= header->dataLength) {
				//缓冲区剩余未处理消息的长度
				int nSize = pClient->getUnDoSize() - header->dataLength;

				OnNetMessage(pClient);

				memcpy(pClient->msgBuf(), pClient->msgBuf() + header->dataLength, nSize);
				pClient->setUnDoSize(nSize);
			}
			else {
				//std::cout << "RecvData break" << std::endl;
				//剩余未消息长度不够一条完整消息
				break;
			}
		}

		return 0;
	}

	int OnNetMessage(std::shared_ptr<ClientSocket> pClient) {
		//_recvCount++;
		_pEvent->OnNetMsg(pClient);
		DataHeader* header = (DataHeader*)(pClient->msgBuf());
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			Login *login = (Login *)(pClient->msgBuf());

			//忽略判断用户名和密码是否正确
			//std::cout << "数据长度：" << login->dataLength << " ,命令：" << "login" << std::endl;

			LoginResult *ret = new LoginResult();
			//send(pClient->sockfd(), (char*)&ret, sizeof(ret), 0);
			//pClient->SendData((DataHeader*)&ret);
			
			addSendTask(pClient, (DataHeader*)ret);
			_msgSend++;

			break;

		}
		case CMD_LOGINOUT:
		{
			LoginOut *loginout = (LoginOut *)(pClient->msgBuf());
			//recv(cSock, (char*)&loginout + sizeof(DataHeader), sizeof(loginout) - sizeof(DataHeader), 0);
			//忽略判断用户名和密码是否正确
			std::cout << "数据长度：" << loginout->dataLength << " ,命令：" << "logout" << std::endl;
			//DataHeader retheader = {};
			//send(_cSock, (char*)&header, sizeof(header), 0);
			LoginOutResult ret = {};
			send(pClient->sockfd(), (char*)&ret, sizeof(ret), 0);
			break;
		}
		default:
			header->cmd = CMD_ERROR;
			header->dataLength = 0;
			send(pClient->sockfd(), (char*)header, sizeof(header), 0);
			break;
		}

		return 0;
	}

	fd_set _fdRead_bak;
	bool _clients_change;
	SOCKET _maxSock;
	void OnRun() {

		_clients_change = false;
		while (true) {
			if (_clientsBuff.size() > 0) {
				std::lock_guard<std::mutex> lgggg(_mutex);
				for (auto pClient : _clientsBuff) {
					_clients.push_back(pClient);

					//FD_SET(pClient->sockfd(), &fdReads);
					//FD_SET(pClient->sockfd(), &fdWrites);
					//FD_SET(pClient->sockfd(), &fdExecpts);
				}
				_clientsBuff.clear();
				_clients_change = true;
			}
			else {
				_clients_change = false;
			}

			if (_clients.size() <= 0) {
				Sleep(1000);
				continue;
			}

			fd_set fdReads;
			fd_set fdWrites;
			fd_set fdExecpts;
			FD_ZERO(&fdReads);
			FD_ZERO(&fdWrites);
			FD_ZERO(&fdExecpts);

			_maxSock = _clients[0]->sockfd();
			if (_clients_change) {
				_clients_change = false;

				for (int n = 0; n < (int)_clients.size(); n++) {

					FD_SET(_clients[n]->sockfd(), &fdReads);

					if (_maxSock < _clients[n]->sockfd()) {
						_maxSock = _clients[n]->sockfd();
					}
				}

				memcpy(&_fdRead_bak, &fdReads, sizeof(fd_set));
			}
			else {
				memcpy(&fdReads, &_fdRead_bak, sizeof(fd_set));
			}

			timeval t = { 0,0 };
			int ret = select(_maxSock + 1, &fdReads, &fdWrites, &fdExecpts, &t);

			if (ret < 0) {
				std::cout << "select发送错误，任务结束" << std::endl;
				return;
			}


#ifdef _WIN32

			for (int n = 0; n <= (int)fdReads.fd_count - 1; n++) {

				std::shared_ptr<ClientSocket>pClient = findClient(fdReads.fd_array[n]);
				if (pClient == nullptr) {
					std::cout << "接受数据时在vector中未发现client" << std::endl;
					return;
				}

				int ret = RecvData(pClient);

				if (-1 == ret) {
					//auto iter = std::find(_clients.begin(), _clients.end(), fdReads.fd_array[n]);
					auto iter = _clients.begin();
					for (; iter != _clients.end(); iter++) {
						if ((*iter)->sockfd() == fdReads.fd_array[n]) {
							break;
						}

					}
					if (iter != _clients.end()) {
						_clients.erase(iter);
						_clients_change = true;
						//_pEvent->OnLeave(*iter);
					}
				}
			}
#else
			for (int n = 0; n < (int)_clients.size(); n++) {
				//FD_SET(g_clients[n], &fdReads);
				if (FD_ISSET(_clients[n], &fdReads)) {
					int ret = RecvData(fdReads.fd_array[n]);
					if (-1 == ret) {
						auto iter = _clients.begin() + n;
						if (iter != _clients.end()) {
							_clients.erase(iter);
							_clients_change = true;
						}
					}
				}
#endif
			}
		}

		void addClient(std::shared_ptr<ClientSocket>pClient) {
		//void addClient(std::shared_ptr<ClientSocket> pClient) {
			std::lock_guard<std::mutex> lg(_mutex);
			_clientsBuff.push_back(pClient);

		}

		void Start() {
			std::cout << "启动数据接收线程" << std::endl;
			_pThread = new std::thread(std::mem_fun(&CellServer::OnRun), this);
			_pThread->detach();
			//启动数据处理线程
			std::cout << "启动数据发送线程" << std::endl;
			_cellTaskServer.start();
			
		}

		size_t getClientNum() {
			return _clients.size() + _clientsBuff.size();
		}

		void timePrintf(SOCKET cSock) {
			auto a1 = _timestemp.getSecond();
			if (a1 >= 1.0) {
				std::cout << "[" << a1 << "]秒中socket[" << cSock << "]--recv的数据个数[" << _recvCount << "]" << std::endl;
				_timestemp.update();
			}
		}

		void addSendTask(std::shared_ptr<ClientSocket> pClient,DataHeader* header) {
			//CellTask *pCellTask = new CellSendMsgTask(pClient, header);
			//std::shared_ptr<CellSendMsgTask> pCellTask =std::make_shared<CellSendMsgTask>(pClient, header);
			//std::shared_ptr<CellTask> pCellTask = std::make_shared<CellSendMsgTask>(pClient, header);
			//_cellTaskServer.addTask(pCellTask);
			_cellTaskServer.addTask([pClient, header]() {
				pClient->SendData(header); 
				//delete header;
			});
		}
private:
	SOCKET _sock;
	//正式客户队列
	std::vector<std::shared_ptr<ClientSocket>> _clients;
	//缓冲客户队列
	std::vector<std::shared_ptr<ClientSocket>> _clientsBuff;
	//std::vector<std::shared_ptr<ClientSocket>> _clientsBuff;
	std::mutex _mutex;
	std::thread* _pThread;

	//char _szRecv[RECV_BUFF_SIZE] = {};
    INetEvent* _pEvent;

	CellTimestemp _timestemp;
	std::atomic_int _recvCount = 0;
	std::atomic_int _msgSend = 0;

	CellTaskServer _cellTaskServer;

	};

#endif //