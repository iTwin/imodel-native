/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/QvViewport.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnViewport.h"

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
// @bsiclass                                                      KeithBentley    10/02
//=======================================================================================
struct QvViewport : DgnViewport
{
    DEFINE_T_SUPER(DgnViewport)
protected:
    Render::CachedDrawP m_cachedOutput;

    DGNVIEW_EXPORT void DestroyQvViewport();
    DGNVIEW_EXPORT virtual void _AllocateOutput() override;
                   virtual void _Destroy() override { DestroyQvViewport(); T_Super::_Destroy(); }
                   virtual Render::CachedDrawP _GetICachedDraw() override {return m_cachedOutput;}
    DGNVIEW_EXPORT virtual BentleyStatus _RefreshViewport(bool always, bool synchHealingFromBs, bool& stopFlag) override;
    DGNVIEW_EXPORT virtual void _AdjustZPlanesToModel(DPoint3dR origin, DVec3dR delta, ViewControllerCR) const override;
                   virtual DgnDisplayCoreTypes::WindowP _GetWindowHandle() const = 0;
                   virtual void _SetICachedDraw(Render::CachedDrawP cachedOutput) override {m_cachedOutput = cachedOutput;}
    DGNVIEW_EXPORT void Initialize();
    DGNVIEW_EXPORT virtual Render::Renderer& _GetRenderer() const override;

public:
    DGNVIEW_EXPORT QvViewport();
    virtual ~QvViewport() {DestroyQvViewport();}

    DGNVIEW_EXPORT void Resized();
    DgnDisplayCoreTypes::WindowP GetWindowHandle() const {return _GetWindowHandle();}
    DGNVIEW_EXPORT static void AddMosaic(Render::Graphic* qvElem, int numX, int numY, uintptr_t* tileIds, DPoint3d const* verts);
};

END_BENTLEY_DGN_NAMESPACE
