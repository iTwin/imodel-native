/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/SchemaLayoutElementHandler.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
typedef bpair<uint16_t, uint16_t>                               SchemaClassIndexPair;

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
    virtual bool VisitSchemaLayoutElement (WCharCP schemaName, ECN::SchemaIndex, DgnElementP) = 0;
    };

void                VisitSchemaLayoutElements (SchemaLayoutElementVisitor& visitor, DgnDbCR dgnFile);

DgnElementECInstancePtr     FindSchemaLayoutInstance (ElementHandleCR element);
DgnElementECInstancePtr     FindClassLayoutInstance (ECN::ClassIndex, ElementHandleCR element);
DgnElementP                 FindSchemaLayoutElement (ECN::SchemaIndex, DgnDbCR);

BentleyStatus       DataFromSchemaLayoutInstance (WStringP schemaName, ECN::SchemaIndex*, IECInstanceCR);
BentleyStatus       DataFromClassLayoutInstance  (WStringP className, ECN::ClassIndex*,  IECInstanceCR);
BentleyStatus       DataFromPropertyLayoutInstance (WString& propertyName, uint32_t& parentStructIndex, ECN::ECTypeDescriptor&, uint32_t& offset, uint32_t& nullFlagsOffset, ECN::NullflagsBitmask&, IECInstanceCR);

BentleyStatus       AddPropertyLayoutToClassLayoutInstance (PropertyLayoutCR propertyLayout, uint32_t propIndex, ECN::IECInstanceR classLayoutInstance);

BentleyStatus       AddSchemaLayoutInstanceToElement (WCharCP schemaName, ECN::SchemaIndex schemaIndexIn, DgnElementP elem);
BentleyStatus       AddClassLayoutInstanceToElement (ECN::ClassLayoutCR classLayout, DgnElementP elem);

ClassLayoutP        CreateClassLayout (ECN::IECInstanceR classLayoutInstance, ECN::SchemaIndex);

DgnElementP         AddNewSchemaLayoutElement (WCharCP schemaName, ECN::SchemaIndex schemaIndexIn, DgnDbR dgnFile);

void                OnUndoRedoSchemaLayoutInstance (XAttributeHandleCR xAttr, bool isUndo);

// Handler methods
ITransactionHandlerP _GetITransactionHandler () override {return this;}

// ITransactionHandler methods
void                _OnUndoRedoXAttributeChange (XAttributeHandleCR xAttr, ChangeTrackAction action, bool isUndo, ChangeTrackSource source) override;

protected:

// Handler methods
DGNPLATFORM_EXPORT virtual void _GetTypeName (WStringR string, uint32_t desiredLength) override;
DGNPLATFORM_EXPORT virtual void _GetDescription (ElementHandleCR el, WStringR string, uint32_t desiredLength) override;

public:
static void Register();

bool                IsInternalECClass (SchemaClassIndexPair& indexOut, ECN::ECClassCR ecClass);
bool                IsInternalECClass (SchemaClassIndexPair& indexOut, WCharCP schemaName, WCharCP className);
//ECXDInstanceEnablerP GetInstanceEnabler (SchemaClassIndexPair const& index);

DgnElementP         FindSchemaLayoutElement (ECN::SchemaIndex& schemaIndexOut, WCharCP schemaName, DgnDbCR dgnFile);
ECN::SchemaLayoutP   LoadSchemaLayout (ElementHandleCR elem);

ECN::ClassLayoutP    ReadClassLayout (ECN::SchemaIndex schemaIndex, ECN::ClassIndex classIndex, DgnDbR dgnFile);
BentleyStatus       WriteClassLayout (ECN::ClassLayoutCR classLayout, WCharCP schemaName, DgnDbR dgnFile);

}; // SchemaLayoutElementHandler

END_BENTLEY_DGNPLATFORM_NAMESPACE

