#ifndef _FRAME_QUEUE
#define _FRAME_QUEUE
/*
 * Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 */

#include "VideoLock.h"
#include <nvcuvid.h>

#include <optional>
#include <thread>
#include <memory>
#include <pthread.h>
#include <cstring>
#include <unistd.h>

typedef pthread_mutex_t CRITICAL_SECTION;
typedef void *HANDLE;

class FrameQueue {
public:
  static const unsigned int cnMaximumSize = 20; // MAX_FRM_CNT;

  explicit FrameQueue(CUvideoctxlock ctxLock);

  explicit FrameQueue(VideoLock &lock);

  virtual ~FrameQueue();

  void waitForQueueUpdate();

  void enter_CS(CRITICAL_SECTION *pCS);

  void leave_CS(CRITICAL_SECTION *pCS);

  void set_event(HANDLE event);

  void reset_event(HANDLE event);

  //virtual void init(int frameWidth, int frameHeight) {}

  virtual void enqueue(const void *pData) = 0;

  // Deque the next frame.
  // Parameters:
  //      pDisplayInfo - New frame info gets placed into this object.
  //          Note: This pointer must point to a valid struct. The method
  //          does not create memory for this.
  // Returns:
  //      true, if a new frame was returned,
  //      false, if the queue was empty and no new frame could be returned.
  //          In that case, pPicParams doesn't contain valid data.
  virtual bool dequeue(void *pData) = 0;

  virtual void releaseFrame(const void *pPicParams) = 0;

  bool isInUse(int nPictureIndex) const;

  bool isEndOfDecode() const;

  void endDecode();

  // Spins until frame becomes available or decoding
  // gets canceled.
  // If the requested frame is available the method returns true.
  // If decoding was interupted before the requested frame becomes
  // available, the method returns false.
  bool waitUntilFrameAvailable(int nPictureIndex);

  void reset() {
      hEvent_ = nullptr;
      nReadPosition_ = 0;
      nWritePosition_ = 0;
      nFramesInQueue_ = 0;
      bEndOfDecode_ = false;
      for (auto &i : aIsFrameInUse_)
          i = 0;
      //memset(reinterpret_cast<void*>(aIsFrameInUse_), 0, cnMaximumSize * sizeof(int));
  }

  size_t getPitch() { return nPitch; }

  bool isEmpty() { return nFramesInQueue_ == 0; }

  bool isComplete() { return isEndOfDecode() && isEmpty(); }

    template<typename T>
    const std::shared_ptr<T> dequeue_wait() {
        std::shared_ptr<T> data(new T, [this](T *data) { this->releaseFrame(data); delete data; });

        while(!dequeue(data.get()))
            std::this_thread::yield();
        return data;
    }

    template<typename T>
    const std::shared_ptr<T> dequeue() {
        T data;

        return dequeue(&data)
            ? std::shared_ptr<T>(new T{data}, [this](T *d) { this->releaseFrame(d); delete d; })
            : nullptr;
    }

protected:
  void signalStatusChange();

  HANDLE hEvent_;
  CRITICAL_SECTION oCriticalSection_;
  volatile int nReadPosition_;
  volatile int nWritePosition_;

  volatile int nFramesInQueue_;
  volatile int aIsFrameInUse_[cnMaximumSize];
  volatile int bEndOfDecode_;

  CUvideoctxlock m_ctxLock;
  size_t nPitch;
};

class CUVIDFrameQueue : public FrameQueue {

public:
  explicit CUVIDFrameQueue(VideoLock &lock);
  explicit CUVIDFrameQueue(CUvideoctxlock ctxLock);

  void enqueue(const void *pData) override;
  //virtual bool dequeue(CUVIDPARSERDISPINFO *data) { return dequeue(static_cast<void*>(data)); }

  const std::shared_ptr<CUVIDPARSERDISPINFO> dequeue() {
      return static_cast<FrameQueue*>(this)->try_dequeue<CUVIDPARSERDISPINFO>();
  }

    //TODO this was protected, move back after debugging
    void releaseFrame(const void *pPicParams) override;

protected:
  bool dequeue(void *pData) override;
  CUVIDPARSERDISPINFO aDisplayQueue_[cnMaximumSize];
};

#endif
