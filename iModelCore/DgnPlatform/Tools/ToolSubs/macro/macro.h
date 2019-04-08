/*--------------------------------------------------------------------------------------+
|
|     $Source: Tools/ToolSubs/macro/macro.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

struct MacroFileProcessor;
struct MacroConfigurationAdmin;

// implemented in evalcnst, used by MacroFileProcessor
bool                evaluateSymbolAsBoolean (WCharCP p, Dgn::ConfigurationVariableLevel level, MacroConfigurationAdmin&, MacroFileProcessor&);
int                 evaluateSymbolAsInt (WCharCP p, Dgn::ConfigurationVariableLevel level, MacroConfigurationAdmin&, MacroFileProcessor&);

END_BENTLEY_DGNPLATFORM_NAMESPACE

