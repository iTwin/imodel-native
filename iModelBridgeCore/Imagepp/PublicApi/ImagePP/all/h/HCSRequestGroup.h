//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCSRequestGroup.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HCSRequestGroup
//-----------------------------------------------------------------------------

#pragma once

class HCSRequest;

/** -----------------------------------------------------------------------------
    @version 1.0
    @author Sebastien Tardif

    The HCSRequestGroup class is used to decompose and hold single requests
    from a concatenated request string from a client connection.

    Some protocols can support multiple requests in one single request string
    up to an end marker.  The requests are separated in the string by yet
    another marker (the request separator).  The request objects must know
    the group to which they belong because some requests may have a different
    behavior when another specific request is grouped with them.

    This becomes clear with the Internet Imaging Protocol where the "FIF"
    request is grouped with the "OBJ=iip,1.0" request, the resulting response
    is that of the "OBJ=basic-info" request.  When the "FIF" is alone, it
    produces no response.
    -----------------------------------------------------------------------------
*/
class HCSRequestGroup
    {
public:
    //--------------------------------------
    // Constructor and destructor
    //--------------------------------------

    HCSRequestGroup(const WString& pi_rRequestSeparator,
                    const WString& pi_rValueSeparator);
    ~HCSRequestGroup();


    //--------------------------------------
    // Basic Methods
    //--------------------------------------

    // Insert a new request
    void        Insert(const WString& pi_rRequest);
    void        Insert(HCSRequest* pi_pRequest);

    // Element count
    uint32_t    CountElements() const;


    //--------------------------------------
    // Request Iterator
    //--------------------------------------
    typedef void*       IteratorHandle;
    IteratorHandle      StartIteration() const;
    const HCSRequest*   Iterate(IteratorHandle pi_Handle) const;
    const HCSRequest*   GetElement(IteratorHandle pi_Handle) const;
    void                StopIteration(IteratorHandle pi_Handle) const;


private:
    //--------------------------------------
    // Private Members
    //--------------------------------------

    // The request separator is used to separate single
    // requests from each other in a batched request string
    WString             m_RequestSeparator;

    // The value separator is used to separate the key-value
    // pair from a single request string.
    WString             m_ValueSeparator;


    // List that will hold all the requests
    typedef list<HCSRequest*, allocator<HCSRequest*> >
    RequestList;
    RequestList         m_RequestList;

    // An iterator structure on the list.
    struct LocalIterator
        {
        RequestList*          m_pList;
        RequestList::iterator m_Itr;
        LocalIterator(RequestList* pi_pList)
            : m_pList(pi_pList), m_Itr(pi_pList->begin()) { }
        };

    // deactivated
    HCSRequestGroup(const HCSRequestGroup&);
    HCSRequestGroup&    operator=(const HCSRequestGroup&);
    };

#include "HCSRequestGroup.hpp"

