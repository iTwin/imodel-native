/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentation/auievent.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

BEGIN_BENTLEY_EC_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECEvent
    {
    private:
        void*   m_source;

    public:
        ECEvent ()
            :m_source (NULL)
            {}

        ECEvent (void* source)
            :m_source(source)
            {}

        void*   GetSource () {return m_source;}
        void    SetSource (void* source) {m_source = source;}
    };
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECSelectionEvent: public ECEvent
    {
    ECSelectionEvent (void* src)
        :ECEvent(src)
        {}
    };


END_BENTLEY_EC_NAMESPACE