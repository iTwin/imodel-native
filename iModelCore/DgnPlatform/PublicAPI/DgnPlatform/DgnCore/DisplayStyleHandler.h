/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/DisplayStyleHandler.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include    "../DgnPlatform.h"

DGNPLATFORM_TYPEDEFS (DisplayStyleHandlerSettingsEditor)

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE


#define XATTRIBUTEID_DisplayStyleHandler         22762

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      10/2011
*  Sub Ids which may be used for Display Style Handlers and related elements
*  (along with major ID XATTRIBUTEID_DisplayStyleHandler).
+===============+===============+===============+===============+===============+======*/
enum  DisplayStyleHandler_XAttributeSubTypes
    {
    DisplayStyleHandlerSubID_SolarExposure                  = 1,
    DisplayStyleHandlerSubID_Height                         = 2,
    DisplayStyleHandlerSubID_OriginDistance                 = 3,
    DisplayStyleHandlerSubID_SlopeAngle                     = 4,
    DisplayStyleHandlerSubID_SlopePercent                   = 5,
    DisplayStyleHandlerSubID_Aspect                         = 6,
    DisplayStyleHandlerSubID_SolarExposed                   = 7,
    DisplayStyleHandlerSubID_SolarShadowed                  = 8,
    DisplayStyleHandlerSubID_InsolationDirect               = 9,
    DisplayStyleHandlerSubID_InsolationDiffuse              = 10,
    DisplayStyleHandlerSubID_InsolationTotal                = 11,
    DisplayStyleHandlerSubID_ElementModificationTime        = 12,
    DisplayStyleHandlerSubID_ElementRange                   = 13,
    DisplayStyleHandlerSubID_Illumination                   = 14,
    DisplayStyleHandlerSubID_ThematicLegend                 = 15,
    DisplayStyleHandlerSubID_ThematicLegendElementHandler   = 16,
    DisplayStyleHandlerSubID_ThematicLegendElementData      = 17,
    DisplayStyleHandlerSubID_HillShade                      = 18,
    DisplayStyleHandlerSubID_ElementNeighborhood            = 19,
    DisplayStyleHandlerSubID_CachedVisibleEdgeHandler       = 20,
    DisplayStyleHandlerSubID_CachedVisibleEdgeCache         = 21,
    DisplayStyleHandlerSubID_Unused1                        = 22,
    DisplayStyleHandlerSubID_DisplayRules                   = 23,
    };

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      10/2011
*  When display style handler settings are persisted, this identifies what kind of
*  settings are stored (or rather what kind of handler was storing the settings).
+===============+===============+===============+===============+===============+======*/
enum    DisplayStyleHandler_SettingsXAttributeSubId     
    {
    DisplayStyleHandler_SettingsXAttributeSubId_Base        = 0x1000,
    DisplayStyleHandler_SettingsXAttributeSubId_Thematic,
    DisplayStyleHandler_SettingsXAttributeSubId_Insolation,
    DisplayStyleHandler_SettingsXAttributeSubId_HillShade,
    DisplayStyleHandler_SettingsXAttributeSubId_Proxy,
    DisplayStyleHandler_SettingsXAttributeSubId_DgnModelProxyHandler,
    DisplayStyleHandler_SettingsXAttributeSubId_DisplayRules,
    };


#if defined (NEEDS_WORK_DGNITEM)
/*=================================================================================**//**
*  Manager class for DisplayStyleHandlers.  A display handler is registered
*  by calling    DisplayStyleHandlerManager::GetManager().RegisterHandler()
* @bsiclass                                                     RayBentley      10/2011
+===============+===============+===============+===============+===============+======*/
struct DisplayStyleHandlerManager
{
private:
        T_DisplayStyleHandlerMap                m_handlerMap;
        T_DisplayStyleHandlerSettingsEditorMap  m_editorMap;

public:

/*---------------------------------------------------------------------------------**//**
* @bsimethod
*   Return reference to (singleton) DisplayStyleHandlerManager
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static DisplayStyleHandlerManagerR     GetManager();

/*---------------------------------------------------------------------------------**//**
* @bsimethod
*   Register a DisplayStyleHandler manager.   Should be called once per session.
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT void                                   RegisterHandler (DisplayStyleHandlerR handler);
DGNPLATFORM_EXPORT void                                   RegisterEditor  (DisplayStyleHandlerSettingsEditorR editor);


T_DisplayStyleHandlerMap&                             GetHandlerMap () { return m_handlerMap; }
DGNPLATFORM_EXPORT DisplayStyleHandlerP                   GetHandler (XAttributeHandlerId id);
DGNPLATFORM_EXPORT DisplayStyleHandlerSettingsEditorP     GetEditor (XAttributeHandlerId id);

};  // DisplayStyleHandlerManager

 
/*=================================================================================**//**
* @bsiclass                                                     Will.Bentley     5/10
+===============+===============+===============+===============+===============+======*/
struct  DisplayStyleHandlerViewFlags
{
    unsigned                        m_unused:32;
    unsigned                        m_unused1:22;
    unsigned                        m_unused2:32;

};  //  DisplayStyleHandlerViewFlags

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      10/2010
+===============+===============+===============+===============+===============+======*/
 struct  DisplayStyleHandlerSettingsData
 {
    UInt16                          m_version;
    DisplayStyleHandlerViewFlags    m_flags;

