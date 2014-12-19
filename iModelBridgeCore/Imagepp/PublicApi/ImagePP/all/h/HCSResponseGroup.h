//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCSResponseGroup.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCSResponse
//-----------------------------------------------------------------------------

#pragma once

#include "HCSResponse.h"

/** -----------------------------------------------------------------------------
    @version 1.0
    @author Sebastien Tardif

    This class implements collection of HCSResponse objects to be sent to clients.
    -----------------------------------------------------------------------------
*/
class HCSResponseGroup
    {
public:
    //--------------------------------------
    // Constructor and destructor
    //--------------------------------------

    HCSResponseGroup();
    ~HCSResponseGroup();


    //--------------------------------------
    // Basic Methods
    //--------------------------------------

    // Insert a new response
    void                Insert(HCSResponse* pi_pResponse);

    // Element counting
    uint32_t            CountElements() const;


    //--------------------------------------
    // Response Iterator
    //--------------------------------------

    typedef void*       IteratorHandle;
    IteratorHandle      StartIteration() const;
    const HCSResponse*  Iterate(IteratorHandle pi_Handle) const;
    const HCSResponse*  GetElement(IteratorHandle pi_Handle) const;
    void                StopIteration(IteratorHandle pi_Handle) const;


private:
    //--------------------------------------
    // Attributes
    //--------------------------------------

    // List that will hold all the Response
    typedef list<HCSResponse*, allocator<HCSResponse*> >
    ResponseList;
    ResponseList        m_ResponseList;

    // An iterator structure on the list.
    struct LocalIterator
        {
        ResponseList*          m_pList;
        ResponseList::iterator m_Itr;
        LocalIterator(ResponseList* pi_pList)
            : m_pList(pi_pList), m_Itr(pi_pList->begin()) { }
        };

    //--------------------------------------
    // Deactivated Methods
    //--------------------------------------

    HCSResponseGroup(const HCSResponseGroup&);
    HCSResponseGroup&   operator=(const HCSResponseGroup&);
    };

#include "HCSResponseGroup.hpp"

