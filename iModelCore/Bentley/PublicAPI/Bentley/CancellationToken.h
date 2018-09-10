/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/Bentley/CancellationToken.h $
 |
 |  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <functional>
#include <atomic>

#if !defined (BENTLEY_CONFIG_NO_THREAD_SUPPORT)
#include <thread>
#endif

#include <Bentley/bvector.h>

BEGIN_BENTLEY_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct ICancellationListener
    {
    virtual ~ICancellationListener() {};
    //! Called when cancellation event occurs
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
        //! Create cancellation listener that will execute provided std::function
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
    //! Check if token is cancelled
    virtual bool IsCanceled() = 0;
    //! Register custom listener for cancellation event
    virtual void Register(std::weak_ptr<ICancellationListener> listener) = 0;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                               Beneditas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct SimpleCancellationToken> SimpleCancellationTokenPtr;
struct EXPORT_VTABLE_ATTRIBUTE SimpleCancellationToken : ICancellationToken
    {
    private:
        BeAtomic<bool> m_canceled;
        bvector<std::weak_ptr<ICancellationListener>> m_listeners;

    private:
        void OnCancelled() const;

    public:
        explicit SimpleCancellationToken(bool canceled) : m_canceled(canceled) {};
        virtual ~SimpleCancellationToken() {};

        //! Create cancellation token that can be cancelled later
        BENTLEYDLL_EXPORT static SimpleCancellationTokenPtr Create(bool canceled = false);

        //! Check if token is cancelled
        BENTLEYDLL_EXPORT virtual bool IsCanceled() override;
        //! Set token as cancelled
        BENTLEYDLL_EXPORT virtual void SetCanceled();
        //! Register custom listener for cancellation event
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
        //! Cancellation token that allows merging multiple tokens into one and using it for cancellation checks
        MergeCancellationToken(const bvector<ICancellationTokenPtr>& tokens);
        virtual ~MergeCancellationToken();

        //! Create token that will be cancelled when any of supplied tokens gets cancelled
        BENTLEYDLL_EXPORT static MergeCancellationTokenPtr Create(const bvector<ICancellationTokenPtr>& tokens);
        //! Create token that will be cancelled when any of supplied tokens gets cancelled
        BENTLEYDLL_EXPORT static MergeCancellationTokenPtr Create (ICancellationTokenPtr ct1, ICancellationTokenPtr ct2);
        
        //! Check if token is cancelled
        BENTLEYDLL_EXPORT virtual bool IsCanceled() override;
        //! Register custom listener for cancellation event
        BENTLEYDLL_EXPORT virtual void Register(std::weak_ptr<ICancellationListener> listener) override;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                               Petras.Sukys          08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct ConjunctiveCancellationToken> ConjunctiveCancellationTokenPtr;
struct ConjunctiveCancellationToken : ICancellationToken
    {
    private:
        bvector <ICancellationTokenPtr> m_tokens;
        bvector<std::weak_ptr<ICancellationListener>> m_listeners;

    private:
        void InternalOnCancelled();

    public:
        //! Cancellation token that combines multiple tokens in a conjunctive logic.
        ConjunctiveCancellationToken(const bvector<ICancellationTokenPtr>& tokens);
        virtual ~ConjunctiveCancellationToken();

        //! Create token that will be cancelled if and only if all of the supplied tokens gets cancelled
        BENTLEYDLL_EXPORT static ConjunctiveCancellationTokenPtr Create(const bvector<ICancellationTokenPtr>& tokens);
        //! Create token that will be cancelled if and only if all of the supplied tokens gets cancelled
        BENTLEYDLL_EXPORT static ConjunctiveCancellationTokenPtr Create(ICancellationTokenPtr ct1, ICancellationTokenPtr ct2);

        //! Check if token is cancelled. Returs true if and only if all the underlying tokens are cancelled.
        BENTLEYDLL_EXPORT virtual bool IsCanceled() override;
        //! Register custom listener for cancellation event
        BENTLEYDLL_EXPORT virtual void Register(std::weak_ptr<ICancellationListener> listener) override;
    };

END_BENTLEY_NAMESPACE
