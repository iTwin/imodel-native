/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/IEditActionSource.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include    <DgnPlatform/DgnPlatform.h>

BEGIN_BENTLEY_DGN_NAMESPACE

struct     IEditActionArray;

//=======================================================================================
//! Applications should implement this interface to become an EditActionSource for right mouse clicks.
//! @private
//=======================================================================================
struct     IEditActionSource
{
protected:

/*---------------------------------------------------------------------------------**//**
* Test a HitDetailCP and add EditActions to the IEditActionArray as appropriate. This method is called
*               every time the user presses the right mouse button from the Select Tool. If the mouse is not over any
*               element, path will be NULL, which indicates the user is requesting EditActions that apply to the view.
* @param        point  IN the active coordinate location of the mouse
* @param        view   IN the view number of the mouse
* @param        path   IN the path to test. May be NULL, in which case the Edit Actions should apply to the view.
* @param        actionArray IN the current array of EditActions. Add new EditActions to this array by calling ~mAddEditAction.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _TestPath (DPoint3dCP point, int view, HitDetailCP path, IEditActionArrayP actionArray) = 0;

}; // IEditActionSource

END_BENTLEY_DGN_NAMESPACE

/** @endcond */
