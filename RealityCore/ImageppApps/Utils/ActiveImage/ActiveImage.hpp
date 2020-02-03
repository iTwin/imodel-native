/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/ActiveImage.hpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// ----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/ActiveImage/ActiveImage.hpp,v 1.1 2003/02/26 20:01:02 SebastienGosselin Exp $
//
// Class: ActiveImage inlines
// ----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// public
// 
//-----------------------------------------------------------------------------
inline const OSVERSIONINFO* CActiveImageApp::GetOSVersion() const
{
    return (&m_OSInfo);
}
