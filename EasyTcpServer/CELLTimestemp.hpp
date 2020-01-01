
#ifndef _CellTimestemp_hpp_
#define _CellTimestemp_hpp_
//#include "alloc.h"
#include <chrono>

using namespace std::chrono;


class CELLTime
{
public:
	CELLTime() {

	}
	~CELLTime() {

	}

public:
	//«@È¡®”Ç°•rég´Á
	static time_t getNowTimeInMillsec() {	
		return duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();
	}

};



class CellTimestemp
{
public:
	CellTimestemp() {
		update();
	}
	virtual ~CellTimestemp() {

	}

	void update() {
		_timePointBegin = high_resolution_clock::now();
	}

	double getSecond() {
		return getMicoSecond() * 0.000001;
	}

	double getMillSecond() {
		return getMicoSecond() * 0.001;
	}

	long long getMicoSecond() {
		return duration_cast<microseconds>(high_resolution_clock::now() - _timePointBegin).count();
	}

private:

	time_point<high_resolution_clock> _timePointBegin;

};

#endif