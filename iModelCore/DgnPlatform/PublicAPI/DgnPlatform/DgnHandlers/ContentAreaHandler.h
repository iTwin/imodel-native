/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/ContentAreaHandler.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__BENTLEY_INTERNAL_ONLY__*/

#include <DgnPlatform/DgnCore/DisplayHandler.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
#if defined (NEEDS_WORK_DGNITEM)

/*=================================================================================**//**
* @bsiclass                                                     Mark.Dane       07/2010
+===============+===============+===============+===============+===============+======*/
struct   ContentAreaHandler :   DisplayHandler
//, ITransactionHandler removed in Graphite
    {
DEFINE_T_SUPER(DisplayHandler)
ELEMENTHANDLER_DECLARE_MEMBERS (ContentAreaHandler, DGNPLATFORM_EXPORT)

private:
    enum QVCacheIndex
        {
        QV_CACHE_INDEX_Display,
        QV_CACHE_INDEX_Pick
        };

    bool IsValidTransform (TransformInfoCR transform) const;

    static DgnPlatform::ElementHandlerId GetHandlerId ();

protected:
    // DisplayHandler
    virtual void        _GetTypeName (WStringR, UInt32 desiredLength) override;
    virtual void        _GetDescription (ElementHandleCR, WStringR, UInt32 desiredLength) override;
    virtual void        _Draw (ElementHandleCR el, ViewContextR) override;
   // virtual bool        _IsClosedCurve (ElementHandleCR) const override {return true;}

    virtual StatusInt   _OnTransform (EditElementHandleR, TransformInfoCR) override;
    virtual bool        _IsTransformGraphics (ElementHandleCR element, TransformInfoCR transform) override;

    // ITransactionHandler
    // virtual ITransactionHandlerP    _GetITransactionHandler () override {return this;}                                                        removed in Graphite
    // virtual Bentley::DgnPlatform::ITransactionHandler::PreActionStatus _OnAdd (EditElementHandleR) override;                                  removed in Graphite
    // virtual Bentley::DgnPlatform::ITransactionHandler::PreActionStatus _OnReplace (EditElementHandleR, ElementHandleCR) override;             removed in Graphite
    // virtual Bentley::DgnPlatform::ITransactionHandler::PreActionStatus _OnDelete (ElementHandleCR) override {return PRE_ACTION_Block;}        removed in Graphite

public:
    // registration (Internal)
    static void         Register ();

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Mark.Dane                       08/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    DGNPLATFORM_EXPORT static void CreateContentArea (EditElementHandleR, DRange2dCR, DgnModelP model);

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Mark.Dane                       08/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    DGNPLATFORM_EXPORT static bool IsContentArea (ElementHandleCR);

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Mark.Dane                       08/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    DGNPLATFORM_EXPORT static DRange2d GetArea (ElementHandleCR);

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Mark.Dane                       08/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    DGNPLATFORM_EXPORT static BentleyStatus SetArea (EditElementHandleR, DRange2dCR);

    /*---------------------------------------------------------------------------------**//**
    * The version of the element, provided for compatiblity with future versions.
    * Version 0 - unreleased version that used scan range to store the area
    * Version 1 - added versioning and stored area in XAttribute data
    * @see GetCompiledVersion
    * @bsimethod                                    Mark.Dane                       10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    DGNPLATFORM_EXPORT static int GetVersion (ElementHandleCR);

    /*---------------------------------------------------------------------------------**//**
    * The version of markup content area element that this implementation of the handler
    * supports.
    * @see GetVersion
    * @bsimethod                                    Mark.Dane                       10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    DGNPLATFORM_EXPORT static int GetCompiledVersion();

    /*---------------------------------------------------------------------------------**//**
    * @param[OUT]   foundElement
    * @param[IN]    model
    * @return true if the element was located and foundElement is successfully populated
    * @bsimethod                                    Mark.Dane                       08/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    DGNPLATFORM_EXPORT static bool FindElement (ElementHandleP foundElement, DgnModelR model);
    }; // AreaHandler
#endif

END_BENTLEY_DGNPLATFORM_NAMESPACE


