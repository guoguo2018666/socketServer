#ifndef _CEllTASK_hpp_
#define _CEllTASK_hpp_
#include "alloc.h"
#include <mutex>
#include <thread>
#include <atomic>
#include <list>
#include <chrono>

#include<functional>

//任务类型-基类
/*class CellTask
{
public:
	CellTask() {

	}
	virtual ~CellTask() {

	}

	//执行任务
	virtual void doTask() {

	}

private:

};*/


//服务类--执行任务的服务类型
class CellTaskServer
{
	typedef std::function<void()> CellTask;
public:
	CellTaskServer() {

	}
	virtual ~CellTaskServer() {

	}
	//void addTask(CellTask* pCellTask) {
	//void addTask(std::shared_ptr<CellTask> pCellTask) {
	void addTask(CellTask pCellTask) {
		//std::cout << "addTask begin" << std::endl;
		std::lock_guard<std::mutex> lg(_mutex);
		_tasksBuf.push_back(pCellTask);
		//std::cout << "addTask end" << std::endl;
	}
	//启动线程
	void start() {
		std::thread t(std::mem_fn(&CellTaskServer::onRun),this);
		t.detach();
	}
	//工作函数
	void onRun() {
		
		while (true)
		{
			{//从缓冲区取数据
				std::lock_guard<std::mutex> lg(_mutex);
				if (!_tasksBuf.empty()) {
					for (auto pTask : _tasksBuf) {
						_tasks.push_back(pTask);
					}
					_tasksBuf.clear();
				}
			}

			if (_tasks.empty()) {
				std::chrono::microseconds t(1);
				std::this_thread::sleep_for(t);
				continue;
			}

			//处理任务
			for (auto pTask : _tasks) {
				//pTask->doTask();
				pTask();
				
			}
			_tasks.clear();

		}
	}
private:
	//任务数据
	//std::list<std::shared_ptr<CellTask>> _tasks;
	std::list<CellTask> _tasks;
	//任务数据缓冲区
	//std::list<std::shared_ptr<CellTask>> _tasksBuf;
	std::list<CellTask> _tasksBuf;
	//改变数据缓冲区时需要加锁
	std::mutex _mutex;
	//线程
	//std::thread* _thread;

};

#endif //