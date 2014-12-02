/*--------------------------------------------------------------------------------------+
|
|     $Source: Tools/ToolSubs/macro/macro.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include    <DgnPlatform/DesktopTools/ConfigurationManager.h>
#include    <DgnPlatform/DesktopTools/MacroConfigurationAdmin.h>
#include    <DgnPlatform/DesktopTools/MacroFileProcessor.h>

// defined in evalcnst, used by MacroFileProcessor
extern bool                 evaluateSymbolAsBoolean (WCharCP p, DgnPlatform::ConfigurationVariableLevel level, DgnPlatform::MacroConfigurationAdmin&, DgnPlatform::MacroFileProcessor&);
extern int                  evaluateSymbolAsInt (WCharCP p, DgnPlatform::ConfigurationVariableLevel level, DgnPlatform::MacroConfigurationAdmin&, DgnPlatform::MacroFileProcessor&);



