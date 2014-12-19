//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCStack.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:> Class : HFCStack
//:>---------------------------------------------------------------------------
#pragma once

#include "HFCPtr.h"

class HFCStackItem;

class HFCStack : public HFCShareableObject<HFCStack>
    {
public:
    HDECLARE_BASECLASS_ID (1369);

    _HDLLu                              HFCStack();
    _HDLLu virtual                      ~HFCStack();

    virtual bool                       IsEmpty     () const;
    virtual void                        Push        (const HFCPtr<HFCStackItem>&  pi_rpItem);
    virtual HFCPtr<HFCStackItem>        Pop         ();
    virtual const HFCPtr<HFCStackItem>& Top         () const;
    virtual size_t                      Size        () const;

protected:

private:

    typedef list<HFCPtr<HFCStackItem> >     StackList;
    StackList               m_Stack;

    // disabled methods
    HFCStack(const HFCStack&);
    HFCStack& operator=(const HFCStack&);
    };


