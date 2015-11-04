/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/StoredExpression.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

#include "../DgnPlatform.h"


BEGIN_BENTLEY_DGN_NAMESPACE

/*---------------------------------------------------------
Format of XAttribute Data
   ==== NE_INITIAL_VERSION ==== (this version was never in a released version, it way in the first 8.11 beta version)
    Int16    nameLength
    WChar* name
    Int16    descriptionLength
    WChar* name
    Int16    expressionLength
    WChar* expression
      --- for each required symbol set ---
        Int16    symbolSetNameLength
        WChar* symbolSetName

   ==== NE_CURRENT_VERSION ====
    Int16    nameLength
    WChar* name
    Int16    labelLength
    WChar* label
    Int16    descriptionLength
    WChar* name
    Int16    expressionLength
    WChar* expression
      --- for each required symbol set ---
        Int16    symbolSetNameLength
        WChar* symbolSetName
    Int16    0 - signifies end of symbol sets
      --- for each keyword ---
        Int16    keywordLength
        WChar* keyword

---------------------------------------------------------*/

// NamedExpression version # is handler's minorId
#define NE_INITIAL_VERSION     0x00
#define NE_CURRENT_VERSION     0x01

struct StoredExpression;
struct StoredExpressionKeyword;

typedef RefCountedPtr<StoredExpression> StoredExpressionPtr;
typedef RefCountedPtr<StoredExpressionKeyword> StoredExpressionKeywordPtr;

//=======================================================================================
//! Helper class to load a Stored Expression (used by NamedExpression system) from an XAttribute
// @bsiclass                                                   Bill.Steinbock  10/2009
//=======================================================================================
struct StoredExpressionHelper
{
public:
    DGNPLATFORM_EXPORT static XAttributeHandlerId  GetPreReleaseExpressionHandlerID         (); // this id should only be in prerelease test dgnfiles

public:
    //! Return Stored Expression stored in the XAttributes defined be the input XAttributeHandle.
    //! @param[in]  xAttr   XAttributeHandle containing the Stored Expression XAttributes.
    DGNPLATFORM_EXPORT static StoredExpressionPtr         CreateExpressionFromXAttributes   (XAttributeHandleCR xAttr);

    //! Return Stored Expression found in file by name. If a matching name is not found the returned pointer .IsValid() method will return false.
    //! @param[in]  dgnfile   Dgn file to process.
    //! @param[in]  name      Named of Stored Expression to return.
    DGNPLATFORM_EXPORT static StoredExpressionPtr         FindExpressionByName      (DgnFile& dgnfile, WCharCP name);

    //! Update a specific XAttribute to match the supplied Stored Expression.
    //! @param[in]  xAttr   Existing XAttributeHandle.
    //! @param[in]  expression   Stored Expression definition used to update existing XAttribute.
    DGNPLATFORM_EXPORT static BentleyStatus UpdateExpression (XAttributeHandleR xAttr, StoredExpressionR expression);

    //! Add or update the XAttribute that define a Stored Expression Keyword.
    //! @param[in]  expression   Stored Expression definition.
    //! @param[in]  dgnfile   Dgn file to process.
    //! @param[in]  originalExpressionName   optional, original name, used when renaming.
    DGNPLATFORM_EXPORT static BentleyStatus               AddOrUpdateExpressionInFile       (StoredExpressionR expression, DgnFile& dgnfile, WCharCP originalExpressionName=NULL);

    //! Return Stored Expression Keyword stored in the XAttributes defined be the input XAttributeHandle.
    //! @param[in]  xAttr   XAttributeHandle containing the Stored Expression Keyword XAttributes.
    DGNPLATFORM_EXPORT static StoredExpressionKeywordPtr  CreateKeywordFromXAttributes      (XAttributeHandleCR xAttr);

