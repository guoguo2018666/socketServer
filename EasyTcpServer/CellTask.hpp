#ifndef _CEllTASK_hpp_
#define _CEllTASK_hpp_
#include "alloc.h"
#include <mutex>
#include <thread>
#include <atomic>
#include <list>
#include <chrono>

#include<functional>

//��������-����
/*class CellTask
{
public:
	CellTask() {

	}
	virtual ~CellTask() {

	}

	//ִ������
	virtual void doTask() {

	}

private:

};*/


//������--ִ������ķ�������
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
	//�����߳�
	void start() {
		std::thread t(std::mem_fn(&CellTaskServer::onRun),this);
		t.detach();
	}
	//��������
	void onRun() {
		
		while (true)
		{
			{//�ӻ�����ȡ����
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

			//��������
			for (auto pTask : _tasks) {
				//pTask->doTask();
				pTask();
				
			}
			_tasks.clear();

		}
	}
private:
	//��������
	//std::list<std::shared_ptr<CellTask>> _tasks;
	std::list<CellTask> _tasks;
	//�������ݻ�����
	//std::list<std::shared_ptr<CellTask>> _tasksBuf;
	std::list<CellTask> _tasksBuf;
	//�ı����ݻ�����ʱ��Ҫ����
	std::mutex _mutex;
	//�߳�
	//std::thread* _thread;

};

#endif //