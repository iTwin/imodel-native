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

        typedef std::priority_queue<DataContainer, bvector<DataContainer>, DataContainerCompare> DataContainerQueue;

        DataContainerQueue m_queue;

        struct EmptyQueuePredicate : IConditionVariablePredicate
            {
            private:
                DataContainerQueue const& m_queue;
                bool m_expectedEmpty;
            public:
                EmptyQueuePredicate(DataContainerQueue const& queue, bool expectedEmpty) : m_queue(queue), m_expectedEmpty(expectedEmpty) {}
                virtual bool  _TestCondition(BeConditionVariable &cv) override { return m_expectedEmpty==m_queue.empty (); }
            };

        uint64_t m_queueIndex;

        mutable BeMutex m_mutex;
        BeConditionVariable m_isEmptyCV;
        BeConditionVariable m_isNotEmptyCV;

    public:
        ConcurrentQueue () :
            m_queueIndex (0)
            {
            };

        BeMutex& GetMutex () const
            {
            return m_mutex;
            }

        void Push (const D& data)
            {
            BeMutexHolder l (m_mutex);

            DataContainer dataContainer;
            dataContainer.data = data;
            dataContainer.index = m_queueIndex++;

            m_queue.push (dataContainer);

            m_isNotEmptyCV.notify_one ();
            }

        bool Empty () const
            {
            BeMutexHolder lk (m_mutex);
            return ProtectedEmpty ();
            }

        bool ProtectedEmpty () const
            {
            return m_queue.empty ();
            }

        inline int Size () const
            {
            BeMutexHolder lk (m_mutex);
            return (int)m_queue.size ();
            }

        void WaitAndPop (D& poppedValue)
            {
            BeMutexHolder lk (m_mutex);
            ProtectedWaitAndPop (poppedValue);
            }

        bool TryPop (D& poppedValue)
            {
            BeMutexHolder lk (m_mutex);
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


        void ProtectedWaitAndPop (D& poppedValue, BeMutexHolder& lk)
            {
            EmptyQueuePredicate predicate(m_queue, false);
            m_isNotEmptyCV.ProtectedWaitOnCondition(lk, &predicate, BeConditionVariable::Infinite);

            DataContainer dataContainer = m_queue.top ();
            m_queue.pop ();

            poppedValue = dataContainer.data;

            if (m_queue.empty ())
                m_isEmptyCV.notify_all ();
            }

        void WaitUntilEmpty () const
            {
            EmptyQueuePredicate predicate(m_queue, true);
            m_isEmptyCV.WaitOnCondition(&predicate, BeConditionVariable::Infinite);
            }
    };
END_BENTLEY_TASKS_NAMESPACE
