/*--------------------------------------------------------------------------------------+
|
|     $Source: PrivateApi/DgnPlatformInternal/DgnHandlers/RenderHandlers.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
// __BENTLEY_INTERNAL_ONLY

#include    <DgnPlatform/DgnPlatform.h>

DGNPLATFORM_TYPEDEFS (RenderCellHandler)

BEGIN_BENTLEY_DGN_NAMESPACE

#if defined (NEEDS_WORK_DGNITEM)
//=======================================================================================
// @bsiclass                                                    MattGooding     07/10
//=======================================================================================
struct RenderCellHandler :  public NormalCellHeaderHandler
{
    DEFINE_T_SUPER(NormalCellHeaderHandler)
    ELEMENTHANDLER_DECLARE_MEMBERS (RenderCellHandler, DGNPLATFORM_EXPORT)

private:
    virtual void _GetTypeName (WStringR descr, uint32_t desiredLength) override;
    virtual bool _IsSupportedOperation (ElementHandleCP eh, SupportOperation stype) override;

public:
    static bool FindLightGeometry (ElementHandleR geometry, ElementHandleCR lightCell);
};


//=======================================================================================
// @bsiclass                                                    PeteSegal       05/05
//=======================================================================================
struct SolarTimeHandler :  public NormalCellHeaderHandler, ISubTypeHandlerQuery
{
    DEFINE_T_SUPER(NormalCellHeaderHandler)
    ELEMENTHANDLER_DECLARE_MEMBERS (SolarTimeHandler,)

private:
    virtual void _GetTypeName (WStringR descr, uint32_t length) override;
    virtual void _Draw (ElementHandleCR eh, ViewContextR context) override;
    virtual bool _ClaimElement (ElementHandleCR) override;

public:
    static bool MatchesCellNameKey (WCharCP cellName) { return 0 == wcscmp (cellName, L"SLRTIM"); }
    static void InvalidParamHandler (WCharCP expression, WCharCP function, WCharCP file, uint32_t line, uintptr_t reserved);
};

//=======================================================================================
// @bsiclass                                                    PeteSegal       05/05
//=======================================================================================
struct RPCHandler :  public NormalCellHeaderHandler, ISubTypeHandlerQuery
{
    DEFINE_T_SUPER(NormalCellHeaderHandler)
    ELEMENTHANDLER_DECLARE_MEMBERS (RPCHandler,)

private:
    virtual void _GetTypeName (WStringR descr, uint32_t length) override;
    virtual void _Draw (ElementHandleCR eh, ViewContextR context) override;
    virtual void                            _GetElemDisplayParams (ElementHandleCR elHandle, ElemDisplayParams& params, bool wantMaterials) override;
    virtual bool _ClaimElement (ElementHandleCR) override;

public:
    static bool MatchesCellNameKey (WCharCP cellName) { return 0 == wcscmp (cellName, L"RPC") || 0 == wcsncmp (cellName, L"RPCPROXY", 8); }
    static void InvalidParamHandler (WCharCP expression, WCharCP function, WCharCP file, uint32_t line, uintptr_t reserved);
};
#endif

END_BENTLEY_DGN_NAMESPACE
