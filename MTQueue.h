//
//  MTQueue.h
//
//  Created by MihailShevchuk on 6/22/18.
//  Copyright © 2018 Mihail Shevchuk. All rights reserved.
//

#ifndef MTQueue_h
#define MTQueue_h

#include <queue>
#include <mutex>
#include <iostream>
#include <stdexcept>
#include <condition_variable>

template <typename Object>
class MTQueue {
public:
    /*!
     The class constructor.
     @param maxSize - Max size of queue.
    */
    MTQueue(uint64_t maxSize = 10000000);
    
    /*!
     The class destructor.
    */
    virtual ~MTQueue(){}
    
    /*!
     Writes the item to the queue.
     @param item - some object.
     @return - result of push.
    */
    bool push(const Object& item);
    
    /*!
     Move the item to the queue.
     @param item - some object.
     @return - result of push.
    */
    bool push(Object&& item);
    
    /*!
     Returns the first object in the queue.
     If the queue is empty, returns an empty object.
     @return - item in the queue.
    */
    Object pop();
    
    /*!
     Writes the first item in the queue to the passed variable.
     @param item - to transfer object from queue.
    */
    void pop(Object& item);
    
    /*!
     Returns the first object in the queue.
     If the queue is empty, waiting for the object to appear.
     @return - item in the queue.
    */
    Object popWithWait();
    
    void removeBuffers();
    
protected:
    uint64_t mMaxQueueSize;        // Max size of queue.
    std::mutex mMutex;             // Сoncurrency control mutex.
    std::queue<Object> mQueue;     // Buffers queue.
    std::condition_variable mCond; //
    
private:
    /*! Closing access to the copy constructor. */
    MTQueue(const MTQueue& src) = delete;
    
    /*! Closing access to the copy. */
    MTQueue& operator = (const MTQueue& rhs) = delete;
};

//******************************************************************************
template <typename Object>
MTQueue<Object>::MTQueue(uint64_t maxSize) {
    mMaxQueueSize = maxSize;
}

//******************************************************************************
template <typename Object>
void MTQueue<Object>::removeBuffers() {
    std::unique_lock<std::mutex> lock(mMutex);
    if (mQueue.size() != 0)
        mQueue = {};
    lock.unlock();
}

//******************************************************************************
template <typename Object>
bool MTQueue<Object>::push(Object&& item) {
    if (mMaxQueueSize > 0 && mQueue.size() == static_cast<size_t>(mMaxQueueSize))
        return false;
    
    std::unique_lock<std::mutex> mlock(mMutex);
    mQueue.push(std::move(item));
    mlock.unlock();
    mCond.notify_one();
    
    return true;
}

//******************************************************************************
template <typename Object>
bool MTQueue<Object>::push(const Object& item) {
    if (mMaxQueueSize > 0 && mQueue.size() == static_cast<size_t>(mMaxQueueSize))
        return false;
    
    std::unique_lock<std::mutex> lock(mMutex);
    mQueue.push(item);
    lock.unlock();
    mCond.notify_one();
    
    return true;
}

//******************************************************************************
template <typename Object>
Object MTQueue<Object>::pop() {
    std::unique_lock<std::mutex> lock(mMutex);
    if (mQueue.empty()) {
        Object temp;
        return temp;
    }

    Object temp = mQueue.front();
    mQueue.pop();
    lock.unlock();

    return temp;
}

//******************************************************************************
template <typename Object>
void MTQueue<Object>::pop(Object& item) {
    std::unique_lock<std::mutex> lock(mMutex);
    item = mQueue.front();
    mQueue.pop();
    lock.unlock();
}

//******************************************************************************
template <typename Object>
Object MTQueue<Object>::popWithWait() {
    std::unique_lock<std::mutex> lock(mMutex);
    
    while (mQueue.empty()) {
        mCond.wait(lock);
    }
    
    auto item = mQueue.front();
    mQueue.pop();
    lock.unlock();
    
    return item;
}

#endif /* MTQueue_h */
