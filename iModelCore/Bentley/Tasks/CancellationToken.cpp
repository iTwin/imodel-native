/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Tasks/CancellationToken.cpp $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include <Bentley/Tasks/CancellationToken.h>

USING_NAMESPACE_BENTLEY_TASKS

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
SimpleCancellationListener::SimpleCancellationListener (std::function<void ()> onCanceled) : m_onCanceled (onCanceled) {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
SimpleCancellationListener::~SimpleCancellationListener () {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void SimpleCancellationListener::OnCanceled ()
    {
    if (m_onCanceled)
        m_onCanceled ();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
SimpleCancellationToken::SimpleCancellationToken (bool canceled) : m_canceled (canceled) {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
SimpleCancellationToken::~SimpleCancellationToken () {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
SimpleCancellationTokenPtr SimpleCancellationToken::Create (bool canceled)
    {
    return std::make_shared<SimpleCancellationToken> (canceled);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool SimpleCancellationToken::IsCanceled ()
    {
    return m_canceled;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void SimpleCancellationToken::SetCanceled ()
    {
    if (!m_canceled)
        OnCancelled ();

    m_canceled.store(true);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void SimpleCancellationToken::Register (std::weak_ptr<ICancellationListener> listener)
    {
    if (m_canceled)
        {
        auto listenerPtr = listener.lock ();
        if (listenerPtr)
            listenerPtr->OnCanceled ();
        }

    m_listeners.push_back (listener);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void SimpleCancellationToken::OnCancelled () const
    {
    for (auto& listenerWearkPtr : m_listeners)
        {
        auto listenerPtr = listenerWearkPtr.lock ();
        if (listenerPtr)
            listenerPtr->OnCanceled ();
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
MergeCancellationToken::MergeCancellationToken (const bvector<ICancellationTokenPtr>& tokens) : m_tokens (tokens) {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
MergeCancellationToken::~MergeCancellationToken () {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
MergeCancellationTokenPtr MergeCancellationToken::Create (const bvector<ICancellationTokenPtr>& tokens)
    {
    return std::make_shared<MergeCancellationToken> (tokens);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
MergeCancellationTokenPtr MergeCancellationToken::Create (ICancellationTokenPtr left, ICancellationTokenPtr right)
    {
    bvector<ICancellationTokenPtr> tokens;
    tokens.push_back (left);
    tokens.push_back (right);
    return std::make_shared<MergeCancellationToken> (tokens);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool MergeCancellationToken::IsCanceled ()
    {
    for (auto& token : m_tokens)
        {
        if (token && token->IsCanceled ())
            return true;
        }

    return false;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void MergeCancellationToken::Register (std::weak_ptr<ICancellationListener> listener)
    {
    for (auto& token : m_tokens)
        {
        if (token)
            token->Register (listener);
        }
    }