    DisplayStyleHandlerSettingsData () : m_version (0) { memset (&m_flags, 0, sizeof (m_flags)); }

 }; //  DisplayStyleHandlerSettingsData


/*=================================================================================**//**
* @bsiclass                                                     RayBentley      10/2011
*
* Base class for Display Style Handler settings.  A handler extends this class to store
* its settings.  The handler settings can consist of persistent data (saved to file) and
* non-persistent data that is used during the display process, but not saved.
* The persistent data is stored in the "_Save" method.
+===============+===============+===============+===============+===============+======*/
struct DisplayStyleHandlerSettings : RefCountedBase
{
private:
    // Directly Persisted
    DisplayStyleHandlerSettingsData m_data;

    // Indirectly Persisted
    bset <DisplayHandlerP>          m_handlersToApply;

    // Not Persisted
    Int32                           m_styleIndex;

    virtual DisplayStyleHandlerSettingsPtr  _Clone () const        { return new DisplayStyleHandlerSettings (*this); }

public:
                                    DisplayStyleHandlerSettings () {}
                                    DisplayStyleHandlerSettings (DisplayStyleHandlerSettingsCR other) {m_data = other.m_data; m_handlersToApply = other.m_handlersToApply; m_styleIndex = other.m_styleIndex; }

    //This function allocates a new DisplayStyleHandlerSettings, which must be deleted by the caller.
    DisplayStyleHandlerSettingsPtr  Clone () const                                              { return _Clone (); }
    bool                            Equals (DisplayStyleHandlerSettingsCP rhs) const            { return _Equals (rhs); }

