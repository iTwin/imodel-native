/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/handler/MrDTMXAttributeHandler.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <DgnPlatform\DgnDocumentManager.h>

#include <DgnPlatform\XAttributeHandler.h>

#include "MrDTMDataRef.h"

BEGIN_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE

USING_NAMESPACE_BENTLEY_MRDTM

class MrDTMXAttributeHandler : public Element::XAttributeHandler, 
                               public Element::IXAttributeTransactionHandler                      
    {    

private:    
    
    static MrDTMXAttributeHandler* s_mrDtmXAttributeHandler;
    
    //XAttribute information
    XAttributeHandlerId m_xAttrMrDTMDetailsHandlerId;

    MrDTMXAttributeHandler();

    virtual ~MrDTMXAttributeHandler();

protected:
    
    //Inherited from IXAttributeTransactionHandler    
    virtual IXAttributeTransactionHandler* _GetIXAttributeTransactionHandler();        
              
public:    

    static MrDTMXAttributeHandler* GetInstance();       

    int RegisterHandlers();
                  
    //Inherited from IXAttributeTransactionHandler
    virtual void    _OnPreReplaceData (XAttributeHandleCR xAttr, void const* newData, UInt32 newSize, TransactionType type);
    };

END_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE
