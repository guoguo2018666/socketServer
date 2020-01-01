#ifndef _MEMORY_MGR_hpp_
#define _MEMORY_MGR_hpp_

#define MAX_MEMORY_SIZE 2048
#define MAX_MEMORY_SIZE_64 64
#define MAX_MEMORY_SIZE_128 128
#define MAX_MEMORY_SIZE_256 256
#define MAX_MEMORY_SIZE_512 512
#define MAX_MEMORY_SIZE_1024 1024

#ifdef _DEBUG
  #include <stdio.h>
  #define xPrintf(...) printf(__VA_ARGS__);
#else
  #define xPrintf(...)
#endif //DEBUG

#include <iostream>
#include <assert.h>
#include <mutex>
class MemoryAlloc;
//内存块，最小单元
class MemoryBlock
{
public:
	MemoryBlock() {

	}
	~MemoryBlock() {
		
	}


	int _nID;//内存块编号ID
	int _nRef;//引用次数
	MemoryAlloc* _pAlloc; //所属最大内存块
	MemoryBlock* _pNext;//下一块位置
	bool _bPool;//是否在内存池中
private:
	char _cNull0;//预留
	char _cNull1;//预留
	char _cNull2;//预留


};





//内存池
class MemoryAlloc
{
public:
	MemoryAlloc() {
		_pBuf = nullptr;
		_pHeader = nullptr;
		_nBlockNum = 0;
		_nEveryBlockMemorySize = 0;
	}
	~MemoryAlloc() {
		free(_pBuf);
	}

	//申请内存
	void * allocMemory(size_t nSize) {
		std::lock_guard<std::mutex> lg(_lockMutex);
		if (nullptr == _pBuf) {
			std::cout <<"["<< _nEveryBlockMemorySize << "]pbuf is null" << std::endl;
			Initmemory();
		}

		MemoryBlock* pReturn = nullptr;
		//不在内存中
		if (nullptr == _pHeader) {
			pReturn = (MemoryBlock*)malloc(nSize + sizeof(MemoryBlock));
			pReturn->_bPool = false;
			pReturn->_nID = -1;
			pReturn->_nRef = 1;
			pReturn->_pAlloc = nullptr;
			pReturn->_pNext = nullptr;
			std::cout << "NO In allocMem=[" << pReturn << "]--BlockSize["<< _nEveryBlockMemorySize <<"]--id=[" << pReturn->_nID << "]--size=[" << nSize << "]" << std::endl;
		}
		else
		{
			pReturn = _pHeader;
			_pHeader = _pHeader->_pNext;
			assert(0 == pReturn->_nRef);
			pReturn->_nRef = 1;
			std::cout << "allocMem=[" << pReturn << "]--BlockSize[" << _nEveryBlockMemorySize << "]--id=[" << pReturn->_nID << "]--size=[" << nSize << "]" << std::endl;
		}

		return (void*)((char*)pReturn+sizeof(MemoryBlock));
	}
	//释放内存
	void  freeMemory(void * pMem) {
		std::lock_guard<std::mutex> lg(_lockMutex);
		//free(p);
		char* pDate = (char*)pMem;
		
		MemoryBlock* pBlock = (MemoryBlock*)(pDate - sizeof(MemoryBlock));
		assert(1 == pBlock->_nRef);
		if (0 != (--pBlock->_nRef)){
			return;
		}

		if (false == pBlock->_bPool) {
			//free(pMem);
			free(pBlock);
		}
		else
		{
			pBlock->_pNext = _pHeader;
			pBlock->_nRef = 0;
			_pHeader = pBlock; 	
			
			//free(pMem);
		}
		
	}

	//初始化内存池
	void Initmemory() {
		//std::lock_guard<std::mutex> lg(_lockMutex);
		assert(nullptr == _pBuf);
		if (nullptr != _pBuf) {
			return;
		}
		//向系统申请池内存
		size_t bufSize = _nBlockNum*(_nEveryBlockMemorySize+sizeof(MemoryBlock));
		_pBuf = (char*)malloc(bufSize);
		//初始化内存池
		_pHeader = (MemoryBlock*)_pBuf;
		_pHeader->_bPool = true;
		_pHeader->_nID = 0;
		_pHeader->_nRef = 0;
		_pHeader->_pAlloc = this;
		_pHeader->_pNext = nullptr;

		MemoryBlock* pTempBefore = _pHeader;
		for (size_t n = 1; n < _nBlockNum; n++) {
			//MemoryBlock* pTemp = (MemoryBlock*)(_pBuf + n * _nEveryBlockMemorySize);
			MemoryBlock* pTemp = (MemoryBlock*)(_pBuf + n * (_nEveryBlockMemorySize + sizeof(MemoryBlock)));
			pTemp->_bPool = true;
			pTemp->_nID = n;
			pTemp->_nRef = 0;
			pTemp->_pAlloc = this;
			pTemp->_pNext = nullptr;

			pTempBefore->_pNext = pTemp;
			pTempBefore = pTemp;
		}
		
	}

protected:
	//内存池地址
	char* _pBuf;
	//内存块头部
	MemoryBlock* _pHeader;
	//内存块的容量大小
	size_t _nEveryBlockMemorySize ;
	//内存块的数量
	size_t _nBlockNum;

