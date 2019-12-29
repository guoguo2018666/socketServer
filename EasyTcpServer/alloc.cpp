
//#include "stdafx.h"
#include "alloc.h"
#include <iostream>
#include "memoryMgr.hpp"

void *operator new(size_t size) {
	return MemoryMgr::Instance().allocMemory(size);
}

void operator delete(void * p) {
	MemoryMgr::Instance().freeMemory(p);
}

void *operator new[](size_t size) {
	//std::cout << "new[]" << std::endl;
	return MemoryMgr::Instance().allocMemory(size);
}

void operator delete[](void * p) {
	//std::cout << "delete[]" << std::endl;
	MemoryMgr::Instance().freeMemory(p);
}