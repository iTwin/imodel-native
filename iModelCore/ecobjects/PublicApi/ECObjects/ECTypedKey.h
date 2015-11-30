/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECTypedKey.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/
#include <Bentley/WString.h>
#include <ECObjects/ECObjects.h>
#include <ECObjects/ECSchema.h>
#include <ECObjects/ECInstance.h>

#define TYPESYSTEM_ECObjects    "ECObjects"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                03/2015
+===============+===============+===============+===============+===============+======*/
struct ECTypedKey
{
private:
    Utf8String m_typeSystem;
    Utf8String m_schemaName;
    Utf8String m_className;
    Utf8String m_instanceId;
    void Init(Utf8CP typeSystem, Utf8CP schemaName, Utf8CP className, Utf8CP instanceId) 
        {
        m_typeSystem = typeSystem;
        m_schemaName = schemaName;
        m_className = className;
        m_instanceId = instanceId;
        }

public:
    ECTypedKey() {}
    ECTypedKey(Utf8CP typeSystem, Utf8CP className, Utf8CP instanceId) {Init(typeSystem, "", className, instanceId);}
    ECTypedKey(Utf8CP typeSystem, Utf8CP schemaName, Utf8CP className, Utf8CP instanceId) {Init(typeSystem, schemaName, className, instanceId);}
    ECTypedKey(IECInstanceCR instance) {Init(TYPESYSTEM_ECObjects, Utf8String(instance.GetClass().GetSchema().GetFullSchemaName()).c_str(), Utf8String(instance.GetClass().GetName()).c_str(), Utf8String(instance.GetInstanceId()).c_str());}
    ECTypedKey(ECClassCR ecClass) {Init(TYPESYSTEM_ECObjects, Utf8String(ecClass.GetSchema().GetFullSchemaName()).c_str(), Utf8String(ecClass.GetName()).c_str(), "");}

    bool operator ==(ECTypedKey const& other) {return Equals(other);}
    bool operator !=(ECTypedKey const& other) {return !Equals(other);}
    operator Utf8String() {return ToString();}

    Utf8CP GetTypeSystem() const {return m_typeSystem.c_str();}
    Utf8CP GetSchemaName() const {return m_schemaName.c_str();}
    Utf8CP GetClassName() const {return m_className.c_str();}
    Utf8CP GetInstanceId() const {return m_instanceId.c_str();}

    bool Equals(ECTypedKey const& other) 
        {
        return m_typeSystem.Equals(other.GetTypeSystem()) 
            && m_schemaName.Equals(other.GetSchemaName()) 
            && m_className.Equals(other.GetClassName()) 
            && m_instanceId.Equals(other.GetInstanceId());
        }
    Utf8String ToString() {return Utf8PrintfString("(%s::%s:%s:%s)", m_typeSystem.c_str(), m_schemaName.c_str(), m_className.c_str(), m_instanceId.c_str());}
};

END_BENTLEY_ECOBJECT_NAMESPACE
