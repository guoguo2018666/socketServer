#ifndef _CELL_lOG_HPP_
#define _CELL_lOG_HPP_
#include<iostream>

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
		printf(pStr);
	}

	template<typename ...Args>
	static void Info(const char* pFormat, Args ... args) {
		//std::cout << pStr << std::endl;
		printf(pFormat, args...);
	}

private:
	CELLLog() {

	}
	~CELLLog() {

	}
private:


};




#endif //