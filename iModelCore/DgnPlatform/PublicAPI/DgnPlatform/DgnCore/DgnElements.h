/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/DgnElements.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include <Bentley/Bentley.h>
#include <DgnPlatform/DgnPlatform.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

#if !defined (DOCUMENTATION_GENERATOR)

//=======================================================================================
// Header for all elements. 
// @bsiclass                                                    Keith.Bentley   06/14
//=======================================================================================
struct DgnElementHeader
{
protected:
    UInt32  m_elementSize;        // number of words in element
    UInt32  m_attrOffset;         // offset (in words) from start of element to attributes
    UInt32  m_level;              // element level
    UInt64  m_elementId;          // unique ID of element
    DgnElementHeader() {} // can not be directly instantiated.

public:
    //! Get the size of this element in words (1 word = 2 bytes)
    UInt32 GetSizeWords() const {return m_elementSize;}
    //! @return the size (in bytes) of this element
    UInt32 Size() const {return 2 * GetSizeWords();}
    //! Get the offset of the start of the attributes on this element (in words)
    UInt32 GetAttributeOffset() const {return m_attrOffset;}
    //! Get the ElemetnId of this element
    ElementId GetElementId() const {return ElementId(m_elementId);}
    //! Get the LevelId of this element
    LevelId GetLevel() const {return LevelId(m_level);}

    //! Copies the data for this element into another element. It is assumed that the target element is allocated with the same number of
    //! bytes as this element
    //! @param[out] rhs The element to copy into
    //! @return The number of bytes copied (the size of this element).
    DGNPLATFORM_EXPORT size_t CopyTo(DgnElementHeaderR rhs) const;

    //! Change the level of this element.
    void SetLevel(UInt32 val) {m_level=val;}

    //! Set the size of this element, in words.
    void SetSizeWords(UInt32 val) {m_elementSize=val;}

    //! Set the offset to the attributes in this element.
    void SetAttributeOffset(UInt32 val) {m_attrOffset=val;}

    //! Set the size of this element, in words, and also set the attribute offset to the end of the element (which means that
    //! it has no attributes).
    void SetSizeWordsNoAttributes(UInt32 val) {SetSizeWords(val); SetAttributeOffset(val);}

    //! Change the ElementId of this element
    void SetElementId(ElementId val) {m_elementId=val.GetValue();}

    //! Invalidate the ElementId of this element.
    void InvalidateElementId() {m_elementId=0;}

//__PUBLISH_SECTION_END__
    //! zero the DgnElementHeader members of this element.
    void ClearDgnElementHeader() {memset(this, 0, sizeof(*this));}

    //! Get the value for this element's level.
    UInt32 GetLevelValue() const {return GetLevel().GetValueUnchecked();}
    //! Get a writable reference to the level of this element
    UInt32& GetLevelR() {return m_level;}

    void IncrementSizeWords(UInt32 delta) {SetSizeWords(GetSizeWords()+delta); SetAttributeOffset(GetAttributeOffset()+delta);}
    void DecrementSizeWords(UInt32 delta) {SetSizeWords(GetSizeWords()-delta); SetAttributeOffset(GetAttributeOffset()-delta);}
//__PUBLISH_SECTION_START__
};