    static DisplayStyleHandlerSettingsPtr Create () { return new DisplayStyleHandlerSettings (); }

protected:

/*---------------------------------------------------------------------------------**//**
* @bsimethod
* Comparator.  This method should also call __super::_Equals
* @param[in]    rhs Settings object to compare against
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool                    _Equals (DisplayStyleHandlerSettingsCP rhs) const           { return NULL != rhs && (0 == memcmp (&m_data, &rhs->m_data, sizeof (m_data))) && m_handlersToApply == rhs->m_handlersToApply; }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod
* Read persistent settings data (using ReadData).  This method should also call
* __super::_Read to allow superclasses to read their settings.
* @param[in]    elementRef (pass through to _ReadData).
* @param[in]    styleIndex (pass through to _ReadData)
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT virtual StatusInt    _Read (ElementRefP elementRef, int styleIndex);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
* Add a block of data to buffer marked with the given type (Helper method for _Save).
* @param[in]    buffer Buffer to append the data to.
* @param[in]    type Id describing the contents of data. Each subclass may have its own set of types,
*               and may add multiple data of a given type.
* @param[in]    dataSize Size in bytes of the data to persist
* @param[in]    data Pointer to the data to be persisted
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static void         AddSettingsSubType (bvector <byte>& buffer, UInt16 type, UInt16 dataSize, void const* data);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
* Save persistent settings data (using SaveData).  This method should also call
* __super::_Save to allow superclasses to save their settings.
* @param[in]    elementRef (pass through to _ReadData).
* @param[in]    styleIndex (pass through to _ReadData)
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT virtual StatusInt    _Save (ElementRefP elementRef, int styleIndex);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
* Read persistent settings data (called from _Read).
* @param[out]   data        buffer resized and filled with settings data.
* @param[in]    elementRef  element (passed through from _Read).
* @param[in]    settingsId  uniqueId for settings data (defined in DisplayStyleHandler.h enumeration and allocated by BSI.
* @param[in]    styleIndex  style index (passed through from _Read);.
*
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     ReadData
(
bvector<byte>&                              data,
ElementRefP                                 elementRef,
DisplayStyleHandler_SettingsXAttributeSubId settingsId,
int styleIndex
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
* Save persistent settings data (called from _Save).
* @param[in]    pData       data to be saved.
* @param[in]    dataSize    size of data to be saved (in bytes).
* @param[in]    elementRef  element (passed through from _Save).
* @param[in]    settingsId  uniqueId for settings data (defined in DisplayStyleHandler.h enumeration and allocated by BSI).
* @param[in]    styleIndex  style index (passed through from _Save).
*
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     SaveData (void* pData, size_t dataSize,  ElementRefP elementRef, DisplayStyleHandler_SettingsXAttributeSubId settingsId, int styleIndex);

public:
    StatusInt               Read (ElementRefP elementRef, int styleIndex)    { return _Read (elementRef, styleIndex);  }
    StatusInt               Save (ElementRefP elementRef, int styleIndex)    { return _Save (elementRef, styleIndex);  }

    Int32                       GetStyleIndex () const                      { return m_styleIndex; }
};

/*=================================================================================**//**
* @bsiclass                                                     BrandonBohrer   06/2011
* Root implementation of the DisplayStyleHandlerKey interface.
* It does not look at the settings and only uses the
* handler ID. Under this scheme multiple styles with one handler will share a cache.
+===============+===============+===============+===============+===============+======*/
typedef RefCountedPtr <struct DisplayStyleHandlerKey> DisplayStyleHandlerKeyPtr;

struct DisplayStyleHandlerKey  : RefCountedBase
{
private:
    HandlerId       m_handlerId;

protected:
    DGNPLATFORM_EXPORT                      DisplayStyleHandlerKey (DisplayStyleHandlerCR handler);
    virtual                             ~DisplayStyleHandlerKey () { }
public:
    static  DisplayStyleHandlerKeyPtr   Create (DisplayStyleHandlerCR handler) { return new DisplayStyleHandlerKey (handler); }
            HandlerId                   GetHandlerId () const                  { return m_handlerId; }
    DGNPLATFORM_EXPORT virtual bool         Matches (DisplayStyleHandlerKey const& other) const;
};


/*=================================================================================**//**
* @bsiclass                                                     RayBentley      10/2011
+===============+===============+===============+===============+===============+======*/
struct          DisplayStyleHandler
{
private:
            struct IDisplayStyleHandlerSettingsEditor*  m_settingsEditor;

protected:


/*---------------------------------------------------------------------------------**//**
* @bsimethod
*   Return true (the default) to allow the user to select this handler in the
*   "Thematic Display:" picker in the Display Style Editor.
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT virtual bool _ShowInThematicDisplayPicker () const { return true; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
*   Return a unique ID used to identify the handler internally.   The
*   majorId "XATTRIBUTEID_DisplayStyleHandler" may be used along with
*   a subId from the DisplayStyleHandler_XAttributeSubTypes enumeration is suggested.
+---------------+---------------+---------------+---------------+---------------+------*/
virtual XAttributeHandlerId    _GetHandlerId () const = 0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
*   Return a (translated) string that is used to identify the handler to the user.
+---------------+---------------+---------------+---------------+---------------+------*/
virtual WString _GetName () const = 0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
*  Query if the handler is valid for viewport.  If the handler return s "false" then the
*   display style will not appear in the display style picker for this view (optional).
*  @param[in]   viewport.
+---------------+---------------+---------------+---------------+---------------+------*/
 virtual bool   _IsValidForViewport (ViewportCR viewport) const { return true; }

