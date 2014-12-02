/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/IRenderDependency.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include    <DgnPlatform/DgnPlatform.h>

DGNPLATFORM_TYPEDEFS (RenderDependencyManager)
DGNPLATFORM_TYPEDEFS (IRenderDependencyListener)

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
// @bsiclass                                                    PaulChater     10/11
//=======================================================================================
struct IRenderDependencyListener : public NonCopyableClass
{
private:

DGNPLATFORM_EXPORT virtual StatusInt   _OnRootsChanged (ElementHandleCR depEh, DependencyLinkage const& depData, UInt8* pRootStatus, UInt8 selfStatus, bool isUndoRedo) {return SUCCESS;}

protected:
    virtual ~IRenderDependencyListener() {}

public:
    StatusInt   OnRootsChanged (ElementHandleCR depEh, DependencyLinkage const& depData, UInt8* pRootStatus, UInt8 selfStatus, bool isUndoRedo);
};

//=======================================================================================
// @bsiclass                                                    PaulChater     10/11
//=======================================================================================
struct RenderDependencyManager : public NonCopyableClass
{
private:
    bvector <IRenderDependencyListenerP>                m_listenerList;

public:
    static void Initialize ();

    bvector <IRenderDependencyListenerP>&    GetListenerList ();

    DGNPLATFORM_EXPORT void RegisterRenderDependencyListener (IRenderDependencyListenerR listener);
    DGNPLATFORM_EXPORT void UnregisterRenderDependencyListener (IRenderDependencyListenerR listener);

    static DGNPLATFORM_EXPORT RenderDependencyManagerR GetManagerR ();
};


END_BENTLEY_DGNPLATFORM_NAMESPACE
