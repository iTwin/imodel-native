/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/ScanCriteria.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include "ElementHandle.h"
//__PUBLISH_SECTION_END__
#include "DgnModel.h"
//__PUBLISH_SECTION_START__

BEGIN_BENTLEY_API_NAMESPACE

//! These values define the data returned by the mdlScanCriteria_filter function.
//! They are passed to mdlScanCriteria_setReturnType.
//! The default set up by mdlScanCriteria_init is MSSCANCRIT_RETURN_FILEPOS.
//! This function pointer is the prototype for callback functions when iterating with the scanner.
typedef int  (*PFScanElemDscrCallback) (MSElementDescrP, void *callbackArg, ScanCriteriaP scP);
typedef int  (*PFScanElemRefCallback) (ElementRefP, void *callbackArg, ScanCriteriaP scP);

//! Return true from scanFilterFunc to reject element, false, to accept.
typedef bool     (*PFScanFilterFunction) (ScanCriteriaCP, DgnElementCP, void* filterArg);
typedef void (*PFFilterArgFreeFunction) (ScanCriteriaCP, void** filterArg);
END_BENTLEY_API_NAMESPACE

//__PUBLISH_SECTION_END__

DGNPLATFORM_TYPEDEF  (PersistentElementRef, PersistentElementRefP);
DGNPLATFORM_TYPEDEFS (ElemRangeIndex)
DGNPLATFORM_TYPEDEFS (DgnRangeTree)
DGNPLATFORM_TYPEDEFS (IRangeNodeCheck)

//__PUBLISH_SECTION_START__
BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//__PUBLISH_SECTION_END__
struct ExtendedAttrBuf
    {
    short    numWords;   // Number of words of attr data to consider.
    UShort   extAttData[32];
    };
//__PUBLISH_SECTION_START__

enum ScanCriteriaIterationType
{
    MSSCANCRIT_ITERATE_ELMDSCR          = 5,
    MSSCANCRIT_ITERATE_ELMREF           = 6,
    MSSCANCRIT_ITERATE_ELMREF_UNORDERED = 7,
};

enum ScanCriteriaConstants
{
    // Values to return from mdlScanCriteria_scanAllModelsOfFile per-element callback function.
    SCANALL_ABORT_SCAN            = -101,
    SCANALL_ABORT_CURRENT_MODEL   = -102,

    // Scanner return values.
    END_OF_DGN      = 10,
    BUFF_FULL       = 11,
    BAD_FILE        = 65,
    BAD_REQUEST     = 67,
    BAD_ELEMENT     = 68,
};

struct RangeTreeProgressMonitor  { virtual bool _MonitorProgress (double fractionComplete) = 0; };

//__PUBLISH_SECTION_END__

enum RangeMatchStatus
{
    RANGEMATCH_Ok           = 0,
    RANGEMATCH_Aborted      = 1,
    RANGEMATCH_TooManyHits  = 2,
};

struct      DynRangeNode
{
    private:
        PersistentElementRefP   m_elRef;

    public:
        DynRangeNode (PersistentElementRefP elRef) {m_elRef = elRef;}
        inline PersistentElementRefP GetKey         () const   {return   m_elRef; }
        inline PersistentElementRefP GetValue       () const {return   m_elRef; }

        static PersistentElementRefP const GetMinKey() {return 0;}
        static PersistentElementRefP const GetMaxKey() {return (PersistentElementRefP) 0xffffffff;}
};

//=======================================================================================
// @bsiclass                                                      Keith.Bentley   05/07
//=======================================================================================
struct  ElemRangeIndex
{
    struct  Traverser
        {
        virtual bool _CheckRangeIndexNode (DRange3dCR, bool is3d, bool isElement) const = 0;   // true == process node
        virtual RangeMatchStatus _VisitRangeIndexElem (PersistentElementRefP) = 0;    // true == keep going, false == stop traversal
        };

private:
    int             m_stamp;      // Useful to tell if the range has changed.
    DgnModelR       m_dgnModel;
    DgnRangeTreeP   m_rangeTree;

public:
    DGNPLATFORM_EXPORT ElemRangeIndex (DgnModelR);
    DGNPLATFORM_EXPORT ~ElemRangeIndex ();

