#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_


//#include "alloc.h"
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
#include "ClientSocket.hpp"
#include "INetEvent.h"
#include "CellServer.hpp"
#include "CellTask.hpp"





class EasyTcpServer:public INetEvent
{
public:
	EasyTcpServer() {
		//_clients = 
	}
	virtual ~EasyTcpServer() {

	}

	void InitSocket() {
#ifdef _WIN32
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif

		//1����1��socket
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock) {
			std::cout << "ERROR,�����׽���ʧ��" << std::endl;
		}
		else {
			std::cout << "�����׽��ֳɹ�" << std::endl;
		}
	}

	void Bind() {
		//2��
		sockaddr_in _sin;
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(14567);
		//_sin.sin_addr.S_un.S_addr = inet_addr(INADDR_ANY);
		//_sin.sin_addr.S_un.S_addr = INADDR_ANY;
#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = INADDR_ANY;
#else
		_sin.sin_addr.s_addr = INADDR_ANY;
#endif 

		if (SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(_sin))) {
			std::cout << "ERROR,�󶨶˿�ʧ��" << std::endl;
		}
		else {
			std::cout << "�󶨶˿ڳɹ�" << std::endl;
		}

		
	}

	void Listen() {
		//3�����˿�
		if (SOCKET_ERROR == listen(_sock, 10002)) {
			std::cout << "ERROR,�����˿�ʧ��" << std::endl;
		}
		else {
			std::cout << "�����˿ڳɹ�" << std::endl;
		}

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
			//4�ر��׽���
			closesocket(_sock);
			WSACleanup();
#else
			close(_sock);
#endif
		}
	}

	void Accept() {
		//4 accept �ȴ����տͻ�������
		sockaddr_in _sinClient = {};
		int nAddrlen = sizeof(_sinClient);
		SOCKET cSock = INVALID_SOCKET;
#ifdef _WIN32
		cSock = accept(_sock, (sockaddr*)&_sinClient, &nAddrlen);
#else
		cSock = accept(_sock, (sockaddr*)&_sinClient, (socklen_t*)&nAddrlen);
#endif
		if (INVALID_SOCKET == cSock) {
			std::cout << "ERROR,acceptʧ��" << std::endl;
		}
		else {
			//std::cout << "accept�ɹ�" << std::endl;

			//std::cout << "�¿ͻ��˼��룺IP = " << inet_ntoa(_sinClient.sin_addr) << std::endl;
			/*for (int n = 0; n < (int)_clients.size(); n++) {
				NewUserJoin userJoin;

				send(_clients[n]->sockfd(), (const char *)&userJoin, sizeof(userJoin), 0);
			}*/
			//_clients.push_back(new ClientSocket(cSock));
			std::shared_ptr<ClientSocket> pClientSocket = std::make_shared<ClientSocket>(cSock);
			//addClientToCellServer(new ClientSocket(cSock));
			addClientToCellServer(pClientSocket);
		}

	}

	void addClientToCellServer(std::shared_ptr<ClientSocket> pClient) {
	//void addClientToCellServer(std::shared_ptr<ClientSocket> pClient) {
		_clients.push_back(pClient);

		auto pMinServer = _cellServers[0];
		for (auto pCellServer : _cellServers) {
			if (pMinServer->getClientNum() > pCellServer->getClientNum()) {
				pMinServer = pCellServer;
			}
		}

		pMinServer->addClient(pClient);
	}
	void Start() {
		for (int n = 0; n < 4; n++) {
			std::shared_ptr<CellServer> ser = std::make_shared<CellServer>(_sock);
			//auto ser = new CellServer(_sock);
			_cellServers.push_back(ser);
			ser->setNetEventObj(this);
			ser->Start();
		}
	}

	bool isRun() {
		return true;
	}
	void onRun() {
		timePrintf();
		fd_set _fdReads;
		fd_set _fdWrites;
		fd_set _fdExecpts;
		
		//if (_bFirst) {
			FD_ZERO(&_fdReads);
			FD_ZERO(&_fdWrites);
			FD_ZERO(&_fdExecpts);
			FD_SET(_sock, &_fdReads);
			FD_SET(_sock, &_fdWrites);
			FD_SET(_sock, &_fdExecpts);
			_bFirst = false;
		//}
		
		//while (isRun()) {
			timeval t = { 0,0 };
			int ret = select(_sock + 1, &_fdReads, &_fdWrites, &_fdExecpts, &t);
			if (ret < 0) {
				std::cout << "select���ʹ����������" << std::endl;
				return;
			}
			if (FD_ISSET(_sock, &_fdReads)) {
				//std::cout << "�ͻ�������Ϣ������" << std::endl;

				FD_CLR(_sock, &_fdReads);

				Accept();
			}

		//}
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
	

	//��������
	int SendData(SOCKET cSock,DataHeader *header) {
		if (isRun() && (header != nullptr)) {
			return send(cSock, (const char*)header, header->dataLength, 0);
		}
		return 0;
	}

	int SendDataToAll(DataHeader *header) {
		if (isRun() && (header != nullptr)) {
			for (int n = 0; n < (int)_clients.size(); n++) {
				SendData(_clients[n]->sockfd(), header);
			}
		}
		return 0;
	}

	void OnLeave(std::shared_ptr<ClientSocket> pClient) {
		for (int n = 0; n < (int)_clients.size(); n++) {
			if (_clients[n] == pClient) {
				auto iter = _clients.begin() + n;
				if (iter != _clients.end()) {
					_clients.erase(iter);
				}
			}
		}
	}

	void OnNetRecv(std::shared_ptr<ClientSocket> pClient) {
		_recvCount++;
	}

	void OnNetMsg(std::shared_ptr<ClientSocket> pClient) {
		_msgCount++;
	}

	void timePrintf() {
		auto a1 = _timestemp.getSecond();
		if (a1 >= 1.0) {
			//std::cout << "[" << a1 << "]����socket[" << cSock << "]--recv�����ݸ���[" << _recvCount << "]" << std::endl;
			std::cout << "[" << a1 << "]s��" << "--recvPackageCount[" << _recvCount << "]--msgCount["<< _msgCount<<"]" << std::endl;
			_recvCount = 0;
			_msgCount = 0;
			_timestemp.update();
			
		}
	}
private:
	SOCKET _sock;
	std::vector<std::shared_ptr<ClientSocket>> _clients;
	//std::vector<std::shared_ptr<ClientSocket>> _clients;
	//std::vector<CellServer*> _cellServers;
	std::vector<std::shared_ptr<CellServer>> _cellServers;

	bool _bFirst = true;

	std::atomic_int _recvCount = 0;
	std::atomic_int _msgCount = 0;

	CellTimestemp _timestemp;
	//int _recvCount = 0;
	//char _szRecv[RECV_BUFF_SIZE] = {};
};






#endif // !_EasyTcpServer_hpp_
