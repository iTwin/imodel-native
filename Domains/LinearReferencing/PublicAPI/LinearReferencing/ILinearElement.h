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
//=======================================================================================
struct ILinearElement
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
//=======================================================================================
struct ILinearElementSource
{
public:

}; // ILinearElementSource

typedef BeSQLite::EC::ECInstanceId LinearlyReferencedLocationId;

//=======================================================================================
//! Base interface for linearly-located elements and attributions.
//=======================================================================================
struct ILinearlyLocated
{
private:
    Dgn::DgnElementId m_linearElementId;

protected:
    ILinearlyLocated(ILinearElementCR);

    virtual Dgn::DgnElementCR _ToElementLRImpl() const = 0;

public:
    DECLARE_LINEARREFERENCING_QUERYCLASS_METHODS(ILinearlyLocated)
    Dgn::DgnElementCR ToElement() const { return _ToElementLRImpl(); } 
    Dgn::DgnElementP ToElementP() { return const_cast<Dgn::DgnElementP>(&_ToElementLRImpl()); }

    LINEARREFERENCING_EXPORT Dgn::DgnElementId GetLinearElementId() const { return m_linearElementId; }
    LINEARREFERENCING_EXPORT ILinearElementCP GetLinearElement() const;
    LINEARREFERENCING_EXPORT bvector<LinearlyReferencedLocationId> QueryLinearlyReferencedLocationIds() const;
    LINEARREFERENCING_EXPORT LinearlyReferencedLocationCP GetLinearlyReferencedLocation(LinearlyReferencedLocationId) const;
    LINEARREFERENCING_EXPORT LinearlyReferencedLocationP GetLinearlyReferencedLocationP(LinearlyReferencedLocationId);
    LINEARREFERENCING_EXPORT void AddLinearlyReferencedLocation(LinearlyReferencedLocationR);
}; // ILinearlyLocated

//=======================================================================================
//! Interface implemented by attribution-elements, representing values of a property
//! of an ILinearElementSource which may apply to only part of it.
//=======================================================================================
struct ILinearlyLocatedAttribution : ILinearlyLocated
{

}; // ILinearlyLocatedAttribution

//=======================================================================================
//! Interface implemented by elements lineraly located existing along 
//! an ILinearElementSource.
//=======================================================================================
struct ILinearlyLocatedElement : ILinearlyLocated
{

}; // ILinearlyLocatedElement

END_BENTLEY_LINEARREFERENCING_NAMESPACE