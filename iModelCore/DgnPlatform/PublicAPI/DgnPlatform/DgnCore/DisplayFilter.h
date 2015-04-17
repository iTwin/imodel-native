/*--------------------------------------------------------------------------------------+                                                         
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/DisplayFilter.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
                                                             
BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//__PUBLISH_SECTION_END__
struct ExtendedElementHandler;
//__PUBLISH_SECTION_START__

#if defined (NEEDS_WORK_DGNITEM)

//*=================================================================================**//**
//! DisplayFilter is used to construct branching structures for conditional presentation
//! in XGraphics.
//! The static If, ElseIf, Else, and End methods are used both to define and to test the
//! branch conditions. Presentation generated within a conditional branch is only drawn
//! if the condition evaluates to true.
//! @bsiclass                                                     RayBentley      07/2012
//===============+===============+===============+===============+===============+======*/
struct DisplayFilter
{

//*=================================================================================**//**
//! Base class for a DisplayFilter operator representing a branch conditions.
//! @bsiclass                                                     RayBentley      08/2012
//===============+===============+===============+===============+===============+======*/
struct  Operator : RefCountedBase
{

//__PUBLISH_SECTION_END__
    DisplayFilterHandlerId      m_handlerId;

protected:
    virtual void                _GetData (bvector<Byte>& data) const = 0;
public:   
    Operator (DisplayFilterHandlerId handlerId) : m_handlerId (handlerId)  { }

    void                        GetData (bvector<Byte>& data) const { return _GetData (data); }
    DisplayFilterHandlerId      GetHandlerId () const               { return m_handlerId; }
//__PUBLISH_SECTION_START__
};  // Operator

typedef RefCountedPtr <Operator> OperatorPtr;

//*---------------------------------------------------------------------------------**//**
//! DisplayFilter based on the boolean result of evaluating an ECExpression. If the
//! The condition resolves to true if the ECExpression was successfully evaluated and
//! its result could be resolved to 'true'.
//! @bsistruct                                                    Paul.Connelly   12/12
//---------------+---------------+---------------+---------------+---------------+------*/
struct  TestExpression : Operator
    {
public:
    //! Creates a new ECExpression-based DisplayFilter operator.
    //! @param[in] expression   An ECExpression evaluating to a boolean value
    //! @param[in] dgnfile      The file in which the element associated with this filter resides.
    DGNPLATFORM_EXPORT static OperatorPtr   Create (WCharCP expression, DgnDbR dgnfile);

    //! Creates a new ECExpression-based DisplayFilter operator to test the view size at the specified point
    //! The expression produced is of the format "ViewContext.GetPixelSizeAt (x,y,z) > pixelSize" if largerThan is true, or
    //! "ViewContext.GetPixelSizeAt (x,y,z) <= pixelSize" if largerThan is false
    //! @param[in] origin       The location to test
    //! @param[in] pixelSize    The pixel size to test against, in UORs
    //! @param[in] largerThan   If true, the expression uses the ">" operator; otherwise, it uses the "<=" operator
    //! @param[in] dgnfile      The file in which the element associated with this filter resides.
    DGNPLATFORM_EXPORT static OperatorPtr   CreateViewSizeTest (DPoint3dCR origin, double pixelSize, bool largerThan, DgnDbR dgnfile);


//__PUBLISH_SECTION_END__
private:
    uint32_t                    m_expressionId;
    bool                        m_storesTransform;

    TestExpression (uint32_t exprId, bool storesTransform) : Operator (DisplayFilterHandlerId_ECExpression), m_expressionId(exprId), m_storesTransform(storesTransform) { }

