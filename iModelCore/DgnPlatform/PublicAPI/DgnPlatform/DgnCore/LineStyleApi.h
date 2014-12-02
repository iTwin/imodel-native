/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/LineStyleApi.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__BENTLEY_INTERNAL_ONLY__*/
#include "LineStyle.h"


#define     LS_ALL_LINES        (~0)
#define     LS_ALL_MLINE_TYPES  (~0)

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

typedef bvector <ElementHandle>  T_SimplifyResults;

//=======================================================================================
// @bsiclass                                                      John.Gooding    08/09
//=======================================================================================
struct LineStyleLinkageUtil
{
DGNPLATFORM_EXPORT static int       ExtractParams (LineStyleParamsP pStyleParamOut, DgnElementCP pElementIn);
DGNPLATFORM_EXPORT static int       CreateRawLinkage (void* pStyleLink, LineStyleParamsCP pParams, bool is3d);
DGNPLATFORM_EXPORT static void      ClearElementStyle (DgnElementP elm, bool clearElementStyle, UInt32 lineMask, UInt32 mlineFlags);
DGNPLATFORM_EXPORT static StatusInt ExtractRawLinkage (LineStyleParamsP pParams, void const* pLink, bool is3d);
DGNPLATFORM_EXPORT static int       SetStyleParams (DgnElementP pElem, LineStyleParamsCP pParams);
DGNPLATFORM_EXPORT static StatusInt GetParamsFromElement (LineStyleParamsP pParams, DgnElementCP elm);
DGNPLATFORM_EXPORT static int       ExtractModifiers (LineStyleParamsP pParams, byte* pBuf, bool is3d);
DGNPLATFORM_EXPORT static int       AppendModifiers (byte* pBuf, LineStyleParamsCP pParams, bool is3d);
};

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
DGNPLATFORM_EXPORT static int           CreateSymbolDscr (MSElementDescrVec&,byte *symBufP, int bufSize, int threeD, UInt32 rscType, DgnModelP model);
DGNPLATFORM_EXPORT static void          AddLsRange (EditElementHandleR);
DGNPLATFORM_EXPORT static bool          ElementHasLineStyle (DgnElementCP pElem, DgnModelP modelRef);
DGNPLATFORM_EXPORT static void          TransformParams (EditElementHandleR elem, TransformCP pTrans, DgnModelP sourceDgnModel, DgnModelP destDgnModel, bool canChangeSize);
DGNPLATFORM_EXPORT static StatusInt     DuplicateStyleDef (MSElementDescrH newDefinitionElement, ElementHandleR oldDef, DgnProjectP destFile);
DGNPLATFORM_EXPORT static Int32         AddStyle (Utf8CP name, DgnProjectR dgnFile, long seedID);
                   static void          AdjustParamUorScale (LineStyleParamsP paramsP, Int32 styleNo, DgnModelP modelRef, LevelId level);
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
DGNPLATFORM_EXPORT static StatusInt GetName (MSElementDescrCP pElm, DgnModelP modelRef, WCharP name, UInt32 nameLength);
DGNPLATFORM_EXPORT static StatusInt SetName (MSElementDescrH ppElm, DgnModelP modelRef, WCharCP newName);
DGNPLATFORM_EXPORT static StatusInt ApplyUorScale (DgnElementP pElm, double unitsFactor);
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
