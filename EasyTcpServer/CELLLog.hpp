#ifndef _CELL_lOG_HPP_
#define _CELL_lOG_HPP_

//#define _CRT_SECURE_NO_DEPRECATE
//#pragma warning(disable:4996);
#include<iostream>
#include "CellTask.hpp"
#include <stdio.h>
#include<fstream>
//#include<>
class CELLLog
{
public:
	static CELLLog& Instance() {
		static CELLLog slog;
		return slog;
	}
	//info debug warning error
	static void Info(const char* pStr) {
		//std::cout << pStr << std::endl;
		printf("%s",pStr);

		auto pLog = &Instance();
		if (pLog->_pLogFile) {
			fprintf(pLog->_pLogFile, "%s",pStr);
			fflush(pLog->_pLogFile);
		}
	}

	template<typename ...Args>
	static void Info(const char* pFormat, Args ... args) {
		//std::cout << pStr << std::endl;
		printf(pFormat, args...);

		auto pLog = &Instance();
		if (pLog->_pLogFile) {
			fprintf(pLog->_pLogFile, pFormat, args...);
			fflush(pLog->_pLogFile);
		}
	}

	void setLogPath(const char* pFilePath,const char* pMode) {
		if (_pLogFile) {
			fclose(_pLogFile);
		}

		fopen_s(&_pLogFile, pFilePath, pMode);
		//_pLogFile = fopen(pFilePath, pMode);
		if (_pLogFile) {
			Info("SetLogPath sccessed,path[%s],mode[%s]", pFilePath, pMode);
		}
		else {
			Info("SetLogPath failed,path[%s],mode[%s]", pFilePath, pMode);
		}
	}

private:
	CELLLog() {
		_taskServer.start();
	}
	~CELLLog() {
		if (_pLogFile) {
			fclose(_pLogFile);
			_pLogFile = nullptr;
		}
	}
private:

	CellTaskServer _taskServer;
	std::FILE* _pLogFile = nullptr;


};




#endif //