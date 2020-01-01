#pragma once
//#include "alloc.h"
#include "ClientSocket.hpp"

class INetEvent
{
public:

	virtual void OnLeave(std::shared_ptr<ClientSocket> pClient) = 0;
	//recv ±º‰
	virtual void OnNetRecv(std::shared_ptr<ClientSocket> pClient) = 0;
	virtual void OnNetMsg(std::shared_ptr<ClientSocket> pClient) = 0;
private:

};

