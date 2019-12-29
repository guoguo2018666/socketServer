#pragma once
#include "CellTask.hpp"
#include "ClientSocket.hpp"
#include "message.hpp"

class CellSendMsgTask: public CellTask
{
public:
	CellSendMsgTask(std::shared_ptr<ClientSocket> pClientSocket,DataHeader* pHeader) {
		_pClientSocket = pClientSocket;
		_pHeader = pHeader;
	}
	virtual ~CellSendMsgTask() {
		/*if (_pHeader != nullptr) {
			delete _pHeader;
		}*/
	}

	void doTask() {
		int ret = _pClientSocket->SendData(_pHeader);
		//if (_pHeader != nullptr) {
		//	delete _pHeader;
		//}
	}

private:
	std::shared_ptr<ClientSocket> _pClientSocket;
	DataHeader* _pHeader;

};


