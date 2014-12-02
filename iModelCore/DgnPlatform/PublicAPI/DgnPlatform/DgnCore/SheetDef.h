/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/SheetDef.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include "DgnElements.h"
#include "UnitDefinition.h"
#include <Bentley/WString.h>

#define SHEET_DEF_ORIGIN_ATTRIBUTE                              0
#define SHEET_DEF_ROTATION_ATTRIBUTE                            1
#define SHEET_DEF_WIDTH_ATTRIBUTE                               2
#define SHEET_DEF_HEIGHT_ATTRIBUTE                              3
#define SHEET_DEF_COLOR_ATTRIBUTE                               4
#define SHEET_DEF_UNIT_ATTRIBUTE                                5
#define SHEET_DEF_ENABLED_ATTRIBUTE                             6
#define SHEET_DEF_DRAWING_SCALE_ATTRIBUTE                       7
#define SHEET_DEF_GEOMETRY_SCALE_ATTRIBUTE                      8
#define SHEET_DEF_ANNOTATION_SCALE_ATTRIBUTE                    9
#define SHEET_DEF_GEOMETRY_AT_DRAWING_SCALE_ATTRIBUTE           10
#define SHEET_DEF_PAPER_ORIGIN_ATTRIBUTE                        11
#define SHEET_DEF_PAPER_ROTATION_ATTRIBUTE                      12
#define SHEET_DEF_PAPER_WIDTH_ATTRIBUTE                         13
#define SHEET_DEF_PAPER_HEIGHT_ATTRIBUTE                        14
#define SHEET_DEF_PAPER_UNIT_ATTRIBUTE                          15
#define SHEET_DEF_TOP_PAPER_MARGIN_ATTRIBUTE                    16
#define SHEET_DEF_LEFT_PAPER_MARGIN_ATTRIBUTE                   17
#define SHEET_DEF_BOTTOM_PAPER_MARGIN_ATTRIBUTE                 18
#define SHEET_DEF_RIGHT_PAPER_MARGIN_ATTRIBUTE                  19
#define SHEET_DEF_PLOT_SCALE_ATTRIBUTE                          20
#define SHEET_DEF_FORM_NAME_ATTRIBUTE                           21
#define SHEET_DEF_PAPER_FORM_NAME_ATTRIBUTE                     22
#define SHEET_DEF_WINDOWS_PRINTER_NAME_ATTRIBUTE                23
#define SHEET_DEF_PLT_FILE_NAME_ATTRIBUTE                       24
#define SHEET_DEF_PST_FILE_NAME_ATTRIBUTE                       25
#define SHEET_DEF_SHEET_NUMBER_ATTRIBUTE                        26
#define SHEET_DEF_BORDER_ATTACHMENT_ID_ATTRIBUTE                27
#define SHEET_DEF_SHEET_NAME_ATTRIBUTE                          28
#define SHEET_DEF_PRINT_STYLE_NAME_ATTRIBUTE                    29

#include <map>
#include <Bentley/bvector.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//__PUBLISH_SECTION_END__
typedef struct msSheetPropertiesLinkageData MSSheetPropertiesLinkageData;

//__PUBLISH_SECTION_START__

//=======================================================================================
//! Sheet definition data
//! @bsiclass
//=======================================================================================
struct  SheetDef
{
//__PUBLISH_SECTION_END__
    friend struct SheetDefTestBackdoor;

private:
    struct  SheetProperties
    {
        SheetProperties ();
        SheetProperties (SheetProperties const&);
        ~SheetProperties ();
        SheetProperties& operator= (SheetProperties const& from);
        SheetProperties& CopyFrom (SheetProperties const& from);

        void                ExtractLinkageV0 (MSSheetPropertiesLinkageData const *);
        void                ExtractLinkageV1 (MSSheetPropertiesLinkageData const *);
        StatusInt           ExtractLinkage (DgnElementCR);
        StatusInt           ExtractLinkageEx (DgnElementCR);
        StatusInt           SetLinkageEx (DgnElementR) const;
        StatusInt           AppendLinkageEx (DgnElementR) const;
        StatusInt           SetLinkage (DgnElementR) const;
        StatusInt           AppendLinkage (DgnElementR) const;
        StatusInt           ToElement (DgnElementR element) const;
        StatusInt           FromElement (DgnElementCR elementIn);

