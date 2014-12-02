/*----------------------------------------------------------------------+
|
|   $Source: PrivateApi/DgnPlatformInternal/DgnHandlers/MarkupPlacemarkHandler.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#pragma once
#include <Bentley/RefCounted.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
#ifdef DGNV10FORMAT_CHANGES_WIP

/*=================================================================================**//**
* @bsiclass                                                     Wil.Maier       09/2010
+===============+===============+===============+===============+===============+======*/
class MarkupPlacemarkHandler :  public DisplayHandler
{
DEFINE_T_SUPER(DisplayHandler);
ELEMENTHANDLER_DECLARE_MEMBERS (MarkupPlacemarkHandler, DGNPLATFORM_EXPORT)

private:
    void    GetProjectedPoint (DPoint3dR inPoint, ViewContextP pViewContext) const;
    void    DrawPickGeometry (DPoint3dCR spritePt, ViewContextP pViewContext) const;

protected:
    virtual void                    _GetTypeName (WStringR, UInt32 desiredLength) override;
    virtual void                    _GetDescription (ElementHandleCR, WStringR, UInt32 desiredLength) override;
    virtual void                    _Draw (ElementHandleCR el, ViewContextP) override;
    virtual void                    _DrawFiltered (ElementHandleCR elemHandle, ViewContextP pViewContext, DPoint3dCP pPoints, double size) override;
    virtual bool                    _IsRenderable (ElementHandleCR) const override {return true;}
    virtual BentleyStatus           _ValidateElementRange (EditElementHandleR elHandle, bool setZ) override {return SUCCESS;}

    void                            DrawPlacemark(ElementHandleCR elemHandle, ViewContextP pViewContext);

public:
    DGNPLATFORM_EXPORT static ElementHandlerId  GetHandlerId ();

}; // MarkupPlacemarkHandler
#endif

END_BENTLEY_DGNPLATFORM_NAMESPACE