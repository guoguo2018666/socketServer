#ifndef _CELL_BUFFER_HPP_
#define _CELL_BUFFER_HPP_
#include <memory>
//#define RECV_BUFF_SIZE 1024*10

class CELLBuffer
{
public:
	CELLBuffer() {

	}
	~CELLBuffer() {

	}

private:

	//第二缓冲区--处理数据缓冲区
	//std::shared_ptr<char> _pMsgBuf;
	char * _pMsgBuf =  nullptr;
	//第二缓冲区中未处理数据的长度
	int _unDoSize;

};





#endif //