#pragma once
#include <vector>
#include <queue>
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>

class ThreadPool {
public:
	explicit ThreadPool(size_t numThreads);
	~ThreadPool();
	void enqueue(std::function<void()> task);
private:
	std::vector<std::thread> workers;
	std::queue<std::function<void()>> tasks;
	std::mutex queueMutex;
	std::condition_variable condition;
	bool stop;
};