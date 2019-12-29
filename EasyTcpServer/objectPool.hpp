#ifndef _CELL_OBJECT_POOL_
#define _CELL_OBJECT_POOL_
#include<stdlib.h>
#include<mutex>
#include<assert.h>

template<class Type, size_t nPoolSize>
class ObjectPool
{
private:

	class NodeHeader
	{
	public:
		int _nID;//内存块编号ID
		char _cRef;//引用次数
		NodeHeader* _pNext;//下一块位置
		bool _bPool;//是否在内存池中
	private:
		char c1;
		char c2;
	};

public:
	ObjectPool() {
		_pBuf = nullptr;
		initPool();
	}
	virtual ~ObjectPool() {
		if (nullptr != _pBuf) {
			delete[] _pBuf;
		}	
	}

	
	//释放内存
	void  freeObjectMemory(void * pMem) {
		char* pDate = (char*)pMem;
		NodeHeader* pBlock = (NodeHeader*)(pDate - sizeof(NodeHeader));
		assert(1 == pBlock->_cRef);
		if (0 != (pBlock->_cRef - 1)) {
			return;
		}

		if (false == pBlock->_bPool) {
			delete []pBlock;
			//free(pBlock);
		}
		else
		{
			std::lock_guard<std::mutex> lg(_lockMutex);
			pBlock->_pNext = _pHeader;
			pBlock->_cRef = 0;
			_pHeader = pBlock;
		}
	}


	//申請對象
	//申请内存
	void * allocObjectMemory(size_t nSize) {
		std::lock_guard<std::mutex> lg(_lockMutex);
		if (nullptr == _pBuf) {
			std::cout << "[" << nSize << "]pbuf is null" << std::endl;
			initPool();
		}

		NodeHeader* pReturn = nullptr;
		//不在内存中
		if (nullptr == _pHeader) {
			size_t nRealSize = sizeof(Type) + sizeof(NodeHeader);
			pReturn = (NodeHeader*)new char(nRealSize);
			pReturn->_bPool = false;
			pReturn->_nID = -1;
			pReturn->_cRef = 1;
			pReturn->_pNext = nullptr;
			std::cout << "NO In object Pool allocMem=[" << pReturn << "]--id=[" << pReturn->_nID << "]--size=[" << nSize << "]" << std::endl;
		}
		else
		{
			pReturn = _pHeader;
			_pHeader = _pHeader->_pNext;
			assert(0 == pReturn->_cRef);
			pReturn->_cRef = 1;
			std::cout << "object allocMem=[" << pReturn << "]--id=[" << pReturn->_nID << "]--size=[" << nSize << "]" << std::endl;
		}

		return (void*)((char*)pReturn + sizeof(NodeHeader));
	}

private:
	//初始化對象池
	void initPool() {
		//申請内存
		size_t nRealSize = sizeof(Type) + sizeof(NodeHeader);
		size_t nSize = nPoolSize * nRealSize;
		_pBuf = new char[nSize];

		_pHeader = (NodeHeader*)_pBuf;
		_pHeader->_nID = 0;
		_pHeader->_cRef = 0;
		_pHeader->_pNext = nullptr;
		_pHeader->_bPool = true;

		NodeHeader* pTempBefore = _pHeader;
		for (size_t n = 1; n < nPoolSize; n++) {
			NodeHeader* pTemp = (NodeHeader*)(_pBuf + n * nRealSize);
			pTemp->_bPool = true;
			pTemp->_nID = n;
			pTemp->_cRef = 0;
			pTemp->_pNext = nullptr;

			pTempBefore->_pNext = pTemp;
			pTempBefore = pTemp;
		}
	}

private:
	NodeHeader* _pHeader;
	//對象池内存池地址
	char* _pBuf;
	std::mutex _lockMutex;

};

template<class Type, size_t nPoolSize>
class ObjectPoolBase
{
public:
	ObjectPoolBase() {

	}
	virtual ~ObjectPoolBase() {

	}

	void *operator new(size_t nSize) {
		std::cout << "new" << std::endl;
		return createObjectPool().allocObjectMemory(nSize);
	}

	void operator delete(void * p) {
		std::cout << "delete" << std::endl;
		createObjectPool().freeObjectMemory(p);
	}

	template<typename ...Args>
	static Type* createObject(Args ... args) {
		Type* pObjectPoolBase = new Type(args...);
		return pObjectPoolBase;
	}

	static void deleteObject(Type* pObject) {
		delete pObject;
	}


private:
	typedef ObjectPool<Type, nPoolSize> ClassTypePool;
	static ClassTypePool& createObjectPool() {
		//靜態對象
		static ClassTypePool objPool;
		return objPool;
	}

};












#endif // !CELL_OBJECT_POOL
