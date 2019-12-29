
#include "alloc.h"
#include "EasyTcpServer.hpp"

int main() {

	EasyTcpServer easyTcpServer;
	easyTcpServer.InitSocket();
	easyTcpServer.Bind();
	easyTcpServer.Listen();
	easyTcpServer.Start();
	while (easyTcpServer.isRun()) {
		easyTcpServer.onRun();
	}
	
	easyTcpServer.CloseSocket();
}