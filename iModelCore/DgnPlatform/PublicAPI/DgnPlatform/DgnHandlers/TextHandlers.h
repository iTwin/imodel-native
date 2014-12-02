/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/TextHandlers.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */
#if defined (NEEDS_WORK_DGNITEM)

#include <DgnPlatform/DgnHandlers/IManipulator.h>
#include <DgnPlatform/DgnHandlers/ITextEdit.h>
#include <DgnPlatform/DgnCore/DisplayHandler.h>
#include <DgnPlatform/DgnCore/IAnnotationHandler.h>
#include <DgnPlatform/DgnCore/TextString.h>

#include <valarray>
#include <Bentley/bvector.h>

//__PUBLISH_SECTION_END__
#include <DgnPlatform/DgnHandlers/MdlTextInternal.h>
//__PUBLISH_SECTION_START__

DGNPLATFORM_TYPEDEFS (TextNodeHandler)
DGNPLATFORM_TYPEDEFS (TextElemHandler)
DGNPLATFORM_TYPEDEFS (AlongTextLinkageData)
DGNPLATFORM_TYPEDEFS (WhiteSpaceBitmaskValueVector)
DGNPLATFORM_TYPEDEFS (EDFieldVector)

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//__PUBLISH_SECTION_END__

struct DimensionHandler;

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     08/08
//=======================================================================================
enum ProcessTextPropertiesStatus
{
ProcessTextPropertiesStatus_LayoutIndepedentOnly,
ProcessTextPropertiesStatus_All,
ProcessTextPropertiesStatus_None

}; // ProcessTextPropertiesStatus

//__PUBLISH_SECTION_START__

//=======================================================================================
// Base class for text element handlers; should NOT be sub-classed directly (see TextNodeHandler and TextElemHandler).
// @note The best way to create TextBlock objects from elements is via ITextQuery::GetText (supports all text-based elements); if you know you are dealing with a text or text node element, you can use GetFirstTextPartValue as a convenience.
// @note Create elements from TextBlock objects via TextHandlerBase::CreateElement.
// @bsiclass                                                    Venkat.Kalyan   11/06
//=======================================================================================
struct TextHandlerBase : DisplayHandler, ITextEdit
{
//__PUBLISH_SECTION_END__
private:
friend struct DimensionHandler; 
friend struct DgnTextSnippetHandler;

typedef std::valarray<byte> ByteValArray;

private:

                    static  bool                        RemapRunProperties                  (RunPropertiesR, PropertyContextR, TextBlockCR, TextParamWide const &);

protected:

                    static  void                        ProcessLayoutPropertiesByTextBlock  (EditElementHandleR, PropertyContextR);
                            ProcessTextPropertiesStatus ProcessTextPropertiesByElement      (PropertyContextR, ElementHandleCR, EditElementHandleP, UInt32* originalColor);
                    static  void                        SetOverridesFromStyle               (PropertyContextR, EditElementHandleR);
                    
                    static  BentleyStatus               GetWhiteSpaceBitmaskValues          (ElementHandleCR eh, WhiteSpaceBitmaskValueVectorR whiteSpaceValues);
                    static  BentleyStatus               SetWhiteSpaceBitmaskValues          (EditElementHandleR eeh, WhiteSpaceBitmaskValueVectorCP whiteSpaceValues);
                    static  BentleyStatus               GetIndentationData                  (ElementHandleCR eh, IndentationDataR indentationData);
                    static  BentleyStatus               SetIndentationData                  (EditElementHandleR eeh, IndentationDataCP indentationData);

// DisplayHandler
DGNPLATFORM_EXPORT virtual  StatusInt                   _OnDrop                             (ElementHandleCR eh, ElementAgendaR dropGeom, DropGeometryCR geometry) override;

// ITextEdit
DGNPLATFORM_EXPORT virtual  bool                        _IsTextElement                      (ElementHandleCR) const override;
DGNPLATFORM_EXPORT virtual  bool                        _DoesSupportFields                  (ElementHandleCR) const override;
DGNPLATFORM_EXPORT virtual  ITextPartIdPtr              _GetTextPartId                      (ElementHandleCR, HitPathCR) const override;
DGNPLATFORM_EXPORT virtual  TextBlockPtr                _GetTextPart                        (ElementHandleCR, ITextPartIdCR) const override;
DGNPLATFORM_EXPORT virtual  ReplaceStatus               _ReplaceTextPart                    (EditElementHandleR, ITextPartIdCR, TextBlockCR) override;

public:
                    static  bool                        ProcessLayoutPropertiesDirect       (TextParamWideR, DPoint2dR textSize, bool isTextNode, int textNodeLineLength, PropertyContextR, bool canChange);
                    static  bool                        ProcessNonLayoutPropertiesDirect    (TextParamWideR, int lineLength, DPoint2dR textSize, PropertyContextR, bool canChange);

