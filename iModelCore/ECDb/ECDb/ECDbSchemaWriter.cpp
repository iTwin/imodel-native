/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbSchemaWriter.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaWriter::Fail(Utf8CP fmt, ...) const
    {
    va_list args;
    va_start(args, fmt);

    Utf8String formattedMessage;
    formattedMessage.VSprintf(fmt, args);

    //Utf8String a;a.VSprintf
    m_ecdb.GetECDbImplR().GetIssueReporter()
        .Report(ECDbIssueSeverity::Error, formattedMessage.c_str());

    return ERROR;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaWriter::CreateECSchemaEntry(ECSchemaCR ecSchema)
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != m_ecdb.GetCachedStatement(stmt, "INSERT INTO ec_Schema(Id,Name,DisplayLabel,Description,NamespacePrefix,VersionDigit1,VersionDigit2,VersionDigit3) VALUES(?,?,?,?,?,?,?,?)"))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1,ecSchema.GetId()))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindText(2, ecSchema.GetName().c_str(), Statement::MakeCopy::No))
        return ERROR;

    if (ecSchema.GetIsDisplayLabelDefined())
        {
        if (BE_SQLITE_OK != stmt->BindText(3, ecSchema.GetDisplayLabel().c_str(), Statement::MakeCopy::No))
            return ERROR;
        }

    if (BE_SQLITE_OK != stmt->BindText(4, ecSchema.GetDescription().c_str(), Statement::MakeCopy::No))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindText(5, ecSchema.GetNamespacePrefix().c_str(), Statement::MakeCopy::No))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt(6, ecSchema.GetVersionMajor()))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt(7, ecSchema.GetVersionWrite()))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt(8, ecSchema.GetVersionMinor()))
        return ERROR;

    return BE_SQLITE_DONE == stmt->Step() ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaWriter::CreateBaseClassEntry(ECClassId ecClassId, ECClassCR baseClass, int ordinal)
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != m_ecdb.GetCachedStatement(stmt, "INSERT INTO ec_BaseClass(ClassId,BaseClassId,Ordinal) VALUES(?,?,?)"))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, ecClassId))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(2, baseClass.GetId()))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt(3, ordinal))
        return ERROR;

    return BE_SQLITE_DONE == stmt->Step() ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaWriter::CreateECRelationshipConstraintEntry(ECClassId relationshipClassId, ECN::ECRelationshipConstraintR relationshipConstraint, ECRelationshipEnd endpoint)
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != m_ecdb.GetCachedStatement(stmt, "INSERT INTO ec_RelationshipConstraint (RelationshipClassId,RelationshipEnd,MultiplicityLowerLimit,MultiplicityUpperLimit,RoleLabel,IsPolymorphic) VALUES (?,?,?,?,?,?)"))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, relationshipClassId))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt(2, endpoint))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt(3, relationshipConstraint.GetCardinality().GetLowerLimit()))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt(4, relationshipConstraint.GetCardinality().GetUpperLimit()))
        return ERROR;

    if (relationshipConstraint.IsRoleLabelDefined())
        {
        if (BE_SQLITE_OK != stmt->BindText(5, relationshipConstraint.GetRoleLabel().c_str(), Statement::MakeCopy::No))
            return ERROR;
        }

    if (BE_SQLITE_OK != stmt->BindInt(6, relationshipConstraint.GetIsPolymorphic() ? 1 : 0))
        return ERROR;

    return BE_SQLITE_DONE == stmt->Step() ? SUCCESS : ERROR;
    }

BentleyStatus ECDbSchemaWriter::DeleteCAEntry(ECClassId ecClassId, ECContainerId containerId, ECContainerType containerType)
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != m_ecdb.GetCachedStatement(stmt, "DELETE FROM ec_CustomAttribute WHERE ContainerId = ? AND ContainerType = ? AND ClassId = ?"))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt64(1, containerId.GetValue()))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt(2, Enum::ToInt(containerType)))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt64(3, ecClassId.GetValue()))
        return ERROR;

    if (stmt->Step() != BE_SQLITE_DONE)
        return ERROR;

    return SUCCESS;
    }
BentleyStatus ECDbSchemaWriter::ReplaceCAEntry(IECInstanceP customAttribute, ECClassId ecClassId, ECContainerId containerId, ECContainerType containerType, int ordinal)
    {
    if (DeleteCAEntry(ecClassId, containerId, containerType) != SUCCESS)
        return ERROR;

    return InsertCAEntry(customAttribute, ecClassId, containerId, containerType, ordinal);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  11/2012
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbSchemaWriter::InsertCAEntry(IECInstanceP customAttribute, ECClassId ecClassId, ECContainerId containerId, ECContainerType containerType, int ordinal)
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != m_ecdb.GetCachedStatement(stmt, "INSERT INTO ec_CustomAttribute(ContainerId,ContainerType,ClassId,Ordinal,Instance) VALUES(?,?,?,?,?)"))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, containerId))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt(2, Enum::ToInt(containerType)))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(3, ecClassId))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt(4, ordinal))
        return ERROR;

    Utf8String caXml;
    if (InstanceWriteStatus::Success != customAttribute->WriteToXmlString(caXml, false, //don't write XML description header as we only store an XML fragment
                                            true)) //store instance id for the rare cases where the client specified one.
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindText(5, caXml.c_str(), Statement::MakeCopy::No))
        return ERROR;

    return BE_SQLITE_DONE == stmt->Step() ? SUCCESS : ERROR;
    }



