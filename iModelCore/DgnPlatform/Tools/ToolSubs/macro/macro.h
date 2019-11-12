/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

struct MacroFileProcessor;
struct MacroConfigurationAdmin;

// implemented in evalcnst, used by MacroFileProcessor
bool                evaluateSymbolAsBoolean (WCharCP p, Dgn::ConfigurationVariableLevel level, MacroConfigurationAdmin&, MacroFileProcessor&);
int                 evaluateSymbolAsInt (WCharCP p, Dgn::ConfigurationVariableLevel level, MacroConfigurationAdmin&, MacroFileProcessor&);

END_BENTLEY_DGNPLATFORM_NAMESPACE