                    static  void                        SetOverridesFromStyle               (PropertyContextR, TextParamWide&, DPoint2d textSize, int lineLength, bool isTextNode);
                    static  void                        ConvertTextParamsTo2D               (double& lngthMult, double& hghtMult, double& rotation, double oldLngthMult, double oldHghtMult, double const & quaternion);
DGNPLATFORM_EXPORT  static  BentleyStatus               UpdateAnnotationScale               (EditElementHandleR eeh, AnnotationScaleAction action, double newAnnotationScale, bool forceTextNode);
DGNPLATFORM_EXPORT  static  bool                        AreEqual                            (ElementHandleCR lhsEh, ElementHandleCR rhsEh);
DGNPLATFORM_EXPORT  static  bool                        TransformTextParams                 (TextParamWideR textParams, DPoint2dCR scaleFactor, bool isTextNode, bool allowSizeChange);
DGNPLATFORM_EXPORT  static  double                      ConvertMultToScale                  (double mult);
                    static  double                      ConvertScaleToMult                  (double scale);
DGNPLATFORM_EXPORT  static  TextElementJustification    RemapNodeToTextJustification        (TextElementJustification nodeJustification);
DGNPLATFORM_EXPORT  static  void                        CreateTemplateTextElemFromNode      (EditElementHandleR outTextElementEeh, ElementHandleCR inTextNodeElementEh);
DGNPLATFORM_EXPORT  static  void                        CreateTemplateNodeFromTextElem      (EditElementHandleR outTextNodeElementEeh, ElementHandleCR inTextElementEh);
                    static  void                        FillTextParamsFromLinkages          (ElementHandleCR, TextParamWideR);
                    static  void                        AppendLinkagesFromTextParams        (EditElementHandleR, TextParamWideCR);
                    static  void                        AppendLinkagesFromTextParams        (DgnElementR, TextParamWideCR);
DGNPLATFORM_EXPORT  static  void                        RemoveTextParamLinkages             (EditElementHandleR);
                    static  void                        RemoveTextParamLinkages             (DgnElementR);
DGNPLATFORM_EXPORT  static  bool                        RemoveWhiteSpaceLinkages            (EditElementHandleR eehToProcess);
DGNPLATFORM_EXPORT  static  void                        RemoveTextDataLinkages              (EditElementHandleR eehToProcess);
DGNPLATFORM_EXPORT  static  bool                        HasMTextRenderingLinkage            (ElementHandleCR eh);

DGNPLATFORM_EXPORT  static BentleyStatus                SetupOffsetAssociation (EditElementHandleR textElement, ElementHandleCR targetElement, AssocPoint const& assoc);

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

//! Creates a new text element from the given TextBlock.
//! \note   If no template element is provided (or it is not a text or text node element), you will get a type 17 text element unless a type 7 text node in required (based on the contents of the TextBlock). If a template element is provided, and it is a text node, the resulting element will be a text node regardless of whether one is actually required (if it is a text element, you may still get a node depending on the contents of the TextBlock).
//! \note   You cannot create elements from a TextString object; see TextBlock.
//! \note   The DgnModelP used is that of the provided TextBlock.
DGNPLATFORM_EXPORT static TextBlockToElementResult CreateElement (EditElementHandleR, ElementHandleCP templateEh, TextBlockCR);

//! A convenience wrapper around ITextQuery::GetText that gets the value of the first text part.
//! @note While normal text and text node elements only expose a single text part, other text-enabled elements may expose more, so this is <b>NOT</b> always safe to use.
//! @see ITextQuery::IsTextElement to determine if an element is a normal text or text node element.
DGNPLATFORM_EXPORT static TextBlockPtr GetFirstTextPartValue (ElementHandleCR);

//! Drops a text element, and if it was view-independent, optionally orients it to the provided (view) rotation.
//! @remarks This essentially replaces the former mdlText_strokeToElementDescr function.
DGNPLATFORM_EXPORT static BentleyStatus DropText (ElementHandleCR, ElementAgendaR, RotMatrixCR);

}; // TextHandlerBase

