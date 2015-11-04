/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/LineStyleApi.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__BENTLEY_INTERNAL_ONLY__*/
#include "LineStyle.h"


#define     LS_ALL_LINES        (~0)
#define     LS_ALL_MLINE_TYPES  (~0)

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

typedef std::map <WString, WString> NameDefinitionList_T;
typedef NameDefinitionList_T::value_type   NameDefinitionValue_T;

struct AddComponentsToDefElm;

//=======================================================================================
// @bsiclass                                                      John.Gooding    06/09
//=======================================================================================
struct LineStyleUtil
{
private:
public:
                   static LineStyleStatus MapToLineStyleStatus(int status);
DGNPLATFORM_EXPORT static int           CreateSymbolDscr (DgnElementPtrVec&,Byte *symBufP, int bufSize, int threeD, uint32_t rscType, DgnModelP model);
DGNPLATFORM_EXPORT static bool          ElementHasLineStyle (GeometricElementCR);
DGNPLATFORM_EXPORT static int32_t       AddStyle (Utf8CP name, DgnDbR dgnFile, long seedID);
                   static void          AdjustParamUorScale (LineStyleParamsP paramsP, int32_t styleNo, DgnModelP modelRef, DgnCategoryId level);
                   static void          MergeParams (LineStyleParamsP outParams, LineStyleParamsP masterParams, LineStyleParamsP paramsToAdd);
DGNPLATFORM_EXPORT static void          InitializeParams (LineStyleParams*params);
                   static StatusInt     GetLinNameList (NameDefinitionList_T&nameDefList, WCharCP linFileName);
DGNPLATFORM_EXPORT static int           LoadLinDefinition (WCharCP linFileName, bvector<WString>&definitionsToLoad, DgnModelP modelRef, double linUnitsToMuFactor);
DGNPLATFORM_EXPORT static int           LoadLinDefinition (WCharCP linFileName, WCharCP oneDefinitionToLoad, DgnModelP modelRef, double linUnitsToMuFactor);
DGNPLATFORM_EXPORT static double        GetDefaultLinScale (DgnModelP destDgnModel);
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
