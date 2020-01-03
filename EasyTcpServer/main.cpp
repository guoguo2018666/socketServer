
//#include "alloc.h"
#include "EasyTcpServer.hpp"
#include"CELLLog.hpp"
int main() {
	CELLLog::Instance().setLogPath("serverLog.txt", "w");
	CELLLog::Info("abc");
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