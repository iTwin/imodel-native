/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/ScaleDefinition.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include    <DgnPlatform/ExportMacros.h>
#include    <DgnPlatform/DgnPlatform.r.h>
#include    <Bentley/WString.h>

DGNPLATFORM_TYPEDEFS (ScaleDefinition);
DGNPLATFORM_TYPEDEFS (ScaleCollection);

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//!
//! @bsiclass
//=======================================================================================
struct ScaleDefinition
    {
private:
    WString  m_name;
    double   m_prescale;
    double   m_postscale;

public:
    DGNPLATFORM_EXPORT ScaleDefinition (WCharCP name, double pre, double post);
    //! Create custom ScaleDefinition with specified scaleFactor.
    //! By default the name will be "CUSTOM <scaleFactor>"
    //! If nameAsRatio==true, the name will be of the format "denominator:numerator"
    DGNPLATFORM_EXPORT ScaleDefinition (double scaleFactor, bool nameAsRatio=false);
    //! Create an unnamed ScaleDefinition with scaleFactor 1.0
    DGNPLATFORM_EXPORT ScaleDefinition ();

    DGNPLATFORM_EXPORT WCharCP         GetName() const;
    DGNPLATFORM_EXPORT double          GetScale() const;
    DGNPLATFORM_EXPORT double          GetPreScale() const              { return m_prescale; }
    DGNPLATFORM_EXPORT double          GetPostScale() const             { return m_postscale; }
    DGNPLATFORM_EXPORT void            SetName (WCharCP name)           { m_name = name; }
    DGNPLATFORM_EXPORT void            SetPreScale (double s)           { m_prescale = s; }
    DGNPLATFORM_EXPORT void            SetPostScale (double s)          { m_postscale = s; }
    DGNPLATFORM_EXPORT bool            operator== (const ScaleDefinition& other) const;
    DGNPLATFORM_EXPORT void            FormatNameAsRatio();

//__PUBLISH_SECTION_END__
    DGNPLATFORM_EXPORT static void              FindByFactor (ScaleDefinitionR scaleDef, double scaleFactor);
//__PUBLISH_SECTION_START__
    };

//=======================================================================================
//! Represents the set of predefined ScaleDefinitions as defined in scales.def
//! @bsiclass
//=======================================================================================
struct ScaleCollection
    {
private:
    typedef bvector<ScaleDefinition> ScaleList;
    ScaleList const&        m_scaleList;
public:
    DGNPLATFORM_EXPORT ScaleCollection();

    typedef ScaleList::const_iterator   const_iterator;

    DGNPLATFORM_EXPORT const_iterator          begin() const;
    DGNPLATFORM_EXPORT const_iterator          end() const;

    //! Find the ScaleDefinition with the specified name.
    DGNPLATFORM_EXPORT ScaleDefinitionCP       FindByName (WCharCP name) const;
    //! Find the ScaleDefinition matching the specified scale factor.
    DGNPLATFORM_EXPORT ScaleDefinitionCP       FindByFactor (double scale) const;
    };

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
