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

	//�ڶ�������--�������ݻ�����
	//std::shared_ptr<char> _pMsgBuf;
	char * _pMsgBuf =  nullptr;
	//�ڶ���������δ�������ݵĳ���
	int _unDoSize;

};





#endif //