        DGNPLATFORM_EXPORT void GetString (WString&, UInt16) const;
        WString const*      GetString (UInt16) const;
        void                SetString (UInt16, WChar const*);
        bool                AnySet () const;

        UInt16              version;
        UInt16              isEnabled:1;
        UInt16              reservedBits:15;
        DPoint2d            origin;
        double              rotation;
        double              width;
        double              height;
        UInt32              color;
        UnitDefinition      unitDef;
        UInt32              reservedInt;
        UInt32              reservedInt2;
        DPoint2d            paperOrigin;
        double              paperRotation;
        double              paperWidth;
        double              paperHeight;
        UnitDefinition      paperUnitDef;
        double              topPaperMargin;
        double              leftPaperMargin;
        double              bottomPaperMargin;
        double              rightPaperMargin;
        double              plotScale;

        std::map<UInt16,WString> m_strings;

        UInt32              sheetNumber;

        ElementId           borderAttachmentId;

        bvector<byte>   m_extraLinkage;
    };

    SheetProperties     properties;

    mutable ElementRef* m_borderElementRef;

    void                SetBorderElementRef (ElementRef* elemRef) const {m_borderElementRef = elemRef;}
    ElementRef*     GetBorderElementRef () const {return m_borderElementRef;}

    SheetDef&           CopyFrom (SheetDef const& from);
    StatusInt           SaveProperties (DgnElementR element) const;
    StatusInt           FromElement (DgnElementCR elementIn);

