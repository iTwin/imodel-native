/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageHunter/EnumOptionComparer.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/Utils/ImageHunter/EnumOptionComparer.cpp,v 1.2 2010/08/27 18:54:32 Jean.Lalande Exp $
//-----------------------------------------------------------------------------
// Methods for class EnumOptionComparer
//-----------------------------------------------------------------------------
#include "StdAfx.h"
#include "EnumOptionComparer.h"
#include "EnumOption.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
int EnumOptionComparer::Compare(Object^ x, Object^ y)
{
    return System::String::Compare(((EnumOption^) x)->GetLabel(), ((EnumOption^) y)->GetLabel());
}