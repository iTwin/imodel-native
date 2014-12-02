/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/TextBlock/Indentation.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include "TextAPICommon.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! @addtogroup TextModule
//! @beginGroup

//=======================================================================================
//! Storage and utility class for managing a Paragraph object's indentation properties.
//! Each Paragraph stores 3 unique indentation properties: paragraph indent, first-line indent, and tab stops.
// @bsiclass                                                    Venkat.Kalyan   05/04
//=======================================================================================
struct IndentationData : public RefCountedBase
    {
//__PUBLISH_SECTION_END__

    private:    static const int DEFAULTTABSTOPHEIGHTFACTOR = 4;

    private:    double          m_firstLineIndent;
    private:    double          m_paragraphIndent;
    private:    T_DoubleVector  m_tabStops;

    public: DGNPLATFORM_EXPORT          IndentationData ();
    public:                             IndentationData (double firstLineIndent, double paragraphIndent, size_t nTabStops, double const* pTabStops);
    public:                             IndentationData (IndentationDataCR);

    public:                     double  GetNextTabStop  (double currentOffset, double textHeight, double lineBreakLength) const;
    public: DGNPLATFORM_EXPORT  void    SetTabStops     (size_t nTabStops, double const* pTabStops);
    public: DGNPLATFORM_EXPORT  bool    IsDefault       () const;
    public:                     void    Scale           (double);
    public:                     void    Clear           ();

    public: DGNPLATFORM_EXPORT  bool    Equals          (IndentationDataCR) const;
    public: DGNPLATFORM_EXPORT  bool    Equals          (IndentationDataCR, double tolerance) const;

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__

    //! Creates a new instance of this class.
    //! Paragraph and first-line indent are 0.0, and no tab stops are created.
    public: static DGNPLATFORM_EXPORT IndentationDataPtr Create ();

    //! Creates a deep copy of this instance.
    public: DGNPLATFORM_EXPORT IndentationDataPtr Clone () const;

    //! Gets the first-line indent for the paragraph.
    //! @note   This is a unqiue indent for the first line of the paragraph; the rest of the lines are indented according to the paragraph indent.
    public: DGNPLATFORM_EXPORT double GetFirstLineIndent () const;

    //! Sets the first-line indent for the paragraph.
    //! @see    GetFirstLineIndent for additional notes
    public: DGNPLATFORM_EXPORT void SetFirstLineIndent (double);

    //! Gets the indent for the non-first lines of the paragraph.
    //! @note   The first line has a unique indent, and never obeys this value; @see GetFirstLineIndent.
    public: DGNPLATFORM_EXPORT double GetHangingIndent () const;

    //! Sets the indent for the non-first lines of the paragraph.
    //! @see    SetHangingIndent for additional notes
    public: DGNPLATFORM_EXPORT void SetHangingIndent (double);

    //! Gets a (read-only) collection of tab stops.
    //! @note   These values are in UORs.
    //! @note   The paragraph indent is treated as a soft tab stop, and is not present in this collection.
    public: DGNPLATFORM_EXPORT T_DoubleVectorCR GetTabStops () const;

    //! Sets the collection of tab stops.
    //! @see    GetTabStops for additional notes
    public: DGNPLATFORM_EXPORT void SetTabStops (T_DoubleVectorCR);

    }; // IndentationData

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
