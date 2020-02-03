/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/ActiveImageFunctionMode.hpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/ActiveImage/ActiveImageFunctionMode.hpp,v 1.1 2003/02/26 20:01:03 SebastienGosselin Exp $
//-----------------------------------------------------------------------------
// Class : CActiveImageFunctionMode
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Public
// GetCursor
//-----------------------------------------------------------------------------
inline HCURSOR CActiveImageFunctionMode::GetCursor() const
{
    // return the default cursor, The Arrow
    return AfxGetApp()->LoadStandardCursor(IDC_ARROW);
}