    int GetStamp () const {return m_stamp;}

    DGNPLATFORM_EXPORT void AddRangeElement (PersistentElementRefP, DgnElementCR);
    StatusInt RemoveElement (PersistentElementRefP);
    DGNPLATFORM_EXPORT RangeMatchStatus FindMatches (Traverser&);
    DGNPLATFORM_EXPORT DgnRangeTreeR GetDgnRangeTree();
    DgnRangeTreeP GetDgnRangeTreeP() {return m_rangeTree;}
    DGNPLATFORM_EXPORT DRange3dCP GetRange();
    StatusInt   GetRangeIfKnown (DRange3dR range);
    static DRange3dCP GetIndexRange(ElementHandleCR eh);
    static DRange3dCP CheckIndexRange (ElementHandleCR eh);
};

enum
{
    MAX_SC_RANGE       = 8,
    HIT_STOP           = 12,
    MAX_ORDERED_HITS   = 3000,
    MIN_EXPANDED_ELEM  = 16,
};

enum
{
    ACCEPTING_CMPLX  = 1,
    ACC_CMPLX_HDR    = 2,
    REJECTING_CMPLX  = 3,
    CHECKING_CMPLX   = 4,
};

enum
{
    U_MASK      = 0x1000,           // attribute data definitions.
    LNGTH_MASK  = 0x00ff,
    M_NOCLASS   = 0xfff0,           // complement of class mask.
};

//=======================================================================================
// @bsiclass                                                      Keith.Bentley   05/07
//=======================================================================================
struct ScanType // Don't change the order of this structure.
{
    unsigned int    returnOneElem:1;
    unsigned int    unused:1;    // was blockbyte
    unsigned int    iteration:2;
    unsigned int    nestCells:1;

    unsigned int    testCellName:1;
    unsigned int    testProperties:1;
    unsigned int    testClass:1;
    unsigned int    testGraphicGroup:1;
    unsigned int    testPriority:1;
    unsigned int    testRange:1;
    unsigned int    testMultiRange:1;
    unsigned int    testModTime:1;
    unsigned int    testSkewScan:1;
    unsigned int    testLevel:1;

    unsigned int    testAttributeEntity:1;
    unsigned int    testAttributeOccurrence:1;
    unsigned int    testAttributeExtended:1;
    unsigned int    freeLevelBitMaskWhenDone:1;
    unsigned int    unused3:1;
    unsigned int    testDisplaySet:1;
    unsigned int    testIncludedForDgnModel:1;
    unsigned int    testXAttributes:1;

    unsigned int    unused2:4;
};

//__PUBLISH_SECTION_START__
//=======================================================================================
//! Criteria object created and used by mdlScanCriteria_* methods. You should prefer to
//! use DgnModel::GetGraphicElementsP or DgnModel::GetControlElementsP and check criteria,
//! on your own, unless you need the range-based criteria supported by the mdlScanCriteria methods.
// @bsiclass                                                      Keith.Bentley   05/07
//=======================================================================================
struct ScanCriteria
//__PUBLISH_SECTION_END__
        : ElemRangeIndex::Traverser
