/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/Bentley/Tasks/ConcurrentQueue.h $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/Tasks/Tasks.h>
#include <Bentley/Bentley.h>
#include <queue>

BEGIN_BENTLEY_TASKS_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                              Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename D>
struct ConcurrentQueue
    {
    private:
        struct DataContainer
            {
            D data;
            uint64_t index;
            };

        struct DataContainerCompare
            {
            bool operator()(const DataContainer &a, const DataContainer &b) const
                {
                if (a.data->GetPriority () < b.data->GetPriority ())
                    {
                    return true;
                    }
                else if (a.data->GetPriority () > b.data->GetPriority ())
                    {
                    return false;
                    }
                else
                    {
                    return a.index > b.index;
                    }
                }
            };

        typedef std::priority_queue<DataContainer, std::vector<DataContainer>, DataContainerCompare> DataContainerQueue;

        DataContainerQueue m_queue;

        uint64_t m_queueIndex;

        mutable std::mutex m_mutex;
        std::condition_variable m_isEmptyCV;
        std::condition_variable m_isNotEmptyCV;

    public:
        ConcurrentQueue () :
            m_queueIndex (0)
            {
            };

        std::mutex& GetMutex () const
            {
            return m_mutex;
            }

        void Push (const D& data)
            {
            std::unique_lock<std::mutex> l (m_mutex);

            DataContainer dataContainer;
            dataContainer.data = data;
            dataContainer.index = m_queueIndex++;

            m_queue.push (dataContainer);

            m_isNotEmptyCV.notify_one ();
            }

        bool Empty () const
            {
            std::lock_guard<std::mutex> lk (m_mutex);
            return ProtectedEmpty ();
            }

        bool ProtectedEmpty () const
            {
            return m_queue.empty ();
            }

        inline int Size () const
            {
            std::lock_guard<std::mutex> lk (m_mutex);
            return (int)m_queue.size ();
            }

        void WaitAndPop (D& poppedValue)
            {
            std::unique_lock<std::mutex> lk (m_mutex);
            ProtectedWaitAndPop (poppedValue);
            }

        bool TryPop (D& poppedValue)
            {
            std::lock_guard<std::mutex> lk (m_mutex);
            return ProtectedTryPop (poppedValue);
            }

        bool ProtectedTryPop (D& poppedValue)
            {
            if (m_queue.empty ())
                {
                return false;
                }

            DataContainer dataContainer = m_queue.top ();
            m_queue.pop ();

            poppedValue = dataContainer.data;

            return true;
            }

        void ProtectedWaitAndPop (D& poppedValue, std::unique_lock<std::mutex>& lk)
            {
            m_isNotEmptyCV.wait (lk, [this]
                {
                return !m_queue.empty ();
                });

            DataContainer dataContainer = m_queue.top ();
            m_queue.pop ();

            poppedValue = dataContainer.data;

            if (m_queue.empty ())
                m_isEmptyCV.notify_all ();
            }

        void WaitUntilEmpty () const
            {
            std::unique_lock<std::mutex> lk (m_mutex);
            m_isEmptyCV.wait (lk, [this]
                {
                return m_queue.empty ();
                });
            }
    };

END_BENTLEY_TASKS_NAMESPACE
