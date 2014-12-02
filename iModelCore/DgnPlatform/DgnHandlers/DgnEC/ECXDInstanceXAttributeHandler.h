/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/DgnEC/ECXDInstanceXAttributeHandler.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/
//! @cond DONTINCLUDEINDOC

#include <DgnPlatform/DgnCore/XAttributeHandler.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/*=================================================================================**//**
* A generic handler for an XAttribute that holds a single pointer to another element.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
class ECXDBaseXAttributeHandler : public DgnPlatform::XAttributeHandler
{
protected:
    virtual DgnPlatform::XAttributeHandlerId    _GetId () = 0;
    virtual WString                             _GetLabel () = 0;

public:
    DgnPlatform::XAttributeHandlerId            GetId ();
    WString                                     GetLabel ();

    StatusInt CreateXAttribute (UInt32& xAttrId, ElementRefP hostElement, byte const * data, UInt32 size);
    StatusInt ModifyXAttribute (XAttributeHandleR xAttr, void const* newData, UInt32 offset, UInt32 size);
    StatusInt ReplaceXAttribute (XAttributeHandleR xAttr, void const* newData, UInt32 size);
    StatusInt DeleteXAttribute (XAttributeHandleR xAttr);
    StatusInt DeleteXAttribute (ElementRefP hostElementRef, UInt32 xAttrId);

};

/*=================================================================================**//**
* A generic handler for an XAttribute that holds a single pointer to another element.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
class ECXDInstanceXAttributeHandler : public ECXDBaseXAttributeHandler
{
friend class ECXDRelationshipXAttributeHandler;
private:
    //  Singleton
    ECXDInstanceXAttributeHandler () {;}

protected:
    virtual DgnPlatform::XAttributeHandlerId    _GetId () override;
    virtual WString                             _GetLabel () override;

public:

    //! Get the singleton instance of this class.
    static ECXDInstanceXAttributeHandler&     GetHandler();

    void                     Register ();
};

namespace LoggingHelpers 
{
WString         Label (ElementHandleCR eh);
WString         Label (ElementRefP elementRef);
}

/*=================================================================================**//**
* A handler for an XAttribute that holds a ECXData representing a struct value that is a
* member of a struct array property of an ECXDInstance stored on the same element.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
class ECXDStructValueXAttributeHandler : public ECXDBaseXAttributeHandler
{

private:
    //  Singleton
    ECXDStructValueXAttributeHandler () {;}

protected:
    virtual DgnPlatform::XAttributeHandlerId    _GetId () override;
    virtual WString                             _GetLabel () override;

public:

    //! Get the singleton instance of this class.
    static ECXDStructValueXAttributeHandler&     GetHandler();

    void                     Register ();
};

/*=================================================================================**//**
* A generic handler for an XAttribute that holds a single pointer to another element.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
class ECXDRelationshipXAttributeHandler : public ECXDInstanceXAttributeHandler
{
private:
    //  Singleton
    ECXDRelationshipXAttributeHandler () {;}

protected:
    
    // ECXDBaseXAttributeHandler
    virtual DgnPlatform::XAttributeHandlerId    _GetId () override;
    virtual WString                             _GetLabel () override;

public:
    //! Get the singleton instance of this class.
    static ECXDRelationshipXAttributeHandler&     GetHandler();

    void                 Register ();
};


END_BENTLEY_DGNPLATFORM_NAMESPACE

//! @endcond
