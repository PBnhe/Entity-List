





#ifndef THREADS_H
#define THREADS_H

#include<thread>
#include<mutex>
#include<queue>
#include<condition_variable>
#include<atomic>
#include<functional>
#include <vector>


class ThreadPool 
{
private:

	std::vector<std::thread> threads;
	std::mutex mutex;
	std::condition_variable cv;
	std::atomic<bool> running;

	void ThreadWork();
	std::queue<std::function<void()>> queue;

public:
	static int N_threads;
	ThreadPool();
	~ThreadPool();
	void enqueueTask(std::function<void()> task);

};







































#endif // !THREADS