//=======================================================================================
//! A graphic element
// @bsiclass                                                    Keith.Bentley   06/14
//=======================================================================================
struct DgnElement : DgnElementHeader
    {
protected:
    Int32  m_displayPriority;  // element display priority
    struct
        {
        UInt16  m_elementClass:4;     // element class
        UInt16  m_is3d:1;             // element is 3d
        UInt16  m_snappable:1;        // 0=snappable, 1=nonsnappable
        } m_props;

    Symbology m_symb;
    DRange3d  m_range;

    DgnElement() {}

public:
    //! Get the range for this element
    DRange3dCR GetRange() const {return m_range;}
    //! Change the range of this element
    void SetRange(DRange3dCR val) {m_range=val;}
    //! Determine whether the 2d range of this element is valid (low is less than high in x and y)
    bool IsRangeValid2d() const {return (m_range.low.x < m_range.high.x) && (m_range.low.y < m_range.high.y);}
    //! Determine whether the 3d range of this element is valid (low is less than high in x,y, and z)
    bool IsRangeValid3d() const {return IsRangeValid2d() && (m_range.low.z < m_range.high.z);}

    //! Get the symbology for this element
    SymbologyCR GetSymbology() const {return m_symb;}
    //! Change the Symbology of this element
    void SetSymbology(SymbologyCR val) {m_symb = val;}

    //! Determine if this is a 3d element
    bool Is3d() const {return m_props.m_is3d;}
    //! Set the "is3d" flag of this element
    void SetIs3d(bool val) {m_props.m_is3d=val;}

    //! Determine whether this element is snappable
    bool IsSnappable() const {return m_props.m_snappable;}
    //! Change the snappable state of this element
    void SetSnappable(bool val) {m_props.m_snappable=val;}

    //! Get the DgnElementClass of this element
    DgnElementClass GetElementClass() const {return (DgnElementClass) m_props.m_elementClass;}
    //! Change the DgnElementClass of this element
    void SetElementClass(DgnElementClass val) {m_props.m_elementClass=(UInt16)val;}

    //! Get the display priority of this element. 
    //! @note Display priority is only valid for 2d elements
    Int32 GetDisplayPriority() const {return m_displayPriority;}
    //! Change the display priority of this element.
    //! @note Display priority is only valid for 2d elements
    void SetDisplayPriority(Int32 val) {m_displayPriority=val;}
//__PUBLISH_SECTION_END__

    void ClearGraphicElement() {memset(this, 0, sizeof(*this));}
    DRange3dR GetRangeR() {return m_range;}
    SymbologyR GetSymbologyR() {return m_symb;}
    Int32& GetDisplayPriorityR() {return m_displayPriority;}
    UInt16 GetElementClassValue() const {return m_props.m_elementClass;}
    UInt16 GetProperties() const {return *((UInt16*)&m_props);}
    void SetProperties(UInt16 val) {*((UInt16*)&m_props) = val;}
//__PUBLISH_SECTION_START__
    };

/*----------------------------------------------------------------------+
//! Linkage Header stored at the start of all element user data
//! Use LinkageUtil::GetWords/SetWords to get/set linkage size.
+----------------------------------------------------------------------*/
struct LinkageHeader
{
    UInt16  wdMantissa:8;           // mantissa words: wtf if wdExponent=0
    UInt16  wdExponent:4;           // exponent for words multiplier
    UInt16  user:1;                 // boolean: user data linkage
    UInt16  modified:1;             // boolean: user linkage modified
    UInt16  remote:1;               // boolean: remote linkage
    UInt16  info:1;                 // boolean: informational linkage
    UInt16  primaryID;              // User ID number
};

//=======================================================================================
//! Declare a "blank" DgnElement of a specified size, generally for creation. The template argument
//! specifies the number of memory bytes allocated for this element. Be careful creating large
//! element blanks on the stack. Also be careful not to attempt to store data past the end of the allocated space.
// @bsiclass                                                    Keith.Bentley   06/14
//=======================================================================================
template<int size> struct DgnElementBlank : DgnElement
{
private:
    byte tmp[size - sizeof(DgnElement)];

public:
    enum class Zeros {None, Full, Header, GraphicHeader};

    //! Construct a DgnElementBlank, optionally clearing memory.
    //! @param[in] clear The number of bytes to zero.
    explicit DgnElementBlank(Zeros clear=Zeros::None) 
        {
        if (clear!=Zeros::None)
            memset (this, 0, (clear==Zeros::Full) ? sizeof(*this) : (clear==Zeros::Header) ? sizeof(DgnElementHeader) : sizeof(DgnElement));
        }

    //! Construct a DgnElementBlank, copying the data from another existing element.
    //! @param[in] from The source element to copy. Data after the size of the source element is left uninitialized.
    //! @note make sure the size of the source element is less than the size of the blank element.
    explicit DgnElementBlank(DgnElementHeaderCR from) {from.CopyTo(*this);}
};

//=======================================================================================
//! Used to create new elements, generally for backwards compatibility. This element has
//! the V8 maximum number of bytes (128K), so it can consume much more stack space than is generally
//! necessary. Also, there really is no fixed maximum element size, but this is for code that previously
//! declared a "full sized" element for creation.
// @bsiclass                                                    Keith.Bentley   06/14
//=======================================================================================
typedef DgnElementBlank<MAX_V8_ELEMENT_WORDS*2> DgnV8ElementBlank;

//=======================================================================================
// @bsiclass
//=======================================================================================
struct SCOverride
    {
    UInt16          level:1;            // level override (true for pnt cells)
    UInt16          relative:1;         // relative or absolute
    UInt16          classValue:1;       // class value
    UInt16          color:1;            // color of all components
    UInt16          weight:1;           // weight of all components
    UInt16          style:1;            // style of all components
    UInt16          assocPnt:1;         // origin of cell is associative point
    UInt16          scaleDimsWysiwyg:1; // true = when scaling, do non-default scaling of dims (scale dim sizes, don't scale dim values)
    UInt16          unused:8;
    };

#endif // DOCUMENTATION_GENERATOR

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