//__PUBLISH_SECTION_END__

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  08/12
+===============+===============+===============+===============+===============+======*/
struct TextHandlerBasePathEntryExtension : IDisplayHandlerPathEntryExtension
{
DGNPLATFORM_EXPORT virtual void _PushDisplayEffects (ElementHandleCR, ViewContextR) override;
virtual bool _GetElemHeaderOverrides (ElementHandleCR el, ElemHeaderOverridesR ovr) override {return false;}
};

//=======================================================================================
// Encapsulates the data stored on an along-text dependency linkage.
// @bsiclass                                                    Jeff.Marker     02/09
//=======================================================================================
struct AlongTextLinkageData
    {
    //=======================================================================================
    // @bsiclass                                                    Jeff.Marker     03/09
    //=======================================================================================
    public: struct CustomDependencyData
        {
        double      m_startOffsetAlongElement;
        double      m_distanceFromElement;
        DPoint3d    m_startPoint;
        
        struct
            {
            ULong   m_areParametersUsed :1;
            ULong   m_isBelowText       :1;
            ULong   m_useStartPoint     :1;
            ULong   m_unused            :29;
        
            } m_parameters;

        ULong m_reserved;
        
        }; // CustomDependencyData

    public: UInt64               m_rootId;
    public: UInt64               m___unusedref;
    public: CustomDependencyData    m_customDependencyData;

    }; // AlongTextLinkageData

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     02/09
//=======================================================================================
struct WhiteSpaceBitmaskValueVector : bvector<WhiteSpaceBitmaskValue> { };

/// @addtogroup DisplayHandler
/// @beginGroup

