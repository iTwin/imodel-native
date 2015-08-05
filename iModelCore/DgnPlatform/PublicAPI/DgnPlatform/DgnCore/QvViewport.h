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

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
// @bsiclass                                                      KeithBentley    10/02
//=======================================================================================
struct QvViewport : public DgnViewport
{
//__PUBLISH_SECTION_END__
    DEFINE_T_SUPER(DgnViewport)
protected:
    ICachedDrawP    m_cachedOutput;

    DGNVIEW_EXPORT void DestroyQvViewport();
    DGNVIEW_EXPORT virtual void _AllocateOutput() override;
                   virtual void _Destroy() override { DestroyQvViewport(); T_Super::_Destroy(); }
                   virtual ICachedDrawP _GetICachedDraw() override {return m_cachedOutput;}
    DGNVIEW_EXPORT virtual BentleyStatus _RefreshViewport(bool always, bool synchHealingFromBs, bool& stopFlag) override;
    DGNVIEW_EXPORT virtual void _AdjustZPlanesToModel(DPoint3dR origin, DVec3dR delta, ViewControllerCR) const override;
                   virtual DgnDisplayCoreTypes::WindowP _GetWindowHandle() const = 0;
                   virtual void _SetICachedDraw(ICachedDrawP cachedOutput) override {m_cachedOutput = cachedOutput;}
    DGNVIEW_EXPORT void Initialize();

public:
    DGNVIEW_EXPORT QvViewport();
    virtual ~QvViewport() {DestroyQvViewport();}

    DGNVIEW_EXPORT void Resized();
    DGNVIEW_EXPORT DgnDisplayCoreTypes::WindowP GetWindowHandle() const;
    DGNVIEW_EXPORT static bool IsTextureIdDefined(uintptr_t textureId);
    DGNVIEW_EXPORT static void DefineTextureId(uintptr_t textureId, Point2dCR imageSize, bool enableAlpha, uint32_t imageFormat, Byte const* imageData);
    DGNVIEW_EXPORT static void DeleteTextureId(uintptr_t textureId);

    DGNVIEW_EXPORT static void DefineTile(uintptr_t textureId, char const* tileName, Point2dCR imageSize, bool enableAlpha, uint32_t imageFormat, uint32_t pitch, Byte const* imageData);
    DGNVIEW_EXPORT static void AddMosaic(QvElem* qvElem, int numX, int numY, uintptr_t* tileIds, DPoint3d const* verts);

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
};

END_BENTLEY_DGNPLATFORM_NAMESPACE

//__PUBLISH_SECTION_END__