    friend struct              ModelInfo;

public:

DGNPLATFORM_EXPORT void         Draw (ViewContextP) const;
DGNPLATFORM_EXPORT static void  DisplaySheetBorder (ViewContextP);

DGNPLATFORM_EXPORT UInt32         GetColor () const; // Color no longer used for display...
DGNPLATFORM_EXPORT void           SetColor (UInt32 sheetColor);

DGNPLATFORM_EXPORT SheetDef ();
DGNPLATFORM_EXPORT SheetDef (SheetDef const&);
DGNPLATFORM_EXPORT ~SheetDef ();
//! @private
DGNPLATFORM_EXPORT SheetDef& operator= (SheetDef const& from);

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

//! Compare if sheet related data between two sheet defs.
//!     The function will compare only those attributes as specified in compareAttributesMaskIn
//!     If compareAttributesMaskIn is NULL, then all the attributes will be compared.
//! @param[out]     compareBitMaskOut           If non-null, then return a set of bits that represent the attributes that are not the same.
//! @param[in]      sheetDef2                   Sheet definition 2
//! @param[in]      compareAttributesMaskIn     Mask of attributes to compare
//! @param[in]      distTol                     Tolerance to use when comparing distances (e.g., point coordinates). Pass 0.0 to require an exact match for distances.
//! @Return true if the sheet data of the two sheet defs matches for attributes specified in compareAttributesMaskIn
//! @bsimethod
DGNPLATFORM_EXPORT bool CompareData (BitMaskP compareBitMaskOut, SheetDef const& sheetDef2, BitMaskCP compareAttributesMaskIn, double distTol) const;

//! Check if the sheet related information is enabled.
//! @Return true if the sheet related information is enabled, else false
//! @bsimethod
DGNPLATFORM_EXPORT bool IsEnabled () const;

//! Set if the sheet related information is to be enabled.
//! @param[in]  isEnabled         true if the sheet related information is to be enabled.
//! @bsimethod
DGNPLATFORM_EXPORT void Enable (bool isEnabled);

//! Set the Sheet name
DGNPLATFORM_EXPORT void SetSheetName (WChar const*);

//! Get the Sheet name
DGNPLATFORM_EXPORT void GetSheetName (WString&) const;

//! Get origin - the lower left hand corner of
//! the sheet relative to the sheet model's origin.
//! @param[out]  sheetOrigin       sheet origin
//! @Remarks Coordinates are in UORs
//! @bsimethod
DGNPLATFORM_EXPORT void GetOrigin (DPoint2d& sheetOrigin) const;

//! Set its origin - the lower left hand corner of
//!     the sheet relative to the sheet model's origin.
//! @param[in]  sheetOrigin       sheet origin
//! @Remarks Coordinates must be in UORs
//! @bsimethod
DGNPLATFORM_EXPORT void SetOrigin (DPoint2d const& sheetOrigin);

//! Get rotation - relative to the X axis of the
//! sheet model in the XY plane.
//! @return sheet rotation in radians
//! @bsimethod
DGNPLATFORM_EXPORT double GetRotation () const;

//! Set rotation - relative to the X axis of the
//! sheet model in the XY plane.
//! @param[in]  sheetRotation     sheet rotation in radians
//! @bsimethod
DGNPLATFORM_EXPORT void SetRotation (double sheetRotation);

//! Get sheet size.
//! @param[out]  sheetWidth           sheet width
//! @param[out]  sheetHeight          sheet height
//! @Remarks Returned values are in UORs
//! @bsimethod
DGNPLATFORM_EXPORT void GetSize (double& sheetWidth, double& sheetHeight) const;

//! Set size.
//! @param[in]  sheetWidth        sheet width
//! @param[in]  sheetHeight       sheet height
//! @Remarks Input values must be in UORs
//! @bsimethod
DGNPLATFORM_EXPORT void SetSize (double sheetWidth, double sheetHeight);

//! Get its form name - this is a name associated with its size.
//! @param[out]  formName         sheet form name
//!  @bsimethod
DGNPLATFORM_EXPORT void GetFormName (WString& formName) const;

//! Set its form name - this is a name associated with its size.
//! @param[in]  formName          sheet form name
//! @bsimethod
DGNPLATFORM_EXPORT void SetFormName (WCharCP formName);

//! Get its units
//! @param[out]  unitDef         sheet units
//! @bsimethod
DGNPLATFORM_EXPORT void GetUnits (UnitDefinitionR unitDef) const;

//! Set its units
//! @param[in]  unitDef          sheet units
//! @bsimethod
DGNPLATFORM_EXPORT void SetUnits (UnitDefinitionCR unitDef);

//! Get its paper origin - the lower left hand corner of
//!     the paper relative to the sheet model's origin.
//! @param[out]  paperOrigin      paper origin
//! @bsimethod
DGNPLATFORM_EXPORT void GetPaperOrigin (DPoint2d& paperOrigin) const;

//! Set its paper origin - the lower left hand corner of
//!     the paper relative to the sheet model's origin.
//! @param[in]  paperOrigin       paper origin
//! @bsimethod
DGNPLATFORM_EXPORT void SetPaperOrigin (DPoint2d const& paperOrigin);

//! Get its paper rotation - relative to the X axis of the
//!     sheet model in the XY plane.
//! @return paper rotation in radians
//! @bsimethod
DGNPLATFORM_EXPORT double GetPaperRotation () const;

//! Set its paper rotation - relative to the X axis of the
//!     sheet model in the XY plane.
//! @param[in]  paperRotation     paper rotation in radians
//! @bsimethod
DGNPLATFORM_EXPORT void SetPaperRotation (double paperRotation);

//! Get paper size.
//! @param[out]  paperWidth           paper width
//! @param[out]  paperHeight          paper height
//! @Remarks Values are in UORs
//! @bsimethod
DGNPLATFORM_EXPORT void GetPaperSize (double& paperWidth, double& paperHeight) const;

//! Set paper size.
//! @param[in]  paperWidth        paper width
//! @param[in]  paperHeight       paper height
//! @Remarks Coordinates must be in UORs
//! @bsimethod
DGNPLATFORM_EXPORT void SetPaperSize (double paperWidth, double paperHeight);

//! Get its paper form name - this is a name associated
//!                 with its paper size.
//! @param[out]  formName         paper form name
//! @bsimethod
DGNPLATFORM_EXPORT void GetPaperFormName (WString& formName ) const;

//! Set its paper form name - this is a name associated
//!                 with its paper size.
//! @param[in]  formName          paper form name
//! @bsimethod
DGNPLATFORM_EXPORT void SetPaperFormName ( WChar const * formName );

//! Get its paper units
//! @param[out]  paperUnitDef       paper units
//! @bsimethod
DGNPLATFORM_EXPORT void GetPaperUnits ( UnitDefinitionR paperUnitDef ) const;

//! Set its paper units
//! @param[in]  paperUnitDef        paper units
//! @bsimethod
DGNPLATFORM_EXPORT void SetPaperUnits ( UnitDefinitionCR paperUnitDef );

//! Get its paper margins. The paper size minus its margins defines the
//!                 effective printable area.
//! @param[out]  topMargin        top paper margin
//! @param[out]  leftMargin       left paper margin
//! @param[out]  bottomMargin     bottom paper margin
//! @param[out]  rightMargin      right paper margin
//! @Remarks Values are in UORs
//! @bsimethod
DGNPLATFORM_EXPORT void GetPaperMargins (double& topMargin, double& leftMargin, double& bottomMargin, double& rightMargin) const;

//! Set its paper margins.
//! @param[in]  topMargin         top paper margin
//! @param[in]  leftMargin        left paper margin
//! @param[in]  bottomMargin      bottom paper margin
//! @param[in]  rightMargin       right paper margin
//! @Remarks Values must be in UORs
//! @bsimethod
DGNPLATFORM_EXPORT void SetPaperMargins (double topMargin, double leftMargin, double bottomMargin, double rightMargin);

//! Get the plot scale factor.
//! @Return plot scale factor
//! @Remarks Returned value is in UORs
//! @bsimethod
DGNPLATFORM_EXPORT double GetPlotScaleFactor () const;

//! Set plot scale factor.
//! @param[in]  scale        the new scale
//! @Return ERROR if the input value is zero or negative.
//! @Remarks Value must be in UORs
//! @bsimethod
DGNPLATFORM_EXPORT StatusInt SetPlotScaleFactor (double scale);

//! Get windows printer name
//! @param[out]  windowsPrinterName   windows printer name string
//! @bsimethod
DGNPLATFORM_EXPORT void GetWindowsPrinterName ( WString& windowsPrinterName ) const;

//! Set windows printer name
//! @param[in]  windowsPrinterName    windows printer name string
//! @bsimethod
DGNPLATFORM_EXPORT void SetWindowsPrinterName ( WChar const * windowsPrinterName );

//! Get plot file name
//! @param[out]  pltFileName          plot file name string
//! @bsimethod
DGNPLATFORM_EXPORT void GetPltFileName ( WString& pltFileName ) const;

//! Set plot file name
//! @param[in]  pltFileName           plot file name name string
//! @bsimethod
DGNPLATFORM_EXPORT void SetPltFileName ( WChar const * pltFileName );

//! Get the plot style table file name
//! @param[out]  pstFileName          plot style table file name string
//! @bsimethod
DGNPLATFORM_EXPORT void GetPlotStyleTableFileName (WString& pstFileName) const;

//! Set the plot style table file name
//! @param[in]  pstFileName           plot style table file name string
//! @bsimethod
DGNPLATFORM_EXPORT void SetPlotStyleTableFileName (WChar const * pstFileName);

//! Get the print style name
//! @param[out]  printStyleName       print style name string
//! @bsimethod
DGNPLATFORM_EXPORT void GetPrintStyleName (WString& printStyleName) const;

//! Set the print style name
//! @param[in]  printStyleName        print style name string
//! @bsimethod
DGNPLATFORM_EXPORT void SetPrintStyleName (WChar const * printStyleName);

//! Get the sheet number.
//! @Returns sheet number
//! @bsimethod
DGNPLATFORM_EXPORT UInt32 GetSheetNumber () const;

//! Set sheet number.
//! @param[in]  sheetNumber  sheet number
//! @bsimethod
DGNPLATFORM_EXPORT void SetSheetNumber (UInt32 sheetNumber);

//! Get the ElementId of the reference attachment element associated with this sheet.
//! @Returns sheet border attachment element ID
//! @bsimethod
DGNPLATFORM_EXPORT ElementId GetBorderAttachmentId () const;

//! Set the ElementId of the reference attachment element associated with this sheet.
//! When a reference attachment is associated with a sheet definition as its boarder,
//! the sheet definition is derived from the border attachment.
//! @Remarks The border attachment element must be in the sheet model.
//! When the SheetDef changes are propagated to the DgnModel, this SheetDefs properties will be synced with the BorderAttachment's SheetDef.
//! @param[in]  borderAttachmentId sheet border attachment element ID
//! @bsimethod
DGNPLATFORM_EXPORT void SetBorderAttachmentId (ElementId borderAttachmentId);

}; // SheetDef