//__PUBLISH_SECTION_START__
//=======================================================================================
// The default MicroStation handler for text nodes.
// @remarks This class contains several static Get/Set/Create methods. They are low-level methods, and you should only use them if you know what you're doing -- i.e. they will let you make internally inconsistent elements! You should consider using the TextBlock API instead; aside from providing a more robust API, it will handle multi-line/multi-format text layout, and has the validation/processing necessary to create proper elements. Additionally, you should use TextBlock (or TextString) if you want to draw text (e.g. do NOT create elements just to draw text).
// @bsiclass                                                    KeithBentley    04/01
//=======================================================================================
struct TextNodeHandler :    TextHandlerBase,
                            //ITransactionHandler, removed in Graphite
                            IAnnotationHandler
{
    DEFINE_T_SUPER(TextHandlerBase)    
ELEMENTHANDLER_DECLARE_MEMBERS (TextNodeHandler, DGNPLATFORM_EXPORT)
//__PUBLISH_SECTION_END__
private:

    friend struct TextHandlerBase;

public:

                    static  void                    VisitNodeChildren               (ElementHandleCR, ViewContextR);

private:

                    static  BentleyStatus           TransformTextNode               (EditElementHandleR eeh, TransformCR transform, UInt32 options);

protected:

                            void                    DrawNodeNumber                  (ElementHandleCR, ViewContextR);

// Handler
DGNPLATFORM_EXPORT virtual  void                    _EditProperties                 (EditElementHandleR, PropertyContextR) override;
//DGNPLATFORM_EXPORT virtual  ITransactionHandlerP    _GetITransactionHandler         () override; removed in Graphite
DGNPLATFORM_EXPORT virtual  void                    _GetTypeName                    (WStringR, UInt32 desiredLength) override;
DGNPLATFORM_EXPORT virtual  bool                    _IsSupportedOperation           (ElementHandleCP, SupportOperation) override;
DGNPLATFORM_EXPORT virtual  bool                    _IsTransformGraphics            (ElementHandleCR, TransformInfoCR) override;
DGNPLATFORM_EXPORT virtual  void                    _OnConvertTo2d                  (EditElementHandleR, TransformCR flattenTrans, DVec3dCR flattenDir) override;
DGNPLATFORM_EXPORT virtual  void                    _OnConvertTo3d                  (EditElementHandleR, double elevation) override;
//DGNPLATFORM_EXPORT virtual  StatusInt               _OnPreprocessCopy               (EditElementHandleR, ElementCopyContextP) override; removed in Graphite
DGNPLATFORM_EXPORT virtual  StatusInt               _OnTransform                    (EditElementHandleR, TransformInfoCR) override;
DGNPLATFORM_EXPORT virtual  void                    _QueryProperties                (ElementHandleCR, PropertyContextR) override;

// DispayHandler
DGNPLATFORM_EXPORT virtual  void                    _Draw                           (ElementHandleCR, ViewContextR) override;
DGNPLATFORM_EXPORT virtual  void                    _DrawFiltered                   (ElementHandleCR, ViewContextR, DPoint3dCP pts, double size) override;
DGNPLATFORM_EXPORT virtual  bool                    _IsPlanar                       (ElementHandleCR, DVec3dP normal, DPoint3dP point, DVec3dCP inputDefaultNormal) override;
DGNPLATFORM_EXPORT virtual  IAnnotationHandlerP     _GetIAnnotationHandler          (ElementHandleCR) override;
DGNPLATFORM_EXPORT virtual  void                    _GetOrientation                 (ElementHandleCR, RotMatrixR) override;
DGNPLATFORM_EXPORT virtual  void                    _GetSnapOrigin                  (ElementHandleCR, DPoint3dR origin) override;
DGNPLATFORM_EXPORT virtual  void                    _GetTransformOrigin             (ElementHandleCR, DPoint3dR) override;
DGNPLATFORM_EXPORT virtual  ReprojectStatus         _OnGeoCoordinateReprojection    (EditElementHandleR, IGeoCoordinateReprojectionHelper&, bool inChain) override;
DGNPLATFORM_EXPORT virtual  SnapStatus              _OnSnap                         (SnapContextP, int snapPathIndex) override;

// ITextEdit
DGNPLATFORM_EXPORT virtual  void                    _GetTextPartIds                 (ElementHandleCR, ITextQueryOptionsCR, T_ITextPartIdPtrVectorR) const override;

// ITransactionHandler
//DGNPLATFORM_EXPORT virtual  PreActionStatus         _OnReplace                      (EditElementHandleR, ElementHandleCR) override; removed in Graphite
DGNPLATFORM_EXPORT virtual PreActionStatus _OnModify (EditElementHandleR, ElementHandleCR) override; // added in Graphite

// IAnnotationHandler
DGNPLATFORM_EXPORT virtual  StatusInt               _ComputeAnnotationScaledRange   (ElementHandleCR, DRange3dR elemRangeOut, double scaleFactor) override;
DGNPLATFORM_EXPORT virtual  bool                    _GetAnnotationScale             (double* annotationScale, ElementHandleCR) const override;

public:

// ***** YOU SHOULD BE USING TextBlock INSTEAD OF THESE METHODS *****
// These Get/Set methods are low-level element-based functions; they do NOT validate arguments for internal consistency, and you can create BAD ELEMENTS using them directly!
DGNPLATFORM_EXPORT  static  BentleyStatus           GetUserOrigin                   (ElementHandleCR eh, DPoint3dR userOrigin);
DGNPLATFORM_EXPORT  static  BentleyStatus           SetUserOrigin                   (EditElementHandleR eeh, DPoint3dCR userOrigin, bool reValidateRange);
DGNPLATFORM_EXPORT  static  BentleyStatus           GetOrientation                  (ElementHandleCR eh, RotMatrixR orientation);
DGNPLATFORM_EXPORT  static  BentleyStatus           SetOrientation                  (EditElementHandleR eeh, RotMatrixCR orientation, bool reValidateRange);
DGNPLATFORM_EXPORT  static  BentleyStatus           GetFontSize                     (ElementHandleCR eh, DPoint2dR textSize);
DGNPLATFORM_EXPORT  static  BentleyStatus           SetFontSize                     (EditElementHandleR eeh, DPoint2dCR textSize, bool reValidateRange);
DGNPLATFORM_EXPORT  static  BentleyStatus           GetTextParams                   (ElementHandleCR eh, TextParamWideR textParams);
DGNPLATFORM_EXPORT  static  BentleyStatus           SetTextParams                   (EditElementHandleR eeh, TextParamWide textParams, bool reValidateRange);
DGNPLATFORM_EXPORT  static  BentleyStatus           GetMaxCharsPerLine              (ElementHandleCR eh, UInt16& maxCharsPerLine);
DGNPLATFORM_EXPORT  static  BentleyStatus           SetMaxCharsPerLine              (EditElementHandleR eeh, UInt16 maxCharsPerLine);
DGNPLATFORM_EXPORT  static  BentleyStatus           GetNodeNumber                   (ElementHandleCR eh, UInt32& nodeNumber);
DGNPLATFORM_EXPORT  static  BentleyStatus           SetNodeNumber                   (EditElementHandleR eeh, UInt32 nodeNumber);
DGNPLATFORM_EXPORT  static  bool                    HasAlongTextData                (ElementHandleCR eh);
DGNPLATFORM_EXPORT  static  BentleyStatus           GetAlongTextData                (ElementHandleCR eh, AlongTextLinkageDataR alongTextData);
DGNPLATFORM_EXPORT  static  BentleyStatus           SetAlongTextData                (EditElementHandleR eeh, AlongTextLinkageDataCP alongTextData);
DGNPLATFORM_EXPORT  static  BentleyStatus           GetWhiteSpaceBitmaskValues      (ElementHandleCR eh, WhiteSpaceBitmaskValueVectorR whiteSpaceValues);
DGNPLATFORM_EXPORT  static  BentleyStatus           SetWhiteSpaceBitmaskValues      (EditElementHandleR eeh, WhiteSpaceBitmaskValueVectorCP whiteSpaceValues);
DGNPLATFORM_EXPORT  static  BentleyStatus           GetIndentationData              (ElementHandleCR eh, IndentationDataR indentationData);
DGNPLATFORM_EXPORT  static  BentleyStatus           SetIndentationData              (EditElementHandleR eeh, IndentationDataCP indentationData);

DGNPLATFORM_EXPORT  static  bool                    AreEqual                        (ElementHandleCR lhsEh, ElementHandleCR rhsEh);
DGNPLATFORM_EXPORT  static  BentleyStatus           CreateElement                   (EditElementHandleR eeh, ElementHandleCP templateEh, DPoint3dCR userOrigin, RotMatrixCR orientation, DPoint2dCR textSize, TextParamWide textParams, UInt16 maxLineLength, bool is3d, DgnModelR modelRef);

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__

}; // TextNodeHandler