//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbSchemaWriter::UpdateECProperty(ECPropertyChange& propertyChange, ECPropertyCR oldProperty, ECPropertyCR newProperty)
    {
    if (propertyChange.IsApplied())
        return SUCCESS;

    auto propertyId = ECDbSchemaManager::GetPropertyIdForECPropertyFromDuplicateECSchema(m_ecdb, newProperty);
    if (!propertyId.IsValid())
        {
        BeAssert(false && "Failed to resolve ecclass id");
        return ERROR;
        }

    SqlUpdater updater("ec_Property");
    if (propertyChange.GetTypeName().IsValid())
        {
        return Fail("ECSCHEMA-UPGRADE: Changing ECProperty::Type is not supported. Failed on ECProperty %s.%s.",
                    oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());
        }

    //if (propertyChange.GetMaximumValue().Exist())
    //    {
    //    return Fail("ECSCHEMA-UPGRADE: Changing ECProperty::MaximumValue is not supported. Failed on ECProperty %s.%s.",
    //                oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());
    //    }
    //if (propertyChange.GetMinimumValue().Exist())
    //    {
    //    return Fail("ECSCHEMA-UPGRADE: Changing ECProperty::MinimumValue is not supported. Failed on ECProperty %s.%s.",
    //                oldProperty.GetClass().GetFullName(), oldProperty.GetName()).c_str();
    //    }
    if (propertyChange.IsStruct().IsValid())
        {
        return Fail("ECSCHEMA-UPGRADE: Changing ECProperty::IsStruct is not supported. Failed on ECProperty %s.%s.",
                    oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());
        }
    if (propertyChange.IsStructArray().IsValid())
        {
        return Fail("ECSCHEMA-UPGRADE: Changing ECProperty::IsStructArray is not supported. Failed on ECProperty %s.%s.",
                    oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());
        }
    if (propertyChange.IsPrimitive().IsValid())
        {
        return Fail("ECSCHEMA-UPGRADE: Changing ECProperty::IsPrimitive is not supported. Failed on ECProperty %s.%s.",
                    oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());
        }
    if (propertyChange.IsPrimitiveArray().IsValid())
        {
        return Fail("ECSCHEMA-UPGRADE: Changing ECProperty::IsPrimitiveArray is not supported. Failed on ECProperty %s.%s.",
                    oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());
        }
    if (propertyChange.GetExtendedTypeName().IsValid())
        {
        return Fail("ECSCHEMA-UPGRADE: Changing ECProperty::ExtendedTypeName is not supported. Failed on ECProperty %s.%s.",
                    oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());
        }
    if (propertyChange.IsNavigation().IsValid())
        {
        return Fail("ECSCHEMA-UPGRADE: Changing ECProperty::IsNavigation is not supported. Failed on ECProperty %s.%s.",
                    oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());
        }
    if (propertyChange.GetArray().IsValid())
        {
        auto& arrayChange = propertyChange.GetArray();
        if (arrayChange.MaxOccurs().IsValid())
            return Fail("ECSCHEMA-UPGRADE: Changing ECProperty::Array.MaxOccurs is not supported. Failed on ECProperty %s.%s.",
                        oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());

        if (arrayChange.MinOccurs().IsValid())
            return Fail("ECSCHEMA-UPGRADE: Changing ECProperty::Array.MinOccurs is not supported. Failed on ECProperty %s.%s.",
                        oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());
        }
    if (propertyChange.GetNavigation().IsValid())
        {
        auto& navigationChange = propertyChange.GetNavigation();
        if (navigationChange.GetRelationshipClassName().IsValid())
            return Fail("ECSCHEMA-UPGRADE: Changing ECProperty::Navigation.RelationshipClassName is not supported. Failed on ECProperty %s.%s.",
                        oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());

        if (navigationChange.Direction().IsValid())
            return Fail("ECSCHEMA-UPGRADE: Changing ECProperty::Navigation.Direction is not supported. Failed on ECProperty %s.%s.",
                        oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());

        if (navigationChange.GetRelationshipClassName().IsValid())
            return Fail("ECSCHEMA-UPGRADE: Changing ECProperty::Navigation.RelationshipClassName is not supported. Failed on ECProperty %s.%s.",
                        oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());
        }
    if (propertyChange.GetName().IsValid())
        {
        if (propertyChange.GetName().GetNew().IsNull())
            return Fail("ECSCHEMA-UPGRADE: Changing ECProperty::Maximum is not supported. Failed on ECProperty %s.%s.",
                        oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());

        updater.Set("Name", propertyChange.GetName().GetNew().Value());
        }
    if (propertyChange.GetDisplayLabel().IsValid())
        {
        updater.Set("DisplayLabel", propertyChange.GetDisplayLabel().GetNew().Value());
        }
    if (propertyChange.GetDescription().IsValid())
        {
        updater.Set("Description", propertyChange.GetDescription().GetNew().Value());
        }

    updater.Where("Id", propertyId.GetValue());
    if (updater.Apply(m_ecdb) != SUCCESS)
        return ERROR;

    return UpdateECCustomAttributes(ECContainerType::Property, propertyId, propertyChange.CustomAttributes(), oldProperty, newProperty);;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbSchemaWriter::UpdateECRelationshipConstraint(ECContainerId containerId, SqlUpdater& sqlUpdater, ECRelationshipConstraintChange& constraintChange, ECRelationshipConstraintCR oldConstraint, ECRelationshipConstraintCR newConstraint , bool isSource, Utf8CP relationshipName)
    {
    Utf8CP constraintType = isSource ? "Source" : "Target";
    if (constraintChange.IsApplied())
        return SUCCESS;

    if (constraintChange.GetCardinality().IsValid())
        {
        return Fail("ECSCHEMA-UPGRADE: Changing ECRelationshipClass::%s::Cardinality is not supported. Failed on ECRelationshipClass %s.",
                    constraintType, relationshipName);
        }
    if (constraintChange.IsPolymorphic().IsValid())
        {
        return Fail("ECSCHEMA-UPGRADE: Changing ECRelationshipClass::%s::IsPolymorphic is not supported. Failed on ECRelationshipClass %s.",
                    constraintType, relationshipName);
        }
    if (constraintChange.ConstraintClasses().IsValid())
        {
        return Fail("ECSCHEMA-UPGRADE: Changing ECRelationshipClass::%s::ConstraintClasses is not supported. Failed on ECRelationshipClass %s.",
                    constraintType, relationshipName);
        }
    if (constraintChange.GetRoleLabel().IsValid())
        {
        sqlUpdater.Set("RoleLabel", constraintChange.GetRoleLabel().GetNew().Value());
        }

    ECContainerType type = isSource ? ECContainerType::RelationshipConstraintSource : ECContainerType::RelationshipConstraintTarget;
    return UpdateECCustomAttributes(type, containerId, constraintChange.CustomAttributes(), oldConstraint, newConstraint);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbSchemaWriter::TryParseId(Utf8StringR schemaName, Utf8StringR className, Utf8StringCR id) const
    {
    auto n = id.find(':');
    BeAssert(n != Utf8String::npos);
    if (n == Utf8String::npos)
        {
        return ERROR;
        }
    schemaName = id.substr(0, n);
    className = id.substr(n + 1);
    return SUCCESS;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbSchemaWriter::UpdateECCustomAttributes(ECContainerType containerType, ECContainerId containerId, ECInstanceChanges& instanceChanges, IECCustomAttributeContainerCR oldContainer, IECCustomAttributeContainerCR newContainer)
    {
    if (instanceChanges.Empty() || instanceChanges.IsApplied())
        return SUCCESS;
    
    for (size_t i = 0; i < instanceChanges.Count(); i++)
        {
        ECPropertyValueChange& change = instanceChanges.At(i);
        if (change.IsApplied())
            continue;

        Utf8String schemaName;
        Utf8String className;
        if (TryParseId(schemaName, className, change.GetId()) == ERROR)
            return ERROR;

        if (change.GetState() == ChangeState::New)
            {
            IECInstancePtr ca = newContainer.GetCustomAttribute(schemaName, className);
            BeAssert(ca.IsValid());
            if (ca.IsNull())
                return ERROR;

            if (ImportECClass(ca->GetClass()) != SUCCESS)
                return ERROR;

            if (InsertCAEntry(ca.get(), ca->GetClass().GetId(), containerId, containerType, 0) != SUCCESS)
                return ERROR;
            }
        else if (change.GetState() == ChangeState::Deleted)
            {
            //DELETE FROM ec_CustomAttribute WHERE ECContainerId =? AND ECContainerType = ? AND ECClassId =?
            IECInstancePtr ca = oldContainer.GetCustomAttribute(schemaName, className);
            BeAssert(ca.IsValid());
            if (ca.IsNull())
                return ERROR;

            if (!ca->GetClass().HasId())
                ECDbSchemaManager::GetClassIdForECClassFromDuplicateECSchema(m_ecdb, ca->GetClass()); //Callers will assume it has a valid Id

            if (DeleteCAEntry(ca->GetClass().GetId(), containerId, containerType) != SUCCESS)
                return ERROR;
            }
        else if (change.GetState() == ChangeState::Modified)
            {
            IECInstancePtr ca = newContainer.GetCustomAttribute(schemaName, className);
            BeAssert(ca.IsValid());
            if (ca.IsNull())
                return ERROR;

            if (ImportECClass(ca->GetClass()) != SUCCESS)
                return ERROR;

            if (ReplaceCAEntry(ca.get(), ca->GetClass().GetId(), containerId, containerType, 0) != SUCCESS)
                return ERROR;
            }

        change.SetIsApplied();
        }

    instanceChanges.SetIsApplied();
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbSchemaWriter::UpdateECClass(ECClassChange& classChange, ECClassCR oldClass, ECClassCR newClass)
    {
    if (classChange.IsApplied())
        return SUCCESS;

    auto classId = ECDbSchemaManager::GetClassIdForECClassFromDuplicateECSchema(m_ecdb, newClass);
    if (!classId.IsValid())
        {
        BeAssert(false && "Failed to resolve ecclass id");
        return ERROR;
        }


    SqlUpdater updater("ec_Class");

    if (classChange.GetClassModifier().IsValid())
        {
        return Fail("ECSCHEMA-UPGRADE: Changing ECClass::Modifier is not supported. Failed on ECClass %s.",
                    oldClass.GetFullName());
        }
    if (classChange.IsEntityClass().IsValid())
        {
        return Fail("ECSCHEMA-UPGRADE: Changing ECClass::IsEntityClass is not supported. Failed on ECClass %s.",
                    oldClass.GetFullName());
        }
    if (classChange.IsStructClass().IsValid())
        {
        return Fail("ECSCHEMA-UPGRADE: Changing ECClass::IsStructClass is not supported. Failed on ECClass %s.",
                    oldClass.GetFullName());
        }
    if (classChange.IsCustomAttributeClass().IsValid())
        {
        return Fail("ECSCHEMA-UPGRADE: Changing ECClass::IsCustomAttributeClass is not supported. Failed on ECClass %s.",
                    oldClass.GetFullName());
        }
    if (classChange.IsRelationshipClass().IsValid())
        {
        return Fail("ECSCHEMA-UPGRADE: Changing ECClass::IsRelationshipClass is not supported. Failed on ECClass %s.",
                    oldClass.GetFullName());
        }

    if (classChange.GetName().IsValid())
        {
        if (classChange.GetName().GetNew().IsNull())
            return Fail("ECSCHEMA-UPGRADE: Cannot set ECClass::Name to null. Failed while renaming ECClass %s.",
                        oldClass.GetFullName());

        updater.Set("Name", classChange.GetName().GetNew().Value());
        }
    if (classChange.GetDisplayLabel().IsValid())
        {
        updater.Set("DisplayLabel", classChange.GetDisplayLabel().GetNew().Value());
        }
    if (classChange.GetDescription().IsValid())
        {
        updater.Set("Description", classChange.GetDescription().GetNew().Value());
        }
    if (classChange.GetRelationship().IsValid())
        {
        auto& relationshipChange = classChange.GetRelationship();
        if (relationshipChange.GetStrength().IsValid())
            {
            return Fail("ECSCHEMA-UPGRADE: Changing ECRelationshipClass::Strength is not supported. Failed on ECRelationshipClass %s.",
                        oldClass.GetFullName());
            }
        if (relationshipChange.GetStrengthDirection().IsValid())
            {
            return Fail("ECSCHEMA-UPGRADE: Changing ECRelationshipClass::StrengthDirection is not supported. Failed on ECRelationshipClass %s.",
                        oldClass.GetFullName());
            }
        auto oldRel = oldClass.GetRelationshipClassCP();
        auto newRel = newClass.GetRelationshipClassCP();
        BeAssert(oldRel != nullptr && newRel != nullptr);
        if (oldRel == nullptr && newRel == nullptr)
            return ERROR;

        if (relationshipChange.GetSource().IsValid())
            if (UpdateECRelationshipConstraint(classId, updater, relationshipChange.GetSource(), newRel->GetSource(), oldRel->GetSource(), true, oldRel->GetFullName()) == ERROR)
                return ERROR;

        if (relationshipChange.GetTarget().IsValid())
            if (UpdateECRelationshipConstraint(classId, updater, relationshipChange.GetTarget(), newRel->GetSource(), oldRel->GetTarget(), false, oldRel->GetFullName()) == ERROR)
                return ERROR;
        }

    updater.Where("Id", classId.GetValue());
    if (updater.Apply(m_ecdb) != SUCCESS)
        return ERROR;

    if (classChange.BaseClasses().IsValid())
        {
        for (size_t i = 0; i < classChange.BaseClasses().Count(); i++)
            {
            auto& change = classChange.BaseClasses().At(i);
            if (change.GetState() == ChangeState::Deleted)
                {
                return Fail("ECSCHEMA-UPGRADE: Deleting BaseClass is not supported. Failed on ECClass %s.",
                            oldClass.GetFullName());
                }
            else if (change.GetState() == ChangeState::New)
                {               
                return Fail("ECSCHEMA-UPGRADE: Adding new BaseClass is not supported. Failed on ECClass %s.",
                            oldClass.GetFullName());
                }
            else if (change.GetState() == ChangeState::Modified)
                {
                return Fail("ECSCHEMA-UPGRADE: Modifing BaseClass is not supported. Failed on ECClass %s.",
                            oldClass.GetFullName());
                }
            }
        }

    if (classChange.Properties().IsValid())
        {
        for (size_t i = 0; i < classChange.Properties().Count(); i++)
            {
            auto& change = classChange.Properties().At(i);
            if (change.GetState() == ChangeState::Deleted)
                {
                return Fail("ECSCHEMA-UPGRADE: Deleting ECProperty is not supported. Failed on ECClass %s.",
                            oldClass.GetFullName());
                }
            else if (change.GetState() == ChangeState::New)
                {
                ECPropertyCP newProperty = newClass.GetPropertyP(change.GetName().GetNew().Value().c_str(), false);
                if (newProperty == nullptr)
                    {
                    BeAssert(false && "Failed to find the class");
                    return ERROR;
                    }

                if (ImportECProperty(*newProperty, static_cast<int>(newClass.GetPropertyCount(false))) == ERROR)
                    return ERROR;
                }
            else if (change.GetState() == ChangeState::Modified)
                {
                ECPropertyCP oldProperty = oldClass.GetPropertyP(change.GetId(), false);
                ECPropertyCP newProperty = newClass.GetPropertyP(change.GetId(), false);
                if (oldProperty == nullptr)
                    {
                    BeAssert(false && "Failed to find property");
                    return ERROR;
                    }
                if (newProperty == nullptr)
                    {
                    BeAssert(false && "Failed to find property");
                    return ERROR;
                    }

                if (UpdateECProperty(change, *oldProperty, *newProperty) != SUCCESS)
                    return ERROR;
                }
            }
        }

    return UpdateECCustomAttributes(ECContainerType::Class, classId, classChange.CustomAttributes(), oldClass, newClass);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbSchemaWriter::UpdateECSchema(ECSchemaChange& schemaChange, ECSchemaCR oldSchema, ECSchemaCR newSchema)
    {
    if (schemaChange.IsApplied())
        return SUCCESS;
    
    auto schemaId =  ECDbSchemaManager::GetSchemaIdForECSchemaFromDuplicateECSchema(m_ecdb, newSchema);
    if (!schemaId.IsValid())
        {
        BeAssert(false && "Failed to resolve ecshema id");
        return ERROR;
        }

    SqlUpdater updater("ec_Schema");
    if (schemaChange.GetName().IsValid())
        {
        if (schemaChange.GetName().GetNew().IsNull())
            return Fail ("ECSCHEMA-UPGRADE: Cannot set ECSchema::Name to null. Failed while renaming ECSchema %s.",
                    oldSchema.GetFullSchemaName().c_str());

        updater.Set("Name", schemaChange.GetName().GetNew().Value());
        }
    if (schemaChange.GetDisplayLabel().IsValid())
        {
        updater.Set("DisplayLabel", schemaChange.GetDisplayLabel().GetNew().Value());
        }
    if (schemaChange.GetDescription().IsValid())
        {
        updater.Set("Description", schemaChange.GetDescription().GetNew().Value());
        }

    if (schemaChange.GetVersionMajor().IsValid())
        {
        return Fail ("ECSCHEMA-UPGRADE: Any change to ECSchema::VersionMajor is not supported. Failed on ECSchema %s.",
                    oldSchema.GetFullSchemaName().c_str());
        //VersionDigit1
        return ERROR;
       }
    if (schemaChange.GetVersionMinor().IsValid())
        {
        if (schemaChange.GetVersionMinor().GetValue(ValueId::Deleted).Value() > schemaChange.GetVersionMinor().GetValue(ValueId::New).Value())
            {
            return Fail ("ECSCHEMA-UPGRADE: Cannot downgrade ECSchema::VersionMinor. Failed on ECSchema %s.",
                        oldSchema.GetFullSchemaName().c_str());

            return ERROR;
            }

        updater.Set("VersionDigit2", schemaChange.GetVersionMinor().GetNew().Value());
        }
    if (schemaChange.GetVersionWrite().IsValid())
        {
        if (schemaChange.GetVersionWrite().GetValue(ValueId::Deleted).Value() > schemaChange.GetVersionWrite().GetValue(ValueId::New).Value())
            {
            return Fail ("ECSCHEMA-UPGRADE: Cannot downgrade ECSchema::VersionWrite. Failed on ECSchema %s.",
                        oldSchema.GetFullSchemaName().c_str());

            return ERROR;
            }

        updater.Set("VersionDigit3", schemaChange.GetVersionWrite().GetNew().Value());
        }
    if (schemaChange.GetNamespacePrefix().IsValid())
        {
        if (schemaChange.GetNamespacePrefix().GetNew().IsNull())
            return Fail("ECSCHEMA-UPGRADE: Cannot set ECSchema::NamespacePrefix to null. Failed on ECSchema %s.",
                    oldSchema.GetFullSchemaName().c_str());

        if (ECDbSchemaPersistenceHelper::ContainsECSchemaWithNamespacePrefix(m_ecdb, schemaChange.GetNamespacePrefix().GetNew().Value().c_str()))
            {
            return Fail ("ECSCHEMA-UPGRADE: Invalid value for ECSchema::NamespacePrefix. Another schema with that namespacePrefix already exist. Failed on ECSchema %s.",
                        oldSchema.GetFullSchemaName().c_str());

            }

        updater.Set("NamespacePrefix", schemaChange.GetNamespacePrefix().GetNew().Value());
        }

    updater.Where("Id", schemaId.GetValue());//this could even be on name
    if (updater.Apply(m_ecdb) != SUCCESS)
        return ERROR;

    schemaChange.SetIsApplied();

    if (schemaChange.References().IsValid())
        {
        for (size_t i = 0; i < schemaChange.References().Count(); i++)
            {
            auto& change = schemaChange.References().At(i);
            if (change.GetState() == ChangeState::Deleted)
                { 
                return Fail ("ECSCHEMA-UPGRADE: Deleting references is not supported. Failed on ECSchema %s.",
                            oldSchema.GetFullSchemaName().c_str());
                }
            else if (change.GetState() == ChangeState::New)
                { 
                //Ensure schema exist
                //INSERT INTO ec_SchemaReference (SchemaId, ReferencedSchemaId) VALUES (?, ?) 
                return Fail("ECSCHEMA-UPGRADE: Adding new references is not supported. Failed on ECSchema %s.",
                            oldSchema.GetFullSchemaName().c_str());
                }
            else if (change.GetState() == ChangeState::Modified)
                {
                return Fail("ECSCHEMA-UPGRADE: Modifing references is not supported. Failed on ECSchema %s.",
                            oldSchema.GetFullSchemaName().c_str());
                }
            }
        }

    if (schemaChange.Classes().IsValid())
        {
        for (size_t i = 0; i < schemaChange.Classes().Count(); i++)
            {
            auto& change = schemaChange.Classes().At(i);
            if (change.GetState() == ChangeState::Deleted)
                {
                return Fail ("ECSCHEMA-UPGRADE: Deleting ECClass is not supported. Failed on ECSchema %s.",
                            oldSchema.GetFullSchemaName().c_str());
                }
            else if (change.GetState() == ChangeState::New)
                {
                ECClassCP newClass = newSchema.GetClassCP(change.GetName().GetNew().Value().c_str());
                if (newClass == nullptr)
                    {
                    BeAssert(false && "Failed to find the class");
                    return ERROR;
                    }

                if (ImportECClass(*newClass) == ERROR)
                    return ERROR; 
                }
            else if (change.GetState() == ChangeState::Modified)
                {
                ECClassCP oldClass = oldSchema.GetClassCP(change.GetId());
                ECClassCP newClass = newSchema.GetClassCP(change.GetId());
                if (oldClass == nullptr)
                    {
                    BeAssert(false && "Failed to find class");
                    return ERROR;
                    }
                if (newClass == nullptr)
                    {
                    BeAssert(false && "Failed to find class");
                    return ERROR;
                    }

                if (UpdateECClass(change, *oldClass, *newClass) != SUCCESS)
                    return ERROR;
                }
            }

        return SUCCESS;
        }

    if (schemaChange.Enumerations().IsValid())
        {
        for (size_t i = 0; i < schemaChange.Enumerations().Count(); i++)
            {

            auto& change = schemaChange.Enumerations().At(i);
            if (change.GetState() == ChangeState::Deleted)
                {
                return Fail("ECSCHEMA-UPGRADE: Deleting enumeration is not supported. Failed on ECSchema %s.",
                            oldSchema.GetFullSchemaName().c_str());
                }
            else if (change.GetState() == ChangeState::New)
                {
                return Fail("ECSCHEMA-UPGRADE: Adding new enumeration is not supported. Failed on ECSchema %s.",
                            oldSchema.GetFullSchemaName().c_str());
                }
            else if (change.GetState() == ChangeState::Modified)
                {
                return Fail("ECSCHEMA-UPGRADE: Modifying enumeration is not supported. Failed on ECSchema %s.",
                            oldSchema.GetFullSchemaName().c_str());
                }
            }
        }

    return UpdateECCustomAttributes(ECContainerType::Schema, schemaId, schemaChange.CustomAttributes(), oldSchema, newSchema);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaWriter::Import(ECSchemaCompareContext& ctx, ECN::ECSchemaCR ecSchema)
    {
    BeMutexHolder lock(m_mutex);
    
    if (ECSchemaChange* schemaChange = ctx.GetChanges().Find(ecSchema.GetName().c_str()))
        {
        if (schemaChange->GetState() == ChangeState::Modified)
            {
            if (schemaChange->IsApplied())
                return SUCCESS;

            ECSchemaCP existingSchema = ctx.FindExistingSchema(schemaChange->GetId());
            BeAssert(existingSchema != nullptr);
            if (existingSchema == nullptr)
                return ERROR;

            return UpdateECSchema(*schemaChange, *existingSchema, ecSchema);
            }
        else if (schemaChange->GetState() == ChangeState::Deleted)
            {            
            if (schemaChange->IsApplied())
                return SUCCESS;

            schemaChange->SetIsApplied();
            Fail ("ECSCHEMA-UPGRADE: Deleting ECSchema is not supported. Failed while deleting ECSchema %s.", 
                        ecSchema.GetFullSchemaName().c_str());

            return ERROR;
            }
        else
            {
            if (schemaChange->IsApplied())
                return SUCCESS;
            }
        }

    if (ECDbSchemaPersistenceHelper::GetECSchemaId(m_ecdb, ecSchema.GetName().c_str()).IsValid())
        return SUCCESS;

    // GenerateId
    BeBriefcaseBasedId nextId;
    if (BE_SQLITE_OK != m_ecdb.GetECDbImplR().GetECSchemaIdSequence().GetNextValue(nextId))
        {
        BeAssert(false && "Could not generate new ECSchemaId");
        return ERROR;
        }

    const ECSchemaId ecSchemaId(nextId.GetValue ());
    const_cast<ECSchemaR>(ecSchema).SetId(ecSchemaId);

    if (SUCCESS != CreateECSchemaEntry(ecSchema))
        {
        DbResult lastErrorCode;
        m_ecdb.GetLastError(&lastErrorCode);
        if (BE_SQLITE_CONSTRAINT_UNIQUE == lastErrorCode)
            m_ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Failed to import ECSchema '%s'. Namespace prefix '%s' is already used by an existing ECSchema.", 
                                                            ecSchema.GetFullSchemaName().c_str(), ecSchema.GetNamespacePrefix().c_str());
        return ERROR;
        }

    ECSchemaReferenceListCR referencedSchemas = ecSchema.GetReferencedSchemas();
    for (ECSchemaReferenceList::const_iterator iter = referencedSchemas.begin(); iter != referencedSchemas.end(); ++iter)
        {
        ECSchemaCP reference = iter->second.get();
        ECSchemaId referenceId = ECDbSchemaPersistenceHelper::GetECSchemaId(m_ecdb, reference->GetName().c_str());
        if (!referenceId.IsValid())
            {
            BeAssert(false && "BuildDependencyOrderedSchemaList used by caller should have ensured that all references are already imported");
            return ERROR;
            }

        if (!reference->HasId())
            {
            // We apparently have a second copy of an ECSchema that has already been imported. Ideally, we would have used the ECDbSchemaManager
            // as an IECSchemaLocater when we loaded the ECSchemas that we are imported, but since we did not, we simply *hope* that 
            // the duplicated loaded from disk matches the one stored in the db.
            // The duplicate copy does not have its Ids set... and so we will have to look them up, here and elsewhere, and set them into 
            // the in-memory duplicate copy. In Graphite02, we might risk cleaning this up to force use of the already-persisted ECSchema
            // or else to do a one-shot validation of the ECSchema and updating of its ids. 
            // Grep for GetClassIdForECClassFromDuplicateECSchema and GetPropertyIdForECPropertyFromDuplicateECSchema for other ramifications of this.
            const_cast<ECSchemaP>(reference)->SetId(referenceId);
            }

        if (SUCCESS != CreateECSchemaReferenceEntry(ecSchemaId, referenceId))
            {
            BeAssert(false && "Could not insert schema reference entry");
            return ERROR;
            }
        }

    //enums must be imported before ECClasses as properties reference enums
    for (ECEnumerationCP ecEnum : ecSchema.GetEnumerations())
        {
        if (SUCCESS != ImportECEnumeration(*ecEnum))
            {
            m_ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Failed to import ECEnumeration '%s'.", ecEnum->GetFullName().c_str());
            return ERROR;
            }
        }

    for (ECClassCP ecClass : ecSchema.GetClasses())
        {
        if (SUCCESS != ImportECClass(*ecClass))
            {
            m_ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Failed to import ECClass '%s'.", ecClass->GetFullName());
            return ERROR;
            }
        }

    if (SUCCESS != ImportCustomAttributes(ecSchema, ECContainerId(ecSchemaId), ECContainerType::Schema))
        {
        m_ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Failed to import custom attributes of ECSchema '%s'.", ecSchema.GetFullSchemaName().c_str());
        return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaWriter::CreateECSchemaReferenceEntry(ECSchemaId schemaId, ECSchemaId referencedSchemaId)
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != m_ecdb.GetCachedStatement(stmt, "INSERT INTO ec_SchemaReference (SchemaId,ReferencedSchemaId) VALUES(?,?)"))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, schemaId))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(2, referencedSchemaId))
        return ERROR;

    return BE_SQLITE_DONE == stmt->Step() ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaWriter::ImportCustomAttributes(IECCustomAttributeContainerCR sourceContainer, ECContainerId sourceContainerId, ECContainerType containerType, Utf8CP onlyImportCAWithClassName)
    {
    //import CA classes first
    for (IECInstancePtr ca : sourceContainer.GetCustomAttributes(false))
        {
        if (SUCCESS != ImportECClass(ca->GetClass()))
            return ERROR;
        }

    bmap<ECClassCP,bvector<IECInstanceP> > customAttributeMap;
    for (auto& customAttribute: sourceContainer.GetCustomAttributes(false))
        {
        if (onlyImportCAWithClassName == nullptr)
            customAttributeMap [&(customAttribute->GetClass())].push_back(customAttribute.get());
        else
            {
            if (customAttribute->GetClass().GetName().Equals(onlyImportCAWithClassName))
                customAttributeMap [&(customAttribute->GetClass())].push_back(customAttribute.get());
            }
        }
    int index = 0; // Its useless if we enumerate map since it doesn't ensure order in which we added it

    bmap<ECClassCP,bvector<IECInstanceP> >::const_iterator itor = customAttributeMap.begin();

    //Here we consider consolidated attribute a primary. This is lossy operation some overridden primary custom attributes would be lost
    for ( ; itor != customAttributeMap.end(); ++itor)
        {
        bvector<IECInstanceP> const& customAttributes = itor->second;
        IECInstanceP customAttribute = customAttributes.size() == 1 ? customAttributes[0] : customAttributes[1];
        ECClassCP ecClass = itor->first;
        ECClassId customAttributeClassId;
        if (ecClass->HasId())
            customAttributeClassId = ecClass->GetId();
        else
            customAttributeClassId = ECDbSchemaManager::GetClassIdForECClassFromDuplicateECSchema (m_ecdb, *ecClass);

        if (SUCCESS != InsertCAEntry (customAttribute, customAttributeClassId, sourceContainerId, containerType, index++))
            return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaWriter::EnsureECSchemaExists(ECClassCR ecClass)
    {
    ECSchemaCR schema = ecClass.GetSchema();
    ECSchemaId ecSchemaId = schema.GetId();

    if (ECDbSchemaPersistenceHelper::ContainsECSchema(m_ecdb, ecSchemaId))
        return SUCCESS;

    BeAssert(false && "I think we just should assume that the entry already exists, rather than relying on just-in-time? Or is this for when we branch off into related ECClasses?");
    //Add ECSchema entry but do not traverse its ECClasses.
    if (SUCCESS != CreateECSchemaEntry(schema))
        return ERROR;

    //import CA classes first
    for (IECInstancePtr ca : schema.GetCustomAttributes(false))
        {
        if (SUCCESS != ImportECClass(ca->GetClass()))
            return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaWriter::ImportECClass(ECN::ECClassCR ecClass)
    {
    if (ECDbSchemaPersistenceHelper::ContainsECClass(m_ecdb, ecClass))
        {
        if (!ecClass.HasId())
            ECDbSchemaManager::GetClassIdForECClassFromDuplicateECSchema(m_ecdb, ecClass); //Callers will assume it has a valid Id

        return SUCCESS;
        }

    // GenerateId
    BeBriefcaseBasedId nextId;
    if (BE_SQLITE_OK != m_ecdb.GetECDbImplR().GetECClassIdSequence().GetNextValue(nextId))
        return ERROR;

    ECClassId ecClassId(nextId.GetValue ());
    const_cast<ECClassR>(ecClass).SetId (ecClassId);

    EnsureECSchemaExists(ecClass);

    //now import actual ECClass
    BeSQLite::CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != m_ecdb.GetCachedStatement(stmt, "INSERT INTO ec_Class(Id,SchemaId,Name,DisplayLabel,Description,Type,Modifier,RelationshipStrength,RelationshipStrengthDirection) "
                                              "VALUES(?,?,?,?,?,?,?,?,?)"))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, ecClassId))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(2, ecClass.GetSchema().GetId()))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindText(3, ecClass.GetName().c_str(), Statement::MakeCopy::No))
        return ERROR;

    if (ecClass.GetIsDisplayLabelDefined())
        {
        if (BE_SQLITE_OK != stmt->BindText(4, ecClass.GetDisplayLabel().c_str(), Statement::MakeCopy::No))
            return ERROR;
        }

    if (BE_SQLITE_OK != stmt->BindText(5, ecClass.GetDescription().c_str(), Statement::MakeCopy::No))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt(6, Enum::ToInt(ecClass.GetClassType())))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt(7, Enum::ToInt(ecClass.GetClassModifier())))
        return ERROR;

    ECRelationshipClassCP relClass = ecClass.GetRelationshipClassCP();
    if (relClass != nullptr)
        {
        if (BE_SQLITE_OK != stmt->BindInt(8, Enum::ToInt(relClass->GetStrength())))
            return ERROR;

        if (BE_SQLITE_OK != stmt->BindInt(9, Enum::ToInt(relClass->GetStrengthDirection())))
            return ERROR;
        }

    if (BE_SQLITE_DONE != stmt->Step())
        return ERROR;

    //release stmt so that it can be reused to insert base classes
    stmt = nullptr;

    //Import All baseCases
    int baseClassIndex = 0;
    for (ECClassCP baseClass : ecClass.GetBaseClasses())
        {
        if (SUCCESS != ImportECClass(*baseClass))
            return ERROR;

        if (SUCCESS != CreateBaseClassEntry(ecClassId, *baseClass, baseClassIndex++))
            return ERROR;
        }

    int propertyIndex = 0;
    for (ECPropertyCP ecProperty: ecClass.GetProperties(false))
        {
        if (SUCCESS != ImportECProperty(*ecProperty, propertyIndex++))
            {
            LOG.errorv("Failed to import ECProperty '%s' of ECClass '%s'.", ecProperty->GetName().c_str(), ecClass.GetFullName());
            return ERROR;
            }
        }

    ECN::ECRelationshipClassCP relationship = ecClass.GetRelationshipClassCP();
    if (relationship != nullptr)
        {
        if (SUCCESS != ImportECRelationshipClass(relationship))
            return ERROR;
        }

    return ImportCustomAttributes(ecClass, ECContainerId(ecClassId), ECContainerType::Class);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  12/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbSchemaWriter::ImportECEnumeration(ECN::ECEnumerationCR ecEnum)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != m_ecdb.GetCachedStatement(stmt, "INSERT INTO ec_Enumeration(Id, SchemaId, Name, DisplayLabel, Description, UnderlyingPrimitiveType, IsStrict, EnumValues) VALUES(?,?,?,?,?,?,?,?)"))
        return ERROR;

    BeBriefcaseBasedId enumId;
    if (m_ecdb.GetECDbImplR().GetECEnumIdSequence().GetNextValue(enumId))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, enumId))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(2, ecEnum.GetSchema().GetId()))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindText(3, ecEnum.GetName().c_str(), Statement::MakeCopy::No))
        return ERROR;

    if (ecEnum.GetIsDisplayLabelDefined())
        {
        if (BE_SQLITE_OK != stmt->BindText(4, ecEnum.GetDisplayLabel().c_str(), Statement::MakeCopy::No))
            return ERROR;
        }

    if (BE_SQLITE_OK != stmt->BindText(5, ecEnum.GetDescription().c_str(), Statement::MakeCopy::No))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt(6, (int) ecEnum.GetType()))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt(7, ecEnum.GetIsStrict() ? 1 : 0))
        return ERROR;

    Utf8String enumValueJson;
    if (SUCCESS != ECDbSchemaPersistenceHelper::SerializeECEnumerationValues(enumValueJson, ecEnum))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindText(8, enumValueJson.c_str(), Statement::MakeCopy::No))
        return ERROR;

    if (BE_SQLITE_DONE != stmt->Step())
        return ERROR;

    //cache the id because the ECEnumeration class itself doesn't have an id.
    m_enumIdCache[&ecEnum] = enumId.GetValue();
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaWriter::ImportECRelationshipClass(ECN::ECRelationshipClassCP relationship)
    {
    const ECClassId relClassId = relationship->GetId();
    if (SUCCESS != ImportECRelationshipConstraint(relClassId, relationship->GetSource(), ECRelationshipEnd_Source))
        return ERROR;

    return ImportECRelationshipConstraint(relClassId, relationship->GetTarget(), ECRelationshipEnd_Target);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaWriter::ImportECRelationshipConstraint(ECClassId relClassId, ECN::ECRelationshipConstraintR relationshipConstraint, ECRelationshipEnd end)
    {
    BeAssert(relClassId.IsValid());

    if (SUCCESS != CreateECRelationshipConstraintEntry(relClassId, relationshipConstraint, end))
        return ERROR;

    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != m_ecdb.GetCachedStatement(stmt, "INSERT INTO ec_RelationshipConstraintClass(RelationshipClassId,RelationshipEnd,ClassId,KeyProperties) VALUES(?,?,?,?)"))
        return ERROR;

    for (ECRelationshipConstraintClassCP constraintClassObj : relationshipConstraint.GetConstraintClasses())
        {
        ECClassCR constraintClass = constraintClassObj->GetClass();
        if (SUCCESS != ImportECClass(constraintClass))
            return ERROR;

        BeAssert(constraintClass.GetId().IsValid());

        if (BE_SQLITE_OK != stmt->BindId(1, relClassId))
            return ERROR;

        if (BE_SQLITE_OK != stmt->BindInt(2, (int) end))
            return ERROR;

        if (BE_SQLITE_OK != stmt->BindId(3, constraintClass.GetId()))
            return ERROR;

        bvector<Utf8String> const& keyPropNames = constraintClassObj->GetKeys();
        Utf8String keyPropJson;
        if (!keyPropNames.empty())
            {
            ECDbSchemaPersistenceHelper::SerializeRelationshipKeyProperties(keyPropJson, keyPropNames);
            if (BE_SQLITE_OK != stmt->BindText(4, keyPropJson.c_str(), Statement::MakeCopy::No))
                return ERROR;
            }

        if (BE_SQLITE_DONE != stmt->Step())
            return ERROR;

        stmt->Reset();
        stmt->ClearBindings();
        }

    stmt = nullptr;

    ECContainerType containerType = end == ECRelationshipEnd_Source ? ECContainerType::RelationshipConstraintSource : ECContainerType::RelationshipConstraintTarget;
    return ImportCustomAttributes(relationshipConstraint, ECContainerId(relClassId), containerType);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaWriter::ImportECProperty(ECN::ECPropertyCR ecProperty, int32_t ordinal)
    {
    // GenerateId
    BeBriefcaseBasedId nextId;
    if (BE_SQLITE_OK != m_ecdb.GetECDbImplR().GetECPropertyIdSequence().GetNextValue(nextId))
        return ERROR;

    ECPropertyId ecPropertyId(nextId.GetValue());
    const_cast<ECPropertyR>(ecProperty).SetId(ecPropertyId);

    if (ecProperty.GetIsStruct())
        {
        if (SUCCESS != ImportECClass(ecProperty.GetAsStructProperty()->GetType()))
            return ERROR;
        }
    else if (ecProperty.GetIsArray())
        {
        StructArrayECPropertyCP structArrayProperty = ecProperty.GetAsStructArrayProperty();
        if (nullptr != structArrayProperty)
            {
            if (SUCCESS != ImportECClass(*structArrayProperty->GetStructElementType()))
                return ERROR;
            }
        }
    else if (ecProperty.GetIsNavigation())
        {
        if (SUCCESS != ImportECClass(*ecProperty.GetAsNavigationProperty()->GetRelationshipClass()))
            return ERROR;
        }

    //now insert the actual property
    BeSQLite::CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != m_ecdb.GetCachedStatement(stmt, "INSERT INTO ec_Property(Id,ClassId,Name,DisplayLabel,Description,IsReadonly,Ordinal,Kind,PrimitiveType,NonPrimitiveType,ExtendedType,Enumeration,ArrayMinOccurs,ArrayMaxOccurs,NavigationPropertyDirection) VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)"))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, ecProperty.GetId()))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(2, ecProperty.GetClass().GetId()))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindText(3, ecProperty.GetName(), Statement::MakeCopy::No))
        return ERROR;

    if (ecProperty.GetIsDisplayLabelDefined())
        {
        if (BE_SQLITE_OK != stmt->BindText(4, ecProperty.GetDisplayLabel().c_str(), Statement::MakeCopy::No))
            return ERROR;
        }

    if (BE_SQLITE_OK != stmt->BindText(5, ecProperty.GetDescription().c_str(), Statement::MakeCopy::No))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt(6, ecProperty.GetIsReadOnly() ? 1 : 0))
        return ERROR;

    //WIP Ordinal
    if (BE_SQLITE_OK != stmt->BindInt(7, ordinal))
        return ERROR;

    const int kindIndex = 8;
    const int primitiveTypeIndex = 9;
    const int nonPrimitiveTypeIndex = 10;
    const int extendedTypeIndex = 11;
    const int enumTypeIndex = 12;
    const int arrayMinIndex = 13;
    const int arrayMaxIndex = 14;
    const int navDirIndex = 15;

    if (ecProperty.GetIsPrimitive())
        {
        PrimitiveECPropertyCP primProp = ecProperty.GetAsPrimitiveProperty();
        ECEnumerationCP ecenum = primProp->GetEnumeration();
        if (ecenum == nullptr)
            {
            if (BE_SQLITE_OK != stmt->BindInt(kindIndex, Enum::ToInt(ECPropertyKind::Primitive)))
                return ERROR;

            if (BE_SQLITE_OK != stmt->BindInt(primitiveTypeIndex, (int) primProp->GetType()))
                return ERROR;
            }
        else
            {
            auto it = m_enumIdCache.find(ecenum);
            if (it == m_enumIdCache.end())
                {
                BeAssert(false && "ECEnumeration should have been imported before any ECProperty");
                return ERROR;
                }

            const uint64_t enumId = it->second;

            if (BE_SQLITE_OK != stmt->BindInt(kindIndex, Enum::ToInt(ECPropertyKind::Enumeration)))
                return ERROR;

            if (BE_SQLITE_OK != stmt->BindInt64(enumTypeIndex, enumId))
                return ERROR;
            }

        if (primProp->HasExtendedType())
            {
            if (BE_SQLITE_OK != stmt->BindText(extendedTypeIndex, primProp->GetExtendedTypeName().c_str(), Statement::MakeCopy::No))
                return ERROR;
            }
        }
    else if (ecProperty.GetIsStruct())
        {
        if (BE_SQLITE_OK != stmt->BindInt(kindIndex, Enum::ToInt(ECPropertyKind::Struct)))
            return ERROR;

        if (BE_SQLITE_OK != stmt->BindId(nonPrimitiveTypeIndex, ecProperty.GetAsStructProperty()->GetType().GetId()))
            return ERROR;
        }
    else if (ecProperty.GetIsArray())
        {
        ArrayECPropertyCP arrayProp = ecProperty.GetAsArrayProperty();
        if (arrayProp->GetKind() == ARRAYKIND_Primitive)
            {
            if (BE_SQLITE_OK != stmt->BindInt(kindIndex, Enum::ToInt(ECPropertyKind::PrimitiveArray)))
                return ERROR;

            if (BE_SQLITE_OK != stmt->BindInt(primitiveTypeIndex, (int) arrayProp->GetPrimitiveElementType()))
                return ERROR;
            }
        else
            {
            if (BE_SQLITE_OK != stmt->BindInt(kindIndex, Enum::ToInt(ECPropertyKind::StructArray)))
                return ERROR;

            if (BE_SQLITE_OK != stmt->BindId(nonPrimitiveTypeIndex, arrayProp->GetAsStructArrayProperty()->GetStructElementType()->GetId()))
                return ERROR;
            }

        if (arrayProp->HasExtendedType())
            {
            if (BE_SQLITE_OK != stmt->BindText(extendedTypeIndex, arrayProp->GetExtendedTypeName().c_str(), Statement::MakeCopy::No))
                return ERROR;
            }

        if (BE_SQLITE_OK != stmt->BindInt(arrayMinIndex, (int) arrayProp->GetMinOccurs()))
            return ERROR;

        //until the max occurs bug in ECObjects (where GetMaxOccurs always returns "unbounded")
        //has been fixed, we need to call GetStoredMaxOccurs to retrieve the proper max occurs
        if (BE_SQLITE_OK != stmt->BindInt(arrayMaxIndex, (int) arrayProp->GetStoredMaxOccurs()))
            return ERROR;
        }
    else if (ecProperty.GetIsNavigation())
        {
        if (BE_SQLITE_OK != stmt->BindInt(kindIndex, Enum::ToInt(ECPropertyKind::Navigation)))
            return ERROR;

        NavigationECPropertyCP navProp = ecProperty.GetAsNavigationProperty();
        if (BE_SQLITE_OK != stmt->BindId(nonPrimitiveTypeIndex, navProp->GetRelationshipClass()->GetId()))
            return ERROR;

        if (BE_SQLITE_OK != stmt->BindInt(navDirIndex, Enum::ToInt(navProp->GetDirection())))
            return ERROR;
        }

    if (BE_SQLITE_DONE != stmt->Step())
        return ERROR;

    return ImportCustomAttributes(ecProperty, ECContainerId(ecPropertyId), ECContainerType::Property);
    }
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SqlUpdater::BindSet(Statement& stmt, Utf8StringCR column, int i) const
    {
    auto itor = m_updateMap.find(column);
    if (itor == m_updateMap.end())
        return ERROR;

    switch (itor->second.GetPrimitiveType())
        {
            case PRIMITIVETYPE_Integer:
                return stmt.BindInt(i, itor->second.GetInteger()) == BE_SQLITE_OK ? SUCCESS : ERROR;
            case PRIMITIVETYPE_Long:
                return stmt.BindInt64(i, itor->second.GetLong()) == BE_SQLITE_OK ? SUCCESS : ERROR;
            case PRIMITIVETYPE_Double:
                return stmt.BindDouble(i, itor->second.GetDouble()) == BE_SQLITE_OK ? SUCCESS : ERROR;
            case PRIMITIVETYPE_String:
                return stmt.BindText(i, itor->second.GetUtf8CP(), Statement::MakeCopy::No) == BE_SQLITE_OK ? SUCCESS : ERROR;
            case PRIMITIVETYPE_Boolean:
                return stmt.BindInt(i, itor->second.GetBoolean()) == BE_SQLITE_OK ? SUCCESS : ERROR;
        }

    BeAssert(false && "Unsupported case");
    return ERROR;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SqlUpdater::BindWhere(Statement& stmt, Utf8StringCR column, int i) const
    {
    auto itor = m_whereMap.find(column);
    if (itor == m_whereMap.end())
        return ERROR;

    switch (itor->second.GetPrimitiveType())
        {
            case PRIMITIVETYPE_Integer:
                return stmt.BindInt(i, itor->second.GetInteger()) == BE_SQLITE_OK ? SUCCESS : ERROR;
            case PRIMITIVETYPE_Long:
                return stmt.BindInt64(i, itor->second.GetLong()) == BE_SQLITE_OK ? SUCCESS : ERROR;
            case PRIMITIVETYPE_Double:
                return stmt.BindDouble(i, itor->second.GetDouble()) == BE_SQLITE_OK ? SUCCESS : ERROR;
            case PRIMITIVETYPE_String:
                return stmt.BindText(i, itor->second.GetUtf8CP(), Statement::MakeCopy::No) == BE_SQLITE_OK ? SUCCESS : ERROR;
            case PRIMITIVETYPE_Boolean:
                return stmt.BindInt(i, itor->second.GetBoolean()) == BE_SQLITE_OK ? SUCCESS : ERROR;
        }

    BeAssert(false && "Unsupported case");
    return ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SqlUpdater::Apply(ECDb const& ecdb) const
    {
    if (m_updateMap.empty())
        return SUCCESS;

    NativeSqlBuilder sql;
    sql.Append("UPDATE ").AppendEscaped(m_table.c_str()).Append(" SET ");
    bool first = true;
    for (auto& key : m_updateMap)
        {
        if (first)
            first = false;
        else
            sql.Append(", ");

        sql.Append(key.first.c_str()).Append(" = ?");
        }

    if (m_whereMap.empty())
        {
        BeAssert(false && "WHERE must not be empty");
        return ERROR;
        }
    sql.Append(" WHERE ");
    first = true;
    for (auto& key : m_whereMap)
        {
        if (first)
            first = false;
        else
            sql.Append(" AND ");

        sql.Append(key.first.c_str()).Append(" = ?");
        }

    Statement stmt;
    if (stmt.Prepare(ecdb, sql.ToString()) != BE_SQLITE_OK)
        return ERROR;

    int i = 1;
    for (auto& key : m_updateMap)
        if (BindSet(stmt, key.first, i++) != SUCCESS)
            return ERROR;
    
    for (auto& key : m_whereMap)
        if (BindWhere(stmt, key.first, i++) != SUCCESS)
            return ERROR;

    auto r =  stmt.Step() == BE_SQLITE_DONE ? SUCCESS : ERROR;
    BeAssert(ecdb.GetModifiedRowCount() > 0);
    return r;
    }
void SqlUpdater::Set(Utf8CP column, Utf8CP value)
    {
    m_updateMap[column] = ECN::ECValue(value);
    }
void SqlUpdater::Set(Utf8CP column, Utf8StringCR value)
    {
    m_updateMap[column] = ECN::ECValue(value.c_str());
    }
void SqlUpdater::Set(Utf8CP column, double value)
    {
    m_updateMap[column] = ECN::ECValue(value);
    }
void SqlUpdater::Set(Utf8CP column, bool value)
    {
    m_updateMap[column] = ECN::ECValue(value);
    }
void SqlUpdater::Set(Utf8CP column, uint32_t value)
    {
    m_updateMap[column] = ECN::ECValue(static_cast<int64_t>(value));
    }
void SqlUpdater::Set(Utf8CP column, uint64_t value)
    {
    m_updateMap[column] = ECN::ECValue(static_cast<int64_t>(value));
    }
void SqlUpdater::Set(Utf8CP column, int32_t value)
    {
    m_updateMap[column] = ECN::ECValue(value);
    }
void SqlUpdater::Set(Utf8CP column, int64_t value)
    {
    m_updateMap[column] = ECN::ECValue(value);
    }
void SqlUpdater::Where(Utf8CP column, int64_t value)
    {
    m_whereMap[column] = ECN::ECValue(value);
    }
END_BENTLEY_SQLITE_EC_NAMESPACE
