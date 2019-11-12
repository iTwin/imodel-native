/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <WebServices/iModelHub/GlobalEvents/GlobalEvent.h>
#include <WebServices/iModelHub/Common.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE
USING_NAMESPACE_BENTLEY_DGN

//=======================================================================================
//@bsiclass                                      Karolis.Uzkuraitis             05/2018
//=======================================================================================
struct GlobalEventParser
    {
    IMODELHUBCLIENT_EXPORT static GlobalEventPtr ParseEvent(Utf8CP responseContentType, Utf8String responseString, Utf8String headerLocation = nullptr);
    };

END_BENTLEY_IMODELHUB_NAMESPACE
