/*----------------------------------------------------------------------+
|
|   $Source: DgnHandlers/RenderHandlers.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

// struct AreaLightData : public ElementRefAppData removed in graphite
// ELEMENTHANDLER_DEFINE_MEMBERS (AreaLightHandler) removed in graphite 
//ELEMENTHANDLER_DEFINE_MEMBERS (PointLightHandler) removed in graphite
//ELEMENTHANDLER_DEFINE_MEMBERS (DistantLightHandler) removed in graphite
//ELEMENTHANDLER_DEFINE_MEMBERS (SpotLightHandler) removed in graphite
//ELEMENTHANDLER_DEFINE_MEMBERS (SolarTimeHandler) removed in graphite
/*=================================================================================**//**
* @bsiclass                                                     PaulChater  03/11
+===============+===============+===============+===============+===============+======*/
struct RPCStroker : IStrokeForCache
    {

    virtual DrawExpense _GetDrawExpense () override                     { return DrawExpense::High; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   PaulChater  03/11
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _StrokeForCache (CachedDrawHandleCR dh, ViewContextR context, double pixelSize) override
    {
    BeAssert(NULL != dh.GetElementHandleCP());
    if (NULL == dh.GetElementHandleCP())
        return;
    DgnPlatformLib::GetHost().GetGraphicsAdmin()._DrawRPC (*dh.GetElementHandleCP(), context);
    }

    }; // RPCStroker

ELEMENTHANDLER_DEFINE_MEMBERS (RPCHandler)

//---------------------------------------------------------------------------------------
// @bsimethod                                                   PaulChater      11/11
//---------------------------------------------------------------------------------------
void RPCHandler::InvalidParamHandler (WCharCP expression, WCharCP function, WCharCP file, UInt32 line, uintptr_t reserved)
    {
    // NEEDS WORK - report error
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   PaulChater      11/11
//---------------------------------------------------------------------------------------
void RPCHandler::_GetTypeName (WStringR descr, UInt32 length)
    {
    descr.assign (DgnHandlersMessage::GetStringW(DgnHandlersMessage::IDS_SubType_RPC));
    }

//StatusInt  RPCHandler::_OnPreprocessCopy (EditElementHandleR eh, ElementCopyContextP context) removed in graphite

//---------------------------------------------------------------------------------------
// @bsimethod                                                   PaulChater      11/11
//---------------------------------------------------------------------------------------
void            RPCHandler::_GetElemDisplayParams (ElementHandleCR elHandle, ElemDisplayParams& params, bool wantMaterials)
    {
    ElementHandle  templateElHandle;

    if (GetComponentForDisplayParams (templateElHandle, elHandle) && CELL_HEADER_ELM != templateElHandle.GetLegacyType())
        {
        templateElHandle.GetDisplayHandler()->GetElemDisplayParams (templateElHandle, params, wantMaterials);

#ifdef WIP_VANCOUVER_MERGE // material
        if (wantMaterials && NULL == params.GetMaterial ()) // look for attachment to top level element if none found yet
            params.SetMaterial (MaterialManager::GetManagerR ().FindMaterialAttachment (NULL, elHandle, *elHandle.GetDgnModel (), false), true);
#endif

        params.SetIsRenderable (true);
        return;
        }

    T_Super::_GetElemDisplayParams (elHandle, params, wantMaterials);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   PaulChater      11/11
//---------------------------------------------------------------------------------------
void RPCHandler::_Draw (ElementHandleCR eh, ViewContextR context)
    {
    ViewFlagsCP viewFlags = context.GetViewFlags();
    
    if (DgnPlatformLib::GetHost().GetGraphicsAdmin()._WantRPCDisplay () && viewFlags &&
        viewFlags->renderMode >= static_cast<UInt32>(MSRenderMode::HiddenLine) && viewFlags->textureMaps &&
        ! context.IsMonochromeDisplayStyleActive ())
        {
        if (DgnPlatformLib::GetHost().GetGraphicsAdmin()._IsSmartContentRPC (eh))
            context.DrawCached (eh, RPCStroker (), 0); 
        else if (SUCCESS != DgnPlatformLib::GetHost().GetGraphicsAdmin()._DrawRPC (eh, context))
            T_Super::_Draw (eh, context);
        }
    else
        T_Super::_Draw (eh, context);
    }


ELEMENTHANDLER_DEFINE_MEMBERS (RenderCellHandler)

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     07/10
//---------------------------------------------------------------------------------------
void RenderCellHandler::_GetTypeName (WStringR descr, UInt32 desiredLength)
    {
    descr.assign (DgnHandlersMessage::GetStringW(DgnHandlersMessage::IDS_TYPENAMES_RENDER_CELL));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     07/10
//---------------------------------------------------------------------------------------
bool RenderCellHandler::_IsSupportedOperation (ElementHandleCP eh, SupportOperation stype)
    {
    return false;
    }

bool RPCHandler::_ClaimElement (ElementHandleCR eh)
    {
    WChar         cellName[MAX_CELLNAME_LENGTH];
    DgnElementCP     elemCP = eh.GetElementCP ();

    if (NULL == elemCP || CELL_HEADER_ELM != elemCP->GetLegacyType() || SUCCESS != CellUtil::GetCellName (cellName, MAX_CELLNAME_LENGTH, *elemCP))
        return false;

    return RPCHandler::MatchesCellNameKey (cellName);
    }
