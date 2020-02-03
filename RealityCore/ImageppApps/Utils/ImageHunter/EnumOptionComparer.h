/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageHunter/EnumOptionComparer.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/Utils/ImageHunter/EnumOptionComparer.h,v 1.2 2010/08/27 18:54:32 Jean.Lalande Exp $
//-----------------------------------------------------------------------------
// Class : EnumOptionComparer
//-----------------------------------------------------------------------------
#pragma once

ref class EnumOptionComparer : public System::Collections::IComparer
{
public:
    virtual int Compare(Object^ x, Object^ y);
};