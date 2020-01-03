#ifndef _CELL_BUFFER_HPP_
#define _CELL_BUFFER_HPP_
#include <memory>
//#include<functional>
//#define RECV_BUFF_SIZE 1024*10

class CELLBuffer
{
public:
	CELLBuffer(int nSize,int nFlag) {
		_nBuffFullCount = 0;
		_unDoSize = 0;
		_nSize = nSize;
		_nSendOrRecvFlag = nFlag;
		_pMsgBuf = new char[_nSize];
	}
	~CELLBuffer() {
		if (! _pMsgBuf) {
			delete[] _pMsgBuf;
		}
	}
	char* getData() {
		return _pMsgBuf;
	}

	int pop(int nLength) {
		int ret = -1;
		//if (nLength > _unDoSize) {
			//return ret;
		//}

		if (_nBuffFullCount > 0) {
			_nBuffFullCount--;
		}

		int nLeftSize = _unDoSize - nLength;
		//std::cout << "pop--_unDoSize[" << _unDoSize << "]--nLength[" << nLength << "]--nLeftSize["<< nLeftSize<<"]" << std::endl;
		memcpy(_pMsgBuf, _pMsgBuf + nLength, nLeftSize);
		_unDoSize = nLeftSize;
		ret = 0;
		return ret;
	}

	bool push(const char * pData,int nDatalength) {
		/*if ((_unDoSize + nDatalength) > _nSize) {
            //需要写入的数据大于可用空间
			int n = (_unDoSize + nDatalength) - _nSize;
			if (n < 8192) {
				n = 8192;
			}
			char* buff = new char[_nSize + n];
			memcpy(buff,_pBuff,_unDoSize);
			delete[] _pBuff;
			_pBuff = buff;
		}*/

		if ((_unDoSize + nDatalength) <= _nSize){
			memcpy(_pMsgBuf + _unDoSize, pData, nDatalength);
			_unDoSize += nDatalength;

			if (_nSize == _unDoSize) {
				_nBuffFullCount++;
			}

			return true;
		}
		else {//数据量超过缓冲区
			std::cout <<"["<< _nSendOrRecvFlag << "]缓冲区满" << std::endl;
			_nBuffFullCount++;
		}

		return false;
	}

	//立即将缓冲区发送给客户端
	int Write2Socket(SOCKET sockfd) {
		int ret = 0;
		if ((_unDoSize > 0) && (INVALID_SOCKET != sockfd)) {
			int ret = send(sockfd, _pMsgBuf, _unDoSize, 0);
			if (ret == SOCKET_ERROR) {
			return ret;
			}
			memset(_pMsgBuf, 0, _nSize);
			_unDoSize = 0;

			
			_nBuffFullCount = 0;
			ret = 0;
		}
		return ret;
	}

	int Read4Socket(SOCKET sockfd) {
		int nRet = -1;
		//std::cout << "_unDoSize[" << _unDoSize << "]--_nSize[" << _nSize << "]--_nSize-_unDoSize[" << _nSize - _unDoSize << "]" << std::endl;
		if ( _unDoSize < _nSize) {
			char * szRecv = _pMsgBuf + _unDoSize;
			int nLength = recv(sockfd, szRecv, _nSize - _unDoSize, 0);
			//std::cout << "nLength[" << nLength<<"]" << std::endl;
			if (nLength <= 0) {
				//std::cout << "[" << sockfd << "]与服务器断开连接，任务结束" << std::endl;
				return nLength;
			}
			_unDoSize += nLength;
			nRet = nLength;
		}
		return nRet;

	}

	bool hasMsg() {
		
		if (_unDoSize >= sizeof(DataHeader)) {
			DataHeader* pDataHeader = (DataHeader*)_pMsgBuf;
			if (_unDoSize >= pDataHeader->dataLength) {
				return true;
			}
		}

		return false;
	}

	bool needWrite() {
		if (_unDoSize > 0) {
			return true;
		}
		return false;
	}

	
	/*void HandleRecvData(std::function<void(std::shared_ptr<ClientSocket>)> dealFun, std::shared_ptr<ClientSocket> pClient) {
		DataHeader* pDataHeader = (DataHeader*)_pMsgBuf;
		if (_unDoSize >= sizeof(DataHeader)) {		
			if (_unDoSize >= pDataHeader->dataLength) {
				//缓冲区剩余未处理消息的长度
				int nLeftSize = _unDoSize - pDataHeader->dataLength;

				//OnNetMessage(pClient);
				dealFun(pClient);
				memcpy(_pMsgBuf, _pMsgBuf + pDataHeader->dataLength, nLeftSize);
				_unDoSize = nLeftSize;
			}
		}
	}*/


private:

	//缓冲区--处理数据缓冲区
	//std::shared_ptr<char> _pMsgBuf;
	char * _pMsgBuf =  nullptr;
	//缓冲区中未处理数据的长度
	int _unDoSize;

	
	int _nSize;

	//缓冲区是否被放满的次数
	int _nBuffFullCount;

	int _nSendOrRecvFlag = 0;

};





#endif //