    //! Return Stored Expression Keyword found in file by name. If a matching name is not found the returned pointer .IsValid() method will return false.
    //! @param[in]  dgnfile   Dgn file to process.
    //! @param[in]  name      Named of Stored Expression Keyword to return.
    DGNPLATFORM_EXPORT static StoredExpressionKeywordPtr  FindKeywordByName         (DgnFile& dgnfile, WCharCP name);

    //! Update a specific XAttributes to match the supplied Stored Expression Keyword.
    //! @param[in]  xAttr   Existing XAttributeHandle.
    //! @param[in]  keyword   Stored Expression Keyword definition used to update existing XAttribute.
    DGNPLATFORM_EXPORT static BentleyStatus               UpdateKeyword             (XAttributeHandleR xAttr, StoredExpressionKeywordR keyword);

    //! Add or update the XAttributes that define a Stored Expression Keyword.
    //! @param[in]  keyword   Stored Expression Keyword definition.
    //! @param[in]  dgnfile   Dgn file to process.
    //! @param[in]  originalKeywordName   optional, original name, used when renaming.
    DGNPLATFORM_EXPORT static BentleyStatus               AddOrUpdateKeywordInFile  (StoredExpressionKeywordR keyword, DgnFile& dgnfile, WCharCP originalKeywordName=NULL);

    //! Get XAttributeHandlerId for the Store Expression XAttributes.
    DGNPLATFORM_EXPORT static XAttributeHandlerId         GetExpressionHandlerID    ();

    //! Get XAttributeHandlerId for the Store Expression Keyword XAttributes.
    DGNPLATFORM_EXPORT static XAttributeHandlerId         GetKeywordHandlerID       ();

    //! Get bvector of Store Expression Names.
    //! @param[out]  expressionVector   empty bvector to populate with the names of the Stored Expressions.
    //! @param[in]   dgnfile   Dgn file to process.
    DGNPLATFORM_EXPORT static size_t                      CollectExpressionNames    (T_WStringVector& expressionVector, DgnFile& dgnfile);

    //! Get bvector of Store Expression Keyword Names.
    //! @param[out]  keywordVector   empty bvector to populate with the names of the Stored Expressions Keywords.
    //! @param[in]   dgnfile   Dgn file to process.
    DGNPLATFORM_EXPORT static size_t                      CollectKeywordNames       (T_WStringVector& keywordVector, DgnFile& dgnfile);

    //! Get the DgnElementP that is associated with the Stored Expressions XAttributes. All the Stored Expressions in the file are associated with this one element.
    //! @param[in]  dgnfile   Dgn file to process.
    DGNPLATFORM_EXPORT static DgnElementP                  GetDgnElement            (DgnFile& dgnfile);
};

//=======================================================================================
//! Stored expression are file-based named expressions. These expressions are loaded
//! and evaulated by the Expression Evaluator.
// @bsiclass                                                   Bill.Steinbock  10/2009
//=======================================================================================
struct  StoredExpression : RefCountedBase
{
friend struct StoredExpressionHelper;

private:
    WString         m_name;
    WString         m_label;
    WString         m_description;
    WString         m_expression;
    T_WStringVector m_symbolSetNames;
    T_WStringVector m_keywords;

                     StoredExpression       (WCharCP name, WCharCP label, WCharCP description, WCharCP expression);
    BentleyStatus    LoadDataExternalizer   (DataExternalizer& sink);

public:
    //! Set the Name of a Stored Expression.
    //! @param[in]  name   New name for expression. This is a non-localizable string that is used to identify the expression.
    DGNPLATFORM_EXPORT void             SetName         (WCharCP name);

    //! Set the Display Label of a Stored Expression.
    //! @param[in]  label   New label for expression. The label is localizable and used in to identify the expression in the UI.
    DGNPLATFORM_EXPORT void             SetDisplayLabel (WCharCP label);

    //! Set the Description of a Stored Expression.
    //! @param[in]  description   New description for expression. The description is localizable.
    DGNPLATFORM_EXPORT void             SetDescription  (WCharCP description);