 /*---------------------------------------------------------------------------------**//**
* @bsimethod
*  Query if the handler is valid for the given display style.  If the handler returns
*    "false" then this handler will be disabled in the handler picker (optional).
*  @param[in]   viewport.
+---------------+---------------+---------------+---------------+---------------+------*/
 virtual bool   _IsValidForDisplayStyle (DisplayStyleCR style) const { return true; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
*   _DrawElement is called to allow the handler to take over display of the element.
*   If the handler returns true then caller assumes that the handler has displayed the element.
*   If the handler returns false then the standard element display code is called.
*   Handlers may use this method to activate override symbology and return false
*   to have the element displayed with those overrides.
*   @param[in] el  Element to display.
*   @param[in] viewContext
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool   _DrawElement (ElementHandleCR el, ViewContextR viewContext) const = 0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
*   _DrawElement is called to allow the handler to take over display of the element cut.
*   If the handler returns true then caller assumes that the handler has displayed the element.
*   If the handler returns false then the standard element display code is called.
*   Handlers may use this method to activate override symbology and return false
*   to have the element displayed with those overrides.
*   @param[in] el  Element to display.
*   @param[in] viewContext
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool   _DrawElementCut (ElementHandleCR el, ViewContextR viewContext) const { return false; };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
*   _DrawLegend is called to allow the handler to draw a "legend" in the view.
*  Called once for each view update. (Optional).
*   @param[in] viewContext
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _DrawLegend (ViewContextR viewContext) const { }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
*   _ApplySymbologyOverrides allows the handler to modify the context's override matSymb.
*   Called just before registered symbology filters and AddContextOverrides as these should
*   have final say about what overrides are applied.
*   @param[in] viewContext
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _ApplySymbologyOverrides (ViewContextR viewContext) const {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
*   Create and return a settings object from the input data provided.
+---------------+---------------+---------------+---------------+---------------+------*/
virtual DisplayStyleHandlerSettingsPtr    _GetSettings () const { return DisplayStyleHandlerSettings::Create(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
*   Create a (arbitrarily large) cache key for the handler.  The cache key is used to look
*   up cached elements from the QuickVision cache.  By default it contains only the
*   handler's minor ID.  If there are settings that necessitate restroking the element
*   then they should be stored in the key as well.  Any change to the cache key will
*   cause new cache elements to be generated (optional).
*   @param[in] modelRef Used as a basis for any model-wide parts of the key. For example, the
*               height handler caches by model range. For this to work on refernce attatchments
*               this function must be called with the master model.
*   @param[in] settings The settings that are active in the view context
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT virtual DisplayStyleHandlerKeyPtr  _GetCacheKey (DgnModelR modelRef, DisplayStyleHandlerSettingsCP settings) const;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
*   Called once when the frustum is changed.  This occurs when view is initiallized.
*   @param[in]  settings
*   @param[in]  viewContext
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _OnFrustumChange (DisplayStyleHandlerSettingsR settings, ViewContextR viewContext, DgnModelR modelRef) const { }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
*   Called when a display style is pushed (instantiated for use)..
*   @param[in]  settings
*   @param[in]  viewContext
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _PushStyle (DisplayStyleHandlerSettingsP settings, ViewContextR viewContext) const { }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
*   Called when a display style is popped (retired from use)..
*   @param[in]  settings
*   @param[in]  viewContext
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _PopStyle (DisplayStyleHandlerSettingsP settings, ViewContextR viewContext) const { }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
*   Used when a display style is copied between DGN files.  Any file specific parameters
*   should be updated to match the destination file (optional).
*   @param[in]  settings
*   @param[in]  sourceDgnFile
*   @param[in]  destinationDgnFile.
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _CloneSettings (DisplayStyleHandlerSettingsR settings, DgnProjectR sourceDgnFile, DgnProjectR destinationDgnFile) const { }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
*   Gets the maximum render mode if this handler overrides that specified in the display style.
*   @return true if the render mode is overridden by the handler.
*   @param[out] maximumRenderMode
*   @param[in]  settings
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _TryGetMaximumRenderMode (MSRenderMode& maximumRenderMode, DisplayStyleHandlerSettingsR settings) const {return false;}
                                                                                                                                                               
virtual struct IViewHandlerHitInfo*     _GetViewHandlerHitInfo (DisplayStyleHandlerSettingsCP settings, DPoint3dCR hitPoint) const                      { return NULL; }    
virtual bool                            _StrokeForCache (CachedDrawHandleCR dh, ViewContextR viewContext, IStrokeForCache& stroker, double pixelSize) const   { return false; }
virtual void                            _OverrideSizeDependence (double& sizeDependentRatio) const                                                      { }
virtual bool                            _HandleChildDisplay (DisplayStyleHandlerSettingsP handlerSettings) const                                        { return false; }
virtual bool                            _DoOcclusionSorting (ViewContextR viewContext, DgnModelR modelRef) const                                     { return true; }
virtual bool                            _RequiresInitializeRendering (DgnModelR modelRef) const                                                      { return true; }
virtual DgnModelP                       _GetSymbologyDgnModel (ViewContextR viewContext) const                                                          { return NULL; }
virtual StatusInt                       _VisitPath (DisplayPathCP path, ViewContextR viewContext) const                                                 { return ERROR; }

public:
    WString                                 GetName () const                                                                                            { return _GetName (); }
    bool                                    ShowInThematicDisplayPicker () const                                                                        { return _ShowInThematicDisplayPicker(); }
    bool                                    IsValidForViewport (ViewportCR viewport) const                                                              { return _IsValidForViewport (viewport); }
    bool                                    IsValidForDisplayStyle (DisplayStyleCR style) const                                                         { return _IsValidForDisplayStyle (style); }
    StatusInt                               VisitPath (DisplayPathCP path, ViewContextR viewContext) const                                              { return _VisitPath (path, viewContext); }
    bool                                    DrawElement (ElementHandleCR el, ViewContextR viewContext) const                                            { return _DrawElement (el, viewContext); }
    bool                                    DrawElementCut (ElementHandleCR el, ViewContextR viewContext) const                                         { return _DrawElementCut (el, viewContext); }
    void                                    DrawLegend (ViewContextR viewContext) const                                                                 { return _DrawLegend (viewContext); }
    DGNPLATFORM_EXPORT void                 ApplySymbologyOverrides (ViewContextR viewContext) const;
    XAttributeHandlerId                     GetHandlerId () const                                                                                       { return _GetHandlerId (); }
    IViewHandlerHitInfo*                    GetViewHandlerHitInfo (DisplayStyleHandlerSettingsCP settings, DPoint3dCR hitPoint) const                   { return _GetViewHandlerHitInfo (settings, hitPoint); }
    DisplayStyleHandlerSettingsPtr          GetSettings () const                                                                                        { return _GetSettings (); }
    DGNPLATFORM_EXPORT DisplayStyleHandlerKeyPtr               GetCacheKey(DgnModelR modelRef, DisplayStyleHandlerSettingsCP settings)  const;
    void                                    CloneSettings (DisplayStyleHandlerSettingsR settings, DgnProjectR source, DgnProjectR destination) const      { _CloneSettings (settings, source, destination); }
    bool                                    StrokeForCache (CachedDrawHandleCR dh, ViewContextR context, IStrokeForCache& stroker, double pixelSize)  const   { return _StrokeForCache (dh, context, stroker, pixelSize); }
    void                                    OnFrustumChange (DisplayStyleHandlerSettingsR settings, ViewContextR viewContext, DgnModelR modelRef) const { _OnFrustumChange (settings, viewContext, modelRef); }
    void                                    PushStyle (DisplayStyleHandlerSettingsP settings, ViewContextR viewContext) const                           { _PushStyle (settings, viewContext); }
    void                                    PopStyle (DisplayStyleHandlerSettingsP settings, ViewContextR viewContext) const                            { _PopStyle (settings, viewContext); }
    void                                    OverrideSizeDependence (double& sizeDependentRatio) const                                                   { _OverrideSizeDependence (sizeDependentRatio); }
    DGNPLATFORM_EXPORT StatusInt                EditSettings (DisplayStyleR style, MdlDesc*& mdlDesc, UInt32& itemType, Int32& itemId) const;
    bool                                    TryGetMaximumRenderMode (MSRenderMode& maximumRenderMode, DisplayStyleHandlerSettingsR settings) const      { return _TryGetMaximumRenderMode (maximumRenderMode, settings);}
    bool                                    HandleChildDisplay (DisplayStyleHandlerSettingsP handlerSettings) const                                     { return _HandleChildDisplay (handlerSettings); }
    bool                                    DoOcclusionSorting (ViewContextR viewContext, DgnModelR modelRef) const                                  { return _DoOcclusionSorting (viewContext, modelRef); }
    bool                                    RequiresInitializeRendering (DgnModelR modelRef) const                                                   { return _RequiresInitializeRendering (modelRef); }
    DgnModelP                               GetSymbologyDgnModel (ViewContextR viewContext) const                                                       { return _GetSymbologyDgnModel (viewContext); }
    void                                    SetSettingsEditor (struct IDisplayStyleHandlerSettingsEditor* settingsEditor)                               { m_settingsEditor = settingsEditor; }

};      //  DisplayStyleHandler

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      05/2012
+===============+===============+===============+===============+===============+======*/
struct DisplayStyleHandlerSettingsEditor
{

protected:
virtual XAttributeHandlerId    _GetHandlerId () const = 0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
*   Called by the "Edit Display Styles" dialog manager to allow the handler to present GUI
*   to allow editing of the settings.
*   @param[in]  style  The display style to be edited.
*   @param[out] mdlDec  MDL Descriptor for the editing application.
*   @param[out] itemType Type of editing resource item (RTYPE_Container).
*   @param[out] itemId  ID of editing resource item.
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _GetEditorItem (DisplayStyleR style, MdlDesc*& mdlDesc, UInt32& itemType, Int32& itemId) const = 0;

public:
    XAttributeHandlerId  GetHandlerId () const                                                                                      { return _GetHandlerId (); }
    StatusInt            GetEditorItem (DisplayStyleR style, MdlDesc*& mdlDesc, UInt32& itemType, Int32& itemId) const              { return _GetEditorItem (style, mdlDesc, itemType, itemId); }


}; // DisplayStyleHandlerSettingsEditor
#endif

END_BENTLEY_DGNPLATFORM_NAMESPACE
