#include"ThreadPool.h"
#include<Windows.h>
int ThreadPool::N_threads = 0;


ThreadPool::ThreadPool() 
{
	running = true;
	N_threads = std::thread::hardware_concurrency();
	
	for (int i = 0; i < N_threads; i++) 
	{
		threads.emplace_back(&ThreadPool::ThreadWork, this);
		DWORD_PTR affinityMask = 1 << i;
		HANDLE threadHandle = threads[i].native_handle();
		//SetThreadAffinityMask(threadHandle,affinityMask);
	}


}

void ThreadPool::ThreadWork() 
{
	while (running) //enquanto a thread estiver viva e rodando fará:
	{
		std::function<void()> task;

		{//O lock so acontece dentro desse escopo , importante!!!
			std::unique_lock<std::mutex> lock(mutex);

			cv.wait(lock, [&] {return !queue.empty() || !running; });

			if (!running && queue.empty()) 
			{
				return;
			}

			task = std::move(queue.front());
			queue.pop();

		}//fim do lock do mutex
		task();
	}

}

void ThreadPool::enqueueTask(std::function<void()> task)
{
	{
		std::unique_lock<std::mutex> lock(mutex);
		queue.push(task);
	}

	cv.notify_one();
}

ThreadPool::~ThreadPool() 
{
	for (std::thread& th : threads) 
	{
		if (th.joinable()) th.join();
	}


}