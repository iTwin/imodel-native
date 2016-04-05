/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/Bentley/Tasks/CancellationToken.h $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <functional>
#include <atomic>
#include <thread>
#include <Bentley/Tasks/Tasks.h>
#include <Bentley/bvector.h>

BEGIN_BENTLEY_TASKS_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct ICancellationListener
    {
    virtual ~ICancellationListener() {};
    virtual void OnCanceled() = 0;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct SimpleCancellationListener : ICancellationListener
    {
    private:
        std::function<void ()> m_onCanceled;

    public:
        BENTLEYDLL_EXPORT SimpleCancellationListener(std::function<void ()> onCanceled);
        BENTLEYDLL_EXPORT virtual ~SimpleCancellationListener();
        BENTLEYDLL_EXPORT virtual void OnCanceled() override;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                               Beneditas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct ICancellationToken> ICancellationTokenPtr;
struct ICancellationToken
    {
    virtual ~ICancellationToken() {};
    virtual bool IsCanceled() = 0;
    virtual void Register(std::weak_ptr<ICancellationListener> listener) = 0;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                               Beneditas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct SimpleCancellationToken> SimpleCancellationTokenPtr;
struct SimpleCancellationToken : ICancellationToken
    {
    private:
        BeAtomic<bool> m_canceled;
        bvector<std::weak_ptr<ICancellationListener>> m_listeners;

    private:
        void OnCancelled() const;

    public:
        explicit SimpleCancellationToken (bool canceled);
        virtual ~SimpleCancellationToken ();

        BENTLEYDLL_EXPORT static SimpleCancellationTokenPtr Create (bool canceled = false);

        BENTLEYDLL_EXPORT virtual bool IsCanceled() override;
        BENTLEYDLL_EXPORT virtual void SetCanceled();
        BENTLEYDLL_EXPORT virtual void Register(std::weak_ptr<ICancellationListener> listener) override;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                               Beneditas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct MergeCancellationToken> MergeCancellationTokenPtr;
struct MergeCancellationToken : ICancellationToken
    {
    private:
        bvector <ICancellationTokenPtr> m_tokens;

    public:
        MergeCancellationToken (const bvector<ICancellationTokenPtr>& tokens);
        virtual ~MergeCancellationToken ();

        BENTLEYDLL_EXPORT static MergeCancellationTokenPtr Create (const bvector<ICancellationTokenPtr>& tokens);
        BENTLEYDLL_EXPORT static MergeCancellationTokenPtr Create (ICancellationTokenPtr left, ICancellationTokenPtr right);

        BENTLEYDLL_EXPORT virtual bool IsCanceled() override;
        BENTLEYDLL_EXPORT virtual void Register (std::weak_ptr<ICancellationListener> listener) override;
    };

END_BENTLEY_TASKS_NAMESPACE