//__PUBLISH_SECTION_START__
{
//__PUBLISH_SECTION_END__

    struct ElemFilter
        {
        virtual  ScanTestResult _FilterElement (ScanCriteriaCP scP, DgnElementCP element) = 0;
        virtual  void           Release(ScanCriteriaCP scP) = 0;
        };

private:
    typedef bvector<PersistentElementRefP>   T_RangeHits;

    // NOTE: For performance, the constructor initializes members using:
    //         memset (&m_firstMember, 0, offsetof (scanCriteria, m_lastMember)- offsetof (scanCriteria, m_firstMember));
    //         So make sure those members are first/last!

    int                     m_firstMember;
    ScanType                m_type;                // INPUT:  scan type bits
    UInt32                  m_classMask;           // INPUT:  class mask, if testClass true
    DgnModelP               m_model;               // DERIVED or INPUT: the model we're scanning
    int                     m_newCriteria;         // INPUT:  new range criteria
    int                     m_numRanges;
    DPoint3d                m_skewVector;          // INPUT:  skewVector if testSkewScan true
    DRange3d                m_skewRange;           // INPUT:  skew range in doubles
    DRange3d                m_range[MAX_SC_RANGE]; // INPUT:  range limits, if testRange true
    long                    m_occurrence;          // INPUT:  occurence if testAttributeOccurrence true
    UShort                  m_entity;              // INPUT:  entity if testAttributeEntity true
    short                   m_overrideNest[8];     // INPUT:  override nesting to return all components if override true for type
    PFScanElemDscrCallback  m_callbackFunc;        // INPUT:  iterator function
    void*                   m_callbackArg;         // INPUT:  argument to pass to callbackFunc
    UShort                  m_propertiesVal;       // INPUT:  properties value, if testProperties true
    UShort                  m_propertiesMask;      // INPUT:  properties mask, if testProperties true
    UInt32                  m_minPriority;         // INPUT:  low limit of priority, if testPriority true
    UInt32                  m_maxPriority;         // INPUT:  high limit of priority, if testPriority true
    double                  m_minModTime;          // INPUT:  low limit of lastModified, if testModTime true
    double                  m_maxModTime;          // INPUT:  high limit of lastModified, if testModTime true
    BitMaskP                m_levelBitMask;        // INPUT:  the level bit mask
    void*                   m_mdlCallback;         // INPUT:  used when callback is in MDL program.
    MdlDesc*                m_mdlDescrP;           // INPUT:  used when callback is in MDL program.
    ElemFilter*             m_elemfilter;
    WCharP                  m_cellName;            // INPUT:  cell name to look for
    struct ExtendedAttrBuf* m_extAttrBuf;          // INPUT:  extended attribute buffer
    XAttributeHandlerId     m_xAttrHandlerId;      // INPUT:  Handler ID for XAttributes
    UInt32                  m_xAttrId;             // INPUT:  Attribute ID for XAttributes
    int                     m_inComplex;           // CONTEXT:  flag for in complex element
    short                   m_elemOk;              // CONTEXT:  current element OK, wont fit in output
    DgnModel::ElementRefIterator m_iterator;
    IRangeNodeCheckP        m_appRangeNodeCheck;
    T_RangeHits*            m_rangeHits;
    int                     m_currRangeHit;
    ViewContextP            m_viewContext;
    int                     m_lastMember;

    StatusInt CallElemRefFunc (PersistentElementRefP el);
    StatusInt CallElemDscrFunc (MSElementDescrP elDscr);
    ScanTestResult                  CheckCellName (DgnElementCP) const;
    ScanTestResult                  CheckEntityandOccurence (DgnElementCP) const;
    ScanTestResult                  CheckExtendedAttributes (DgnElementCP) const;
    StatusInt ProcessRangeIndexResults ();
    bool      UseRangeTree (ElemRangeIndexP);
    bool      UseRangeTreeOrdering () {return 3 == m_type.iteration;}
    bool      IsElemRefIter () {return 3 == m_type.iteration || 2 == m_type.iteration;}
    bool      IsElemDescrIter () {return 1 == m_type.iteration;}
    bool      CheckElementRange (ElementHandleCR) const;
    StatusInt DoElemDscrCallback ();
    StatusInt DoElemRefCallback ();
    bool      TransferElement (int* scanStatus);
    void      ResetState ();
    void      Empty ();
    RangeMatchStatus FindRangeHits (ElemRangeIndexP);
    StatusInt ProcessElemRefRangeList ();

    virtual bool _CheckRangeIndexNode (DRange3dCR, bool, bool) const override;
    virtual RangeMatchStatus _VisitRangeIndexElem (PersistentElementRefP) override;

    private: static ScanCriteria* s_legacyScanCriteria;
    public: static ScanCriteria& LegacyScanCriteria ()
        {
        if (NULL == s_legacyScanCriteria)
            s_legacyScanCriteria = new ScanCriteria ();

        return *s_legacyScanCriteria;
        }

public:
    DGNPLATFORM_EXPORT ScanCriteria ();
    DGNPLATFORM_EXPORT explicit ScanCriteria (ScanCriteriaCR);
    DGNPLATFORM_EXPORT virtual ~ScanCriteria ();

    void         CopyRangeTest (ScanCriteriaCR from);
    ScanType     GetScanType () const {return m_type;}
    DRange3dCR   GetScanRange () const {return m_range[0];}
    DPoint3dCR   GetSkewVector () const {return m_skewVector;}
    DgnModelP    GetDgnModelP () {return m_model;}
    void*        GetMdlCallback () {return m_mdlCallback;}
    MdlDesc*     GetMdlDescr () {return m_mdlDescrP;}
    PFScanElemDscrCallback GetCallbackFunc () {return m_callbackFunc;}
    void         SetRangeNodeCheck (IRangeNodeCheckP checker) {m_appRangeNodeCheck = checker;}
    void         SetDisplaySetTest (bool yesNo) {m_type.testDisplaySet = yesNo;}
    BitMaskCP    GetLevelBitMask () const {return m_levelBitMask;}
    int          GetClassMask () const {return m_type.testClass ? m_classMask : 0xffffffff;}

    DGNPLATFORM_EXPORT ScanTestResult CheckRange (DRange3dCR elemRange, bool isElem3d) const;
    DGNPLATFORM_EXPORT StatusInt GetExtendedRangeTest (DRange3dP srP, int rangeNum) const;
    DGNPLATFORM_EXPORT void      SetNestOverride (short* mask, bool operation);
    DGNPLATFORM_EXPORT bool      IsLevelActive (int level);
    DGNPLATFORM_EXPORT bool      IsClassActive (int dgnClass);
    DGNPLATFORM_EXPORT void      ClearClassMask ();
    DGNPLATFORM_EXPORT void      GetProperties (int* propertiesValP, int* propertiesMaskP);
    DGNPLATFORM_EXPORT void      SetCellNameTest (WCharCP cellName);
    DGNPLATFORM_EXPORT void      SetTimeTest (double minModTime, double maxModTime);
    DGNPLATFORM_EXPORT void      SetAttributeTest (UShort entity, long occurrence, ExtendedAttrBuf* extAttrBuf);
    DGNPLATFORM_EXPORT void      SetXAttributeTest (XAttributeHandlerIdP handlerId, UInt32 attrId);
    DGNPLATFORM_EXPORT StatusInt SetExtendedRangeTest (DRange3dP srP, int rangeNum);
    DGNPLATFORM_EXPORT void      SetSkewRangeTest (DRange3dP mainRange, DRange3dP skewRange, DPoint3dP skewVector);
    DGNPLATFORM_EXPORT void      SetElmDscrCallback (PFScanElemDscrCallback callbackFunc, void* callbackArg);
    DGNPLATFORM_EXPORT StatusInt SetReturnType(int returnType, int oneElementOnly, int nestCells);
    DGNPLATFORM_EXPORT void      SetFilter (ElemFilter* filter);
    void SetMdlCallback (void* callback, MdlDesc* mdlDescr) {m_mdlCallback = callback; m_mdlDescrP = mdlDescr;}
    DGNPLATFORM_EXPORT StatusInt Scan (ViewContextP);

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:
    //! Create a new instance of ScanCriteria
    DGNPLATFORM_EXPORT static ScanCriteriaP Create();

    //! Copy an existing ScanCriteria.
    //! @param[in] source   The existing ScanCriteria to copy.
    DGNPLATFORM_EXPORT static ScanCriteriaP Clone(ScanCriteriaCR source);

    //! Frees a ScanCriteria instance.
    //! @param[in] scanCriteria The ScanCriteria to free.
    DGNPLATFORM_EXPORT static void          Delete(ScanCriteriaP scanCriteria);

    //! Remove an Element Class from the Class filter.
    //! @param[in] elementClass The class to remove from the class filter. Elements of that element class will not be returned by the Scan.
    DGNPLATFORM_EXPORT StatusInt            RemoveSingleClassTest (DgnElementClass elementClass);

    //! Add an Element Class to the Class filter.
    //! @param[in] elementClass The class to add to the class filter. Elements of that element class will be returned by the Scan.
    DGNPLATFORM_EXPORT StatusInt            AddSingleClassTest (DgnElementClass elementClass);

    //! Set the Element Class filter.
    //! @param[in] classMask A mask with a bit set for each class. This is formed as an OR of the desired classes,
    //!         using (1 << class) for the desired classes from the DgnElementClass enum.
    //! @remark If the classMask argument is set to 0, then class testing is disabled.
    DGNPLATFORM_EXPORT void                 SetClassTest (int classMask);

    //! Clears the Levels filter. Elements from all levels will be returned by the scan.
    DGNPLATFORM_EXPORT void                 ClearLevelMask ();

    //! Set the Levels filter. Only elements on levels that pass the level test will be returned by the scan.
    //! @param[in] levelBitMask  The bitmask indicating the acceptable levels. It is formed by setting bit (LevelId-1) in the bitmask for each LevelId that is desired.
    //! @param[in] owned         If true, ownership of the levelBitMask and responsibility for freeing it is passed to the ScanCriteria. If false, the ScanCriteria keeps a 
    //!                          pointer to the levelBitMask and the caller is responsible for ensuring that the levelBitMask is not freed before the ScanCriteria.
    DGNPLATFORM_EXPORT void                 SetLevelTest (BitMaskP levelBitMask, bool owned);

    //! Remove the specified level from the Levels filter. Elements on that level will not be returned by the scan.
    //! @param[in] level    The level to remove from the Levels filter.
    DGNPLATFORM_EXPORT void                 RemoveSingleLevelTest (LevelId level);

    //! Add the specified level to the Levels filter. Elements on that level will be returned by the scan.
    //! @param[in] level    The level to add to the Levels filter.
    DGNPLATFORM_EXPORT void                 AddSingleLevelTest (LevelId level);

    //! Sets the DgnModel that is to be scanned.
    //! @param[in] modelRef     The model to be scanned.
    DGNPLATFORM_EXPORT StatusInt            SetDgnModel (DgnModelP model);

    //! Sets the function that is to be called for each acceptable element when the #Scan method is called.
    //! @param[in] callbackFunc The user function that is to be called for each accepted element.
    //! @param[in] callbackArg  A user-specified argument passed to the callbackFunc.
    DGNPLATFORM_EXPORT void                 SetElemRefCallback (PFScanElemRefCallback callbackFunc, CallbackArgP callbackArg);

    //! Sets the Element properties test. If the propertiesVal argument is set to 0, then properties testing is disabled.
    //! @Param[in] propertiesVal    The value part of the properties test.
    //! @Param[in] propertiesMask   The mask part of the properties test.
    //! @remarks The scanner checks the element's properties by ANDing propertiesMask with the element's properties bits and then comparing the result with 
    //!          propertiesVal. If these values do not match, the element is rejected.
    DGNPLATFORM_EXPORT void                 SetPropertiesTest (UShort propertiesVal, UShort propertiesMask);

    //! Sets the element priority testing. If maxPriority is less than minPriority, priorty testing is disabled.
    //! @param[in] minPriority      The minimum priority value.
    //! @param[in] maxPriority      The maximum priority value.
    DGNPLATFORM_EXPORT void                 SetPriorityTest (UInt32 minPriority, UInt32 maxPriority);

    //! Sets the range testing for the scan. If scanRange is NULL, then no range testing is performed.
    //! @param[in] scanRange    The range to test. An element whose range overlaps any part of scanRange is returned by the scan.
    DGNPLATFORM_EXPORT void                 SetRangeTest (DRange3dP scanRange);

    //! Perform the scan, filtering elements as dictated by this ScanCriteria, calling the callbackFunc specified in #SetElemRefCallback.
    DGNPLATFORM_EXPORT StatusInt            Scan ();

    //! Get the DgnModel set by #SetDgnModel. This method is often useful from the callbackFunc set in #SetElemRefCallback.
    DGNPLATFORM_EXPORT DgnModelP         GetDgnModel ();

    //! Check one particular element agains this ScanCriteria
    //! @param[in] elHandle     The element to test.
    //! @param[in] doRangeTest  Check the range.
    //! @param[in] doAttrTest   Check attributes. 
    DGNPLATFORM_EXPORT ScanTestResult       CheckElement(ElementHandleCR elHandle, bool doRangeTest, bool doAttrTest) const;
};

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
