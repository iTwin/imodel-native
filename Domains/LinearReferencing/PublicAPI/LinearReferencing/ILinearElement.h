/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/LinearReferencing/ILinearElement.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

//__PUBLISH_SECTION_START__
#include "LinearReferencingApi.h"

BEGIN_BENTLEY_LINEARREFERENCING_NAMESPACE

//=======================================================================================
//! Interface implemented by elements that can be used as a scale 
//! along which linear referencing is performed.
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ILinearElement
{
protected:
    virtual double _GetLength() const = 0;
    virtual double _GetStartValue() const = 0;
    virtual Dgn::DgnElementCR _ToElementLRImpl() const = 0;

public:
    DECLARE_LINEARREFERENCING_QUERYCLASS_METHODS(ILinearElement)
    Dgn::DgnElementCR ToElement() const { return _ToElementLRImpl(); }
    Dgn::DgnElementP ToElementP() { return const_cast<Dgn::DgnElementP>(&_ToElementLRImpl()); }

    LINEARREFERENCING_EXPORT double GetLength() const { return _GetLength(); }
    LINEARREFERENCING_EXPORT double GetStartValue() const { return _GetStartValue(); }
}; // ILinearElement

//=======================================================================================
//! Interface implemented by elements that can realize Linear-Elements along which
//! linear referencing is performed.
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ILinearElementSource
{
public:

}; // ILinearElementSource

typedef BeSQLite::EC::ECInstanceId LinearlyReferencedLocationId;

//=======================================================================================
//! Base interface for linearly-located elements and attributions.
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ILinearlyLocated
{
private:
    Dgn::DgnElementId m_linearElementId;

protected:
    LINEARREFERENCING_EXPORT ILinearlyLocated(Dgn::DgnElementId linearElementId);

    virtual Dgn::DgnElementCR _ToElementLRImpl() const = 0;
    LINEARREFERENCING_EXPORT void _AddLinearlyReferencedLocation(LinearlyReferencedLocationR);

public:
    DECLARE_LINEARREFERENCING_QUERYCLASS_METHODS(ILinearlyLocated)
    Dgn::DgnElementCR ToElement() const { return _ToElementLRImpl(); } 
    Dgn::DgnElementP ToElementP() { return const_cast<Dgn::DgnElementP>(&_ToElementLRImpl()); }

    LINEARREFERENCING_EXPORT Dgn::DgnElementId GetLinearElementId() const { return m_linearElementId; }
    LINEARREFERENCING_EXPORT ILinearElementCP GetLinearElement() const;
    LINEARREFERENCING_EXPORT bvector<LinearlyReferencedLocationId> QueryLinearlyReferencedLocationIds() const;
    LINEARREFERENCING_EXPORT LinearlyReferencedLocationCP GetLinearlyReferencedLocation(LinearlyReferencedLocationId) const;
    LINEARREFERENCING_EXPORT LinearlyReferencedLocationP GetLinearlyReferencedLocationP(LinearlyReferencedLocationId);    
}; // ILinearlyLocated

//=======================================================================================
//! Interface implemented by attribution-elements, representing values of a property
//! of an ILinearElementSource which may apply to only part of it.
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ILinearlyLocatedAttribution : ILinearlyLocated
{
    DEFINE_T_SUPER(ILinearlyLocated)

protected:
    LINEARREFERENCING_EXPORT ILinearlyLocatedAttribution(Dgn::DgnElementId linearElementId);
}; // ILinearlyLocatedAttribution

//=======================================================================================
//! Interface implemented by elements lineraly located existing along 
//! an ILinearElementSource.
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ILinearlyLocatedElement : ILinearlyLocated
{
    DEFINE_T_SUPER(ILinearlyLocated)

protected:
    LINEARREFERENCING_EXPORT ILinearlyLocatedElement(Dgn::DgnElementId linearElementId);
}; // ILinearlyLocatedElement

//=======================================================================================
//! Interface implemented by elements representing known locations along 
//! an ILinearElement.
//! @see ILinearElement
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE IReferent
{
protected:
    virtual double _GetRestartValue() const { return 0.0; }
    virtual ILinearlyLocatedElementCP _ToLinearlyLocatedElement() const { return nullptr; }

public:
    LINEARREFERENCING_EXPORT double GetRestartValue() const { return _GetRestartValue(); }
    LINEARREFERENCING_EXPORT ILinearlyLocatedElementCP ToLinearlyLocatedElement() const { return _ToLinearlyLocatedElement(); }
}; // IReferent

END_BENTLEY_LINEARREFERENCING_NAMESPACE