	std::mutex _lockMutex;
};

template<size_t nEveryBlockMemorySize,size_t nBlockNum>
class MemoryAlloctor:public MemoryAlloc
{
public:
	MemoryAlloctor() {
		//字节对齐
		const size_t n = sizeof(void*);
		_nEveryBlockMemorySize = (nEveryBlockMemorySize / n)*n+(nEveryBlockMemorySize%n ? n:0);
		
		//_nEveryBlockMemorySize = nEveryBlockMemorySize;
		_nBlockNum = nBlockNum;
	}
	virtual ~MemoryAlloctor() {

	}

private:

};


//内存管理工具
class MemoryMgr
{
private:
	MemoryMgr() {
		init(0, 64, &_mem64);
		init(65, 128, &_mem128);
		init(129, 256, &_mem256);
		init(257, 512,  &_mem512);
		init(513, 1024, &_mem1024);
		
	}
	virtual ~MemoryMgr() {

	}

	//初始化内存池映射数组
	void init(int nBegin,int nEnd, MemoryAlloc* pMemAlloc) {
		for (int n = nBegin; n <= nEnd; n++) {
			_szAlloc[n] = pMemAlloc;
			//szAlloc[n] = pMemAlloc;
		}
	}


public:
	//单例模式
	static MemoryMgr& Instance() {
		static MemoryMgr mgr;
		return mgr;
	}


	//申请内存
	void * allocMemory(size_t nSize) {
		//return malloc(size);
		if (nSize <= MAX_MEMORY_SIZE) {
			return _szAlloc[nSize]->allocMemory(nSize);
		}
		else {
			//printf("+++allocMemory\n");
			//xPrintf("allocMemory\n");
			//std::cout << "sizeof(MemoryBlock)[" << sizeof(MemoryBlock) << "]" << std::endl;
			MemoryBlock* pReturn = (MemoryBlock*)malloc(nSize + sizeof(MemoryBlock));
			pReturn->_bPool = false;
			pReturn->_nID = -1;
			pReturn->_nRef = 1;
			pReturn->_pAlloc = nullptr;
			pReturn->_pNext = nullptr;
			//std::cout << "pReturn[" << pReturn << "]--allocReturn["<< (char*)pReturn + sizeof(MemoryBlock)<<"]" << std::endl;
			//std::cout << "NO111 In allocMem=[" << pReturn << "]--id=[" << pReturn->_nID << "]--size=[" << nSize << "]" << std::endl;
			//freeMemory((char*)pReturn + sizeof(MemoryBlock));
			return (char*)pReturn + sizeof(MemoryBlock);
		}
	}
	//释放内存
	void  freeMemory(void * pMem) {
		//std::cout << "sizeof(MemoryBlock)[" << sizeof(MemoryBlock) << "]" << std::endl;
		//std::cout << "freeMemory pMem[" << pMem << "]"<< std::endl;
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
		//std::cout << "freeMemory pMem[" << pMem << "]--pBlock[" << pBlock << "]" << std::endl;
		if (pBlock->_bPool) {
			pBlock->_pAlloc->freeMemory(pMem);
		}
		else {
			if (--pBlock->_nRef == 0) {
				free(pBlock);//TODO:应该删除pBlock;
			}
		}
	}

	void addRef(void* pMem) {
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
		pBlock->_nRef++;
	}

private:
	MemoryAlloctor<MAX_MEMORY_SIZE_64, 200> _mem64;
	MemoryAlloctor<MAX_MEMORY_SIZE_128, 200> _mem128;
	MemoryAlloctor<MAX_MEMORY_SIZE_256, 200> _mem256;
	MemoryAlloctor<MAX_MEMORY_SIZE_512, 200> _mem512;
	MemoryAlloctor<MAX_MEMORY_SIZE_1024, 200> _mem1024;

	MemoryAlloc* _szAlloc[MAX_MEMORY_SIZE + 1];
	
	
	
};





#endif