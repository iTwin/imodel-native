/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/SchemaLayoutElementHandler.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/
#include <DgnPlatform/DgnHandlers/IEditActionSource.h>
#include <DgnPlatform/DgnHandlers/IManipulator.h>
#include <DgnPlatform/DgnCore/ITransactionHandler.h>
#include <DgnPlatform/DgnCore/XAttributeHandler.h>
#include <DgnPlatform/DgnCore/DisplayHandler.h>

USING_NAMESPACE_BENTLEY_EC
BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
typedef bpair<UInt16, UInt16>                               SchemaClassIndexPair;

/*=================================================================================**//**
* @bsiclass                                                     JoshSchifter    12/09
+===============+===============+===============+===============+===============+======*/
struct SchemaLayoutElementHandler : DgnPlatform::Handler,
                                    ITransactionHandler
{
    DEFINE_T_SUPER(DgnPlatform::Handler)
    ELEMENTHANDLER_DECLARE_MEMBERS (SchemaLayoutElementHandler, DGNPLATFORM_EXPORT)

private:

struct  SchemaLayoutElementVisitor
    {
    virtual bool VisitSchemaLayoutElement (WCharCP schemaName, ECN::SchemaIndex, ElementRefP) = 0;
    };

void                VisitSchemaLayoutElements (SchemaLayoutElementVisitor& visitor, DgnProjectCR dgnFile);

DgnElementECInstancePtr     FindSchemaLayoutInstance (ElementHandleCR element);
DgnElementECInstancePtr     FindClassLayoutInstance (ECN::ClassIndex, ElementHandleCR element);
ElementRefP                 FindSchemaLayoutElement (ECN::SchemaIndex, DgnProjectCR);

BentleyStatus       DataFromSchemaLayoutInstance (WStringP schemaName, ECN::SchemaIndex*, IECInstanceCR);
BentleyStatus       DataFromClassLayoutInstance  (WStringP className, ECN::ClassIndex*,  IECInstanceCR);
BentleyStatus       DataFromPropertyLayoutInstance (WString& propertyName, UInt32& parentStructIndex, ECN::ECTypeDescriptor&, UInt32& offset, UInt32& nullFlagsOffset, ECN::NullflagsBitmask&, IECInstanceCR);

BentleyStatus       AddPropertyLayoutToClassLayoutInstance (PropertyLayoutCR propertyLayout, UInt32 propIndex, ECN::IECInstanceR classLayoutInstance);

BentleyStatus       AddSchemaLayoutInstanceToElement (WCharCP schemaName, ECN::SchemaIndex schemaIndexIn, ElementRefP elem);
BentleyStatus       AddClassLayoutInstanceToElement (ECN::ClassLayoutCR classLayout, ElementRefP elem);

ClassLayoutP        CreateClassLayout (ECN::IECInstanceR classLayoutInstance, ECN::SchemaIndex);

ElementRefP         AddNewSchemaLayoutElement (WCharCP schemaName, ECN::SchemaIndex schemaIndexIn, DgnProjectR dgnFile);

void                OnUndoRedoSchemaLayoutInstance (XAttributeHandleCR xAttr, bool isUndo);

// Handler methods
ITransactionHandlerP _GetITransactionHandler () override {return this;}

// ITransactionHandler methods
void                _OnUndoRedoXAttributeChange (XAttributeHandleCR xAttr, ChangeTrackAction action, bool isUndo, ChangeTrackSource source) override;

protected:

// Handler methods
DGNPLATFORM_EXPORT virtual void _GetTypeName (WStringR string, UInt32 desiredLength) override;
DGNPLATFORM_EXPORT virtual void _GetDescription (ElementHandleCR el, WStringR string, UInt32 desiredLength) override;

public:
static void Register();

bool                IsInternalECClass (SchemaClassIndexPair& indexOut, ECN::ECClassCR ecClass);
bool                IsInternalECClass (SchemaClassIndexPair& indexOut, WCharCP schemaName, WCharCP className);
//ECXDInstanceEnablerP GetInstanceEnabler (SchemaClassIndexPair const& index);

ElementRefP         FindSchemaLayoutElement (ECN::SchemaIndex& schemaIndexOut, WCharCP schemaName, DgnProjectCR dgnFile);
ECN::SchemaLayoutP   LoadSchemaLayout (ElementHandleCR elem);

ECN::ClassLayoutP    ReadClassLayout (ECN::SchemaIndex schemaIndex, ECN::ClassIndex classIndex, DgnProjectR dgnFile);
BentleyStatus       WriteClassLayout (ECN::ClassLayoutCR classLayout, WCharCP schemaName, DgnProjectR dgnFile);

}; // SchemaLayoutElementHandler

END_BENTLEY_DGNPLATFORM_NAMESPACE