//__PUBLISH_SECTION_END__
/*----------------------------------------------------------------------+
|                                                                       |
|   SheetScalesLinkage                                                  |
|                                                                       |
+----------------------------------------------------------------------*/
#define SHEET_SCALES_LINKAGE_LATEST_VERSION       0

typedef struct msSheetScalesLinkageData
{
    UInt16                  version;                        // linkage version number
    UInt16                  reservedShort;
    UInt32                  geometryAtDrawingScale:1;
    UInt32                  reservedBits:31;
    double                  drawingScale;
    double                  geometryScale;
    double                  annotationScale;
    UInt32                  color;
} MSSheetScalesLinkageData;

typedef struct msSheetScalesLinkage
{
    LinkageHeader             header;
    MSSheetScalesLinkageData  data;
} MSSheetScalesLinkage;

/*----------------------------------------------------------------------+
|                                                                       |
|   SheetPropertiesLinkage                                              |
|                                                                       |
+----------------------------------------------------------------------*/
#define SHEET_PROPERTIES_LINKAGE_LATEST_VERSION     1

#if defined (NEEDS_WORK_DGNITEM)
struct msSheetPropertiesLinkageData
{
    UInt16                  version;                // linkage version number
    UInt16                  isEnabled:1;
    UInt16                  reservedBits:15;
    UInt32                  reservedInt;
    DPoint2d                origin;
    double                  rotation;
    double                  width;
    double                  height;
    UInt32                  color;
    UInt32                  reservedInt2;
    UnitInfo                unitInfo;