    //! Set the Stored Expression's expression string.
    //! @param[in]  expression   New expression string that is processed by the Expression Evaluator.
    DGNPLATFORM_EXPORT void             SetExpression   (WCharCP expression);

    //! Add a Symbol Set name to the Stored Expression.
    //! @param[in]  symbolSetName   Symbol set name to add to the list of Symbol Set required to evaluate the expression.
    DGNPLATFORM_EXPORT void             AddSymbolSet    (WCharCP symbolSetName);

    //! Add a Keyword name to the Stored Expression.
    //! @param[in]  keyword         Keyword name to add to the list of Keyword that exposes this expression.
    DGNPLATFORM_EXPORT void             AddKeyWord      (WCharCP keyword);

    //! Get the Name for this Stored Expression.
    DGNPLATFORM_EXPORT WCharCP        GetNameCP           () const;

    //! Get the Display Label for this Stored Expression.
    DGNPLATFORM_EXPORT WCharCP        GetDisplayLabelCP   () const;

    //! Get the Description for this Stored Expression.
    DGNPLATFORM_EXPORT WCharCP        GetDescriptionCP    () const;

    //! Get the Expression string that will be evaluated for this Stored Expression.
    DGNPLATFORM_EXPORT WCharCP        GetExpressionCP     () const;

    //! Get a bvector of keywords for this Stored Expression.
    DGNPLATFORM_EXPORT T_WStringVector* GetKeyWordVector    ();

    //! Get a bvector of symbol sets for this Stored Expression.
    DGNPLATFORM_EXPORT T_WStringVector* GetSymbolSetVector  ();
};

//=======================================================================================
//! Stored expression keywords are user defined keyword that can be used to filter
//! stored expressions for presentation in the user interface. Multiple keywords can
//! be associated with a single
//! expression.
// @bsiclass                                                   Bill.Steinbock  10/2009
//=======================================================================================
struct  StoredExpressionKeyword : RefCountedBase
{
friend struct StoredExpressionHelper;

private:
    WString         m_name;
    WString         m_label;
    WString         m_description;
    WString         m_iconName;

                     StoredExpressionKeyword (WCharCP name, WCharCP label, WCharCP description, WCharCP iconName);
    BentleyStatus    LoadDataExternalizer (DataExternalizer& sink);

public:
    //! Set the Name of a Stored Expression Keyword.
    //! @param[in]  name   New name for expression. This is a non-localizable string that is used to identify the expression keyword.
    DGNPLATFORM_EXPORT void             SetName         (WCharCP name);

    //! Set the Display Label of a Stored Expression Keyword.
    //! @param[in]  label   New label for expression keyword. The label is localizable and used in to identify the expression keyword in the UI.
    DGNPLATFORM_EXPORT void             SetDisplayLabel (WCharCP label);

    //! Set the Description of a Stored Expression Keyword.
    //! @param[in]  description   New description for expression keyword. The description is localizable.
    DGNPLATFORM_EXPORT void             SetDescription  (WCharCP description);

    //! Set the name of an icon that is used to represent this Stored Expression Keyword.
    //! @param[in]  iconName   Name of icon.
    DGNPLATFORM_EXPORT void             SetIconName     (WCharCP iconName);

    //! Get the Name for this Stored Expression Keyword.
    DGNPLATFORM_EXPORT WCharCP        GetNameCP           () const;

    //! Get the Display Label for this Stored Expression Keyword.
    DGNPLATFORM_EXPORT WCharCP        GetDisplayLabelCP   () const;

    //! Get the Description for this Stored Expression Keyword.
    DGNPLATFORM_EXPORT WCharCP        GetDescriptionCP    () const;

    //! Get the Icon Name for this Stored Expression Keyword.
    DGNPLATFORM_EXPORT WCharCP        GetIconNameCP       () const;
};


END_BENTLEY_DGN_NAMESPACE
