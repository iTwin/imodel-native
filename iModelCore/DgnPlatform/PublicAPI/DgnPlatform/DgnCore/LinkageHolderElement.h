/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/LinkageHolderElement.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__BENTLEY_INTERNAL_ONLY__*/
#include "../DgnPlatform.h"
#include "ElementHandle.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//! @bsiinterface                                 Sam.Wilson                      09/2009
//=======================================================================================
struct LinkageHolderElement
{
    MSElementDescrPtr m_descr;

    void Clear () {m_descr=NULL;}
    DGNPLATFORM_EXPORT void Copy (LinkageHolderElement const&);
    LinkageHolderElement (){}
    LinkageHolderElement(LinkageHolderElement const& rhs) {Copy(rhs);}
    ~LinkageHolderElement () {Clear();}
    DGNPLATFORM_EXPORT LinkageHolderElement& operator=(LinkageHolderElement const&);
    DGNPLATFORM_EXPORT MSElementDescrCP GetDescrCP () const {return m_descr.get();}
    DGNPLATFORM_EXPORT MSElementDescrPtr& GetDescrH(DgnModelP model);
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