    DPoint2d                paperOrigin;
    double                  paperRotation;
    double                  paperWidth;
    double                  paperHeight;
    StoredUnitInfo          paperUnitInfo;
    double                  topPaperMargin;
    double                  leftPaperMargin;
    double                  bottomPaperMargin;
    double                  rightPaperMargin;

    double                  plotScale;
};

typedef struct msSheetPropertiesLinkage
{
    LinkageHeader                   header;
    MSSheetPropertiesLinkageData    data;
} MSSheetPropertiesLinkage;

typedef struct msSheetPropertiesLinkageDataEx
{
    UInt32                  sheetNumber;
    ElementId               borderAttachmentId;
} MSSheetPropertiesLinkageDataEx;

typedef struct msSheetPropertiesLinkageEx
{
    LinkageHeader                     header;
    MSSheetPropertiesLinkageDataEx    data;
} MSSheetPropertiesLinkageEx;

/*================================================================================**//**
* Only used to expose private members for testing.
* @bsiclass                                                     Kevin.Nyman     11/11
+===============+===============+===============+===============+===============+======*/
struct SheetDefTestBackdoor
{
    DGNPLATFORM_EXPORT static void                SetBorderElementRef (SheetDefR sheetDef, ElementRef* elemRef) {sheetDef.SetBorderElementRef (elemRef);}
    DGNPLATFORM_EXPORT static ElementRef*     GetBorderElementRef (SheetDefR sheetDef) {return sheetDef.GetBorderElementRef();}
};
#endif

//__PUBLISH_SECTION_START__
END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
