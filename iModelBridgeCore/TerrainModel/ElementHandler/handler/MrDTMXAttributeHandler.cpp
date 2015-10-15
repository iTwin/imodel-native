/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/handler/MrDTMXAttributeHandler.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "StdAfx.h"

// It is important that this is the first file included
#include <TerrainModel/ElementHandler/TerrainModelElementHandler.h>
//#include "material.fdf"
//#include "material.h"
//#include "msoutput.fdf"
#include <io.h>
#include "time.h"

#include "DTMBinaryData.h"
#include <TerrainModel/ElementHandler/DTMReferenceXAttributeHandler.h>
#include "MrDTMDataRef.h"
#include <ScalableTerrainModel/MrDTMUtilityFunctions.h>

using namespace Bentley::GeoCoordinates;

/*----------------------------------------------------------------------+
| Include Rasterlib header files                                        |
+----------------------------------------------------------------------*/
BEGIN_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE

MrDTMXAttributeHandler* MrDTMXAttributeHandler::s_mrDtmXAttributeHandler = 0;

//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 03/11
//=======================================================================================
MrDTMXAttributeHandler::MrDTMXAttributeHandler()
    {
    }

//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 03/11
//=======================================================================================
MrDTMXAttributeHandler::~MrDTMXAttributeHandler()
    {
    }

//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 03/11
//=======================================================================================
MrDTMXAttributeHandler* MrDTMXAttributeHandler::GetInstance()
    {
    if (s_mrDtmXAttributeHandler == 0)
        {
        s_mrDtmXAttributeHandler = new MrDTMXAttributeHandler;
        }

    return s_mrDtmXAttributeHandler;
    }

//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 03/11
//=======================================================================================
int MrDTMXAttributeHandler::RegisterHandlers()
    {
    XAttributeHandlerManager::RegisterHandler(XAttributeHandlerId(MrDTMElementMajorId, XATTRIBUTES_SUBID_MRDTM_DETAILS), this);    

    return 0;
    }

//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 03/11
//=======================================================================================
IXAttributeTransactionHandler* MrDTMXAttributeHandler::_GetIXAttributeTransactionHandler() 
    {
    return this;
    }

//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 03/11
//=======================================================================================
void MrDTMXAttributeHandler::_OnPreReplaceData(XAttributeHandleCR xAttr, void const* newData, uint32_t newSize, TransactionType type)
    {
    bool needRedraw = false;
    
    if ((type == TRANSACTIONTYPE_Reverse) || (type == TRANSACTIONTYPE_Reinstate))
        {                        
        //Open using any modelRef we found
        DgnModelRefP modelRef = xAttr.GetElementRef()->GetDgnModelP();
        
        if (NULL == modelRef)
            return;      //A valid modelRef is needed

        ElementHandle elemHandle(xAttr.GetElementRef(), modelRef);

        RefCountedPtr<DTMDataRef> dtmDataRef;
        DTMElementHandlerManager::GetDTMDataRef(dtmDataRef, elemHandle);

        if (0 == dtmDataRef.get())
            return;

        BeAssert(dtmDataRef->IsMrDTM() == true);

        RefCountedPtr<MrDTMDataRef> mrDtmDataRef((MrDTMDataRef*)dtmDataRef.get());       

        switch (xAttr.GetId())
            {
            case MRDTM_DETAILS_VISIBILITY_PER_VIEW : 
                BeAssert(newSize == sizeof(mrDtmDataRef->m_visibilityPerView));
                memcpy(&mrDtmDataRef->m_visibilityPerView, (byte*)newData, sizeof(mrDtmDataRef->m_visibilityPerView));                
                needRedraw = true;
                break;

            case MRDTM_DETAILS_DESCRIPTION : 
                {
                DataInternalizer source ((byte*)newData, newSize);
                source.get(mrDtmDataRef->m_description);
                }
                break;

            case MRDTM_DETAILS_CAN_LOCATE : 
                BeAssert(newSize == sizeof(mrDtmDataRef->m_canLocate));
                memcpy(&mrDtmDataRef->m_canLocate, (byte*)newData, sizeof(mrDtmDataRef->m_canLocate));     
                break;

            case MRDTM_DETAILS_IS_ANCHORED : 
                BeAssert(newSize == sizeof(mrDtmDataRef->m_isAnchored));
                memcpy(&mrDtmDataRef->m_isAnchored, (byte*)newData, sizeof(mrDtmDataRef->m_isAnchored));     
                break;
         
            case MRDTM_DETAILS_IS_READ_ONLY :                 
                BeAssert(newSize == sizeof(mrDtmDataRef->m_isReadOnly));
                memcpy(&mrDtmDataRef->m_isReadOnly, (byte*)newData, sizeof(mrDtmDataRef->m_isReadOnly));                                                   
                mrDtmDataRef->ReloadMrDTM(mrDtmDataRef->GetDestinationGCS(), false);                        
                break;

            case MRDTM_DETAILS_CLIPS : 
                {                                
                int status = mrDtmDataRef->ReadClipsFromSerializationData((byte*)newData, newSize);                
                BeAssert(status == 0);
                needRedraw = true;
                }
                break;

            case MRDTM_DETAILS_FILE_MONIKER :             
                {
                int status = mrDtmDataRef->ReadMonikerFromSerializationData((byte*)newData, newSize, modelRef);
                BeAssert(status == 0);
                mrDtmDataRef->ReloadMrDTM(mrDtmDataRef->GetDestinationGCS(), false);    
                }
                break;

            case MRDTM_DETAILS_POINT_DENSITY : 
                {
                DataInternalizer source((byte*)newData, newSize);       
                source.get(&mrDtmDataRef->m_pointDensityForShadedView);        
                source.get(&mrDtmDataRef->m_pointDensityForWireframeView);   
                needRedraw = true;
                }
                break;                                                                             

            case MRDTM_DETAILS_TRIANGULATION_EDGE_OPTIONS : 
                {
                DataInternalizer source((byte*)newData, newSize);       
                source.get(&mrDtmDataRef->m_edgeMethod);        
                source.get(&mrDtmDataRef->m_edgeMethodLength);   
                needRedraw = true;
                }
                break; 
            
            case MRDTM_DETAILS_MIN_POINT :
                BeAssert(newSize == sizeof(mrDtmDataRef->m_minPoint));
                memcpy(&mrDtmDataRef->m_minPoint, (byte*)newData, sizeof(mrDtmDataRef->m_minPoint));
                break;

            case MRDTM_DETAILS_MAX_POINT :
                BeAssert(newSize == sizeof(mrDtmDataRef->m_maxPoint));
                memcpy(&mrDtmDataRef->m_maxPoint, (byte*)newData, sizeof(mrDtmDataRef->m_maxPoint));
                break;

            default :                 
                BeAssert(!"Unknown XAttribute SubID");
                break;

            }  

        if (needRedraw == true)
            {
            mrDtmDataRef->FlushAllViewData();        
            }    
        }    
    }

END_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE
