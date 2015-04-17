/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/LineStyleApi.h $
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

typedef bvector <ElementHandle>  T_SimplifyResults;

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
DGNPLATFORM_EXPORT static void          AddLsRange (EditElementHandleR);
DGNPLATFORM_EXPORT static bool          ElementHasLineStyle (GeometricElementCP pElem, DgnModelP modelRef);
DGNPLATFORM_EXPORT static void          TransformParams (EditElementHandleR elem, TransformCP pTrans, DgnModelP sourceDgnModel, DgnModelP destDgnModel, bool canChangeSize);
DGNPLATFORM_EXPORT static StatusInt     DuplicateStyleDef (DgnElementP* newDefinitionElement, ElementHandleR oldDef, DgnDbP destFile);
DGNPLATFORM_EXPORT static int32_t       AddStyle (Utf8CP name, DgnDbR dgnFile, long seedID);
                   static void          AdjustParamUorScale (LineStyleParamsP paramsP, int32_t styleNo, DgnModelP modelRef, DgnCategoryId level);
                   static void          MergeParams (LineStyleParamsP outParams, LineStyleParamsP masterParams, LineStyleParamsP paramsToAdd);
DGNPLATFORM_EXPORT static BentleyStatus SimplifySymbol (T_SimplifyResults& results, ElementHandleCR eh);
DGNPLATFORM_EXPORT static void          InitializeParams (LineStyleParams*params);
DGNPLATFORM_EXPORT static StatusInt     GetParamsFromElement (LineStyleParamsP pParams, ElementHandleR eh);
                   static StatusInt     GetLinNameList (NameDefinitionList_T&nameDefList, WCharCP linFileName);
DGNPLATFORM_EXPORT static int           LoadLinDefinition (WCharCP linFileName, bvector<WString>&definitionsToLoad, DgnModelP modelRef, double linUnitsToMuFactor);
DGNPLATFORM_EXPORT static int           LoadLinDefinition (WCharCP linFileName, WCharCP oneDefinitionToLoad, DgnModelP modelRef, double linUnitsToMuFactor);
DGNPLATFORM_EXPORT static double        GetDefaultLinScale (DgnModelP destDgnModel);
};

//=======================================================================================
// @bsiclass                                                      John.Gooding    06/09
//=======================================================================================
struct LineStyleElementUtil
{
DGNPLATFORM_EXPORT static StatusInt GetName (DgnElementCP pElm, DgnModelP modelRef, WCharP name, uint32_t nameLength);
DGNPLATFORM_EXPORT static StatusInt SetName (DgnElementP* ppElm, DgnModelP modelRef, WCharCP newName);
DGNPLATFORM_EXPORT static StatusInt ApplyUorScale (GeometricElementP pElm, double unitsFactor);
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
