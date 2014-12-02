/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/DisplayFilterManager.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

     
//__BENTLEY_INTERNAL_ONLY__
                                                                 
BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
#define     HandlerId_DisplayFilter   22809 
                                                                                             
//=================================================================================**//**
//! Handler associated with a specific type of DisplayFilter.
//! @bsiclass                                                     RayBentley      07/2012
//==============+===============+===============+===============+===============+======*/                                                                        
struct DisplayFilterHandler 
{
protected:
virtual             bool        _DoConditionalDraw (ViewContextR viewContext, ElementHandleCP element, void const* data, size_t dataSize) const = 0;
virtual             StatusInt   _OnTransform (TransformInfoCR transform, void* pData, size_t dataSize) const { return SUCCESS;  }
virtual             bool        _IsEqual (void const* data, void const* rhsData, size_t dataSize, double distanceTolerance) const {return 0 == memcmp (data, rhsData, dataSize);}
virtual             WString     _GetDumpString (void const* data, size_t dataSize, DgnProjectR) const { return WString (L"DisplayFilterHandler"); }
#ifdef NEEDS_WORK_TopazMerge_DisplayFilter
    virtual             void        _DoClone (void* data, size_t dataSize, ElementCopyContextR context) const { }
#else
    virtual             void        _DoClone (void* data, size_t dataSize) const { }
#endif
virtual             StatusInt   _OnWriteToElement (void* data, size_t dataSize, ElementHandleCR eh) const { return SUCCESS; }
virtual             BentleyStatus _GetExpressionData (bvector<byte>& data, WCharCP expression, DgnProjectR dgnFile) const { return ERROR; }

public:
DGNPLATFORM_EXPORT      bool        DoConditionalDraw (ViewContextR viewContext, ElementHandleCP element, void const* data, size_t dataSize) const;
DGNPLATFORM_EXPORT      StatusInt   OnTransform (TransformInfoCR transform, void* pData, size_t dataSize) const;
DGNPLATFORM_EXPORT      bool        IsEqual (void const* data, void const* rhsData, size_t dataSize, double distanceTolerance);
DGNPLATFORM_EXPORT      WString     GetDumpString (void const* data, size_t dataSize, DgnProjectR) const;  
#ifdef NEEDS_WORK_TopazMerge_DisplayFilter
    DGNPLATFORM_EXPORT      void        DoClone (void* data, size_t dataSize, ElementCopyContextR context) const;
#else
    DGNPLATFORM_EXPORT      void        DoClone (void* data, size_t dataSize) const;
#endif
DGNPLATFORM_EXPORT      StatusInt   OnWriteToElement (void* data, size_t dataSize, ElementHandleCR eh) const;
DGNPLATFORM_EXPORT      BentleyStatus GetExpressionData (bvector<byte>& data, WCharCP expression, DgnProjectR dgnFile) const;

};  // DisplayFilterHandler

typedef bmap <DisplayFilterHandlerId, DisplayFilterHandlerP>        T_DisplayFilterHandlerMap;

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      07/2012

*  Manager class for DisplayStyleManager.  A display filter handler is registered
*  by calling    mDisplayFilterHandlerManager::GetManager().RegisterHandler()
+===============+===============+===============+===============+===============+======*/                                                                        
struct DisplayFilterHandlerManager
{
private:
        T_DisplayFilterHandlerMap               m_handlerMap;

public:

/*---------------------------------------------------------------------------------**//**
* @bsimethod
*   Return reference to (singleton) DisplayFilterHandlerManager
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static DisplayFilterHandlerManagerR     GetManager();

/*---------------------------------------------------------------------------------**//**
* @bsimethod
*   Register a DisplayFilterHandler manager.   Should be called once per session.
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT void                         RegisterHandler (DisplayFilterHandlerId id,  DisplayFilterHandlerR handler);
DGNPLATFORM_EXPORT DisplayFilterHandlerP        GetHandler (DisplayFilterHandlerId id); 

};  // DisplayFilterHandlerManager                               


/*=================================================================================**//**
* @bsiclass                                                     RayBentley      07/2012
+===============+===============+===============+===============+===============+======*/                                                                        
struct DisplayFilterKey : RefCountedBase
{
private:
    typedef bvector<struct DisplayFilterState*> T_States;

    T_States            m_states;

public:
DGNPLATFORM_EXPORT    static  DisplayFilterKeyPtr   Create () { return new DisplayFilterKey(); }
DGNPLATFORM_EXPORT            ~DisplayFilterKey();
DGNPLATFORM_EXPORT    bool    Matches (ViewContextR viewContext, ElementHandleCP element) const;
DGNPLATFORM_EXPORT    void    PushState (DisplayFilterHandlerId filterId, void const* data, size_t dataSize, bool state);
                  bool    IsEmpty() { return m_states.empty(); }
                  void    Clear ()  { m_states.clear(); }
          bool operator < (DisplayFilterKey const& rhs); 

};  //  DisplayFilterKey



END_BENTLEY_DGNPLATFORM_NAMESPACE