//__PUBLISH_SECTION_END__
//=======================================================================================
// @bsiclass                                                    Jeff.Marker     01/09
//=======================================================================================
struct EDFieldVector : bvector<TextEDField> { };
//__PUBLISH_SECTION_START__

//=======================================================================================
// The default MicroStation handler for text nodes.
// @remarks This class contains several static Get/Set/Create methods. They are low-level methods, and you should only use them if you know what you're doing -- i.e. they will let you make internally inconsistent elements! You should consider using the TextBlock API instead; aside from providing a more robust API, it will handle multi-line/multi-format text layout, and has the validation/processing necessary to create proper elements. Additionally, you should use TextBlock (or TextString) if you want to draw text (e.g. do NOT create elements just to draw text).
// @bsiclass                                                    KeithBentley    04/01
//=======================================================================================
struct TextElemHandler  :   TextHandlerBase,
                            IAnnotationHandler
{
    DEFINE_T_SUPER(TextHandlerBase)    
    ELEMENTHANDLER_DECLARE_MEMBERS (TextElemHandler, DGNPLATFORM_EXPORT)
//__PUBLISH_SECTION_END__
private:

    friend struct TextHandlerBase;

public:

                            ProcessTextPropertiesStatus     ProcessPropertiesByElement      (EditElementHandleR, PropertyContextR);

private:

                    static  BentleyStatus                   TransformTextElement            (EditElementHandleR eeh, TransformCR transform, UInt32 options);

protected:

// Handler
DGNPLATFORM_EXPORT virtual  void                            _EditProperties                 (EditElementHandleR, PropertyContextR) override;
DGNPLATFORM_EXPORT virtual  void                            _GetDescription                 (ElementHandleCR, WStringR, UInt32 desiredLength) override;
DGNPLATFORM_EXPORT virtual  void                            _GetTypeName                    (WStringR, UInt32 desiredLength) override;
DGNPLATFORM_EXPORT virtual  bool                            _IsTransformGraphics            (ElementHandleCR, TransformInfoCR) override;
DGNPLATFORM_EXPORT virtual  void                            _OnConvertTo2d                  (EditElementHandleR, TransformCR flattenTrans, DVec3dCR flattenDir) override;
DGNPLATFORM_EXPORT virtual  void                            _OnConvertTo3d                  (EditElementHandleR, double elevation) override;
//DGNPLATFORM_EXPORT virtual  StatusInt                       _OnPostprocessCopyRemapRestore  (EditElementHandleR, DgnModelP) override; removed in graphite
//DGNPLATFORM_EXPORT virtual  StatusInt                       _OnPreprocessCopy               (EditElementHandleR, ElementCopyContextP) override; removed in graphite
DGNPLATFORM_EXPORT virtual  StatusInt                       _OnTransform                    (EditElementHandleR, TransformInfoCR) override;
DGNPLATFORM_EXPORT virtual  void                            _QueryProperties                (ElementHandleCR, PropertyContextR) override;

// DispayHandler
DGNPLATFORM_EXPORT virtual  void                            _Draw                           (ElementHandleCR, ViewContextR) override;
DGNPLATFORM_EXPORT virtual  void                            _DrawFiltered                   (ElementHandleCR, ViewContextR, DPoint3dCP pts, double size) override;
DGNPLATFORM_EXPORT virtual  bool                            _IsPlanar                       (ElementHandleCR, DVec3dP normal, DPoint3dP point, DVec3dCP inputDefaultNormal) override;
DGNPLATFORM_EXPORT virtual  void                            _GetElemDisplayParams           (ElementHandleCR, ElemDisplayParams&, bool wantMaterials) override;
DGNPLATFORM_EXPORT virtual  IAnnotationHandlerP             _GetIAnnotationHandler          (ElementHandleCR) override;
DGNPLATFORM_EXPORT virtual  void                            _GetOrientation                 (ElementHandleCR, RotMatrixR) override;
DGNPLATFORM_EXPORT virtual  void                            _GetSnapOrigin                  (ElementHandleCR, DPoint3dR origin) override;
DGNPLATFORM_EXPORT virtual  void                            _GetTransformOrigin             (ElementHandleCR, DPoint3dR) override;
DGNPLATFORM_EXPORT virtual  ReprojectStatus                 _OnGeoCoordinateReprojection    (EditElementHandleR, IGeoCoordinateReprojectionHelper&, bool inChain) override;
DGNPLATFORM_EXPORT virtual  SnapStatus                      _OnSnap                         (SnapContextP, int snapPathIndex) override;

// ITextEdit
DGNPLATFORM_EXPORT virtual  void                            _GetTextPartIds                 (ElementHandleCR, ITextQueryOptionsCR, T_ITextPartIdPtrVectorR) const override;

// IAnnotationHandler
DGNPLATFORM_EXPORT virtual  StatusInt                       _ComputeAnnotationScaledRange   (ElementHandleCR, DRange3dR elemRangeOut, double scaleFactor) override;
DGNPLATFORM_EXPORT virtual  bool                            _GetAnnotationScale             (double* annotationScale, ElementHandleCR eh) const override;

// TextElemHandler
DGNPLATFORM_EXPORT virtual  BentleyStatus                   _InitTextString                 (ElementHandleCR, TextStringR);

public:

// ***** YOU SHOULD BE USING TextBlock INSTEAD OF THESE METHODS *****
// These Get/Set methods are low-level element-based functions; they do NOT validate arguments for internal consistency, and you can create BAD ELEMENTS using them directly!
DGNPLATFORM_EXPORT  static  BentleyStatus                   GetElementOrigin                (ElementHandleCR eh, DPoint3dR elementOrigin);
DGNPLATFORM_EXPORT  static  BentleyStatus                   SetElementOrigin                (EditElementHandleR eeh, DPoint3dCR elementOrigin, bool reValidateRange);
DGNPLATFORM_EXPORT  static  BentleyStatus                   ComputeUserOrigin               (ElementHandleCR eh, DPoint3dR userOrigin);
DGNPLATFORM_EXPORT  static  BentleyStatus                   GetOrientation                  (ElementHandleCR eh, RotMatrixR orientation);
DGNPLATFORM_EXPORT  static  BentleyStatus                   SetOrientation                  (EditElementHandleR eeh, RotMatrixCR orientation, bool reValidateRange);
DGNPLATFORM_EXPORT  static  BentleyStatus                   GetFontSize                     (ElementHandleCR eh, DPoint2dR textSize);
DGNPLATFORM_EXPORT  static  BentleyStatus                   SetFontSize                     (EditElementHandleR eeh, DPoint2dCR textSize, bool reValidateRange);
DGNPLATFORM_EXPORT  static  BentleyStatus                   GetTextParams                   (ElementHandleCR eh, TextParamWideR textParams);
DGNPLATFORM_EXPORT  static  BentleyStatus                   SetTextParams                   (EditElementHandleR eeh, TextParamWide textParams, bool reValidateRange);
DGNPLATFORM_EXPORT  static  BentleyStatus                   GetFontChars                    (ElementHandleCR eh, bvector<FontChar>&);
DGNPLATFORM_EXPORT  static  BentleyStatus                   GetString                       (ElementHandleCR eh, WStringR unicodeString);
DGNPLATFORM_EXPORT  static  BentleyStatus                   GetString                       (ElementHandleCR eh, WStringR unicodeString, DgnFontCR fontForCodePage);
DGNPLATFORM_EXPORT  static  BentleyStatus                   SetString                       (EditElementHandleR eeh, WCharCP unicodeString, TextParamWideCR textParams, DPoint2dCR textSize, bool reValidateRange);
DGNPLATFORM_EXPORT  static  BentleyStatus                   GetEDFields                     (ElementHandleCR eh, EDFieldVectorR edFields);
DGNPLATFORM_EXPORT  static  BentleyStatus                   SetEDFields                     (EditElementHandleR eeh, EDFieldVectorCP edFields);
DGNPLATFORM_EXPORT  static  BentleyStatus                   GetWhiteSpaceBitmaskValues      (ElementHandleCR eh, WhiteSpaceBitmaskValueVectorR whiteSpaceValues);
DGNPLATFORM_EXPORT  static  BentleyStatus                   SetWhiteSpaceBitmaskValues      (EditElementHandleR eeh, WhiteSpaceBitmaskValueVectorCP whiteSpaceValues);
DGNPLATFORM_EXPORT  static  BentleyStatus                   GetIndentationData              (ElementHandleCR eh, IndentationDataR indentationData);
DGNPLATFORM_EXPORT  static  BentleyStatus                   SetIndentationData              (EditElementHandleR eeh, IndentationDataCP indentationData);

DGNPLATFORM_EXPORT  static  bool                            AreEqual                        (ElementHandleCR lhsEh, ElementHandleCR rhsEh);

DGNPLATFORM_EXPORT  static  BentleyStatus                   CreateElement                   (EditElementHandleR eeh, ElementHandleCP templateEh, DPoint3dCR elementOrigin, RotMatrixCR orientation, DPoint2dCR textSize, TextParamWideCR textParams, WCharCP unicodeString, bool is3d, DgnModelR modelRef);
DGNPLATFORM_EXPORT  static  BentleyStatus                   CreateElementFromShapeCodes     (EditElementHandleR eeh, ElementHandleCP templateEh, DPoint3dCR elementOrigin, RotMatrixCR orientation, DPoint2dCR textSize, TextParamWideCR textParams, UInt8 const* shapeCodes, size_t numShapeCodes, bool is3d, DgnModelR modelRef);

                    static  void                            DrawTextWithThickness           (ElementHandleCR eh, ViewContextR context, UInt32 qvIndex);

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

//! Initializes a TextString to represent a text element. Any existing data in the TextString is cleared before use.
//! \note   This is here on TextElemHandler instead of TextHandlerBase because you cannot make TextString objects out of text nodes; see TextBlock for more generic support.
DGNPLATFORM_EXPORT BentleyStatus InitTextString (ElementHandleCR, TextStringR);

}; // TextElemHandler

/// @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE

#endif
/** @endcond */