    virtual void                _GetData (bvector<Byte>& data) const override;
//__PUBLISH_SECTION_START__
    };  // ExpressionOperator

//! Creates a new  DisplayFilter operator to test for a match to the supplied formId.
//! @param[in] formId       The form Id to test against.
DGNPLATFORM_EXPORT static OperatorPtr   CreatePresentationFormIdTest (WCharCP formId);

//! Creates a new  DisplayFilter operator to test for a existence of the to the supplied form flag.
//! @param[in] formFlag       The form flag to test against.
DGNPLATFORM_EXPORT static OperatorPtr   CreatePresentationFormFlagTest (WCharCP formFlag);

enum ViewFlag 
    {
    ViewFlag_BoundaryDisplay = 1,
    ViewFlag_Constructions   = 2,
    ViewFlag_Dimensions      = 3,
    ViewFlag_DataFields      = 4,
    ViewFlag_Fill            = 5,
    ViewFlag_Grid            = 6,
    ViewFlag_LevelOverrides  = 7,
    ViewFlag_LineStyles      = 8,
    ViewFlag_LineWeights     = 9,
    ViewFlag_Patterns        = 10,
    ViewFlag_Tags            = 11,
    ViewFlag_Text            = 12,
    ViewFlag_Textnodes       = 13,
    ViewFlag_Transparency    = 14,
    };

DGNPLATFORM_EXPORT static OperatorPtr CreateViewFlagTest (ViewFlag viewFlag, bool testState);


enum TestMode
    {
    TestMode_Equal          = 1,
    TestMode_NotEqual       = 2,
    TestMode_LessThan       = 3,
    TestMode_GreaterThan    = 4,
    };


DGNPLATFORM_EXPORT static OperatorPtr  CreateRenderModeTest (DgnRenderMode renderMode, TestMode testMode);

//! Begins a conditional branch block, with filterOperator as the condition to be tested
//! Must be matched by a corresponding call to End. May be followed by zero or more calls to ElseIf,
//! or one call to Else.
//! @param[in]  viewContext             the context in which the condition is being evaluated
//! @param[in]  element                 the element being rendered
//! @param[in]  filterOperator          the conditional operator being tested
//! @return true if presentation should be generated for this conditional branch
DGNPLATFORM_EXPORT static bool If (ViewContextR viewContext, ElementHandleCP element, Operator const& filterOperator);

//! Follows a call to If or ElseIf to define or test an alternate conditional branch.
//! @param[in]  viewContext             the context in which the condition is being evaluated
//! @param[in]  element                 the element being rendered
//! @param[in]  filterOperator          the conditional operator being evaluated
//! @return true if presentation should be generated for this conditional branch
DGNPLATFORM_EXPORT static bool ElseIf (ViewContextR viewContext, ElementHandleCP element, Operator const& filterOperator);

//! Follows a call to If or ElseIf to define or test an alternate conditional branch. Must be followed by
//! a call to If or End.
//! @param[in]  viewContext             the context in which DisplayFilter is being evaluated
//! @return true if presentation should be generated for this conditional branch
DGNPLATFORM_EXPORT static bool Else (ViewContextR viewContext);

//! Ends a conditional branch block. Must match exactly one prior call to If
//! @param[in]  viewContext             the context in which DisplayFilter is being evaluated
DGNPLATFORM_EXPORT static void End (ViewContextR viewContext);

//__PUBLISH_SECTION_END__
DGNPLATFORM_EXPORT  static void RegisterCoreHandlers ();


//__PUBLISH_SECTION_START__
};  // DisplayFilter


//__PUBLISH_SECTION_END__

// The following structures are only here as they are needed during XGraphics generation
// To support fill display.

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      07/2012
+===============+===============+===============+===============+===============+======*/                                                                        
struct  ViewFlagFilterData
{
    DisplayFilter::ViewFlag         m_flag;
    bool                            m_testState;

    ViewFlagFilterData (DisplayFilter::ViewFlag flag, bool testState)
        {
        memset (this, 0, sizeof (*this));
        m_flag = flag;
        m_testState = testState;
        }

};  // ViewFlagFilterData


/*=================================================================================**//**
* @bsiclass                                                     RayBentley      07/2012
+===============+===============+===============+===============+===============+======*/                                                                        
struct  ViewParameterFilterData
{
    enum    Parameter
        {
        Parameter_DrawPurpose           = 1,
        Parameter_ViewOrientation       = 2,
        Parameter_RenderMode            = 3,
        Parameter_ClipVolumePass        = 4,
        Parameter_InViewlet             = 5,
        Parameter_DrawingAttachmentType = 6,
        };


    int32_t                         m_testValue;
    DisplayFilter::TestMode         m_testMode;
    Parameter                       m_parameter;

    ViewParameterFilterData (uint32_t testValue, DisplayFilter::TestMode testMode, Parameter parameter)
        {
        memset (this, 0, sizeof (*this));
        m_testMode = testMode;
        m_testValue  = testValue;
        m_parameter  =parameter;
        }
};  // ViewParameterFilterData
//__PUBLISH_SECTION_START__
#endif



END_BENTLEY_DGNPLATFORM_NAMESPACE









