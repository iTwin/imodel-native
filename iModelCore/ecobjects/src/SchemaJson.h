/*--------------------------------------------------------------------------------------+
|
|     $Source: src/SchemaJson.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

// =====================================================================================
// SchemaJsonWriter class
// =====================================================================================
struct SchemaJsonWriter
    {
    private:
        Json::Value& m_jsonRoot;
        ECSchemaCR m_ecSchema;

    protected:
        bool WriteSchemaReferences();
        bool WriteClass(ECClassCR ecClass);
        bool WriteEnumeration(ECEnumerationCR ecEnumeration);
        bool WriteKindOfQuantity(KindOfQuantityCR kindOfQuantity);
        bool WritePropertyCategory(PropertyCategoryCR propertyCategory);
        bool WriteUnitSystem(UnitSystemCR unitSystem);
        bool WritePhenomenon(PhenomenonCR phenomenon);
        bool WriteUnit(ECUnitCR unit);

        bool WriteSchemaItems();

    public:
        SchemaJsonWriter(Json::Value& jsonRoot, ECSchemaCR ecSchema);
        virtual bool Serialize();
    };

END_BENTLEY_ECOBJECT_NAMESPACE
