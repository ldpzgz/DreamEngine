#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <functional>
#include <queue>
#include <atomic>
class Event;
class EventHandler {
public:
	explicit EventHandler(const std::function<void(Event*)>& functor) :
		mFunctor(functor) {

	}
	virtual void handleEvent(Event* p) {
		if (mFunctor) {
			mFunctor(p);
		}
	}
private:
	std::function<void(Event*)> mFunctor;
};

class Event {
public:
	Event() {

	}
	virtual ~Event() {

	}
	virtual void setHandler(EventHandler pHandler) {

	}
	virtual EventHandler* getHandler() {
		return mpHandler;
	}
	virtual void handle() {
		if (mpHandler != nullptr) {
			mpHandler->handleEvent(this);
		}
	}
private:
	EventHandler* mpHandler{nullptr};
};



class EventLoop {
public:
	EventLoop() {

	}
	~EventLoop() {
	}
	//exit loop
	void exit() {
		mbExit = true;
		wakeUp();
	}
	//push event into queue and wakeup loop to work
	void pushEvent(std::unique_ptr<Event>&& pEvent) {
		{
			std::lock_guard<std::mutex> lk(mMutex);
			mEventQueue.push(std::move(pEvent));
			mFlag = true;
		}
		mCondition.notify_one();
	}

	void start() {
		std::thread t([this] {
			poll();
			});
		t.join();
	}
	
	void poll() {
		while (!mbExit) {
			std::queue<std::unique_ptr<Event>> tempQueue;
			{
				std::unique_lock<std::mutex> lk(mMutex);
				if (!mFlag) {
					mCondition.wait(lk, [this] {return mFlag; });
				}
				mFlag = false;
				mEventQueue.swap(tempQueue);
			}
			while (!tempQueue.empty()) {
				auto& pEvent = tempQueue.front();
				if (pEvent) {
					pEvent->handle();
				}
				tempQueue.pop();
			}
		}
	}

	void pollInMain() {
		std::queue<std::unique_ptr<Event>> tempQueue;
		{
			std::lock_guard<std::mutex> lk(mMutex);
			if (!mEventQueue.empty()) {
				mEventQueue.swap(tempQueue);
			}
		}
		while (!tempQueue.empty()) {
			auto& pEvent = tempQueue.front();
			if (pEvent) {
				pEvent->handle();
			}
			tempQueue.pop();
		}
	}

	int getId() {
		return mId;
	}
private:
	//wakeUp loop to work
	void wakeUp() {
		{
			std::lock_guard<std::mutex> lk(mMutex);
			mFlag = true;
		}
		mCondition.notify_one();
	}

	int mId{0};
	bool mFlag{ false };
	std::atomic<bool> mbExit{false};
	std::mutex mMutex;
	std::condition_variable mCondition;
	std::queue<std::unique_ptr<Event>> mEventQueue;
};

class JobSystem {
public:
	explicit JobSystem();
	~JobSystem();
	JobSystem& getInstance();
	void setThreadCount(int counts);
	void postEvent(std::unique_ptr<Event>&& pEvent,bool runInMain=false);
private:
	int mThreadCounts{1};
	//std::vector<
};
