/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "LineStyle.h"


#define     LS_ALL_LINES        (~0)
#define     LS_ALL_MLINE_TYPES  (~0)

BEGIN_BENTLEY_DGN_NAMESPACE

typedef std::map <WString, WString> NameDefinitionList_T;
typedef NameDefinitionList_T::value_type   NameDefinitionValue_T;

struct AddComponentsToDefElm;

//=======================================================================================
// @bsiclass
//=======================================================================================
struct LineStyleUtil
{
private:
public:
                   static LineStyleStatus MapToLineStyleStatus(int status);
DGNPLATFORM_EXPORT static int32_t       AddStyle (Utf8CP name, DgnDbR dgnFile, long seedID);
                   static void          AdjustParamUorScale (Render::LineStyleParamsP paramsP, int32_t styleNo, DgnModelP modelRef, DgnCategoryId level);
                   static void          MergeParams (Render::LineStyleParamsP outParams, Render::LineStyleParamsP masterParams, Render::LineStyleParamsP paramsToAdd);
DGNPLATFORM_EXPORT static void          InitializeParams (Render::LineStyleParams* params);
                   static StatusInt     GetLinNameList (NameDefinitionList_T&nameDefList, WCharCP linFileName);
DGNPLATFORM_EXPORT static int           LoadLinDefinition (WCharCP linFileName, bvector<WString>&definitionsToLoad, DgnModelP modelRef, double linUnitsToMuFactor);
DGNPLATFORM_EXPORT static int           LoadLinDefinition (WCharCP linFileName, WCharCP oneDefinitionToLoad, DgnModelP modelRef, double linUnitsToMuFactor);
DGNPLATFORM_EXPORT static double        GetDefaultLinScale (DgnModelP destDgnModel);
};

END_BENTLEY_DGN_NAMESPACE
