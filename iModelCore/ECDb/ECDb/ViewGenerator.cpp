/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ViewGenerator.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECDbPolicyManager.h"
#include <set>
#include "ECSql/ECSqlPrepareContext.h"
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
#define ECDB_HOLDING_VIEW "ec_RelationshipHoldingStatistics"
    //---------------------------------------------------------------------------------------
    // @bsimethod                                 Affan.Khan                         06/2015
    //---------------------------------------------------------------------------------------
    BentleyStatus SqlGenerator::BuildPropertyExpression (NativeSqlBuilder& viewSql, PropertyMapCR propertyMap, Utf8CP tablePrefix, bool addECPropertyPathAlias, bool nullValue)
        {
        if (auto o = dynamic_cast<PropertyMapToColumn const*>(&propertyMap))
            {
            return BuildPrimitivePropertyExpression (viewSql, *o, tablePrefix, addECPropertyPathAlias, nullValue);
            }
        if (auto o = dynamic_cast<PropertyMapPoint const*>(&propertyMap))
            {
            return BuildPointPropertyExpression (viewSql, *o, tablePrefix, addECPropertyPathAlias, nullValue);
            }
        else if (auto o = dynamic_cast<PropertyMapToInLineStruct const*>(&propertyMap))
            {
            return BuildStructPropertyExpression (viewSql, *o, tablePrefix, addECPropertyPathAlias, nullValue);
            }
        else if (/*auto o = */dynamic_cast<PropertyMapToTable const*>(&propertyMap))
            {
            return BentleyStatus::SUCCESS;
            }
        else if (/*auto o = */dynamic_cast<PropertyMapSystem const*>(&propertyMap))
            {
            return BentleyStatus::SUCCESS;
            }

        BeAssert (false && "Case not handled");
        return BentleyStatus::ERROR;
        }
    //---------------------------------------------------------------------------------------
    // @bsimethod                                 Affan.Khan                         06/2015
    //---------------------------------------------------------------------------------------
BentleyStatus SqlGenerator::BuildColumnExpression (NativeSqlBuilder::List& viewSql, Utf8CP tablePrefix, Utf8CP columnName, Utf8CP accessString, bool addECPropertyPathAlias, bool nullValue, bool escapeColumName)
    {
    NativeSqlBuilder sqlBuilder;

    if (nullValue)
        {
        columnName = "NULL";
        sqlBuilder.Append (columnName);
        }
    else
        {
        if (columnName == nullptr)
            {
            BeAssert (columnName != nullptr);
            return BentleyStatus::ERROR;
            }

        if (escapeColumName)
            {
            if (tablePrefix)
                {
                sqlBuilder.AppendEscaped (tablePrefix).AppendDot ();
                }

            sqlBuilder.AppendEscaped (columnName);
            }
        else
            sqlBuilder.Append (columnName);
        }

    if (addECPropertyPathAlias)
        {
        if (accessString == nullptr)
            {
            BeAssert (accessString != nullptr);
            return BentleyStatus::ERROR;
            }

        else if (strcmp (accessString, columnName) != 0)
            sqlBuilder.AppendSpace ().AppendEscaped (accessString);
        }

    viewSql.push_back (std::move (sqlBuilder));
    return BentleyStatus::SUCCESS;
    }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                 Affan.Khan                         06/2015
    //---------------------------------------------------------------------------------------
    BentleyStatus SqlGenerator::BuildPointPropertyExpression (NativeSqlBuilder& viewSql, PropertyMapPoint const& propertyMap, Utf8CP tablePrefix, bool addECPropertyPathAlias, bool nullValue)
        {
        auto accessString = Utf8String (propertyMap.GetPropertyAccessString ());
        auto primitiveProperty = propertyMap.GetProperty ().GetAsPrimitiveProperty ();
        // accessString.ReplaceAll (".", "_");

        NativeSqlBuilder::List fragments;
        std::vector<ECDbSqlColumn const*> columns;
        propertyMap.GetColumns (columns);


        if (!primitiveProperty->GetIsArray () && primitiveProperty->GetType () == PrimitiveType::PRIMITIVETYPE_Point2D)
            {
            if (columns.size () != 2LL)
                {
                BeAssert (columns.size () == 2LL);
                return BentleyStatus::ERROR;
                }
             
            BuildColumnExpression (fragments, tablePrefix, columns[0]->GetName ().c_str (),  (accessString + ".X").c_str (), addECPropertyPathAlias, nullValue);
            BuildColumnExpression (fragments, tablePrefix, columns[1]->GetName ().c_str (),  (accessString + ".Y").c_str (), addECPropertyPathAlias, nullValue);
            }
        else if (!primitiveProperty->GetIsArray () && primitiveProperty->GetType () == PrimitiveType::PRIMITIVETYPE_Point3D)
            {
            if (columns.size () != 3LL)
                {
                BeAssert (columns.size () == 3LL);
                return BentleyStatus::ERROR;
                }

            BuildColumnExpression (fragments, tablePrefix, columns[0]->GetName ().c_str (),  (accessString + ".X").c_str (), addECPropertyPathAlias, nullValue);
            BuildColumnExpression (fragments, tablePrefix, columns[1]->GetName ().c_str (),  (accessString + ".Y").c_str (), addECPropertyPathAlias, nullValue);
            BuildColumnExpression (fragments, tablePrefix, columns[2]->GetName ().c_str (),  (accessString + ".Z").c_str (), addECPropertyPathAlias, nullValue);
            }

        for (auto const& fragment : fragments)
            {
            if (&(fragments.front ()) != &(fragment))
                viewSql.Append (", ");

            viewSql.Append (fragment);
            }

        return BentleyStatus::SUCCESS;
        }
    //---------------------------------------------------------------------------------------
    // @bsimethod                                 Affan.Khan                         06/2015
    //---------------------------------------------------------------------------------------
    BentleyStatus SqlGenerator::BuildPrimitivePropertyExpression (NativeSqlBuilder& viewSql, PropertyMapToColumn const& propertyMap, Utf8CP tablePrefix, bool addECPropertyPathAlias, bool nullValue)
        {
        auto accessString = Utf8String (propertyMap.GetPropertyAccessString ());
        NativeSqlBuilder::List fragments;
        std::vector<ECDbSqlColumn const*> columns;
        propertyMap.GetColumns (columns);

        if (columns.size () != 1LL)
            {
            BeAssert (columns.size () == 1LL);
            return BentleyStatus::ERROR;
            }

        if (BuildColumnExpression (fragments, tablePrefix, columns[0]->GetName ().c_str (), accessString.c_str (), addECPropertyPathAlias, nullValue) != BentleyStatus::SUCCESS)
            return BentleyStatus::ERROR;

        viewSql.Append (fragments.front ());
        return BentleyStatus::SUCCESS;
        }
    //---------------------------------------------------------------------------------------
    // @bsimethod                                 Affan.Khan                         06/2015
    //---------------------------------------------------------------------------------------
    BentleyStatus SqlGenerator::BuildECInstanceIdConstraintExpression (NativeSqlBuilder::List& fragments, RelationshipClassMapCR classMap, ECN::ECRelationshipEnd endPoint, Utf8CP tablePrefix, bool addECPropertyPathAlias, bool nullValue)
        {
        auto propertyMap = static_cast<PropertyMapRelationshipConstraintECInstanceId const*>(endPoint == ECRelationshipEnd::ECRelationshipEnd_Source ? classMap.GetSourceECInstanceIdPropMap () : classMap.GetTargetECInstanceIdPropMap ());
        auto column = propertyMap->GetFirstColumn ();
        auto accessString = Utf8String (propertyMap->GetPropertyAccessString ());
        if (column->GetPersistenceType () == PersistenceType::Virtual)
            {
            BeAssert (false && "Source/Target ECInstanceId cannot be mapped to virtual column");
            return BentleyStatus::ERROR;
            }

        if (BuildColumnExpression (fragments, tablePrefix, column->GetName ().c_str (), accessString.c_str (), addECPropertyPathAlias, nullValue) != BentleyStatus::SUCCESS)
            return BentleyStatus::ERROR;

        return BentleyStatus::SUCCESS;
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                 Affan.Khan                         06/2015
    //---------------------------------------------------------------------------------------
    BentleyStatus SqlGenerator::BuildECClassIdConstraintExpression (NativeSqlBuilder::List& fragments, RelationshipClassMapCR classMap, ECN::ECRelationshipEnd endPoint, Utf8CP tablePrefix, bool addECPropertyPathAlias, bool nullValue)
        {
        auto propertyMap = static_cast<PropertyMapRelationshipConstraintClassId const*>(endPoint == ECRelationshipEnd::ECRelationshipEnd_Source ? classMap.GetSourceECClassIdPropMap () : classMap.GetTargetECClassIdPropMap ());
        auto& constraint = endPoint == ECRelationshipEnd::ECRelationshipEnd_Source ? classMap.GetClass ().GetRelationshipClassCP ()->GetSource () : classMap.GetClass ().GetRelationshipClassCP ()->GetTarget ();
        auto column = propertyMap->GetFirstColumn ();
        auto accessString = Utf8String (propertyMap->GetPropertyAccessString ());
        auto tableAlias = GetECClassIdPrimaryTableAlias (endPoint);

        if (column->GetPersistenceType () == PersistenceType::Persisted)
            {
            if (propertyMap->IsMappedToPrimaryTable())
                {
                if (BuildColumnExpression (fragments, tablePrefix, column->GetName ().c_str (), accessString.c_str (), addECPropertyPathAlias, nullValue) != BentleyStatus::SUCCESS)
                    return BentleyStatus::ERROR;
                }
            else
                {
                Utf8String columnQualifiedName;
                columnQualifiedName.append (tableAlias);
                columnQualifiedName.append (".");
                columnQualifiedName.append (column->GetName ());
                if (BuildColumnExpression (fragments, nullptr, columnQualifiedName.c_str (), accessString.c_str (), addECPropertyPathAlias, nullValue, false) != BentleyStatus::SUCCESS)
                    return BentleyStatus::ERROR;
                }
            }
        else
            {
            //if (propertyMap->GetDefaultConstraintECClassId () == 0)
            //    {
            //    BeAssert (false && "Default constraint classid must be provided");
            //    return BentleyStatus::ERROR;
            //    }
            auto eclassId = constraint.GetClasses ().front ()->GetId ();
            Utf8String columnValueExpr;
            columnValueExpr.Sprintf ("%lld", eclassId);

            if (BuildColumnExpression (fragments, nullptr, columnValueExpr.c_str (), accessString.c_str (), addECPropertyPathAlias, nullValue, false) != BentleyStatus::SUCCESS)
                return BentleyStatus::ERROR;
            }

        return BentleyStatus::SUCCESS;
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                 Affan.Khan                         06/2015
    //---------------------------------------------------------------------------------------
    BentleyStatus SqlGenerator::BuildStructPropertyExpression (NativeSqlBuilder& viewSql, PropertyMapToInLineStructCR propertyMap, Utf8CP tablePrefix, bool addECPropertyPathAlias, bool nullValue)
        {
        NativeSqlBuilder::List fragments;
        for (auto const childMap : propertyMap.GetChildren ())
            {
            NativeSqlBuilder fragment;
            if (BuildPropertyExpression (fragment, *childMap, tablePrefix, addECPropertyPathAlias, nullValue) != BentleyStatus::SUCCESS)
                return BentleyStatus::ERROR;

            if (!fragment.IsEmpty ())
                fragments.push_back (std::move (fragment));
            }

        for (auto const& fragment : fragments)
            {
            if (&(fragments.front ()) != &(fragment))
                viewSql.Append (", ");

            viewSql.Append (fragment);
            }

        return BentleyStatus::SUCCESS;
        }
    //---------------------------------------------------------------------------------------
    // @bsimethod                                 Affan.Khan                         06/2015
    //---------------------------------------------------------------------------------------
    BentleyStatus SqlGenerator::BuildSystemSelectionClause (NativeSqlBuilder::List& fragments, ClassMapCR baseClassMap, ClassMapCR classMap, Utf8CP tablePrefix, bool addECPropertyPathAlias, bool nullValue)
        {
        if (auto column = classMap.GetTable ().GetFilteredColumnFirst (ECDbKnownColumns::ECInstanceId))
            {
            if (BuildColumnExpression (fragments, tablePrefix, column->GetName ().c_str (), ECDbSystemSchemaHelper::ECINSTANCEID_PROPNAME, addECPropertyPathAlias, nullValue) != BentleyStatus::SUCCESS)
                return BentleyStatus::ERROR;
            }
        else
            {
            BeAssert (false && "Failed to find ECInstanceId column");
            return BentleyStatus::ERROR;
            }

        if (auto column = classMap.GetTable ().GetFilteredColumnFirst (ECDbKnownColumns::ECClassId))
            {
            if (BuildColumnExpression (fragments, tablePrefix, column->GetName ().c_str (), "ECClassId", addECPropertyPathAlias, nullValue) != BentleyStatus::SUCCESS)
                return BentleyStatus::ERROR;
            }
        else
            {
            Utf8String classIdStr;
            classIdStr.Sprintf ("%lld", classMap.GetClass ().GetId ());
            if (BuildColumnExpression (fragments, tablePrefix, classIdStr.c_str (), "ECClassId", addECPropertyPathAlias, nullValue, false) != BentleyStatus::SUCCESS)
                return BentleyStatus::ERROR;
            }

        if (baseClassMap.GetClass ().GetIsStruct ())
            {
            if (!classMap.GetClass ().GetIsStruct ())
                {
                BeAssert (false && "BaseClass is of type struct but not the child class which must also be struct type");
                return BentleyStatus::ERROR;
                }

            if (auto column = classMap.GetTable ().GetFilteredColumnFirst (ECDbKnownColumns::ParentECInstanceId))
                {
                if (BuildColumnExpression (fragments, tablePrefix, column->GetName ().c_str (), ECDbSystemSchemaHelper::PARENTECINSTANCEID_PROPNAME, addECPropertyPathAlias, nullValue) != BentleyStatus::SUCCESS)
                    return BentleyStatus::ERROR;
                }

            if (auto column = classMap.GetTable ().GetFilteredColumnFirst (ECDbKnownColumns::ECPropertyPathId))
                {
                if (BuildColumnExpression (fragments, tablePrefix, column->GetName ().c_str (), ECDbSystemSchemaHelper::ECPROPERTYPATHID_PROPNAME, addECPropertyPathAlias, nullValue) != BentleyStatus::SUCCESS)
                    return BentleyStatus::ERROR;
                }

            if (auto column = classMap.GetTable ().GetFilteredColumnFirst (ECDbKnownColumns::ECArrayIndex))
                {

                if (BuildColumnExpression (fragments, tablePrefix, column->GetName ().c_str (), ECDbSystemSchemaHelper::ECARRAYINDEX_PROPNAME, addECPropertyPathAlias, nullValue) != BentleyStatus::SUCCESS)
                    return BentleyStatus::ERROR;
                }
            }
        
        if (classMap.GetClassMapType () == IClassMap::Type::RelationshipEndTable || classMap.GetClassMapType () == IClassMap::Type::RelationshipLinkTable)
            {
            auto const& rel = static_cast<RelationshipClassMapCR>(classMap);
            if (BuildECInstanceIdConstraintExpression (fragments, rel, ECRelationshipEnd::ECRelationshipEnd_Source, tablePrefix, addECPropertyPathAlias, nullValue) != BentleyStatus::SUCCESS)
                return BentleyStatus::ERROR;

            if (BuildECClassIdConstraintExpression (fragments, rel, ECRelationshipEnd::ECRelationshipEnd_Source, tablePrefix,  addECPropertyPathAlias, nullValue) != BentleyStatus::SUCCESS)
                return BentleyStatus::ERROR;

            if (BuildECInstanceIdConstraintExpression (fragments, rel, ECRelationshipEnd::ECRelationshipEnd_Target, tablePrefix, addECPropertyPathAlias, nullValue) != BentleyStatus::SUCCESS)
                return BentleyStatus::ERROR;

            if (BuildECClassIdConstraintExpression (fragments, rel, ECRelationshipEnd::ECRelationshipEnd_Target, tablePrefix, addECPropertyPathAlias, nullValue) != BentleyStatus::SUCCESS)
                return BentleyStatus::ERROR;
            }

        return BentleyStatus::SUCCESS;
        }
    //---------------------------------------------------------------------------------------
    // @bsimethod                                 Affan.Khan                         06/2015
    //---------------------------------------------------------------------------------------
    BentleyStatus SqlGenerator::BuildSelectionClause (NativeSqlBuilder& viewSql, ClassMapCR baseClassMap, ClassMapCR classMap, Utf8CP tablePrefix, bool addECPropertyPathAlias, bool nullValue)
        {
        NativeSqlBuilder::List fragments;

        if (BuildSystemSelectionClause (fragments, baseClassMap, classMap, tablePrefix, true, nullValue) != BentleyStatus::SUCCESS)
            return BentleyStatus::ERROR;

        for (auto basePropertyMap : baseClassMap.GetPropertyMaps ())
            {
            auto propertyMap = classMap.GetPropertyMap (basePropertyMap->GetPropertyAccessString ());
            if (propertyMap == nullptr)
                {
                BeAssert (propertyMap != nullptr);
                return BentleyStatus::ERROR;
                }

            NativeSqlBuilder fragment;
            if (BuildPropertyExpression (fragment, *propertyMap, tablePrefix, addECPropertyPathAlias, nullValue)
                != BentleyStatus::SUCCESS)
                {
                return BentleyStatus::ERROR;
                }

            if (!fragment.IsEmpty ())
                fragments.push_back (std::move (fragment));
            }

        if (fragments.empty ())
            {
            BeAssert (false && "There is no property to create selection");
            return BentleyStatus::ERROR;
            }

        for (auto const& fragment : fragments)
            {
            if (&(fragments.front ()) != &(fragment))
                viewSql.Append (",").AppendEOL ();

            viewSql.Append ("\t\t");
            viewSql.Append (fragment);
            }

        return BentleyStatus::SUCCESS;
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                 Affan.Khan                         06/2015
    //---------------------------------------------------------------------------------------
     Utf8String SqlGenerator::BuildSchemaQualifiedClassName (ECClassCR ecClass)
        {
        Utf8String name;
        name.append (ecClass.GetSchema ().GetNamespacePrefix ());
        name.append ("_");
        name.append (ecClass.GetName ());
        return name;
        }
     //---------------------------------------------------------------------------------------
     // @bsimethod                                 Affan.Khan                         06/2015
     //---------------------------------------------------------------------------------------
     Utf8String SqlGenerator::BuildViewClassName (ECClassCR ecClass)
        {
        if (ecClass.GetRelationshipClassCP() != nullptr)
            return  "VR_" + BuildSchemaQualifiedClassName (ecClass);

        if (ecClass.GetIsStruct ())
            return  "VS_" + BuildSchemaQualifiedClassName (ecClass);

        return  "VC_" + BuildSchemaQualifiedClassName (ecClass);
        }
     //---------------------------------------------------------------------------------------
     // @bsimethod                                 Affan.Khan                         06/2015
     //---------------------------------------------------------------------------------------
     BentleyStatus SqlGenerator::BuildRelationshipJoinIfAny (NativeSqlBuilder& sqlBuilder, RelationshipClassMapCR classMap, ECN::ECRelationshipEnd endPoint, bool topLevel)
         {
         auto ecclassIdPropertyMap = static_cast<PropertyMapRelationshipConstraintClassId const*>(endPoint == ECRelationshipEnd::ECRelationshipEnd_Source ? classMap.GetSourceECClassIdPropMap () : classMap.GetTargetECClassIdPropMap ());
         if (!ecclassIdPropertyMap->IsMappedToPrimaryTable ())
             {
             auto ecInstanceIdPropertyMap = static_cast<PropertyMapRelationshipConstraintECInstanceId const*>(endPoint == ECRelationshipEnd::ECRelationshipEnd_Source ? classMap.GetSourceECInstanceIdPropMap () : classMap.GetTargetECInstanceIdPropMap ());
             auto classSqlName = BuildSchemaQualifiedClassName (classMap.GetClass ());

             auto const& targetTable = ecclassIdPropertyMap->GetFirstColumn ()->GetTable ();
             sqlBuilder.Append (" INNER JOIN ");
             sqlBuilder.AppendEscaped (targetTable.GetName ().c_str ());
             sqlBuilder.AppendSpace ();
             sqlBuilder.Append (GetECClassIdPrimaryTableAlias (endPoint));
             sqlBuilder.Append (" ON ");
             sqlBuilder.Append (GetECClassIdPrimaryTableAlias (endPoint));
             sqlBuilder.AppendDot ();
             auto targetECInstanceIdColumn = targetTable.GetFilteredColumnFirst (ECDbKnownColumns::ECInstanceId);
             if (targetECInstanceIdColumn == nullptr)
                 {
                 BeAssert (false && "Failed to find ECInstanceId column in target table");
                 return BentleyStatus::ERROR;
                 }
             sqlBuilder.AppendEscaped (targetECInstanceIdColumn->GetName ().c_str ());
             sqlBuilder.Append (" = ");
             if (topLevel)
                sqlBuilder.AppendEscaped (classSqlName.c_str ());
             else
                 sqlBuilder.AppendEscaped (ecInstanceIdPropertyMap->GetFirstColumn ()->GetTable().GetName().c_str());

             sqlBuilder.AppendDot ();
             sqlBuilder.Append (ecInstanceIdPropertyMap->GetFirstColumn()->GetName().c_str());
             sqlBuilder.AppendSpace ();
             }

         return BentleyStatus::SUCCESS;
         }
     //---------------------------------------------------------------------------------------
     // @bsimethod                                 Affan.Khan                         06/2015
     //---------------------------------------------------------------------------------------
     void SqlGenerator::CollectDerivedEndTableRelationships (std::set<RelationshipClassEndTableMapCP>& childMaps, RelationshipClassMapCR classMap)
         {
         if (classMap.GetMapStrategy ().IsForeignKeyMapping ())
             childMaps.insert (static_cast<RelationshipClassEndTableMapCP>(&classMap));

         for (auto derviedMap : classMap.GetDerivedClassMaps ())
             {
             if (derviedMap->IsRelationshipClassMap ())
                 CollectDerivedEndTableRelationships (childMaps, static_cast<RelationshipClassMapCR>(*derviedMap));
             }
         }
     //---------------------------------------------------------------------------------------
     // @bsimethod                                 Affan.Khan                         06/2015
     //---------------------------------------------------------------------------------------
     BentleyStatus SqlGenerator::BuildEndTableRelationshipView (NativeSqlBuilder::List& unionList, RelationshipClassMapCR classMap)
         {
         std::set<RelationshipClassEndTableMapCP> childMaps;
         CollectDerivedEndTableRelationships (childMaps, classMap);
         for (auto endClassMap : childMaps)
             {
             NativeSqlBuilder sqlBuilder;
             bool topLevel = &classMap == endClassMap;

             sqlBuilder.Append ("SELECT ");
             auto classSqlName = BuildSchemaQualifiedClassName (endClassMap->GetClass());
             Utf8String tabelPrefix;
             if (topLevel)
                 {
                 tabelPrefix = classSqlName;
                 }
             else
                 {
                 tabelPrefix = endClassMap->GetTable ().GetName ();
                 }

             if (BuildSelectionClause (sqlBuilder, classMap, *endClassMap, tabelPrefix.c_str (), /*addECPropertyPathAlias = */ topLevel, false)
                 != BentleyStatus::SUCCESS)
                 {
                 return BentleyStatus::ERROR;
                 }
             sqlBuilder.AppendEOL ();
             sqlBuilder.Append (" FROM ")
                 .AppendEscaped (endClassMap->GetTable ().GetName ().c_str ());

             Utf8String tableAlias = endClassMap->GetTable ().GetName();
             if (topLevel)
                 {
                 if (!classSqlName.EqualsI (endClassMap->GetTable ().GetName ()))
                     {
                     sqlBuilder.AppendSpace ();
                     sqlBuilder.AppendEscaped (classSqlName.c_str ());
                     tableAlias = classSqlName;
                     }
                 }

             if (BuildRelationshipJoinIfAny (sqlBuilder, *endClassMap, ECRelationshipEnd::ECRelationshipEnd_Source, topLevel) != BentleyStatus::SUCCESS)
                 return BentleyStatus::ERROR;

             if (BuildRelationshipJoinIfAny (sqlBuilder, *endClassMap, ECRelationshipEnd::ECRelationshipEnd_Target, topLevel) != BentleyStatus::SUCCESS)
                 return BentleyStatus::ERROR;

             sqlBuilder.Append (" WHERE (");
             sqlBuilder.AppendEscaped (tableAlias.c_str ());
             sqlBuilder.AppendDot ();
             sqlBuilder.Append (endClassMap->GetOtherEndECInstanceIdPropMap ()->GetFirstColumn ()->GetName ().c_str ());
             sqlBuilder.Append (" IS NOT NULL)");

             if (topLevel)
                 {
                 unionList.insert (unionList.begin (), std::move (sqlBuilder));
                 }
             else
                 {
                 unionList.push_back (std::move (sqlBuilder));
                 }
             }

         return BentleyStatus::SUCCESS;
         }
     //---------------------------------------------------------------------------------------
     // @bsimethod                                 Affan.Khan                         06/2015
     //---------------------------------------------------------------------------------------
    BentleyStatus SqlGenerator::BuildClassView (SqlClassPersistenceMethod& scpm)
        {
        auto & viewBuilder = scpm.GetViewBuilder ();
        auto& classMap = scpm.GetClassMap ();
        viewBuilder.GetNameBuilder ().Append (SqlGenerator::BuildViewClassName (classMap.GetClass ()).c_str ());
        NativeSqlBuilder::List unionList;
        
        if (classMap.IsRelationshipClassMap() && classMap.GetMapStrategy().IsForeignKeyMapping())
            {
            if (BuildEndTableRelationshipView (unionList, static_cast<RelationshipClassMapCR> (classMap)) != BentleyStatus::SUCCESS)
                return BentleyStatus::ERROR;
            }
        else
            {
            auto& disp = classMap.GetStorageDescription ();
            bool nextClassMapIsTopLevel = false;
            for (auto& part : disp.GetHorizontalPartitions ())
                {
                bool topLevel = (&disp.GetRootHorizontalPartition () == &part);
                if (part.GetTable ().GetPersistenceType () == PersistenceType::Virtual)
                    {
                    if (topLevel)
                        nextClassMapIsTopLevel = true;

                    continue;
                    }

                if (nextClassMapIsTopLevel)
                    {
                    topLevel = true;
                    nextClassMapIsTopLevel = false;
                    }
                NativeSqlBuilder sqlBuilder;
                sqlBuilder.Append ("SELECT ").AppendEOL();
                auto derivedClassId = part.GetClassIds ().front ();
                auto derivedClass = m_map.GetECDbR ().Schemas ().GetECClass (derivedClassId);

                if (derivedClass == nullptr)
                    {
                    BeAssert (derivedClass != nullptr);
                    return BentleyStatus::ERROR;
                    }

                auto derivedClassMap = m_map.GetClassMap (*derivedClass);
                if (derivedClassMap == nullptr)
                    {
                    BeAssert (derivedClassMap != nullptr);
                    return BentleyStatus::ERROR;
                    }

                if (derivedClassMap->GetMapStrategy().IsForeignKeyMapping())
                    continue;

                auto classSqlName = BuildSchemaQualifiedClassName (*derivedClass);
                Utf8String tabelPrefix;
                if (topLevel)
                    {
                    tabelPrefix = classSqlName;
                    }
                else
                    {
                    tabelPrefix = part.GetTable ().GetName ();
                    }

                if (BuildSelectionClause (sqlBuilder, classMap, static_cast<ClassMapCR>(*derivedClassMap), tabelPrefix.c_str (), /*addECPropertyPathAlias = */ topLevel, false)
                    != BentleyStatus::SUCCESS)
                    {
                    return BentleyStatus::ERROR;
                    }

                sqlBuilder.Append (" FROM ")
                    .AppendEscaped (part.GetTable ().GetName ().c_str ());

                if (topLevel)
                    {
                    if (!classSqlName.EqualsI (part.GetTable ().GetName ()))
                        {
                        sqlBuilder.AppendSpace ();
                        sqlBuilder.AppendEscaped (classSqlName.c_str ());
                        }
                    }

                if (derivedClassMap->IsRelationshipClassMap ())
                    {
                    auto const relationshipMap = static_cast<RelationshipClassMapCP>(derivedClassMap);
                    if (BuildRelationshipJoinIfAny (sqlBuilder, *relationshipMap, ECRelationshipEnd::ECRelationshipEnd_Source, topLevel) != BentleyStatus::SUCCESS)
                        return BentleyStatus::ERROR;

                    if (BuildRelationshipJoinIfAny (sqlBuilder, *relationshipMap, ECRelationshipEnd::ECRelationshipEnd_Target, topLevel) != BentleyStatus::SUCCESS)
                        return BentleyStatus::ERROR;
                    }

                if (auto classIdcolumn = part.GetTable ().GetFilteredColumnFirst (ECDbKnownColumns::ECClassId))
                    {
                    if (part.NeedsClassIdFilter ())
                        {
                        sqlBuilder.Append (" WHERE ");
                        sqlBuilder.AppendParenLeft ();
                        if (topLevel)
                            sqlBuilder.AppendEscaped (classSqlName.c_str ()).AppendDot ();

                        else
                            sqlBuilder.AppendEscaped (part.GetTable ().GetName ().c_str ()).AppendDot ();

                        sqlBuilder.AppendEscaped (classIdcolumn->GetName ().c_str ());
                        sqlBuilder.AppendSpace ();
                        part.AppendECClassIdFilterSql (sqlBuilder);
                        sqlBuilder.AppendParenRight ();
                        }       
                    }

                if (topLevel)
                    {
                    unionList.insert (unionList.begin (), std::move (sqlBuilder));
                    }
                else
                    {
                    unionList.push_back (std::move (sqlBuilder));
                    }
                }
            }

        if (!unionList.empty ())
            {
            for (auto const& unionEntry : unionList)
                {
                viewBuilder.AddSelect ().Append (unionEntry);
                }

            return BentleyStatus::SUCCESS;
            }

        //Create null view
       // printf ("NULL VIEW for %s\n", Utf8String (classMap.GetClass ().GetName ().c_str ()).c_str ());
        NativeSqlBuilder nullView;
        nullView.Append ("SELECT ");
        if (BuildSelectionClause (nullView, classMap, classMap, nullptr, true, true) != BentleyStatus::SUCCESS)
            {
            return BentleyStatus::ERROR;
            }

        nullView.Append (" LIMIT 0;").AppendEOL ();
        viewBuilder.AddSelect ().Append (nullView);
        viewBuilder.MarkAsNullView ();
        return BentleyStatus::SUCCESS;
        }

    //-----------------------------------------------------------------------------------------
    // @bsimethod                                    Affan.Khan                      07/2015
    //+---------------+---------------+---------------+---------------+---------------+--------
    SqlClassPersistenceMethod* SqlGenerator::GetClassPersistenceMethod (ClassMapCR classMap)
        {
        auto itor = m_scpms.find (classMap.GetClass ().GetId ());
        if (itor == m_scpms.end ())
            {
            auto scpm = new SqlClassPersistenceMethod (classMap);
            m_scpms[classMap.GetClass ().GetId ()] = std::unique_ptr<SqlClassPersistenceMethod> (scpm);

            if (BuildClassView (*scpm) != BentleyStatus::SUCCESS)
                {
                BeAssert (false && "Failed to build class view");
                return nullptr;
                }
            return scpm;
            }

        return itor->second.get ();
        }
            

    //-----------------------------------------------------------------------------------------
    // @bsimethod                                    Affan.Khan                      06/2015
    //+---------------+---------------+---------------+---------------+---------------+--------
    BentleyStatus SqlGenerator::BuildViewInfrastructure (std::set<ClassMap const*>& classMaps)
        {
        for (auto classMap : classMaps)
            {
            GetClassPersistenceMethod (*classMap);
            }
        
        
        NativeSqlBuilder holdingView;
        DropViewIfExists (m_map.GetECDbR (), ECDB_HOLDING_VIEW);
        if (BuildHoldingView (holdingView) != BentleyStatus::SUCCESS)
            return BentleyStatus::ERROR;

        if (m_map.GetECDbR ().ExecuteSql (holdingView.ToString ()) != BE_SQLITE_OK)
            return BentleyStatus::ERROR;

       auto curSize = m_scpms.size ();
       do
            {
            curSize = m_scpms.size ();
            for (auto& kp : m_scpms)
                {
                if (!kp.second->IsFinished ())
                    {
                    kp.second->ResetTriggers ();
                    if (BuildDeleteTriggers (*kp.second) != BentleyStatus::SUCCESS)
                        return BentleyStatus::ERROR;

                    kp.second->MarkAsFinish ();
                    }
                }
           } while (curSize != m_scpms.size ());



//        auto file = fopen ("d:\\Temp\\Out.sql", "w+");
        for (auto& scpm : m_scpms)
            {
            Utf8String dropSql = scpm.second->ToString( SqlOption::DropIfExists);
            //fwrite (dropSql.c_str(), dropSql.size (),1, file);
            //fflush (file);
            if (m_map.GetECDbR ().ExecuteSql (dropSql.c_str ()) != BE_SQLITE_OK)
                {
                return BentleyStatus::ERROR;
                }

            Utf8String createSql;
            createSql.append ("--> ").append (scpm.second->GetClassMap ().GetClass().GetFullName()).append("\n");
            createSql.append ("--> ").append (scpm.second->GetClassMap ().GetDMLPolicy ().ToString ()).append ("\n");;
            createSql.append(scpm.second->ToString (SqlOption::Create));

            //fwrite (createSql.c_str(), createSql.size (),1, file);
            //            fflush (file);

            if (m_map.GetECDbR ().ExecuteSql (createSql.c_str ()) != BE_SQLITE_OK)
                { 
                return BentleyStatus::ERROR;
                //printf ("%s\n", m_map.GetECDbR ().GetLastError ());
                }
            }
        //        fclose (file);

        return BentleyStatus::SUCCESS;
        }
    //---------------------------------------------------------------------------------------
    // @bsimethod                                 Affan.Khan                         06/2015
    //----------------------------------------------------------------------------------------
    BentleyStatus SqlGenerator::BuildDeleteTriggerForStructArrays (SqlClassPersistenceMethod& scpm)
        {
        //printf ("BuildDeleteTriggerForStructArrays\n");
        auto primaryTarget = scpm.GetClassMap ().GetDMLPolicy ().Get (DMLPolicy::Operation::Delete);
        if (primaryTarget == DMLPolicy::Target::None)
            return BentleyStatus::SUCCESS;

        auto& primaryClassMap = scpm.GetClassMap ();
        std::map<IClassMap const*, std::vector<PropertyMapToTableCP>> structPropertyMaps;
        primaryClassMap.GetPropertyMaps ().Traverse ([&] (TraversalFeedback& feedback, PropertyMapCP propertyMap)
            {
            if (auto mapToTable = dynamic_cast<PropertyMapToTableCP>(propertyMap))
                {
                if (auto associatedClasMap = m_map.GetClassMap (mapToTable->GetElementType ()))
                    {
                    if (associatedClasMap->GetTable ().GetPersistenceType () == PersistenceType::Persisted)
                        {
                        structPropertyMaps[associatedClasMap].push_back (mapToTable);
                        }
                    }
                }

            feedback = TraversalFeedback::Next;
            }, true);

        if (!structPropertyMaps.empty ())
            {
            auto primaryTriggerCondition = primaryTarget == DMLPolicy::Target::View ? SqlTriggerBuilder::Condition::InsteadOf : SqlTriggerBuilder::Condition::After;
            auto primaryTargetId = scpm.GetAffectedTargetId (DMLPolicy::Operation::Delete);
            auto& deleteTrigger = scpm.AddTrigger (SqlTriggerBuilder::Type::Delete, primaryTriggerCondition, false);
            auto ecinstanceIdMap = primaryClassMap.GetECInstanceIdPropertyMap ();
            BeAssert (ecinstanceIdMap != nullptr);
            BeAssert (ecinstanceIdMap->GetFirstColumn () != nullptr);
            deleteTrigger.GetNameBuilder ().Append (BuildSchemaQualifiedClassName(primaryClassMap.GetClass()).c_str()).Append ("_Delete_StructArrays");
            deleteTrigger.GetOnBuilder ().Append (primaryTargetId);
            auto& deleteTriggerBody = deleteTrigger.GetBodyBuilder ();
            auto primaryECInstanceId = primaryTarget == DMLPolicy::Target::Table ? ecinstanceIdMap->GetFirstColumn ()->GetName ().c_str () : "ECInstanceId";
            deleteTriggerBody.Append ("\t-- Struct Properties").AppendEOL ();

#ifdef ENABLE_TRIGGER_DEBUGGING
            deleteTriggerBody.Append ("--#ifdef ENABLE_TRIGGER_DEBUGGING").AppendEOL ();
            Utf8String primaryECClassId;
            if (primaryTarget == DMLPolicy::Target::Table)
                {
                auto classId = primaryClassMap.GetTable ().GetFilteredColumnFirst (ECDbKnownColumns::ECClassId);
                if (classId == nullptr)
                    primaryECClassId.Sprintf ("%lld", primaryClassMap.GetClass ().GetId ());
                else
                    primaryECClassId = "OLD." + classId->GetName ();
                }
            else
                {
                primaryECClassId = "OLD.ECClassId";
                }

            deleteTriggerBody.AppendFormatted ("\tINSERT INTO ec_TriggerLog (TriggerId, AffectedECInstanceId, AffectedECClassId, Scope) VALUES ('%s', OLD.%s, %s, 'BEGIN'); ", deleteTrigger.GetName (), primaryECInstanceId, primaryECClassId.c_str()).AppendEOL ();
            deleteTriggerBody.Append ("--#endif").AppendEOL ();
#endif
            for (auto pair : structPropertyMaps)
                {
                for (auto propertyMap : pair.second)
                    {
                    deleteTriggerBody.Append ("\t-- > ").Append (propertyMap->GetPropertyAccessString ()).AppendEOL ();
                    }

                auto refClassMap = m_map.GetClassMapCP (pair.first->GetClass ());
                auto refSCPM = GetClassPersistenceMethod (*refClassMap);
                auto refDeleteTarget = refClassMap->GetDMLPolicy ().Get (DMLPolicy::Operation::Delete);
                auto refTargetId = refSCPM->GetAffectedTargetId (DMLPolicy::Operation::Delete);
                auto parentECInstanceIdColumn = refClassMap->GetTable ().GetFilteredColumnFirst (ECDbKnownColumns::ParentECInstanceId);
                BeAssert (parentECInstanceIdColumn != nullptr);
                auto refParentECInstanceId = refDeleteTarget == DMLPolicy::Target::Table ? parentECInstanceIdColumn->GetName ().c_str () : "ParentECInstanceId";
                deleteTriggerBody
                    .Append ("\tDELETE FROM ")
                    .AppendEscaped (refTargetId)
                    .AppendFormatted (" WHERE %s = OLD.%s;", refParentECInstanceId, primaryECInstanceId)
                    .AppendEOL ();
                }

#ifdef ENABLE_TRIGGER_DEBUGGING
            deleteTriggerBody.Append ("--#ifdef ENABLE_TRIGGER_DEBUGGING").AppendEOL ();
            deleteTriggerBody.AppendFormatted ("\tINSERT INTO ec_TriggerLog (TriggerId, AffectedECInstanceId, AffectedECClassId, Scope) VALUES ('%s', OLD.%s, %s, 'BEGIN'); ", deleteTrigger.GetName (), primaryECInstanceId, primaryECClassId.c_str()).AppendEOL ();
            deleteTriggerBody.Append ("--#endif").AppendEOL ();
#endif
            }

        return BentleyStatus::SUCCESS;
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                 Affan.Khan                         06/2015
    //---------------------------------------------------------------------------------------
    BentleyStatus SqlGenerator::BuildDerivedFilterClause (Utf8StringR filter, ECDb& db, ECClassId baseClassId)
        {
        Utf8CP sql =
            "WITH RECURSIVE "
            "    DerivedClassList (CurrentClassId, DerivedClassId) "
            "    AS ( "
            "       SELECT ?1, ?1 FROM ec_Class "
            "    UNION "
            "       SELECT BC.BaseClassId, BC.ClassId "
            "           FROM DerivedClassList DCL "
            "       INNER JOIN ec_BaseClass BC ON BC.BaseClassId = DCL.DerivedClassId "
            "    ) "
            "SELECT 'IN (' || group_concat(DISTINCT DerivedClassId) || ')' FROM DerivedClassList ORDER BY DerivedClassId";

        auto stmt = db.GetCachedStatement (sql);
        if (stmt.IsValid())
            {
            stmt->BindInt64 (1, baseClassId);
            if (stmt->Step () == BE_SQLITE_ROW)
                {
                if (!stmt->IsColumnNull (0))
                    filter = stmt->GetValueText (0);

                return BentleyStatus::SUCCESS;
                }
            }
        return BentleyStatus::ERROR;
        }
    //---------------------------------------------------------------------------------------
    // @bsimethod                                 Affan.Khan                         06/2015
    //---------------------------------------------------------------------------------------
    BentleyStatus SqlGenerator::FindRelationshipReferences (bmap<RelationshipClassMapCP, ECDbMap::LightweightCache::RelationshipEnd>& relationships, ClassMapCR classMap)
        {
        for (auto& pair : m_map.GetLightweightCache ().GetRelationshipsForConstraintClass (classMap.GetClass ().GetId ()))
            {
            auto classMap = m_map.GetRelationshipClassMap (pair.first);
            if (classMap == nullptr)
                continue;

            if (classMap->GetMapStrategy ().IsNotMapped())
                continue;

            relationships[classMap] = pair.second;
            }

        return BentleyStatus::SUCCESS;
        }
    //-----------------------------------------------------------------------------------------
    // @bsimethod                                    Affan.Khan                      06/2015
    //+---------------+---------------+---------------+---------------+---------------+--------
    BentleyStatus SqlGenerator::BuildDeleteTriggersForDerivedClasses (SqlClassPersistenceMethod& scpm)
        {
       // printf ("BuildDeleteTriggersForDerivedClasses\n");

        auto& primaryClassMap = scpm.GetClassMap ();
        auto primaryTarget = primaryClassMap.GetDMLPolicy ().Get (DMLPolicy::Operation::Delete);
        if (primaryTarget == DMLPolicy::Target::None)
            return BentleyStatus::SUCCESS;

        auto primaryTargetId = scpm.GetAffectedTargetId (DMLPolicy::Operation::Delete);
        auto primaryTriggerCondition = primaryTarget == DMLPolicy::Target::View ? SqlTriggerBuilder::Condition::InsteadOf : SqlTriggerBuilder::Condition::After;
        auto primaryECInstanceId = primaryTarget == DMLPolicy::Target::Table ? primaryClassMap.GetECInstanceIdPropertyMap ()->GetFirstColumn ()->GetName ().c_str () : "ECInstanceId";
        for (auto derivedClassMap : primaryClassMap.GetDerivedClassMaps ())
            {
            if (derivedClassMap == nullptr || derivedClassMap->GetMapStrategy ().IsNotMapped ())
                continue;

            auto& refClassMap = static_cast<ClassMapCR>(*derivedClassMap);
            auto refSCPM = GetClassPersistenceMethod (refClassMap);
            auto refTargetId = refSCPM->GetAffectedTargetId (DMLPolicy::Operation::Delete);
            auto refTarget = refClassMap.GetDMLPolicy ().Get (DMLPolicy::Operation::Delete);
            auto& deleteTrigger = scpm.AddTrigger (SqlTriggerBuilder::Type::Delete, primaryTriggerCondition, false);
            auto refECInstanceId = refTarget == DMLPolicy::Target::Table ? refClassMap.GetECInstanceIdPropertyMap ()->GetFirstColumn ()->GetName ().c_str () : "ECInstanceId";

            deleteTrigger.GetNameBuilder ().Append (BuildSchemaQualifiedClassName (primaryClassMap.GetClass ()).c_str ()).Append ("_Delete_").Append (BuildSchemaQualifiedClassName (refClassMap.GetClass ()).c_str ());
            deleteTrigger.GetOnBuilder ().Append (primaryTargetId);

            Utf8String filterClause;
            if (BuildDerivedFilterClause (filterClause, m_map.GetECDbR (), derivedClassMap->GetClass ().GetId ()) != BentleyStatus::SUCCESS)
                return BentleyStatus::ERROR;

            if (!filterClause.empty ())
                {
                if (primaryTarget == DMLPolicy::Target::Table)
                    {
                    auto classIdColumn = primaryClassMap.GetTable ().GetFilteredColumnFirst (ECDbKnownColumns::ECClassId);
                    if (classIdColumn != nullptr)
                        {
                        deleteTrigger.GetWhenBuilder ().AppendFormatted ("OLD.%s ", classIdColumn->GetName ().c_str ()).Append (filterClause.c_str ());
                        }
                    }
                else
                    deleteTrigger.GetWhenBuilder ().Append ("OLD.ECClassId ").Append (filterClause.c_str ());
                }

            auto& deleteTriggerBody = deleteTrigger.GetBodyBuilder ();

#ifdef ENABLE_TRIGGER_DEBUGGING
            deleteTriggerBody.Append ("--#ifdef ENABLE_TRIGGER_DEBUGGING").AppendEOL ();
            Utf8String primaryECClassId;
            if (primaryTarget == DMLPolicy::Target::Table)
                {
                auto classId = primaryClassMap.GetTable ().GetFilteredColumnFirst (ECDbKnownColumns::ECClassId);
                if (classId == nullptr)
                    primaryECClassId.Sprintf ("%lld", primaryClassMap.GetClass ().GetId ());
                else
                    primaryECClassId = "OLD." + classId->GetName ();
                }
            else
                {
                primaryECClassId = "OLD.ECClassId";
                }

            deleteTriggerBody.AppendFormatted ("\tINSERT INTO ec_TriggerLog (TriggerId, AffectedECInstanceId, AffectedECClassId, Scope) VALUES ('%s', OLD.%s, %s, 'BEGIN'); ", deleteTrigger.GetName (), primaryECInstanceId, primaryECClassId.c_str ()).AppendEOL ();
            deleteTriggerBody.Append ("--#endif").AppendEOL ();
#endif
            deleteTriggerBody
                .Append ("\tDELETE FROM ")
                .AppendEscaped (refTargetId)
                .AppendFormatted (" WHERE %s = OLD.%s;", refECInstanceId, primaryECInstanceId)
                .AppendEOL ();

#ifdef ENABLE_TRIGGER_DEBUGGING
            deleteTriggerBody.Append ("--#ifdef ENABLE_TRIGGER_DEBUGGING").AppendEOL ();
            deleteTriggerBody.AppendFormatted ("\tINSERT INTO ec_TriggerLog (TriggerId, AffectedECInstanceId, AffectedECClassId, Scope) VALUES ('%s', OLD.%s, %s, 'BEGIN'); ", deleteTrigger.GetName (), primaryECInstanceId, primaryECClassId.c_str ()).AppendEOL ();
            deleteTriggerBody.Append ("--#endif").AppendEOL ();
#endif
            }

        return BentleyStatus::SUCCESS;
        }

    //-----------------------------------------------------------------------------------------
    // @bsimethod                                    Affan.Khan                      06/2015
    //+---------------+---------------+---------------+---------------+---------------+--------
    const std::vector<ClassMapCP> SqlGenerator::GetEndClassMaps (ECRelationshipClassCR relationship, ECRelationshipEnd end)
        {
        std::vector<ClassMapCP> classes;
        auto& constraints = end == ECRelationshipEnd::ECRelationshipEnd_Source ? relationship.GetSource () : relationship.GetTarget ();
        auto anyClassId = m_map.GetLightweightCache ().GetAnyClassId ();
        ClassMapCP classMap = nullptr;
        for (auto constraint : constraints.GetConstraintClasses ())
            {
            if (constraint->GetClass().GetId() == anyClassId)
                {
                classes.clear();
                for (auto classId : m_map.GetLightweightCache().GetAnyClassReplacements())
                    {
                    classMap = m_map.GetClassMapCP (classId);
                    if (classMap != nullptr)
                        classes.push_back(classMap);
                    }

                return classes;
                }

            classMap = m_map.GetClassMapCP (constraint->GetClass());
            if (classMap != nullptr)
                classes.push_back (classMap);
            }

        return classes;
        }
    //-----------------------------------------------------------------------------------------
    // @bsimethod                                    Affan.Khan                      06/2015
    //+---------------+---------------+---------------+---------------+---------------+--------
    BentleyStatus SqlGenerator::BuildEmbeddingConstraint (NativeSqlBuilder& trigger, RelationshipClassMapCR classMap)
        {
        auto strengthType = classMap.GetRelationshipClass ().GetStrength ();
        auto strengthDirection = classMap.GetRelationshipClass ().GetStrengthDirection ();
        auto primarySCPM = GetClassPersistenceMethod (classMap);
        BeAssert (primarySCPM != nullptr);
        auto primaryTarget = primarySCPM->GetClassMap ().GetDMLPolicy ().Get (DMLPolicy::Operation::Delete);

        if (StrengthType::STRENGTHTYPE_Embedding == strengthType)
            {
            trigger.Append (" -- StrengthType == Embedding  ").AppendEOL ();
            ECRelationshipEnd endToDelete =
                strengthDirection == ECRelatedInstanceDirection::Forward ? ECRelationshipEnd::ECRelationshipEnd_Target : ECRelationshipEnd::ECRelationshipEnd_Source;
            
            auto primaryKey = ECRelationshipEnd::ECRelationshipEnd_Source == endToDelete ? classMap.GetSourceECInstanceIdPropMap () : classMap.GetTargetECInstanceIdPropMap ();
            auto primaryKeyPath = ECRelationshipEnd::ECRelationshipEnd_Source == endToDelete ? "SourceECInstanceId" : "TargetECInstanceId";
            auto primaryKeyId = primaryTarget == DMLPolicy::Target::Table ? primaryKey->GetFirstColumn ()->GetName ().c_str () : primaryKeyPath;
            for (auto endToBeDeleted : GetEndClassMaps (classMap.GetRelationshipClass (), endToDelete))
                {
                auto refSCPM = GetClassPersistenceMethod (*endToBeDeleted);
                auto refTarget = refSCPM->GetClassMap ().GetDMLPolicy ().Get (DMLPolicy::Operation::Delete);
                auto refTargetId = refSCPM->GetAffectedTargetId (DMLPolicy::Operation::Delete);
                auto refECInstanceIdKey = refTarget == DMLPolicy::Target::Table ? refSCPM->GetClassMap ().GetECInstanceIdPropertyMap ()->GetFirstColumn ()->GetName ().c_str () : "ECInstanceId";
                trigger.Append ("\tDELETE FROM ");
                trigger.AppendEscaped (refTargetId);
                trigger.AppendFormatted (" WHERE  %s = OLD.%s;", refECInstanceIdKey, primaryKeyId);
                trigger.AppendEOL ();
                }
            }
        return BentleyStatus::SUCCESS;
        }

    //-----------------------------------------------------------------------------------------
    // @bsimethod                                    Affan.Khan                      06/2015
    //+---------------+---------------+---------------+---------------+---------------+--------
    BentleyStatus SqlGenerator::BuildHoldingConstraint (NativeSqlBuilder& trigger, RelationshipClassMapCR classMap)
        {
        auto strengthType = classMap.GetRelationshipClass ().GetStrength ();
        auto strengthDirection = classMap.GetRelationshipClass ().GetStrengthDirection ();
        auto primarySCPM = GetClassPersistenceMethod (classMap);
        BeAssert (primarySCPM != nullptr);
        auto primaryTarget = primarySCPM->GetClassMap ().GetDMLPolicy ().Get (DMLPolicy::Operation::Delete);

        if (StrengthType::STRENGTHTYPE_Holding == strengthType)
            {
            trigger.Append (" -- StrengthType == Holding  ").AppendEOL ();
            ECRelationshipEnd endToDelete =
                strengthDirection == ECRelatedInstanceDirection::Forward ? ECRelationshipEnd::ECRelationshipEnd_Target : ECRelationshipEnd::ECRelationshipEnd_Source;

            auto primaryKey = ECRelationshipEnd::ECRelationshipEnd_Source == endToDelete ? classMap.GetSourceECInstanceIdPropMap () : classMap.GetTargetECInstanceIdPropMap ();
            auto primaryKeyPath = ECRelationshipEnd::ECRelationshipEnd_Source == endToDelete ? "SourceECInstanceId" : "TargetECInstanceId";
            auto primaryKeyId = primaryTarget == DMLPolicy::Target::Table ? primaryKey->GetFirstColumn ()->GetName ().c_str () : primaryKeyPath;
            for (auto endToBeDeleted : GetEndClassMaps (classMap.GetRelationshipClass (), endToDelete))
                {
                auto refSCPM = GetClassPersistenceMethod (*endToBeDeleted);
                auto refTarget = refSCPM->GetClassMap ().GetDMLPolicy ().Get (DMLPolicy::Operation::Delete);
                auto refTargetId = refSCPM->GetAffectedTargetId (DMLPolicy::Operation::Delete);
                auto refECInstanceIdKey = refTarget == DMLPolicy::Target::Table ? refSCPM->GetClassMap ().GetECInstanceIdPropertyMap ()->GetFirstColumn ()->GetName ().c_str () : "ECInstanceId";
                trigger.Append ("\tDELETE FROM ");
                trigger.AppendEscaped (refTargetId);
                trigger.AppendFormatted (" WHERE  %s = OLD.%s AND (SELECT COUNT(*) FROM " ECDB_HOLDING_VIEW "  WHERE ECInstanceId = OLD.%s) = 0;", refECInstanceIdKey, primaryKeyId, primaryKeyId);
                trigger.AppendEOL ();
                }
            }

        return BentleyStatus::SUCCESS;
        }

    //-----------------------------------------------------------------------------------------
    // @bsimethod                                    Affan.Khan                      06/2015
    //+---------------+---------------+---------------+---------------+---------------+--------
    BentleyStatus SqlGenerator::BuildDeleteTriggerForEndTableMe (SqlClassPersistenceMethod& scpm)
        {

        auto& classMap = scpm.GetClassMap ();
        if (classMap.GetDMLPolicy ().Get (DMLPolicy::Operation::Delete) != DMLPolicy::Target::View)
            {
            BeAssert (false && "ECDb donot support table policy for delete in case of endtable");
            return BentleyStatus::ERROR;
            }
        auto name = BuildSchemaQualifiedClassName (classMap.GetClass ());
        auto& endTableMap = static_cast<RelationshipClassEndTableMapCR>(classMap);
        auto strengthType = endTableMap.GetRelationshipClass ().GetStrength ();
        NativeSqlBuilder updateStatement;
        if (!endTableMap.GetOtherEndECInstanceIdPropMap ()->GetFirstColumn ()->GetConstraint ().IsNotNull ())
            {
            if (StrengthType::STRENGTHTYPE_Referencing == strengthType)
                {
                updateStatement.Append (" -- StrengthType == Referencing ").AppendEOL ();
                }
            updateStatement.Append ("\tUPDATE ");
            updateStatement.AppendEscaped (endTableMap.GetTable ().GetName ().c_str ());
            updateStatement.Append (" SET ");
            updateStatement.AppendEscaped (endTableMap.GetOtherEndECInstanceIdPropMap ()->GetFirstColumn ()->GetName ().c_str ());
            updateStatement.Append (" = NULL");
            if (endTableMap.GetOtherEndECClassIdPropMap ()->IsMappedToPrimaryTable () && endTableMap.GetOtherEndECClassIdPropMap ()->GetFirstColumn ()->GetPersistenceType () == PersistenceType::Persisted)
                {
                updateStatement.AppendComma ().AppendSpace ();
                updateStatement.AppendEscaped (endTableMap.GetOtherEndECClassIdPropMap ()->GetFirstColumn ()->GetName ().c_str ());
                updateStatement.Append (" = NULL");
                }

            updateStatement.Append (" WHERE ");
            if (auto column = endTableMap.GetTable ().GetFilteredColumnFirst (ECDbKnownColumns::ECInstanceId))
                {
                updateStatement.AppendEscaped (column->GetName ().c_str ());
                }
            else
                {
                BeAssert (false && "Failed to find ECInstanceId column");
                return BentleyStatus::ERROR;
                }

            updateStatement.Append (" = ").Append ("OLD.ECInstanceId;").AppendEOL ();
            }

        NativeSqlBuilder deleteTarget;
        if (strengthType == StrengthType::STRENGTHTYPE_Embedding)
            {
            if (BuildEmbeddingConstraint (deleteTarget, endTableMap) != BentleyStatus::SUCCESS)
                return BentleyStatus::ERROR;
            }
        else if (strengthType == StrengthType::STRENGTHTYPE_Holding)
            {
            if (BuildHoldingConstraint (deleteTarget, endTableMap) != BentleyStatus::SUCCESS)
                return BentleyStatus::ERROR;
            }

        auto& stb = scpm.AddTrigger (SqlTriggerBuilder::Type::Delete, SqlTriggerBuilder::Condition::InsteadOf, false);
        stb.GetNameBuilder().Append(name.c_str()).Append ("_Delete_Me");
        stb.GetOnBuilder ().Append (BuildViewClassName (endTableMap.GetClass ()).c_str ());
        auto& body = stb.GetBodyBuilder ();

#ifdef ENABLE_TRIGGER_DEBUGGING
        body.Append ("--#ifdef ENABLE_TRIGGER_DEBUGGING").AppendEOL ();
        body.Append (SqlPrintfString ("\tINSERT INTO ec_TriggerLog (TriggerId, AffectedECInstanceId, AffectedECClassId, Scope) VALUES ('%s', OLD.ECInstanceId, OLD.ECClassId, 'BEGIN'); ", stb.GetName()).GetUtf8CP ()).AppendEOL ();
        body.Append ("--#endif").AppendEOL ();
#endif
        if (updateStatement.IsEmpty () && deleteTarget.IsEmpty ())
            body.Append ("SELECT 1;").AppendEOL ();
        else
            {
            body.Append (updateStatement);
            body.Append (deleteTarget);
            }
#ifdef ENABLE_TRIGGER_DEBUGGING
        body.Append ("--#ifdef ENABLE_TRIGGER_DEBUGGING").AppendEOL ();
        body.Append (SqlPrintfString ("\tINSERT INTO ec_TriggerLog (TriggerId, AffectedECInstanceId, AffectedECClassId, Scope) VALUES ('%s', OLD.ECInstanceId, OLD.ECClassId, 'END'); ", stb.GetName()).GetUtf8CP ()).AppendEOL ();
        body.Append ("--#endif").AppendEOL ();
#endif

        return BentleyStatus::SUCCESS;
        }
    //---------------------------------------------------------------------------------------
    // @bsimethod                                 Affan.Khan                         06/2015
    //---------------------------------------------------------------------------------------
    BentleyStatus SqlGenerator::DropViewIfExists (ECDbR db, Utf8CP viewName)
        {
        return db.ExecuteSql (SqlPrintfString ("DROP VIEW IF EXISTS [%s];", viewName).GetUtf8CP ()) != BE_SQLITE_OK ? BentleyStatus::ERROR : BentleyStatus::SUCCESS;
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                 Affan.Khan                         06/2015
    //---------------------------------------------------------------------------------------
    BentleyStatus SqlGenerator::BuildHoldingView (NativeSqlBuilder& viewSql)
        {
        Utf8CP sql = "SELECT Id FROM ec_Class WHERE ec_Class.RelationStrength = 1"; // Holding relationships       
        NativeSqlBuilder::List unionList;
        auto stmt = m_map.GetECDbR().GetCachedStatement (sql);
        if (!stmt.IsValid ())
            {
            BeAssert (false && "Failed to prepared statement");
            return BentleyStatus::ERROR;
            }
        std::map<ECDbSqlTable const*, ECDbMap::LightweightCache::RelationshipEnd> doneSet;
        while (stmt->Step () == BE_SQLITE_ROW)
            {
            ECClassId ecClassId = stmt->GetValueInt64 (0);
            auto holdingRelationshipClass = m_map.GetECDbR ().Schemas ().GetECClass (ecClassId);
            if (holdingRelationshipClass == nullptr)
                {
                BeAssert (false && "Fail to find class for holding relationship");
                return BentleyStatus::ERROR;
                }

            auto holdingRelationshipClassMap = static_cast<RelationshipClassMapCP>(m_map.GetClassMap (*holdingRelationshipClass));
            if (holdingRelationshipClassMap == nullptr || holdingRelationshipClassMap->GetTable ().GetPersistenceType () == PersistenceType::Virtual)
                continue;


            Utf8CP column;
            ECDbMap::LightweightCache::RelationshipEnd filter;
            if (holdingRelationshipClassMap->GetRelationshipClass ().GetStrengthDirection () == ECRelatedInstanceDirection::Forward)
                {
                column = holdingRelationshipClassMap->GetTargetECInstanceIdPropMap ()->GetFirstColumn ()->GetName ().c_str ();
                filter = ECDbMap::LightweightCache::RelationshipEnd::Source;
                }
            else
                {
                column = holdingRelationshipClassMap->GetSourceECInstanceIdPropMap ()->GetFirstColumn ()->GetName ().c_str ();
                filter = ECDbMap::LightweightCache::RelationshipEnd::Target;
                }

            auto table = &holdingRelationshipClassMap->GetTable ();
            auto itor = doneSet.find (table);
            if (itor == doneSet.end () || (((int)(itor->second) & (int)filter) == 0))
                {
                NativeSqlBuilder relaitonshipView;
                relaitonshipView.Append ("SELECT ");
                relaitonshipView.Append (column);
                relaitonshipView.Append (" ECInstanceId FROM ").Append (table->GetName ().c_str ());
                //relaitonshipView.Append (" WHERE (").Append (column).Append(" IS NOT NULL)").AppendEOL();
                unionList.push_back (std::move (relaitonshipView));
                if (itor == doneSet.end ())
                    doneSet[table] = filter;
                else
                    doneSet[table] = static_cast<ECDbMap::LightweightCache::RelationshipEnd>((int)(itor->second) & (int)(filter));
                }
            }

        viewSql.Append ("CREATE VIEW ").Append (ECDB_HOLDING_VIEW).Append (" AS ").AppendEOL ();
        if (!unionList.empty ())
            {

            for (auto& select : unionList)
                {
                viewSql.Append (select);
                if (&select != &(unionList.back ()))
                    viewSql.Append (" \r\n UNION ALL \r\n");
                } 
            }
        else
            {
            viewSql.Append ("SELECT NULL ECInstanceId LIMIT 0;");
            }

        return BentleyStatus::SUCCESS;
        }
    //-----------------------------------------------------------------------------------------
    // @bsimethod                                    Affan.Khan                      12/2013
    //+---------------+---------------+---------------+---------------+---------------+--------
    BentleyStatus SqlGenerator::BuildDeleteTriggerForMe (SqlClassPersistenceMethod& scpm)
        {
        //printf ("BuildDeleteTriggerForMe\n");
        //We will not create delete me trigger for a table;
        if (scpm.GetClassMap ().GetDMLPolicy ().Get (DMLPolicy::Operation::Delete) != DMLPolicy::Target::View)
            return BentleyStatus::SUCCESS;

        if (scpm.GetClassMap().GetTable ().GetPersistenceType () == PersistenceType::Persisted)
            {
            if (scpm.GetClassMap ().GetMapStrategy ().IsForeignKeyMapping ())
                {
                return BuildDeleteTriggerForEndTableMe (scpm);
                }
            else
                {
                auto& stb = scpm.AddTrigger (SqlTriggerBuilder::Type::Delete, SqlTriggerBuilder::Condition::InsteadOf, false);
                stb.GetNameBuilder ().Append (BuildSchemaQualifiedClassName (scpm.GetClassMap ().GetClass ()).c_str ()).Append ("_Delete_Me");
                stb.GetOnBuilder ().Append (BuildViewClassName (scpm.GetClassMap ().GetClass ()).c_str ());
                stb.GetWhenBuilder().Append ("OLD.ECClassId = ").Append (scpm.GetClassMap ().GetClass ().GetId ());
                auto& body = stb.GetBodyBuilder ();

#ifdef ENABLE_TRIGGER_DEBUGGING
                body.Append ("--#ifdef ENABLE_TRIGGER_DEBUGGING").AppendEOL ();
                body.Append (SqlPrintfString ("\tINSERT INTO ec_TriggerLog (TriggerId, AffectedECInstanceId, AffectedECClassId, Scope) VALUES ('%s', OLD.ECInstanceId, OLD.ECClassId, NULL); ", stb.GetName()).GetUtf8CP ()).AppendEOL ();
                body.Append ("--#endif").AppendEOL ();
#endif
                body.Append ("\tDELETE FROM ");
                body.AppendEscaped (scpm.GetClassMap ().GetTable ().GetName ().c_str ());
                body.Append (" WHERE ");

                if (auto column = scpm.GetClassMap ().GetTable ().GetFilteredColumnFirst (ECDbKnownColumns::ECInstanceId))
                    {
                    body.AppendEscaped (column->GetName ().c_str ());
                    }
                else
                    {
                    BeAssert (false && "Failed to find ECInstanceId column");
                    return BentleyStatus::ERROR;
                    }

                body.Append (" = ").Append ("OLD.ECInstanceId;").AppendEOL ();


                if (scpm.GetClassMap ().IsRelationshipClassMap ())
                    {
                    auto& linkTableMap = static_cast<RelationshipClassLinkTableMapCR>(scpm.GetClassMap());
                    auto strengthType = linkTableMap.GetRelationshipClass ().GetStrength ();

                    if (strengthType == StrengthType::STRENGTHTYPE_Embedding)
                        {
                        if (BuildEmbeddingConstraint (body, linkTableMap) != BentleyStatus::SUCCESS)
                            return BentleyStatus::ERROR;
                        }
                    else if (strengthType == StrengthType::STRENGTHTYPE_Holding)
                        {
                        if (BuildHoldingConstraint (body, linkTableMap) != BentleyStatus::SUCCESS)
                            return BentleyStatus::ERROR;
                        }
                    }
                }
            }

        return BentleyStatus::SUCCESS;
        }
    
            

    //-----------------------------------------------------------------------------------------
    // @bsimethod                                    Affan.Khan                      06/2015
    //+---------------+---------------+---------------+---------------+---------------+--------
    BentleyStatus SqlGenerator::BuildDeleteTriggersForRelationships (SqlClassPersistenceMethod& scpm)
        {
        //printf ("BuildDeleteTriggersForRelationships\n");
        auto& primaryClassMap = scpm.GetClassMap ();
        if (primaryClassMap.GetClass ().GetRelationshipClassCP () != nullptr)
            return BentleyStatus::SUCCESS;

        auto primaryTarget = primaryClassMap.GetDMLPolicy ().Get (DMLPolicy::Operation::Delete);
        if (primaryTarget == DMLPolicy::Target::None)
            return BentleyStatus::SUCCESS;

        auto primaryTriggerCondition = primaryTarget == DMLPolicy::Target::View ? SqlTriggerBuilder::Condition::InsteadOf : SqlTriggerBuilder::Condition::After;
        auto primaryTargetId = scpm.GetAffectedTargetId (DMLPolicy::Operation::Delete);
        bmap<RelationshipClassMapCP, ECDbMap::LightweightCache::RelationshipEnd> relationshipRefs;        
        if (FindRelationshipReferences (relationshipRefs, primaryClassMap) != BentleyStatus::SUCCESS)
            return BentleyStatus::ERROR;
        
        if (relationshipRefs.empty ())
            return BentleyStatus::SUCCESS;
   
        Utf8CP primaryECInstanceIdKey = primaryTarget == DMLPolicy::Target::Table ? primaryClassMap.GetECInstanceIdPropertyMap ()->GetFirstColumn ()->GetName ().c_str () : "ECInstanceId";
        SqlTriggerBuilder& deleteTrigger = scpm.AddTrigger (SqlTriggerBuilder::Type::Delete, primaryTriggerCondition, /*Temprary = */false);
        deleteTrigger.GetNameBuilder ().Append (BuildSchemaQualifiedClassName (primaryClassMap.GetClass()).c_str()).Append ("_Delete_Relationships");
        if (primaryTarget == DMLPolicy::Target::Table)
            {
            auto classIdColumn = primaryClassMap.GetTable ().GetFilteredColumnFirst (ECDbKnownColumns::ECClassId);
            if (classIdColumn != nullptr)
                {
                deleteTrigger.GetWhenBuilder ().AppendFormatted ("OLD.%s = ", classIdColumn->GetName ().c_str ()).Append (primaryClassMap.GetClass ().GetId ());
                }
            }
        else
            deleteTrigger.GetWhenBuilder ().Append ("OLD.ECClassId = ").Append (primaryClassMap.GetClass ().GetId ());

        deleteTrigger.GetOnBuilder ().Append (primaryTargetId);
        auto& deleteTriggerBody = deleteTrigger.GetBodyBuilder ();

#ifdef ENABLE_TRIGGER_DEBUGGING
        deleteTriggerBody.Append ("--#ifdef ENABLE_TRIGGER_DEBUGGING").AppendEOL ();
        Utf8String primaryECClassId;
        if (primaryTarget == DMLPolicy::Target::Table)
            {
            auto classId = primaryClassMap.GetTable ().GetFilteredColumnFirst (ECDbKnownColumns::ECClassId);
            if (classId == nullptr)
                primaryECClassId.Sprintf ("%lld", primaryClassMap.GetClass ().GetId ());
            else
                primaryECClassId = "OLD." + classId->GetName ();
            }
        else
            {
            primaryECClassId = "OLD.ECClassId";
            }

        deleteTriggerBody.AppendFormatted ("\tINSERT INTO ec_TriggerLog (TriggerId, AffectedECInstanceId, AffectedECClassId, Scope) VALUES ('%s', OLD.%s, %s, 'BEGIN'); ", deleteTrigger.GetName (), primaryECInstanceIdKey, primaryECClassId.c_str ()).AppendEOL ();
        deleteTriggerBody.Append ("--#endif").AppendEOL ();
#endif
        
        for (auto& ref : relationshipRefs)
            {           
            auto refSPM = GetClassPersistenceMethod (*(ref.first));
            auto refTarget = refSPM->GetClassMap ().GetDMLPolicy ().Get (DMLPolicy::Operation::Delete);
            if (refTarget == DMLPolicy::Target::None)
                continue;

            auto refTargetId = refSPM->GetAffectedTargetId (DMLPolicy::Operation::Delete);
            deleteTriggerBody.AppendFormatted ("-- %s", Utf8String(refSPM->GetClassMap ().GetClass().GetName().c_str()).c_str()).AppendEOL();
            deleteTriggerBody.Append ("\tDELETE FROM ");
            deleteTriggerBody.AppendEscaped (refTargetId);
            Utf8CP refSourceKey = refTarget == DMLPolicy::Target::Table ? ref.first->GetSourceECInstanceIdPropMap ()->GetFirstColumn ()->GetName ().c_str() : "SourceECInstanceId";
            Utf8CP refTargetKey =  refTarget == DMLPolicy::Target::Table ? ref.first->GetTargetECInstanceIdPropMap ()->GetFirstColumn ()->GetName ().c_str () : "TargetECInstanceId";
            if (ref.second == ECDbMap::LightweightCache::RelationshipEnd::Source)
                {               
                deleteTriggerBody.AppendFormatted (" WHERE (%s = OLD.%s);", refSourceKey, primaryECInstanceIdKey).AppendEOL ();
                }
            else if (ref.second == ECDbMap::LightweightCache::RelationshipEnd::Target)
                deleteTriggerBody.AppendFormatted (" WHERE (%s = OLD.%s);", refTargetKey, primaryECInstanceIdKey).AppendEOL ();
            else
                deleteTriggerBody.AppendFormatted (" WHERE (%s = OLD.%s OR %s = OLD.%s);", refSourceKey, primaryECInstanceIdKey, refTargetKey, primaryECInstanceIdKey).AppendEOL ();
            }

#ifdef ENABLE_TRIGGER_DEBUGGING
        deleteTriggerBody.Append ("--#ifdef ENABLE_TRIGGER_DEBUGGING").AppendEOL ();
        deleteTriggerBody.Append (SqlPrintfString ("\tINSERT INTO ec_TriggerLog (TriggerId, AffectedECInstanceId, AffectedECClassId, Scope) VALUES ('%s', OLD.ECInstanceId, OLD.ECClassId, 'END'); ", deleteTrigger.GetName ()).GetUtf8CP ()).AppendEOL ();
        deleteTriggerBody.Append ("--#endif").AppendEOL ();
#endif
      
        return BentleyStatus::SUCCESS;
        }
    //-----------------------------------------------------------------------------------------
    // @bsimethod                                    Affan.Khan                      12/2013
    //+---------------+---------------+---------------+---------------+---------------+--------
    BentleyStatus SqlGenerator::BuildDeleteTriggers (SqlClassPersistenceMethod& scpm)
        {
        if (BuildDeleteTriggersForRelationships (scpm) != BentleyStatus::SUCCESS)
            return BentleyStatus::ERROR;

        if (BuildDeleteTriggersForDerivedClasses (scpm) != BentleyStatus::SUCCESS)
            return BentleyStatus::ERROR;

        if (BuildDeleteTriggerForStructArrays (scpm) != BentleyStatus::SUCCESS)
            return BentleyStatus::ERROR;

        if (BuildDeleteTriggerForMe (scpm) != BentleyStatus::SUCCESS)
            return BentleyStatus::ERROR;

        return BentleyStatus::SUCCESS;
        }
        
//************************** ViewGenerator ***************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    10/2013
//---------------------------------------------------------------------------------------
//static
Utf8CP const ViewGenerator::ECCLASSID_COLUMNNAME = "ECClassId";

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      07/2013
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ViewGenerator::CreateView (NativeSqlBuilder& viewSql, ECDbMapCR map, IClassMap const& classMap, bool isPolymorphicQuery,  ECSqlPrepareContext const& prepareContext, bool optimizeByIncludingOnlyRealTables)
    {
    //isPolymorphic is not implemented. By default all query are polymorphic
    if (classMap.IsRelationshipClassMap ())
        return CreateViewForRelationship (viewSql, map, prepareContext, classMap, isPolymorphicQuery, optimizeByIncludingOnlyRealTables);

    viewSql.AppendParenLeft ();
    auto& db = map.GetECDbR();
    auto objectType = DbMetaDataHelper::GetObjectType(db, classMap.GetTable().GetName().c_str());   
    if (objectType == DbMetaDataHelper::ObjectType::None && !isPolymorphicQuery)
        {
        auto status = CreateNullView (viewSql, prepareContext, classMap);
        if (status != BentleyStatus::SUCCESS)
            return status;

        viewSql.AppendParenRight ();
        return BentleyStatus::SUCCESS;
        }

    ViewMemberByTable viewMembers;
    BentleyStatus status;
    if (ClassMap::IsAnyClass (classMap.GetClass ()))
        {
        PRECONDITION (isPolymorphicQuery && "This operation require require polymorphic query to be enabled", BentleyStatus::ERROR);

        std::vector<IClassMap const*> rootClassMaps;
        status = GetRootClasses(rootClassMaps, map.GetECDbR());
        if (status != BentleyStatus::SUCCESS)
            return status;

        for (auto classMap : rootClassMaps)
            {
            status = ComputeViewMembers (viewMembers, map, classMap->GetClass (), isPolymorphicQuery, optimizeByIncludingOnlyRealTables, /*ensureDerivedClassesAreLoaded=*/ false);
            if (status != BentleyStatus::SUCCESS)
                return status;
            }//a.Add(ecClassId, tables, 
        }
    else
        {
        status = ComputeViewMembers (viewMembers,  map, classMap.GetClass(), isPolymorphicQuery, optimizeByIncludingOnlyRealTables, /*ensureDerivedClassesAreLoaded=*/ true);
        }

    if (status == BentleyStatus::SUCCESS)
        {
        int queriesAddedToUnion = 0;
        for (auto& pvm : viewMembers)
            {
            if (optimizeByIncludingOnlyRealTables)
                if (pvm.second.GetStorageType () == DbMetaDataHelper::ObjectType::None)
                    continue;

            if (queriesAddedToUnion > 0)
                viewSql.Append (" UNION ");

            status = GetViewQueryForChild (viewSql, map, prepareContext, *pvm.first, pvm.second.GetClassMaps (), classMap, isPolymorphicQuery);
            if (status != BentleyStatus::SUCCESS)
                return status;

            queriesAddedToUnion++;
            }

        if (queriesAddedToUnion == 0)
            {
            auto status = CreateNullView (viewSql, prepareContext, classMap);
            if (status != BentleyStatus::SUCCESS)
                return status;

            viewSql.AppendParenRight ();
            return BentleyStatus::SUCCESS;
            }
        else
            viewSql.AppendParenRight ();
        }

    return status;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      12/2013
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ViewGenerator::CreateNullView (NativeSqlBuilder& viewSql, ECSqlPrepareContext const& prepareContext, IClassMap const& classMap)
    {
    if (classMap.IsMappedToSecondaryTable ())
        viewSql.Append ("SELECT NULL ECClassId, NULL " ECDB_COL_ECInstanceId ", NULL ParentECInstanceId, NULL ECPropertyPathId, NULL ECArrayIndex");
    else
        viewSql.Append ("SELECT NULL ECClassId, NULL " ECDB_COL_ECInstanceId);

    std::vector<std::pair<PropertyMapCP, PropertyMapCP>> viewPropMaps;
    auto status = GetPropertyMapsOfDerivedClassCastAsBaseClass (viewPropMaps, prepareContext, classMap, classMap, false, false);
    if (status != SUCCESS)
        return status;

 
    AppendViewPropMapsToQuery (viewSql, classMap.GetECDbMap ().GetECDbR (), prepareContext, classMap.GetTable (), viewPropMaps, true /*forNullView*/);
 
    viewSql.Append (" LIMIT 0");
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      07/2013
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ViewGenerator::GetRootClasses (std::vector<IClassMap const*>& rootClasses, ECDbR db)
    {
    ECSchemaList schemas;
    if (db.Schemas ().GetECSchemas (schemas, true) != SUCCESS)
        return ERROR;
        
    std::vector<IClassMap const*> rootClassMaps;
    for (auto schema : schemas)
        {
        if (schema->IsStandardSchema())
            continue;

        for(auto ecClass : schema->GetClasses())
            {
            if (ecClass->GetDerivedClasses().empty())
                {
                auto classMap = db.GetECDbImplR().GetECDbMap ().GetClassMap (*ecClass);
                BeAssert (classMap != nullptr);
                if (!ECDbPolicyManager::GetClassPolicy (*classMap, IsValidInECSqlPolicyAssertion::Get ()).IsSupported ())
                    continue;

                rootClassMaps.push_back(classMap);
                }
            }
        }
    return BentleyStatus::SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      07/2013
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus  ViewGenerator::ComputeViewMembers 
(
ViewMemberByTable& viewMembers, 
ECDbMapCR map,
ECClassCR ecClass, 
bool isPolymorphic, 
bool optimizeByIncludingOnlyRealTables, 
bool ensureDerivedClassesAreLoaded
)
    {
    auto classMap = map.GetClassMap (ecClass);
    if (classMap == nullptr)
        return SUCCESS;

    if (!classMap->IsRelationshipClassMap ())
        {
        if (!ECDbPolicyManager::GetClassPolicy (*classMap, IsValidInECSqlPolicyAssertion::Get ()).IsSupported ())
            //ECSQL_WIP Not supported yet
            return BentleyStatus::SUCCESS;
        }
        
    if (!classMap->GetMapStrategy ().IsNotMapped ())
        {
        if (classMap->GetTable ().GetColumns ().empty ())
            return BentleyStatus::SUCCESS;

        auto itor = viewMembers.find (&classMap->GetTable ());
        if (itor == viewMembers.end ())
            {
            DbMetaDataHelper::ObjectType storageType = DbMetaDataHelper::ObjectType::Table;
            if (optimizeByIncludingOnlyRealTables)
                {
                //This is a db query so optimization comes at a cost
                storageType = DbMetaDataHelper::GetObjectType (map.GetECDbR (), classMap->GetTable ().GetName ().c_str ());
                }
            viewMembers.insert (
                ViewMemberByTable::value_type (&classMap->GetTable (), ViewMember (storageType, *classMap)));
            }
        else
            {
            if (optimizeByIncludingOnlyRealTables)
                {
                if (itor->second.GetStorageType () == DbMetaDataHelper::ObjectType::Table)
                    itor->second.GetClassMaps ().push_back (classMap);
                }
            else
                itor->second.GetClassMaps ().push_back (classMap);
            }
        }

    if (isPolymorphic)
        {
        auto const& derivedClasses = ensureDerivedClassesAreLoaded ? map.GetECDbR ().Schemas ().GetDerivedECClasses (ecClass) : ecClass.GetDerivedClasses ();
        for (auto derivedClass : derivedClasses)
            {
            auto status = ComputeViewMembers (viewMembers, map, *derivedClass, isPolymorphic, optimizeByIncludingOnlyRealTables, ensureDerivedClassesAreLoaded);
            if (status != BentleyStatus::SUCCESS)
                return status;
            }
        }
    return BentleyStatus::SUCCESS;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      09/2013
//+---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ViewGenerator::GetPropertyMapsOfDerivedClassCastAsBaseClass (std::vector<std::pair<PropertyMapCP, PropertyMapCP>>& propMaps, ECSqlPrepareContext const& prepareContext, IClassMap const& baseClassMap, IClassMap const& childClassMap, bool skipSystemProperties, bool embededStatement)
    {
    propMaps.clear ();    
   
    auto& parentMap = embededStatement ? baseClassMap.GetView (IClassMap::View::EmbeddedType)
        : baseClassMap;


    auto& childMap = embededStatement ? childClassMap.GetView (IClassMap::View::EmbeddedType)
        : childClassMap;



    for (auto baseClassPropertyMap : parentMap.GetPropertyMaps ())
        {
        if ((skipSystemProperties && baseClassPropertyMap->IsSystemPropertyMap ()) ||
            baseClassPropertyMap->GetAsPropertyMapToTable ())
            continue;

        auto accessString = baseClassPropertyMap->GetPropertyAccessString ();
        auto childClassCounterpartPropMap = childMap.GetPropertyMap (accessString);
        if (childClassCounterpartPropMap == nullptr)
            return ERROR;

        std::vector<ECDbSqlColumn const*> baseClassPropMapColumns;
        std::vector<ECDbSqlColumn const*> childClassPropMapColumns;
        baseClassPropertyMap->GetColumns (baseClassPropMapColumns);
        childClassCounterpartPropMap->GetColumns (childClassPropMapColumns);
        if (baseClassPropMapColumns.size () != childClassPropMapColumns.size ())
            return ERROR;

        propMaps.push_back ({baseClassPropertyMap, childClassCounterpartPropMap});
        }

    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      09/2013
//+---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ViewGenerator::AppendViewPropMapsToQuery (NativeSqlBuilder& viewQuery, ECDbR ecdb, ECSqlPrepareContext const& prepareContext, ECDbSqlTable const& table, std::vector<std::pair<PropertyMapCP, PropertyMapCP>> const& viewPropMaps, bool forNullView)
    {
    for (auto const& propMapPair : viewPropMaps)
        {
        auto basePropMap = propMapPair.first;
        auto actualPropMap = propMapPair.second;
        if (!prepareContext.GetSelectionOptions ().IsSelected (actualPropMap->GetPropertyAccessString()))
            continue;

        auto aliasSqlSnippets = basePropMap->ToNativeSql(nullptr, ECSqlType::Select, false);
        auto colSqlSnippets = actualPropMap->ToNativeSql(nullptr, ECSqlType::Select, false);

        const size_t snippetCount = colSqlSnippets.size ();
        if (aliasSqlSnippets.size () != snippetCount)
            {
            BeAssert (false && "Number of alias SQL snippets is expected to be the same as number of column SQL snippets.");
            return ERROR;
            }

        for (size_t i = 0; i < snippetCount; i++)
            {
            viewQuery.AppendComma (true);
            auto const& aliasSqlSnippet = aliasSqlSnippets[i];
            if (forNullView)
                viewQuery.Append ("NULL ");
            else
                viewQuery.Append (colSqlSnippets[i]).AppendSpace ();
            
            viewQuery.Append (aliasSqlSnippet);
            }        
        }

    return SUCCESS;
    }
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      07/2013
//+---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ViewGenerator::GetViewQueryForChild (NativeSqlBuilder& viewSql, ECDbMapCR map, ECSqlPrepareContext const& prepareContext, ECDbSqlTable const& table, const std::vector<IClassMap const*>& childClassMap, IClassMap const& baseClassMap, bool isPolymorphic)
    {
    PRECONDITION(!childClassMap.empty(), BentleyStatus::ERROR);  
    PRECONDITION(!table.GetColumns().empty (), BentleyStatus::ERROR);  

    std::vector<ECClassId> classesMappedToTable;

    if (ECDbSchemaPersistence::GetClassesMappedToTable (classesMappedToTable, table, true, map.GetECDbR ()) != SUCCESS)
        return ERROR;
 
    bool oneToManyMapping = classesMappedToTable.size() > 1;

    auto firstChildClassMap = *childClassMap.begin ();

    //Generate Select statement
    viewSql.Append ("SELECT ");

    auto classIdColumn = table.FindColumnCP (ECDB_COL_ECClassId);
    if (classIdColumn != nullptr)
        viewSql.Append (classIdColumn->GetName ().c_str());
    else
        viewSql.Append (firstChildClassMap->GetClass ().GetId ()).AppendSpace ().Append (ECCLASSID_COLUMNNAME);

    std::vector<std::pair<PropertyMapCP, PropertyMapCP>> viewPropMaps;
    
    //auto skipSystemProperties = structArrayProperty == nullptr;
    auto isEmbeded = prepareContext.GetParentArrayProperty () != nullptr;
    auto status = GetPropertyMapsOfDerivedClassCastAsBaseClass (viewPropMaps, prepareContext, baseClassMap, *firstChildClassMap, false, isEmbeded);
    if (status != BentleyStatus::SUCCESS)
        return status;

    //Append prop map columns to query [col1],[col2], ...
    AppendViewPropMapsToQuery (viewSql, map.GetECDbR (), prepareContext, table, viewPropMaps);

    viewSql.Append (" FROM ").AppendEscaped (table.GetName().c_str());

    NativeSqlBuilder where;
    if (classIdColumn != nullptr)
        {
        if (oneToManyMapping)
            {
            if (isPolymorphic)
                {
                std::set<ECClassId> inConstraintCIDs;
                for (auto classMap : childClassMap)
                    inConstraintCIDs.insert (classMap->GetClass ().GetId ());

                std::vector<ECClassId> notInConstraintCIDs;
                for (auto classId : classesMappedToTable)
                    {
                    if (inConstraintCIDs.find (classId) == inConstraintCIDs.end ())
                        notInConstraintCIDs.push_back (classId);
                    }

                //Here we want to create minimum size IN() query. So we will either exclude or include base on which one is minimum
                if (notInConstraintCIDs.size () > inConstraintCIDs.size ()) // include class ids of class we do want.
                    {
                    if (!inConstraintCIDs.empty ())
                        {
                        where.AppendParenLeft ().AppendEscaped (table.GetName ().c_str ()).AppendDot ().AppendEscaped (classIdColumn->GetName ().c_str ()).Append (" IN (");
                        bool isFirstItem = true;
                        for (auto& classMap : childClassMap)
                            {
                            if (!isFirstItem)
                            where.AppendComma (true);

                            where.Append (classMap->GetClass ().GetId ());

                            isFirstItem = false;
                            }
                        where.AppendParenRight ().AppendParenRight ();
                        }
                    }
                else //exclude class ids of class we don't want. 
                    {
                    if (!notInConstraintCIDs.empty ())
                        {
                        where.AppendParenLeft ().AppendEscaped (table.GetName ().c_str ()).AppendDot ().AppendEscaped (classIdColumn->GetName ().c_str ()).Append (" NOT IN (");
                        bool isFirstItem = true;
                        for (auto classId : notInConstraintCIDs)
                            {
                            if (!isFirstItem)
                            where.AppendComma (true);

                            where.Append (classId);

                            isFirstItem = false;
                            }
                        where.AppendParenRight ().AppendParenRight ();
                        }
                    }
                }
            else
            where.AppendParenLeft ().AppendEscaped (table.GetName ().c_str ()).AppendDot ().AppendEscaped (classIdColumn->GetName ().c_str ()).Append (" = ").Append (firstChildClassMap->GetClass ().GetId ()).AppendParenRight ();
            }
        }
    //We allow query of struct classes.
    if (firstChildClassMap->IsMappedToSecondaryTable ())
        {
        if (!where.IsEmpty ())
        where.Append (" AND ");

        if (prepareContext.GetParentArrayProperty() == nullptr)
            where.Append ("(ECPropertyPathId IS NULL AND ECArrayIndex IS NULL)");
        }

    if (!where.IsEmpty())
        viewSql.Append (" WHERE ").Append (where);
    
    return BentleyStatus::SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      09/2013
//+---------------+---------------+---------------+---------------+---------------+-------
    BentleyStatus ViewGenerator::CreateNullViewForRelationshipClassEndTableMap (NativeSqlBuilder& viewSql, ECSqlPrepareContext const& prepareContext, RelationshipClassMapCR relationMap, IClassMap const& baseClassMap)
    {
    AppendSystemPropMapsToNullView (viewSql, prepareContext, relationMap, false /*endWithComma*/);

    viewSql.Append (" LIMIT 0");

    return BentleyStatus::SUCCESS;
    }
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      09/2013
//+---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ViewGenerator::CreateNullViewForRelationshipClassLinkTableMap (NativeSqlBuilder& viewSql, ECSqlPrepareContext const& prepareContext, RelationshipClassMapCR relationMap, IClassMap const& baseClassMap)
    {
    AppendSystemPropMapsToNullView (viewSql, prepareContext, relationMap, false /*endWithComma*/);

    //! Only link table mapped relationship properties are persisted
    std::vector<std::pair<PropertyMapCP, PropertyMapCP>> viewPropMaps;
    auto status = GetPropertyMapsOfDerivedClassCastAsBaseClass (viewPropMaps, prepareContext, baseClassMap, relationMap, true, false);
    if (status != BentleyStatus::SUCCESS)
        return status;

    //Append columns to query [col1],[col2], ...
    AppendViewPropMapsToQuery (viewSql, relationMap.GetECDbMap ().GetECDbR (), prepareContext, relationMap.GetTable (), viewPropMaps, true);
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      09/2013
//+---------------+---------------+---------------+---------------+---------------+-------
    BentleyStatus ViewGenerator::GetAllChildRelationships (std::vector<RelationshipClassMapCP>& relationshipMaps, ECDbMapCR map, ECSqlPrepareContext const& prepareContext, IClassMap const& baseRelationMap)
    {
    auto& baseClass = baseRelationMap.GetClass();
    for (auto childClass : map.GetECDbR ().Schemas ().GetDerivedECClasses (const_cast<ECClassR>(baseClass)))
        {
        auto childClassMap = map.GetClassMap (*childClass);
        if (childClassMap == nullptr)
            continue;

        auto policy = ECDbPolicyManager::GetClassPolicy (*childClassMap, IsValidInECSqlPolicyAssertion::Get ());
        if (policy.IsSupported ())
            {
            if (!childClassMap->IsRelationshipClassMap ())
                {
                LOG.errorv ("ECDb does not support relationships that have derived non-relationship classes.");
                BeAssert (childClassMap->IsRelationshipClassMap ());
                return ERROR;
                }

            relationshipMaps.push_back (static_cast<RelationshipClassMapCP>(childClassMap));
            }

        auto status = GetAllChildRelationships (relationshipMaps, map, prepareContext, *childClassMap);
        if (status != BentleyStatus::SUCCESS)
            return status;
        }

    return BentleyStatus::SUCCESS;  
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      09/2013
//+---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ViewGenerator::CreateViewForRelationshipClassLinkTableMap (NativeSqlBuilder& viewSql, ECDbMapCR ecdbMap, ECSqlPrepareContext const& prepareContext, RelationshipClassMapCR relationMap, IClassMap const& baseClassMap)
    {
    viewSql.Append ("SELECT ");
    AppendSystemPropMaps (viewSql, ecdbMap, prepareContext, relationMap);

    //! Only link table mapped relationship properties are persisted
    std::vector<std::pair<PropertyMapCP, PropertyMapCP>> viewPropMaps;
    auto status = GetPropertyMapsOfDerivedClassCastAsBaseClass (viewPropMaps, prepareContext, baseClassMap, relationMap, true, false);
    if (status != SUCCESS)
        return status;

    //Append prop maps' columns to query [col1],[col2], ...
    AppendViewPropMapsToQuery (viewSql, ecdbMap.GetECDbR (), prepareContext, relationMap.GetTable (), viewPropMaps);

    viewSql.Append (" FROM ").AppendEscaped (relationMap.GetTable ().GetName ().c_str ());
   
    //Append secondary table JOIN
    auto const secondaryTables = relationMap.GetSecondaryTables ();
    auto primaryTable = &relationMap.GetTable ();
    auto primaryECInstanceIdColumn = primaryTable->GetFilteredColumnFirst (ECDbKnownColumns::ECInstanceId);
    BeAssert (primaryECInstanceIdColumn != nullptr);
    if (BuildRelationshipJoinIfAny (viewSql, relationMap, ECN::ECRelationshipEnd::ECRelationshipEnd_Source) != BentleyStatus::SUCCESS)
        return BentleyStatus::ERROR;

    if (BuildRelationshipJoinIfAny (viewSql, relationMap, ECN::ECRelationshipEnd::ECRelationshipEnd_Target) != BentleyStatus::SUCCESS)
        return BentleyStatus::ERROR;

    return BentleyStatus::SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      09/2013
//+---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ViewGenerator::CreateViewForRelationshipClassEndTableMap (NativeSqlBuilder& viewSql, ECDbMapCR ecdbMap, ECSqlPrepareContext const& prepareContext, RelationshipClassEndTableMapCR relationMap, IClassMap const& baseClassMap)
    {
    //ECInstanceId, ECClassId of the relationship instance
    viewSql.Append ("SELECT ");
    AppendSystemPropMaps (viewSql, ecdbMap, prepareContext,  relationMap);
    viewSql.Append (" FROM ").AppendEscaped (relationMap.GetTable ().GetName ().c_str ());

    //Append secondary table JOIN
    auto const secondaryTables = relationMap.GetSecondaryTables ();
    auto primaryTable = &relationMap.GetTable ();
    auto primaryECInstanceIdColumn = primaryTable->GetFilteredColumnFirst (ECDbKnownColumns::ECInstanceId);
    BeAssert (primaryECInstanceIdColumn != nullptr);

    if (BuildRelationshipJoinIfAny (viewSql, relationMap, ECN::ECRelationshipEnd::ECRelationshipEnd_Source) != BentleyStatus::SUCCESS)
        return BentleyStatus::ERROR;

    if (BuildRelationshipJoinIfAny (viewSql, relationMap, ECN::ECRelationshipEnd::ECRelationshipEnd_Target) != BentleyStatus::SUCCESS)
    return BentleyStatus::ERROR;


    viewSql.Append (" WHERE ").Append (relationMap.GetOtherEndECInstanceIdPropMap ()->ToNativeSql (nullptr, ECSqlType::Select, false)).Append (" IS NOT NULL");
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      07/2015
//+---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ViewGenerator::BuildRelationshipJoinIfAny (NativeSqlBuilder& sqlBuilder, RelationshipClassMapCR classMap, ECN::ECRelationshipEnd endPoint)
    {
    auto ecclassIdPropertyMap = static_cast<PropertyMapRelationshipConstraintClassId const*>(endPoint == ECRelationshipEnd::ECRelationshipEnd_Source ? classMap.GetSourceECClassIdPropMap () : classMap.GetTargetECClassIdPropMap ());
    if (!ecclassIdPropertyMap->IsMappedToPrimaryTable ())
        {
        auto ecInstanceIdPropertyMap = static_cast<PropertyMapRelationshipConstraintECInstanceId const*>(endPoint == ECRelationshipEnd::ECRelationshipEnd_Source ? classMap.GetSourceECInstanceIdPropMap () : classMap.GetTargetECInstanceIdPropMap ());
        auto const& targetTable = ecclassIdPropertyMap->GetFirstColumn ()->GetTable ();

        sqlBuilder.Append (" INNER JOIN ");
        sqlBuilder.AppendEscaped (targetTable.GetName ().c_str ());
        sqlBuilder.AppendSpace ();
        sqlBuilder.Append (GetECClassIdPrimaryTableAlias (endPoint));
        sqlBuilder.Append (" ON ");
        sqlBuilder.Append (GetECClassIdPrimaryTableAlias (endPoint));
        sqlBuilder.AppendDot ();
        auto targetECInstanceIdColumn = targetTable.GetFilteredColumnFirst (ECDbKnownColumns::ECInstanceId);
        if (targetECInstanceIdColumn == nullptr)
            {
            BeAssert (false && "Failed to find ECInstanceId column in target table");
            return BentleyStatus::ERROR;
            }
        sqlBuilder.AppendEscaped (targetECInstanceIdColumn->GetName ().c_str ());
        sqlBuilder.Append (" = ");
        sqlBuilder.AppendEscaped (ecInstanceIdPropertyMap->GetFirstColumn ()->GetTable ().GetName ().c_str ());

        sqlBuilder.AppendDot ();
        sqlBuilder.Append (ecInstanceIdPropertyMap->GetFirstColumn ()->GetName ().c_str ());
        sqlBuilder.AppendSpace ();
        }

    return BentleyStatus::SUCCESS;
    }
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      09/2013
//+---------------+---------------+---------------+---------------+---------------+-------
    BentleyStatus ViewGenerator::CreateViewForRelationship (NativeSqlBuilder& viewSql, ECDbMapCR map, ECSqlPrepareContext const& prepareContext, IClassMap const& relationMap, IClassMap const& baseClassMap)
    {
    switch(relationMap.GetClassMapType())
        {
        case ClassMap::Type::RelationshipEndTable:
            return CreateViewForRelationshipClassEndTableMap (viewSql, map, prepareContext, static_cast<RelationshipClassEndTableMapCR>(relationMap), baseClassMap);
        case ClassMap::Type::RelationshipLinkTable:
            return CreateViewForRelationshipClassLinkTableMap (viewSql, map, prepareContext, static_cast<RelationshipClassMapCR>(relationMap), baseClassMap);
        }

    return ERROR;
    }
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      09/2013
//+---------------+---------------+---------------+---------------+---------------+-------
    BentleyStatus ViewGenerator::CreateNullViewForRelationship (NativeSqlBuilder& viewSql, ECDbMapCR map, ECSqlPrepareContext const& prepareContext, IClassMap const& relationMap, IClassMap const& baseClassMap)
    {
    switch (relationMap.GetClassMapType ())
        {
        case ClassMap::Type::RelationshipEndTable:
            return CreateNullViewForRelationshipClassEndTableMap (viewSql, prepareContext, static_cast<RelationshipClassMapCR>(relationMap), baseClassMap);
        case ClassMap::Type::RelationshipLinkTable:
            return CreateNullViewForRelationshipClassLinkTableMap (viewSql, prepareContext, static_cast<RelationshipClassMapCR>(relationMap), baseClassMap);
        default:
            return ERROR;
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      07/2013
//+---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ViewGenerator::CreateViewForRelationship (NativeSqlBuilder& viewSql, ECDbMapCR map, ECSqlPrepareContext const& prepareContext, IClassMap const& relationMap, bool isPolymorphic, bool optimizeByIncludingOnlyRealTables)
    {
    BeAssert (relationMap.IsRelationshipClassMap ());
    BentleyStatus status = BentleyStatus::SUCCESS;

    if (relationMap.GetMapStrategy().IsNotMapped ())
        return BentleyStatus::ERROR;

    ViewMemberByTable vmt;
    status = ComputeViewMembers (vmt, map, relationMap.GetClass (), isPolymorphic, optimizeByIncludingOnlyRealTables, true);
    if (status != BentleyStatus::SUCCESS)
        return status;
    if (vmt.empty ())
        {
        return CreateNullViewForRelationship (viewSql, map, prepareContext, relationMap, relationMap);
        }
     ViewMember viewMember = vmt[&relationMap.GetTable()];
     NativeSqlBuilder unionQuery;

     for (auto cm : viewMember.GetClassMaps())
         {
         switch (cm->GetClassMapType())
             {
             case ClassMap::Type::RelationshipEndTable:
                 if (!unionQuery.IsEmpty())
                     unionQuery.Append(" UNION ");
                 status = CreateViewForRelationshipClassEndTableMap(unionQuery, map, prepareContext, *static_cast<RelationshipClassEndTableMapCP>(cm), relationMap);
                 if (status != BentleyStatus::SUCCESS)
                     return status;
                     break;
             case ClassMap::Type::RelationshipLinkTable:
             {
                 if (!unionQuery.IsEmpty())
                     unionQuery.Append(" UNION ");

                 status = CreateViewForRelationshipClassLinkTableMap(unionQuery, map, prepareContext, *static_cast<RelationshipClassLinkTableMapCP>(cm), relationMap);
                 if (status != BentleyStatus::SUCCESS)
                     return status;

                 auto column = relationMap.GetTable().GetFilteredColumnFirst(ECDbKnownColumns::ECClassId);
                 if (column != nullptr)
                     {

                     std::vector<ECClassId> classesMappedToTable;
                     if (ECDbSchemaPersistence::GetClassesMappedToTable(classesMappedToTable, relationMap.GetTable(), false, map.GetECDbR()) != SUCCESS)
                         return ERROR;

                     if (classesMappedToTable.size() != 1)
                         {
                         unionQuery.Append(" WHERE ");
                         unionQuery.AppendEscaped (relationMap.GetTable ().GetName ().c_str ()).AppendDot ().AppendEscaped (column->GetName ().c_str ()).Append (" IN ");
                         unionQuery.AppendParenLeft();
                         
                             unionQuery.Append(cm->GetClass().GetId());
                             
                         unionQuery.AppendParenRight();
                         }
                     }
                }
             }
         }
     vmt.erase(&relationMap.GetTable());
    for (auto& vm : vmt)
        {
        auto table = vm.first;
        if (vm.second.GetStorageType () != DbMetaDataHelper::ObjectType::Table)
            continue;



        std::vector<RelationshipClassEndTableMapCP> etm;
        std::vector<RelationshipClassLinkTableMapCP> ltm;
        for (auto cm : vm.second.GetClassMaps ())
            {
            switch (cm->GetClassMapType ())
                {
                case ClassMap::Type::RelationshipEndTable:
                    etm.push_back (static_cast<RelationshipClassEndTableMapCP>(cm)); break;
                case ClassMap::Type::RelationshipLinkTable:
                    ltm.push_back (static_cast<RelationshipClassLinkTableMapCP>(cm)); break;
                }
            }

        if (!ltm.empty ())
            {
            if (!unionQuery.IsEmpty ())
                unionQuery.Append (" UNION ");

            status = CreateViewForRelationshipClassLinkTableMap (unionQuery, map, prepareContext, *ltm.front (), relationMap);
            if (status != SUCCESS)
                return status;

            auto column = table->GetFilteredColumnFirst (ECDbKnownColumns::ECClassId);
            if (column != nullptr)
                {

                std::vector<ECClassId> classesMappedToTable;
                if (ECDbSchemaPersistence::GetClassesMappedToTable (classesMappedToTable, *table, false, map.GetECDbR ()) != SUCCESS)
                    return ERROR;

                if (classesMappedToTable.size () != ltm.size ())
                    {
                    unionQuery.Append (" WHERE ");
                    unionQuery.AppendEscaped(table->GetName().c_str()).AppendDot().AppendEscaped (column->GetName ().c_str ()).Append (" IN ");
                    unionQuery.AppendParenLeft ();
                    for (auto lt : ltm)
                        {
                        unionQuery.Append (lt->GetClass ().GetId ());
                        if (ltm.back () != lt)
                            {
                            unionQuery.AppendComma ();
                            }
                        }
                    unionQuery.AppendParenRight ();
                    }
                }
            }

        for (auto et : etm)
            {
            if (!unionQuery.IsEmpty ())
                unionQuery.Append (" UNION ");

            status = CreateViewForRelationshipClassEndTableMap (unionQuery, map, prepareContext, *et, relationMap);
            if (status != BentleyStatus::SUCCESS)
                return status;
            }
        }

    viewSql.AppendParenLeft ().Append (unionQuery.ToString ()).AppendParenRight ();
    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    12/2013
//---------------------------------------------------------------------------------------
//static
BentleyStatus ViewGenerator::AppendSystemPropMaps (NativeSqlBuilder& viewSql, ECDbMapCR ecdbMap, ECSqlPrepareContext const& prepareContext, RelationshipClassMapCR relationMap)
    {
    auto ecId = relationMap.GetECInstanceIdPropertyMap ();
    if (!ecId->IsVirtual ())
        {
        viewSql.AppendEscaped (ecId->GetFirstColumn ()->GetTable ().GetName ().c_str ()).AppendDot ();
        }

    viewSql.Append (relationMap.GetECInstanceIdPropertyMap ()->ToNativeSql ( nullptr, ECSqlType::Select, false)).AppendComma (true);
    viewSql.Append (relationMap.GetClass ().GetId ()).AppendSpace ().Append (ECCLASSID_COLUMNNAME).AppendComma (true);

    //Source
    BeAssert (dynamic_cast<PropertyMapRelationshipConstraint const*> (relationMap.GetSourceECInstanceIdPropMap ()) != nullptr);
    auto propMap = static_cast<PropertyMapRelationshipConstraint const*> (relationMap.GetSourceECInstanceIdPropMap ());
    if (!propMap->IsVirtual ())
        {
        viewSql.AppendEscaped (propMap->GetFirstColumn ()->GetTable ().GetName ().c_str ()).AppendDot ();
        }

    propMap->AppendSelectClauseSqlSnippetForView (viewSql);
    viewSql.AppendComma (true);

    BeAssert (dynamic_cast<PropertyMapRelationshipConstraint const*> (relationMap.GetSourceECClassIdPropMap ()) != nullptr);
    propMap = static_cast<PropertyMapRelationshipConstraint const*> (relationMap.GetSourceECClassIdPropMap ());
    if (!propMap->IsVirtual ())
        {
        if (!propMap->IsMappedToPrimaryTable ())
            viewSql.AppendEscaped (GetECClassIdPrimaryTableAlias (ECRelationshipEnd::ECRelationshipEnd_Source)).AppendDot ();
        else
            viewSql.AppendEscaped (propMap->GetFirstColumn ()->GetTable ().GetName ().c_str ()).AppendDot ();
        }

    AppendConstraintClassIdPropMap (viewSql, prepareContext, *propMap, ecdbMap, relationMap, relationMap.GetRelationshipClass ().GetSource ());
    viewSql.AppendComma (true);

    //Target
    BeAssert (dynamic_cast<PropertyMapRelationshipConstraint const*> (relationMap.GetTargetECInstanceIdPropMap ()) != nullptr);
    propMap = static_cast<PropertyMapRelationshipConstraint const*> (relationMap.GetTargetECInstanceIdPropMap ());
    if (!propMap->IsVirtual ())
        {
        viewSql.AppendEscaped (propMap->GetFirstColumn ()->GetTable ().GetName ().c_str ()).AppendDot ();
        }

    propMap->AppendSelectClauseSqlSnippetForView (viewSql);
    viewSql.AppendComma (true);

    BeAssert (dynamic_cast<PropertyMapRelationshipConstraint const*> (relationMap.GetTargetECClassIdPropMap ()) != nullptr);
    propMap = static_cast<PropertyMapRelationshipConstraint const*> (relationMap.GetTargetECClassIdPropMap ());
    if (!propMap->IsVirtual ())
        {
        if (!propMap->IsMappedToPrimaryTable ())
            viewSql.AppendEscaped (GetECClassIdPrimaryTableAlias (ECRelationshipEnd::ECRelationshipEnd_Target)).AppendDot ();
        else
            viewSql.AppendEscaped (propMap->GetFirstColumn ()->GetTable ().GetName ().c_str ()).AppendDot ();
        }

    AppendConstraintClassIdPropMap (viewSql, prepareContext, *propMap, ecdbMap, relationMap, relationMap.GetRelationshipClass ().GetTarget ());

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    12/2013
//---------------------------------------------------------------------------------------
//static
    BentleyStatus ViewGenerator::AppendSystemPropMapsToNullView (NativeSqlBuilder& viewSql, ECSqlPrepareContext const& prepareContext, RelationshipClassMapCR relationMap, bool endWithComma)
    {
    //ECInstanceId and ECClassId
    auto sqlSnippets = relationMap.GetECInstanceIdPropertyMap ()->ToNativeSql (nullptr, ECSqlType::Select, false);
    if (sqlSnippets.size () != 1)
        return ERROR;
    viewSql.Append ("SELECT NULL ").Append (sqlSnippets).AppendComma (true);
    viewSql.Append ("NULL ").Append (ECCLASSID_COLUMNNAME).AppendComma (true);

    //Source constraint
    sqlSnippets = relationMap.GetSourceECInstanceIdPropMap ()->ToNativeSql (nullptr, ECSqlType::Select, false);
    if (sqlSnippets.size () != 1)
        return ERROR;
    viewSql.Append ("NULL ").Append (sqlSnippets).AppendComma (true);

    sqlSnippets = relationMap.GetSourceECClassIdPropMap ()->ToNativeSql (nullptr, ECSqlType::Select, false);
    if (sqlSnippets.size () != 1)
        return ERROR;
    viewSql.Append ("NULL ").Append (sqlSnippets).AppendComma (true);

    //Target constraint
    sqlSnippets = relationMap.GetTargetECInstanceIdPropMap ()->ToNativeSql (nullptr, ECSqlType::Select, false);
    if (sqlSnippets.size () != 1)
        return ERROR;
    viewSql.Append ("NULL ").Append (sqlSnippets).AppendComma (true);

    sqlSnippets = relationMap.GetTargetECClassIdPropMap ()->ToNativeSql (nullptr, ECSqlType::Select, false);
    if (sqlSnippets.size () != 1)
        return ERROR;
    viewSql.Append ("NULL ").Append (sqlSnippets);

    if (endWithComma)
        viewSql.AppendComma (true);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    12/2013
//---------------------------------------------------------------------------------------
//static
BentleyStatus ViewGenerator::AppendConstraintClassIdPropMap (NativeSqlBuilder& viewSql, ECSqlPrepareContext const& prepareContext, PropertyMapRelationshipConstraint const& propMap, ECDbMapCR ecdbMap, RelationshipClassMapCR relationMap, ECRelationshipConstraintCR constraint)
    {
    if (propMap.IsVirtual ())
        {
        bset<IClassMap const*> classMaps;
        ecdbMap.GetClassMapsFromRelationshipEnd (classMaps, constraint, true);
        if (classMaps.size () != 1)
            {
            BeAssert (false && "Expecting exactly one ClassMap at end");
            return BentleyStatus::ERROR;
            }
        auto classMap = *classMaps.begin ();
        BeAssert (classMap != nullptr);
        const auto endClassId = classMap->GetClass ().GetId ();

        viewSql.Append (endClassId).AppendSpace ();
        viewSql.Append (propMap.ToNativeSql ( nullptr, ECSqlType::Select, false));
        }
    else
        propMap.AppendSelectClauseSqlSnippetForView (viewSql);

    return SUCCESS;
    }


//static 
void ViewGenerator::LoadDerivedClassMaps (std::map<ECClassId, IClassMap const *>& viewClasses, ECDbMapCR map, IClassMap const* classMap)
    {
    if (viewClasses.find (classMap->GetClass().GetId ()) != viewClasses.end ())
        return;

    ECDbSchemaManagerCR schemaManager = map.GetECDbR ().Schemas ();
    
    for (ECClassCP ecClass : schemaManager.GetDerivedECClasses (classMap->GetClass ()))
        {
        if (viewClasses.find (ecClass->GetId ()) == viewClasses.end ())
            continue;

        IClassMap const* classMap = map.GetClassMap (*ecClass);
        if (classMap == nullptr || classMap->GetMapStrategy ().IsNotMapped ())
            {
            continue;
            }

        viewClasses.insert (std::map<ECClassId, IClassMap const*>::value_type (ecClass->GetId (), classMap));

        if (!ecClass->GetDerivedClasses ().empty ())
            {
            LoadDerivedClassMaps (viewClasses, map, classMap);
            }
        }
    }

//static
void ViewGenerator::CreateSystemClassView (NativeSqlBuilder &viewSql, std::map<ECDbSqlTable const*, std::vector<IClassMap const*>> &tableMap, std::set<ECDbSqlTable const*> &tableToIncludeEntirly, bool forStructArray, ECSqlPrepareContext const& prepareContext)
    {
    bool first = true;
    for (auto& pair : tableMap)
        {

        std::vector<IClassMap const*>& classMaps = pair.second;
        ECDbSqlTable const* tableP = pair.first;

        bool includeEntireTable = tableToIncludeEntirly.find (tableP) != tableToIncludeEntirly.end ();
        IClassMap const* classMap = classMaps[0];
        ECDbSqlColumn const* ecInstanceIdColumn = classMap->GetPropertyMap ("ECInstanceId")->GetFirstColumn ();
        ECDbSqlColumn const* ecClassIdColumn = tableP->FindColumnCP ("ECClassId");

        if (tableP->GetPersistenceType() == PersistenceType::Virtual)
            continue;

        DbMetaDataHelper::ObjectType objectType = DbMetaDataHelper::GetObjectType (classMap->GetECDbMap ().GetECDbR (), tableP->GetName ().c_str ());
        if (objectType == DbMetaDataHelper::ObjectType::None)
            continue;

        if (!first)
            {
            viewSql.Append ("\r\nUNION", true);
            }

        viewSql.Append ("SELECT", true);
        viewSql.Append (ecInstanceIdColumn->GetName ().c_str (), true);
        if (first)
            {
            viewSql.Append (ECDB_COL_ECInstanceId, true);
            }

        viewSql.AppendComma (true);

        if (ecClassIdColumn)
            viewSql.Append (ecClassIdColumn->GetName ().c_str (), true);
        else
            viewSql.Append (classMap->GetClass ().GetId ()).AppendSpace();
        
        if (first)
            {
            viewSql.Append ("ECClassId", true);
            }

        if (forStructArray)
            {
            viewSql.AppendComma (true).Append (ECDbSystemSchemaHelper::ECPROPERTYPATHID_PROPNAME).AppendComma (true).Append (ECDbSystemSchemaHelper::ECARRAYINDEX_PROPNAME);
            }

        viewSql.AppendSpace ().Append ("FROM", true).AppendEscaped (tableP->GetName ().c_str ()).AppendSpace ();

        bool wherePreset = false;
        if (!includeEntireTable && classMaps.size () > 1)
            {
            if (classMaps.size () == 1)
                viewSql.Append ("WHERE ECClassID = ").Append (classMap->GetClass ().GetId ());
            else
                {
                viewSql.Append ("WHERE ECClassID IN (");
                for (auto itor = classMaps.begin (); itor != classMaps.end (); ++itor)
                    {
                    viewSql.Append ((*itor)->GetClass ().GetId ());
                    if (itor != (classMaps.end () - 1))
                        viewSql.AppendComma (true);
                    }

                viewSql.AppendParenRight ();
                }

            wherePreset = true;
            }

        if (classMap->IsMappedToSecondaryTable ())
            {
            viewSql.AppendSpace ();
            if (!wherePreset)
                viewSql.Append ("WHERE", true);
            else
                viewSql.Append ("AND", true);

            if (forStructArray)
                viewSql.Append ("(ECPropertyPathId IS NOT NULL AND ECArrayIndex IS NOT NULL)");
            else
                viewSql.Append ("(ECPropertyPathId IS NULL AND ECArrayIndex IS NULL)");
            }
        first = false;
        }
    }

//=================================================================================================================
SqlTriggerBuilder::SqlTriggerBuilder (Type type, Condition condition, bool temprary)
    :m_type (type), m_condition (condition), m_temprory (temprary)
    {
    }
SqlTriggerBuilder::SqlTriggerBuilder (SqlTriggerBuilder && rhs)
    :m_type (std::move (rhs.m_type)), m_condition (std::move (rhs.m_condition)), m_temprory (std::move (rhs.m_temprory)), m_name (std::move (rhs.m_name)),
    m_when (std::move (rhs.m_when)), m_body (std::move (rhs.m_body)), m_on (rhs.m_on), m_ofColumns (std::move (rhs.m_ofColumns))
    {
    }
SqlTriggerBuilder& SqlTriggerBuilder::operator = (SqlTriggerBuilder&& rhs)
    {
    if (this != &rhs)
        {
        m_name = rhs.m_name;
        m_type = rhs.m_type;
        m_body = rhs.m_body;
        m_condition = rhs.m_condition;
        m_on = rhs.m_on;
        m_temprory = rhs.m_temprory;
        m_when = rhs.m_when;
        rhs.m_ofColumns = rhs.m_ofColumns;
        }

    return *this;
    }
SqlTriggerBuilder& SqlTriggerBuilder::operator = (SqlTriggerBuilder const& rhs)
    {
    if (this != &rhs)
        {
        m_name = std::move (rhs.m_name);
        m_type = std::move(rhs.m_type);
        m_body = std::move (rhs.m_body);
        m_condition = std::move (rhs.m_condition);
        m_on = std::move (rhs.m_on);
        m_temprory = std::move (rhs.m_temprory);
        m_when = std::move (rhs.m_when);
        m_ofColumns = std::move (rhs.m_ofColumns);
        }

    return *this;
    }
NativeSqlBuilder& SqlTriggerBuilder::GetNameBuilder () { return m_name; }
NativeSqlBuilder& SqlTriggerBuilder::GetWhenBuilder () { return m_when; }
NativeSqlBuilder& SqlTriggerBuilder::GetBodyBuilder () { return m_body; }
NativeSqlBuilder& SqlTriggerBuilder::GetOnBuilder () { return m_on; }
SqlTriggerBuilder::Type SqlTriggerBuilder::GetType () const { return m_type; }
SqlTriggerBuilder::Condition SqlTriggerBuilder::GetCondition () const { return m_condition; }
std::vector<Utf8String> const& SqlTriggerBuilder::GetUpdateOfColumns () const
    {
    BeAssert (m_type == Type::UpdateOf);
    return m_ofColumns;
    }
std::vector<Utf8String>& SqlTriggerBuilder::GetUpdateOfColumnsR ()
    {
    BeAssert (m_type == Type::UpdateOf);
    return m_ofColumns;
    }
Utf8CP SqlTriggerBuilder::GetName () const { return m_name.ToString (); }
Utf8CP SqlTriggerBuilder::GetWhen () const { return m_when.ToString (); }
Utf8CP SqlTriggerBuilder::GetBody () const { return m_body.ToString (); }
Utf8CP SqlTriggerBuilder::GetOn () const { return m_on.ToString (); }
bool SqlTriggerBuilder::IsTemprory () const { return m_temprory; }
bool SqlTriggerBuilder::IsValid () const
    {
    if (m_name.IsEmpty ())
        {
        BeAssert (false && "Must specify a trigger name");
        return false;
        }

    if (m_on.IsEmpty ())
        {
        BeAssert (false && "Must specify a trigger ON Table/View");
        return false;
        }

    if (m_body.IsEmpty ())
        {
        BeAssert (false && "Must specify a trigger body");
        return false;
        }

    if (m_type == Type::UpdateOf && m_ofColumns.empty ())
        {
        BeAssert (false && "For UPDATE OF trigger must specify atleast one column");
        return false;
        }

    return true;
    }
const Utf8String SqlTriggerBuilder::ToString (SqlOption option, bool escape) const
    {
    if (!IsValid ())
        {
        BeAssert (false && "Trigger specification is not valid");
        }

    NativeSqlBuilder sql;
    if (option == SqlOption::Drop || option == SqlOption::DropIfExists)
        {
        sql.Append ("DROP TRIGGER ").AppendIf (option == SqlOption::DropIfExists, "IF EXISTS ").AppendEscapedIf (escape, GetName ()).Append (";");
        }
    else
        {
        sql.AppendLine ("\n--### WARNING: SYSTEM GENERATED TRIGGER. DO NOT CHANGE THIS TRIGGER IN ANYWAY. ####");
        sql.Append ("CREATE TRIGGER ").AppendIf (IsTemprory (), "TEMP ").AppendIf (option == SqlOption::CreateIfNotExist, "IF NOT EXISTS ").AppendEscapedIf (escape, GetName ()).AppendEOL ();
        switch (m_condition)
            {
            case Condition::After:
                sql.Append ("AFTER "); break;
            case Condition::Before:
                sql.Append ("BEFORE "); break;
            case Condition::InsteadOf:
                sql.Append ("INSTEAD OF "); break;
            }

        switch (m_type)
            {
            case Type::Delete:
                sql.Append ("DELETE "); break;
            case Type::Insert:
                sql.Append ("INSERT "); break;
            case Type::Update:
                sql.Append ("UPDATE "); break;
            case Type::UpdateOf:
                sql.Append ("UPDATE OF ");
                for (auto& column : m_ofColumns)
                    {
                    if (&column != &m_ofColumns.front ())
                        sql.Append (", ");

                    sql.AppendEscapedIf (escape, column.c_str ());
                    }
                break;
            }

        sql.AppendEOL ();
        sql.Append ("ON ").AppendEscapedIf (escape, GetOn ()).AppendEOL ();
        if (!m_when.IsEmpty ())
            {
            sql.Append ("\tWHEN ").Append (GetWhen ()).AppendEOL ();
            }

        sql.Append ("BEGIN").AppendEOL ();
        sql.Append (GetBody ());
        sql.Append ("END;").AppendEOL ();;
        }

    return sql.ToString ();
    }
//=================================================================================================================

ECDbMapAnalyser::Class::Class (ClassMapCR classMap, ECDbMapAnalyser::Storage& storage, ECDbMapAnalyser::Class* parent)
    :m_classMap (classMap), m_storage (storage), m_inQueue (true), m_parent (parent)
    {
    m_name = GetClassMap ().GetClass ().GetSchema ().GetNamespacePrefix () + "_" + GetClassMap ().GetClass ().GetName ();
    }
Utf8CP ECDbMapAnalyser::Class::GetSqlName () const { return  m_name.c_str (); }
ECDbMapAnalyser::Storage& ECDbMapAnalyser::Class::GetStorageR () { return m_storage; }
ClassMapCR ECDbMapAnalyser::Class::GetClassMap () const { return m_classMap; }
ECDbMapAnalyser::Class* ECDbMapAnalyser::Class::GetParent () { return m_parent; }
std::map <ECDbMapAnalyser::Storage const*, std::set<ECDbMapAnalyser::Class const*>>& ECDbMapAnalyser::Class::GetPartitionsR () { return m_partitions; }
bool ECDbMapAnalyser::Class::InQueue () const { return m_inQueue; }
void ECDbMapAnalyser::Class::Done () { m_inQueue = false; }
std::vector<ECDbMapAnalyser::Storage const*> ECDbMapAnalyser::Class::GetNoneVirtualStorages () const
    {
    std::vector<Storage const*> tmp;
    for (auto& key : m_partitions)
        {
        if (key.first->IsVirtual ())
            continue;

        tmp.push_back (key.first);
        }

    return std::move (tmp);
    }
bool ECDbMapAnalyser::Class::IsAbstract () const
    {
    return GetNoneVirtualStorages ().empty ();
    }
bool ECDbMapAnalyser::Class::RequireView () const
    {
    return GetNoneVirtualStorages ().size () > 1 && !m_classMap.GetMapStrategy().IsNotMapped();
    }
    




ECDbMapAnalyser::Relationship::EndPoint::EndPoint (RelationshipClassMapCR map, EndType type)
    :m_ecid (nullptr), m_classId (nullptr), m_type (type)
    {
    auto direction = map.GetRelationshipClass ().GetStrengthDirection ();
    if (direction == ECRelatedInstanceDirection::Forward)
        {
        if (type == EndType::From)
            {
            m_ecid = map.GetSourceECInstanceIdPropMap ();
            m_classId = map.GetSourceECClassIdPropMap ();
            }
        else{
            m_ecid = map.GetTargetECInstanceIdPropMap ();
            m_classId = map.GetTargetECClassIdPropMap ();
            }
        }
    else
        {
        if (type == EndType::From)
            {
            m_ecid = map.GetTargetECInstanceIdPropMap ();
            m_classId = map.GetTargetECClassIdPropMap ();
            }
        else{
            m_ecid = map.GetSourceECInstanceIdPropMap ();
            m_classId = map.GetSourceECClassIdPropMap ();
            }
        }
    }
std::set<ECDbMapAnalyser::Class*>& ECDbMapAnalyser::Relationship::EndPoint::GetClassesR (){ return m_classes; }
std::set<ECDbMapAnalyser::Storage const*> ECDbMapAnalyser::Relationship::EndPoint::GetStorages () const
    {
    std::set<Storage const*> storages;
    for (auto cl : m_classes)
        {
        for (auto& s1 : cl->GetPartitionsR ())
            if (!s1.first->IsVirtual ())
                storages.insert (s1.first);
        }
    return std::move (storages);
    }

PropertyMapCP ECDbMapAnalyser::Relationship::EndPoint::GetInstanceId () const { return m_ecid; }
PropertyMapCP ECDbMapAnalyser::Relationship::EndPoint::GetClassId () const { return m_classId; }
ECDbMapAnalyser::Relationship::EndType ECDbMapAnalyser::Relationship::EndPoint::GetEnd () const { return m_type; }
       
ECDbMapAnalyser::Relationship::Relationship (RelationshipClassMapCR classMap, Storage& storage, Class* parent)
    :Class (classMap, storage, parent), m_from (classMap, EndType::From), m_to (classMap, EndType::To), m_onDeleteAction (ECDbSqlForeignKeyConstraint::ActionType::NotSpecified), m_onUpdateAction (ECDbSqlForeignKeyConstraint::ActionType::NotSpecified)
    {
    ECDbForeignKeyRelationshipMap foreignKeyRelMap;
    ECDbMapCustomAttributeHelper::TryGetForeignKeyRelationshipMap (foreignKeyRelMap, GetRelationshipClassMap ().GetRelationshipClass ());
    bool createConstraint = false;
    if (ECOBJECTS_STATUS_Success != foreignKeyRelMap.TryGetCreateConstraint (createConstraint))
        return;

    if (createConstraint)
        {
        Utf8String onDeleteActionStr;
        if (ECOBJECTS_STATUS_Success != foreignKeyRelMap.TryGetOnDeleteAction (onDeleteActionStr))
            return;

        Utf8String onUpdateActionStr;
        if (ECOBJECTS_STATUS_Success != foreignKeyRelMap.TryGetOnUpdateAction (onUpdateActionStr))
            return;

        m_onDeleteAction = ECDbSqlForeignKeyConstraint::ToActionType (onDeleteActionStr.c_str ());
        m_onUpdateAction = ECDbSqlForeignKeyConstraint::ToActionType (onUpdateActionStr.c_str ());
        }
    }
RelationshipClassMapCR ECDbMapAnalyser::Relationship::GetRelationshipClassMap () const
    { 
    return static_cast<RelationshipClassMapCR>(GetClassMap ()); 
    }
ECDbMapAnalyser::Relationship::PersistanceLocation ECDbMapAnalyser::Relationship::GetPersistanceLocation () const
    {
    if (IsLinkTable ())
        return PersistanceLocation::Self;

    auto& endTable = static_cast<RelationshipClassEndTableMapCR>(GetRelationshipClassMap ());
    if (endTable.GetThisEnd () == ECN::ECRelationshipEnd::ECRelationshipEnd_Source)
        {
        if (endTable.GetRelationshipClass ().GetStrengthDirection () == ECN::ECRelatedInstanceDirection::Forward)
            return PersistanceLocation::From;
        else
            return PersistanceLocation::To;
        }
    else
        {
        if (endTable.GetRelationshipClass ().GetStrengthDirection () == ECN::ECRelatedInstanceDirection::Forward)
            return PersistanceLocation::To;
        else
            return PersistanceLocation::From;
        }
    }
bool ECDbMapAnalyser::Relationship::RequireCascade () const
    {
    return GetRelationshipClassMap ().GetRelationshipClass ().GetStrength () != StrengthType::STRENGTHTYPE_Referencing;
    }
bool ECDbMapAnalyser::Relationship::IsLinkTable () const
    {
    return GetRelationshipClassMap ().GetClassMapType () == IClassMap::Type::RelationshipLinkTable;
    }
ECDbMapAnalyser::Relationship::EndPoint& ECDbMapAnalyser::Relationship::From () { return m_from; }
ECDbMapAnalyser::Relationship::EndPoint& ECDbMapAnalyser::Relationship::To () { return m_to; }


ECDbMapAnalyser::Storage& ECDbMapAnalyser::GetStorage (Utf8CP tableName)
    {
    auto itor = m_storage.find (tableName);
    if (itor != m_storage.end ())
        return *(itor->second);

    auto table = m_map.GetSQLManager ().GetDbSchema ().FindTable (tableName);
    BeAssert (table != nullptr);
    auto ptr = Storage::Ptr (new Storage (*table));
    auto p = ptr.get ();
    m_storage[table->GetName ().c_str ()] = std::move (ptr);
    return *p;
    }
ECDbMapAnalyser::Storage& ECDbMapAnalyser::GetStorage (ClassMapCR classMap)
    {
    return GetStorage (classMap.GetTable ().GetName ().c_str ());
    }
ECDbMapAnalyser::Class& ECDbMapAnalyser::GetClass (ClassMapCR classMap)
    {
    if (classMap.GetClass ().GetRelationshipClassCP () != nullptr)
        {
        return GetRelationship (static_cast<RelationshipClassMapCR>(classMap));
        }

    auto itor = m_classes.find (classMap.GetClass ().GetId ());
    if (itor != m_classes.end ())
        return *(itor->second);

    Storage& storage = GetStorage (classMap);
    if (classMap.GetClass ().GetIsStruct ())
        m_classes[classMap.GetClass ().GetId ()] = Struct::Ptr (new Struct (classMap, storage, nullptr));
    else
        m_classes[classMap.GetClass ().GetId ()] = Class::Ptr (new Class (classMap, storage, nullptr));

    auto ptr = m_classes[classMap.GetClass ().GetId ()].get ();
    if (classMap.GetParentMapClassId () != 0LL)
        {
        ptr->SetParent(GetClass (*GetClassMap (classMap.GetParentMapClassId ())));
        }

    storage.GetClassesR ().insert (ptr);

    for (auto& part : classMap.GetStorageDescription ().GetHorizontalPartitions ())
        {
        auto& storage = GetStorage (part.GetTable ().GetName ().c_str ());
        for (auto id : part.GetClassIds ())
            {
            auto refClassMap = GetClassMap (id);
            BeAssert (refClassMap != nullptr);
            auto classM = &(GetClass (*refClassMap));
            BeAssert (classM != nullptr);
            ptr->GetPartitionsR ()[&storage].insert (classM);
            }
        }


    return *ptr;
    }
ECDbMapAnalyser::Relationship&  ECDbMapAnalyser::GetRelationship (RelationshipClassMapCR classMap)
    {
    auto itor = m_relationships.find (classMap.GetClass ().GetId ());
    if (itor != m_relationships.end ())
        return *(itor->second.get ());

    Storage& storage = GetStorage (classMap);
    m_relationships[classMap.GetClass ().GetId ()] = Relationship::Ptr (new Relationship (classMap, GetStorage (classMap), nullptr));
    auto ptr = m_relationships[classMap.GetClass ().GetId ()].get ();
    if (classMap.GetParentMapClassId () != 0LL)
        {
        ptr->SetParent (GetClass (*GetClassMap (classMap.GetParentMapClassId ())));
        }


    for (auto& part : classMap.GetStorageDescription ().GetHorizontalPartitions ())
        {
        auto& storage = GetStorage (part.GetTable ().GetName ().c_str ());
        for (auto id : part.GetClassIds ())
            {
            ptr->GetPartitionsR ()[&storage].insert (&GetClass (*GetClassMap (id)));
            }
        }
    storage.GetRelationshipsR ().insert (ptr);
    auto isForward = classMap.GetRelationshipClass ().GetStrengthDirection () == ECRelatedInstanceDirection::Forward;
    for (auto& key : m_map.GetLightweightCache ().GetRelationships (classMap.GetClass ().GetId ()))
        {
        auto& constraitClass = GetClass (*GetClassMap (key.first));
        if (Enum::In (ECDbMap::LightweightCache::RelationshipEnd::Source, key.second))
            {
            if (isForward)
                ptr->From ().GetClassesR ().insert (&constraitClass);
            else
                ptr->To ().GetClassesR ().insert (&constraitClass);
            }
        if (Enum::In (ECDbMap::LightweightCache::RelationshipEnd::Target, key.second))
            {
            if (!isForward)
                ptr->From ().GetClassesR ().insert (&constraitClass);
            else
                ptr->To ().GetClassesR ().insert (&constraitClass);
            }
        }

    for (auto from : ptr->From ().GetStorages ())
        {
        for (auto to : ptr->To ().GetStorages ())
            {
            switch (ptr->GetPersistanceLocation ())
                {
                case Relationship::PersistanceLocation::From:
                    const_cast<Storage *>(to)->CascadesTo ()[const_cast<Storage*>(from)].insert (ptr); break;
                case Relationship::PersistanceLocation::To:
                    const_cast<Storage *>(from)->CascadesTo ()[const_cast<Storage*>(to)].insert (ptr); break;
                case Relationship::PersistanceLocation::Self:
                    const_cast<Storage *>(from)->CascadesTo ()[const_cast<Storage*>(to)].insert (ptr); break;
                }
            }
        }
    return *ptr;
    }
BentleyStatus ECDbMapAnalyser::AnalyseClass (ClassMapCR ecClassMap)
    {
    auto& ptr = GetClass (ecClassMap);
    if (!ptr.InQueue ())
        {
        return BentleyStatus::SUCCESS;
        }

    AnalyseStruct (ptr);
    ptr.Done (); //mark it as done
    for (auto derivedClassId : GetDerivedClassIds (ecClassMap.GetClass ().GetId ()))
        {
        if (AnalyseClass (*GetClassMap (derivedClassId)) != BentleyStatus::SUCCESS)
            return BentleyStatus::ERROR;
        }
    return BentleyStatus::SUCCESS;
    }

void ECDbMapAnalyser::AnalyseStruct (Class& classInfo)
    {
    std::map<IClassMap const*, std::vector<PropertyMapToTableCP>> structPropertyMaps;
    classInfo.GetClassMap ().GetPropertyMaps ().Traverse ([&] (TraversalFeedback& feedback, PropertyMapCP propertyMap)
        {
        if (auto mapToTable = dynamic_cast<PropertyMapToTableCP>(propertyMap))
            {
            if (auto associatedClasMap = m_map.GetClassMap (mapToTable->GetElementType ()))
                {
                if (associatedClasMap->GetTable ().GetPersistenceType () == PersistenceType::Persisted)
                    {
                    structPropertyMaps[associatedClasMap].push_back (mapToTable);
                    }
                }
            }
        feedback = TraversalFeedback::Next;
        }, true);

    for (auto& key : structPropertyMaps)
        {
        auto& elementType = static_cast<Struct&>(GetClass (static_cast<ClassMap const&>(*key.first)));
        classInfo.GetStorageR ().StructCascadeTo ().insert (&elementType);
        }
    }
BentleyStatus ECDbMapAnalyser::AnalyseRelationshipClass (RelationshipClassMapCR ecRelationshipClassMap)
    {
    auto& ptr = GetRelationship (ecRelationshipClassMap);
    if (!ptr.InQueue ())
        return BentleyStatus::SUCCESS;


    ptr.Done ();
    return BentleyStatus::SUCCESS;
    }
const std::vector<ECN::ECClassId> ECDbMapAnalyser::GetRootClassIds () const
    {
    Utf8CP sql0 =
        "SELECT C.Id"
        "	FROM ec_Class C "
        "   	INNER JOIN ec_ClassMap M ON M.ClassId = C.Id "
        "       LEFT JOIN ec_BaseClass B ON B.ClassId = C.Id "
        "	WHERE B.BaseClassId IS NULL And C.IsRelationship = 0";

    std::vector<ECN::ECClassId> classIds;
    Statement stmt;
    stmt.Prepare (GetMap ().GetECDbR (), sql0);
    while (stmt.Step () == BE_SQLITE_ROW)
        classIds.push_back (stmt.GetValueInt64 (0));

    return std::move (classIds);
    }
const std::vector<ECN::ECClassId> ECDbMapAnalyser::GetRelationshipClassIds () const
    {
    Utf8CP sql0 =
        "SELECT C.Id"
        "	FROM ec_Class C "
        "   	INNER JOIN ec_ClassMap M ON M.ClassId = C.Id "
        "       LEFT JOIN ec_BaseClass B ON B.ClassId = C.Id "
        "	WHERE B.BaseClassId IS NULL And C.IsRelationship = 1";

    std::vector<ECN::ECClassId> classIds;
    Statement stmt;
    stmt.Prepare (GetMap ().GetECDbR (), sql0);
    while (stmt.Step () == BE_SQLITE_ROW)
        classIds.push_back (stmt.GetValueInt64 (0));

    return std::move (classIds);
    }
std::set<ECN::ECClassId> const& ECDbMapAnalyser::GetDerivedClassIds (ECN::ECClassId baseClassId) const
    {
    return m_derivedClassLookup[baseClassId];
    }
ClassMapCP ECDbMapAnalyser::GetClassMap (ECN::ECClassId classId) const
    {
    auto classMap = GetMap ().GetClassMapCP (classId);
    BeAssert (classMap != nullptr && "ClassMap not found");
    return classMap;
    }
void ECDbMapAnalyser::SetupDerivedClassLookup ()
    {
    m_derivedClassLookup.clear ();
    Utf8CP sql0 =
        "SELECT  BaseClassId, ClassId FROM ec_BaseClass ORDER BY BaseClassId";

    Statement stmt;
    m_derivedClassLookup.clear ();
    stmt.Prepare (GetMap ().GetECDbR (), sql0);
    while (stmt.Step () == BE_SQLITE_ROW)
        m_derivedClassLookup[stmt.GetValueInt64 (0)].insert (stmt.GetValueInt64 (1));
    }
ECDbMapAnalyser::ECDbMapAnalyser (ECDbMapR ecdbMap)
    :m_map (ecdbMap)
    {
    }
ECDbMapAnalyser::ViewInfo* ECDbMapAnalyser::GetViewInfoForClass (Class const& nclass)
    {
    auto itor = m_viewInfos.find (&nclass);
    if (itor == m_viewInfos.end ())
        return nullptr;

    return &(itor->second);
    }

const NativeSqlBuilder ECDbMapAnalyser::GetClassFilter (std::pair<ECDbMapAnalyser::Storage const*, std::set<ECDbMapAnalyser::Class const*>> const& partition)
    {
    auto storage = partition.first;
    auto& classes = partition.second;
    std::set<ECN::ECClassId> classSet;
    std::set<ECN::ECClassId> classSubset;
    std::set<ECN::ECClassId> classSubsetNotIn;
    NativeSqlBuilder sql;
    for (auto c : storage->GetClasses ())
        classSet.insert (c->GetClassMap ().GetClass ().GetId ());

    for (auto c : classes)
        {
        BeAssert (classSet.find (c->GetClassMap ().GetClass ().GetId ()) != classSet.end ());
        classSubset.insert (c->GetClassMap ().GetClass ().GetId ());
        }

    for (auto c : classSet)
        {
        if (classSubset.find (c) == classSubset.end ())
            classSubsetNotIn.insert (c);
        }


    if (classSubset.size () <= classSubsetNotIn.size () || classSubsetNotIn.empty())
        {
        sql.Append ("IN (");
        for (auto id : classSubset)
            {
            sql.Append (id);
            if (id != *(classSubset.rbegin ()))
                sql.Append (",");
            }
        sql.Append (")");
        }
    else
        {
        sql.Append (" NOT IN (");
        for (auto id : classSubsetNotIn)
            {
            sql.Append (id);
            if (id != *(classSubsetNotIn.rbegin ()))
                sql.Append (",");
            }
        sql.Append (")");
        }

    return std::move (sql);
    }
BentleyStatus ECDbMapAnalyser::BuildPolymorphicDeleteTrigger (Class& nclass)
    {
    BeAssert (nclass.RequireView ());
    auto viewInfo = GetViewInfoForClass (nclass);
    BeAssert (viewInfo != nullptr && !viewInfo->GetView ().IsEmpty ());
    auto p = nclass.GetStorage ().GetTable ().GetFilteredColumnFirst (ECDbKnownColumns::ECInstanceId);
    auto c = nclass.GetStorage ().GetTable ().GetFilteredColumnFirst (ECDbKnownColumns::ECClassId);
    for (auto & i : nclass.GetPartitionsR ())
        {
        auto storage = i.first;
        if (storage->IsVirtual ())
            continue;
        
        SqlTriggerBuilder& builder = viewInfo->GetTriggersR ().Create (SqlTriggerBuilder::Type::Delete, SqlTriggerBuilder::Condition::InsteadOf, false);
        builder.GetOnBuilder ().Append (viewInfo->GetViewR ().GetName ());
        builder.GetNameBuilder ().Append (nclass.GetSqlName ()).Append ("_").Append (storage->GetTable ().GetName ().c_str()).Append ("_Delete");
        NativeSqlBuilder classFilter = GetClassFilter (i);
        if (!classFilter.IsEmpty ())
            {
            if (c == nullptr)
                builder.GetWhenBuilder().Append("OLD.").Append ("ECClassId ").Append (classFilter);
            else
                builder.GetWhenBuilder ().Append ("OLD.").Append (c->GetName ().c_str ()).AppendSpace ().Append (classFilter);
            }

        auto& body = builder.GetBodyBuilder ();
        auto f = storage->GetTable ().GetFilteredColumnFirst (ECDbKnownColumns::ECInstanceId);
        body.Append ("DELETE FROM ").AppendEscaped (storage->GetTable ().GetName ().c_str ());
        body.AppendFormatted (" WHERE OLD.[%s] = [%s]", p->GetName ().c_str (), f->GetName ().c_str ());
        body.Append (";").AppendEOL ();
        }

    return BentleyStatus::SUCCESS;
    }

BentleyStatus ECDbMapAnalyser::BuildPolymorphicUpdateTrigger (Class& nclass)
    {
    BeAssert (nclass.RequireView ());
    BeAssert (nclass.RequireView ());
    auto viewInfo = GetViewInfoForClass (nclass);
    BeAssert (viewInfo != nullptr && !viewInfo->GetView ().IsEmpty ());

    auto rootPMS = PropertyMapSet::Create (nclass.GetClassMap ());
    auto rootEndPoints = rootPMS->GetEndPoints ();
    auto p = nclass.GetStorage ().GetTable ().GetFilteredColumnFirst (ECDbKnownColumns::ECInstanceId);
    auto c = nclass.GetStorage ().GetTable ().GetFilteredColumnFirst (ECDbKnownColumns::ECClassId);

    for (auto & i : nclass.GetPartitionsR ())
        {
        auto storage = i.first;
        if (storage->IsVirtual ())
            continue;

        SqlTriggerBuilder& builder = viewInfo->GetTriggersR().Create (SqlTriggerBuilder::Type::Update, SqlTriggerBuilder::Condition::InsteadOf, false);
        builder.GetOnBuilder ().Append (viewInfo->GetViewR ().GetName ());
        builder.GetNameBuilder ().Append (nclass.GetSqlName ()).Append ("_").Append ("Update");
        NativeSqlBuilder classFilter = GetClassFilter (i);
        if (!classFilter.IsEmpty ())
            {
            if (c == nullptr)
                builder.GetWhenBuilder ().Append ("OLD.").Append ("ECClassId ").Append (classFilter);
            else
                builder.GetWhenBuilder ().Append ("OLD.").Append (c->GetName ().c_str ()).AppendSpace ().Append (classFilter);
            }

        auto& body = builder.GetBodyBuilder ();
        auto& firstClass = **(i.second.begin());
        auto f = storage->GetTable ().GetFilteredColumnFirst (ECDbKnownColumns::ECInstanceId);
        auto childPMS = PropertyMapSet::Create (firstClass.GetClassMap());
        body.Append ("UPDATE ").AppendEscaped (storage->GetTable ().GetName ().c_str ());
        body.Append ("SET ");
        int nColumns = 0;

        for (auto const rootE : rootEndPoints)
            {
            auto childE = childPMS->GetEndPointByAccessString (rootE->GetAccessString ().c_str ());
            if (rootE->GetKnownColumnId () != ECDbKnownColumns::DataColumn)
                continue;

            if (childE->GetValue ().IsNull ())
                {
                body.AppendFormatted ("[%s] = NEW.[%s]",childE->GetColumn ()->GetName ().c_str (), rootE->GetColumn()->GetName().c_str());
                }

            if (rootE != rootEndPoints.back ())
                body.Append (", ");

            nColumns++;
            }

        if (nColumns == 0)
            {
            viewInfo->GetTriggersR ().Delete (builder);
            return BentleyStatus::SUCCESS;
            }

        body.AppendFormatted (" WHERE OLD.[%s] = [%s]", p->GetName ().c_str (), f->GetName ().c_str ());
        body.Append (";").AppendEOL ();
        }
    return BentleyStatus::SUCCESS;

    }

SqlViewBuilder ECDbMapAnalyser::BuildView (Class& nclass)
    {
    //BeAssert (nclass.GetType ().)
    auto classMap = &nclass.GetClassMap ();
    auto rootPMS = PropertyMapSet::Create (*classMap);
    auto const& storageDescription = classMap->GetStorageDescription ();

    SqlViewBuilder builder;
    builder.GetNameBuilder ()
        .Append ("_")
        .Append (classMap->GetClass ().GetSchema ().GetNamespacePrefix ().c_str ())
        .Append ("_")
        .Append (classMap->GetClass ().GetName ().c_str ());

    NativeSqlBuilder::List selects;
    HorizontalPartition const* root = &storageDescription.GetRootHorizontalPartition();
    if (root->GetTable ().GetPersistenceType () == PersistenceType::Virtual)
        root = nullptr;

    bool bFirst = true;
    for (auto const& hp : storageDescription.GetHorizontalPartitions ())
        {
        if (hp.GetTable ().GetPersistenceType () == PersistenceType::Virtual)
            continue;
        
        NativeSqlBuilder select;
        select.Append ("SELECT ");
        auto firstChildMap = m_map.GetClassMapCP (hp.GetClassIds ().front ());
        auto childPMS = PropertyMapSet::Create (*firstChildMap);
        auto rootEndPoints = rootPMS->GetEndPoints ();
        for (auto const rootE : rootEndPoints)
            {
            auto childE = childPMS->GetEndPointByAccessString (rootE->GetAccessString ().c_str ());
            if (childE->GetValue ().IsNull ())
                {
                select.AppendEscaped (childE->GetColumn ()->GetName ().c_str ());
                if (bFirst && root == nullptr)
                    { 
                    bFirst = false;
                    select.AppendSpace().AppendEscaped (rootE->GetColumn ()->GetName ().c_str ());
                    }
                }
            else
                {
                if (rootE->GetColumn () != nullptr)
                    select.Append (Utf8PrintfString ("%lld [%s]", childE->GetValue ().GetLong (), rootE->GetColumn ()->GetName ().c_str ()));
                else
                    select.Append (Utf8PrintfString ("%lld [%s]", childE->GetValue ().GetLong (), (rootE->GetAccessString ().c_str ())));
                }

            if (rootE != rootEndPoints.back ())
                select.Append (", ");
            }

        select.Append (" FROM ").AppendEscaped (firstChildMap->GetTable ().GetName ().c_str ());
        if (auto classIdColumn = hp.GetTable ().GetFilteredColumnFirst (ECDbKnownColumns::ECClassId))
            {
            if (classIdColumn->GetPersistenceType () == PersistenceType::Persisted && hp.NeedsClassIdFilter ())
                {
                select.Append (" WHERE ");
                select.AppendEscaped (classIdColumn->GetName ().c_str ());
                hp.AppendECClassIdFilterSql (select);
                }
            }

        if (&hp == root)
            selects.insert (selects.begin (), std::move (select));
        else
            selects.push_back (std::move (select));
        }

    if (!selects.empty ())
        {
        for (auto const& select : selects)
            {
            builder.AddSelect ().Append (select);
            }
        }
    else
        {
        auto& select = builder.AddSelect ();
        select.Append ("SELECT ");
        auto rootEndPoints = rootPMS->GetEndPoints ();
        for (auto const rootE : rootEndPoints)
            {
            select.Append ("NULL ");
            select.AppendEscaped (rootE->GetColumn ()->GetName ().c_str ());
            if (rootE != rootEndPoints.back ())
                select.Append (", ");
            }

        select.Append (" LIMIT 0");
        builder.MarkAsNullView ();
        }

    return std::move (builder);
    }

DbResult ECDbMapAnalyser::ExecuteDDL (Utf8CP sql)
    {
    auto r = m_map.GetECDbR ().ExecuteSql (sql);
    if (r != BE_SQLITE_OK)
        {
        BeAssert (false && "Failed to run DDL statement");
        LOG.errorv ("Failed to execute DLL statement during mapping - %s", sql);
        }
    return r;
    }

DbResult ECDbMapAnalyser::ApplyChanges ()
    {
    Utf8String sql;
    DbResult r = UpdateHoldingView ();
    if (r != BE_SQLITE_OK)
        return r;

    for (auto& i : m_viewInfos)
        {
        ViewInfo const& viewInfo = i.second;
        if (viewInfo.GetView ().IsEmpty ())
            {
            BeAssert (false && "must have a view");
            continue;
            }

        sql = viewInfo.GetView ().ToString (SqlOption::DropIfExists, true);
        r = ExecuteDDL (sql.c_str ());
        if (r != BE_SQLITE_OK)
            return r;

        sql = viewInfo.GetView ().ToString (SqlOption::Create, true);
        r = ExecuteDDL (sql.c_str ());
        if (r != BE_SQLITE_OK)
            return r;

        for (auto & trigger : viewInfo.GetTriggers ().GetTriggers ())
            {
            if (&trigger == nullptr)
                {
                printf ("");
                }
            if (!trigger.IsEmpty ())
                {
                sql = trigger.ToString (SqlOption::DropIfExists, true);
                r = ExecuteDDL (sql.c_str ());
                if (r != BE_SQLITE_OK)
                    return r;

                sql = trigger.ToString (SqlOption::Create, true);
                r = ExecuteDDL (sql.c_str ());
                if (r != BE_SQLITE_OK)
                    return r;
                }
            }

        }

    for (auto& i : m_storage)
        {
        auto& storage = *i.second;
        if (storage.GetTable ().GetOwnerType () == OwnerType::ExistingTable)
            {
            continue;
            }

        for (auto const& trigger : storage.GetTriggerList ().GetTriggers())
            {
            if (storage.IsVirtual ())
                continue;

            BeAssert (trigger.IsEmpty () == false);

            sql = trigger.ToString (SqlOption::DropIfExists, true);
            r = ExecuteDDL (sql.c_str ());
            if (r != BE_SQLITE_OK)
                return r;

            sql = trigger.ToString (SqlOption::Create, true);
            r = ExecuteDDL (sql.c_str ());
            if (r != BE_SQLITE_OK)
                return r;
            }
        }

    return r;
    }
BentleyStatus ECDbMapAnalyser::Analyser (bool applyChanges)
    {
    m_classes.clear ();
    m_derivedClassLookup.clear ();
    m_relationships.clear ();
    m_storage.clear ();
    m_viewInfos.clear ();

    SetupDerivedClassLookup ();
    for (auto rootClassId : GetRootClassIds ())
        {
        if (AnalyseClass (*GetClassMap (rootClassId)) != BentleyStatus::SUCCESS)
            return BentleyStatus::ERROR;
        }

    for (auto rootClassId : GetRelationshipClassIds ())
        {
        if (AnalyseRelationshipClass (static_cast<RelationshipClassMapCR> (*GetClassMap (rootClassId))) != BentleyStatus::SUCCESS)
            return BentleyStatus::ERROR;
        }
    

    for (auto& i : m_classes)
        {
        auto ecclass = i.second.get ();
        if (!ecclass->RequireView ())
            continue;

        auto& viewInfo = m_viewInfos[ecclass];
        viewInfo.GetViewR () = std::move (BuildView (*ecclass));
        BuildPolymorphicDeleteTrigger (*ecclass);
        BuildPolymorphicUpdateTrigger (*ecclass);
        }

    ProcessEndTableRelationships ();
    ProcessLinkTableRelationships ();
    for (auto& storage : m_storage)
        {
        storage.second->Generate ();
        }

    if (applyChanges)
        return ApplyChanges () == DbResult::BE_SQLITE_OK ? BentleyStatus::SUCCESS : BentleyStatus::ERROR;

    return BentleyStatus::SUCCESS;
    }


ECDbMapAnalyser::Storage::Storage (ECDbSqlTable const& table)
        :m_table (table)
        {}

ECDbSqlTable const& ECDbMapAnalyser::Storage::GetTable () const { return m_table; }
bool ECDbMapAnalyser::Storage::IsVirtual () const { return m_table.GetPersistenceType () == PersistenceType::Virtual; }
std::set<ECDbMapAnalyser::Class*> & ECDbMapAnalyser::Storage::GetClassesR ()  { return m_classes; }
std::set<ECDbMapAnalyser::Relationship*> & ECDbMapAnalyser::Storage::GetRelationshipsR ()  { return m_relationships; }
std::map<ECDbMapAnalyser::Storage*, std::set<ECDbMapAnalyser::Relationship*>> & ECDbMapAnalyser::Storage::CascadesTo ()  { return m_cascades; } //OnDelete_tableA

SqlTriggerBuilder::TriggerList& ECDbMapAnalyser::Storage::GetTriggerListR () { return m_triggers; }
SqlTriggerBuilder::TriggerList const& ECDbMapAnalyser::Storage::GetTriggerList () const
    {
    return m_triggers;
    }


DbResult ECDbMapAnalyser::UpdateHoldingView ()
    {
    NativeSqlBuilder viewSql;
    Utf8CP sql = "SELECT Id FROM ec_Class WHERE ec_Class.RelationStrength = 1"; // Holding relationships       
    NativeSqlBuilder::List unionList;
    auto stmt = m_map.GetECDbR ().GetCachedStatement (sql);
    if (!stmt.IsValid ())
        {
        BeAssert (false && "Failed to prepared statement");
        return DbResult::BE_SQLITE_ERROR;
        }
    std::map<ECDbSqlTable const*, ECDbMap::LightweightCache::RelationshipEnd> doneSet;
    while (stmt->Step () == BE_SQLITE_ROW)
        {
        ECClassId ecClassId = stmt->GetValueInt64 (0);
        auto holdingRelationshipClass = m_map.GetECDbR ().Schemas ().GetECClass (ecClassId);
        if (holdingRelationshipClass == nullptr)
            {
            BeAssert (false && "Fail to find class for holding relationship");
            return DbResult::BE_SQLITE_ERROR;
            }

        auto holdingRelationshipClassMap = static_cast<RelationshipClassMapCP>(m_map.GetClassMap (*holdingRelationshipClass));
        if (holdingRelationshipClassMap == nullptr || holdingRelationshipClassMap->GetTable ().GetPersistenceType () == PersistenceType::Virtual)
            continue;


        Utf8CP column;
        ECDbMap::LightweightCache::RelationshipEnd filter;
        if (holdingRelationshipClassMap->GetRelationshipClass ().GetStrengthDirection () == ECRelatedInstanceDirection::Forward)
            {
            column = holdingRelationshipClassMap->GetTargetECInstanceIdPropMap ()->GetFirstColumn ()->GetName ().c_str ();
            filter = ECDbMap::LightweightCache::RelationshipEnd::Source;
            }
        else
            {
            column = holdingRelationshipClassMap->GetSourceECInstanceIdPropMap ()->GetFirstColumn ()->GetName ().c_str ();
            filter = ECDbMap::LightweightCache::RelationshipEnd::Target;
            }

        auto table = &holdingRelationshipClassMap->GetTable ();
        auto itor = doneSet.find (table);
        if (itor == doneSet.end () || (((int)(itor->second) & (int)filter) == 0))
            {
            NativeSqlBuilder relaitonshipView;
            relaitonshipView.Append ("SELECT ");
            relaitonshipView.Append (column);
            relaitonshipView.Append (" ECInstanceId FROM ").Append (table->GetName ().c_str ());
            //relaitonshipView.Append (" WHERE (").Append (column).Append(" IS NOT NULL)").AppendEOL();
            unionList.push_back (std::move (relaitonshipView));
            if (itor == doneSet.end ())
                doneSet[table] = filter;
            else
                doneSet[table] = static_cast<ECDbMap::LightweightCache::RelationshipEnd>((int)(itor->second) & (int)(filter));
            }
        }
    viewSql.Append ("DROP VIEW IF EXISTS ").Append (ECDB_HOLDING_VIEW).Append (";").AppendEOL ();
    viewSql.Append ("CREATE VIEW ").Append (ECDB_HOLDING_VIEW).Append (" AS ").AppendEOL ();
    if (!unionList.empty ())
        {

        for (auto& select : unionList)
            {
            viewSql.Append (select);
            if (&select != &(unionList.back ()))
                viewSql.Append (" \r\n UNION ALL \r\n");
            }
        viewSql.Append (";\n");
        }
    else
        {
        viewSql.Append ("SELECT NULL ECInstanceId LIMIT 0;");
        }

    return ExecuteDDL (viewSql.ToString ());
    }

void ECDbMapAnalyser::HandleLinkTable (Storage* fromStorage, std::map<ECDbMapAnalyser::Storage*, std::set<ECDbMapAnalyser::Relationship*>> const& relationshipsByStorage, bool isFrom)
    {
    // table_delete_linkTable
    for (auto& i : relationshipsByStorage)
        {
        auto relationshipStorage = i.first;
        auto& relationships = i.second;
        if (relationshipStorage->IsVirtual ())
            continue;

        //Determine which source/target or both side to delete
        std::set<ECDbSqlColumn const*> forignKeys;
        for (auto relationship : relationships)
            {
            if (isFrom)
                forignKeys.insert (relationship->From ().GetInstanceId ()->GetFirstColumn ());
            else
                forignKeys.insert (relationship->To ().GetInstanceId ()->GetFirstColumn ());
            }

        auto& builder = fromStorage->GetTriggerListR ().Create (SqlTriggerBuilder::Type::Delete, SqlTriggerBuilder::Condition::After, false);
        builder.GetNameBuilder ()
            .Append (fromStorage->GetTable ().GetName ().c_str ())
            .Append ("_")
            .Append (relationshipStorage->GetTable ().GetName ().c_str ())
            .AppendIIf (isFrom, "_From", "_To")
            .Append ("_Delete");
        

        builder.GetOnBuilder ().Append (fromStorage->GetTable ().GetName ().c_str ());
        auto& body = builder.GetBodyBuilder ();
        for (auto relationship : relationships)
            {
            body.Append ("--4 ").Append (relationship->GetRelationshipClassMap ().GetRelationshipClass ().GetFullName ()).AppendEOL ();
            }

        body
            .Append ("DELETE FROM ")
            .AppendEscaped (relationshipStorage->GetTable ().GetName ().c_str ())
            .Append (" WHERE ");

        auto thisECInstanceIdColumn = fromStorage->GetTable ().GetFilteredColumnFirst (ECDbKnownColumns::ECInstanceId);
        for (auto forignKey : forignKeys)
            {
            body.AppendFormatted ("(OLD.[%s] = [%s])", thisECInstanceIdColumn->GetName ().c_str (), forignKey->GetName ().c_str ());
            if (forignKey != *forignKeys.rbegin ())
                {
                body.Append (" OR ");
                }
            }
        body.Append (";").AppendEOL ();
        }
    }

void ECDbMapAnalyser::ProcessLinkTableRelationships ()
    {
    //       lhs/from                rel table         rel
    std::map<Storage*, std::map<Storage*, std::set<Relationship*>>> lhs2rel;
    //       rhs/to                 rel table          rel
    std::map<Storage*, std::map<Storage*, std::set<Relationship*>>> rhs2rel;

    for (auto& i : m_relationships)
        {
        auto relationship = i.second.get ();
        if (!relationship->IsLinkTable ())
            continue;

        for (auto from : relationship->From ().GetStorages ())
            for (auto to : relationship->To ().GetStorages ())
                {
                lhs2rel[const_cast<Storage*>(from)][const_cast<Storage*>(&relationship->GetStorage ())].insert (relationship);
                rhs2rel[const_cast<Storage*>(to)][const_cast<Storage*>(&relationship->GetStorage ())].insert (relationship);
                }
        }

    for (auto& i : lhs2rel)
        {
        auto lhsStorage = i.first;
        auto& relByStorage = i.second;
        HandleLinkTable (lhsStorage, relByStorage, true);
        }
    for (auto& i : rhs2rel)
        {
        auto rhsStorage = i.first;
        auto& relByStorage = i.second;
        HandleLinkTable (rhsStorage, relByStorage, false);
        }
    }
void ECDbMapAnalyser::ProcessEndTableRelationships ()
    {
    // DELETE FROM to WHERE to.Id = from.Key
    // FROM            TO
    //  [A] -> AHasB -> B
    //  
    //  DELETE B / UPDATE A.K = NULL
    //  A -> AHasB -> [B]
    //                
    //  DELETE B 
    // relationship UPDATE ....
    for (auto& i : m_relationships)
        {
        auto relationship = i.second.get ();
        if (relationship->IsLinkTable ())
            continue;

        if (relationship->IsMarkedForCascadeDelete ())
            continue;

        bool isSelfRelationship = false;
        auto const lhsStorages = relationship->From ().GetStorages ();
        auto const rhsStorages = relationship->To ().GetStorages ();
        if (lhsStorages.size () == 1 && rhsStorages.size () == 1)
            {
            isSelfRelationship = *lhsStorages.begin () == *rhsStorages.begin ();
            }

        bool persistedInFrom = &relationship->From () == &relationship->ForeignEnd ();

        if (persistedInFrom)
            {
            for (auto toStorage : relationship->To ().GetStorages ())
                {
                auto& builder = const_cast<Storage*>(toStorage)->GetTriggerListR ().Create (SqlTriggerBuilder::Type::Delete, SqlTriggerBuilder::Condition::After, false);
                builder.GetNameBuilder ()
                    .Append (toStorage->GetTable ().GetName ().c_str ())
                    .Append ("_")
                    .Append (relationship->GetSqlName ())
                    .Append ("_Delete");

                builder.GetOnBuilder ().Append (toStorage->GetTable ().GetName ().c_str ());
                auto& body = builder.GetBodyBuilder ();
                body.Append ("--1").Append (relationship->GetRelationshipClassMap ().GetRelationshipClass ().GetFullName ()).AppendEOL ();
                for (auto fromStorage : relationship->From ().GetStorages ())
                    {
                    body.Append ("UPDATE ")
                        .AppendEscaped (fromStorage->GetTable ().GetName ().c_str ())
                        .Append (" SET ")
                        .AppendEscaped (relationship->To ().GetInstanceId ()->GetFirstColumn ()->GetName ().c_str ())
                        .Append (" = NULL");

                    if (!relationship->To ().GetClassId ()->IsVirtual () && relationship->To ().GetClassId ()->IsMappedToPrimaryTable ())
                        {
                        body.Append (", ")
                            .AppendEscaped (relationship->To ().GetClassId ()->GetFirstColumn ()->GetName ().c_str ())
                            .Append (" = NULL");
                        }

                    body.Append (" WHERE OLD.")
                        .AppendEscaped (relationship->From ().GetInstanceId ()->GetFirstColumn ()->GetName ().c_str ())
                        .Append (" = ")
                        .AppendEscaped (relationship->To ().GetInstanceId ()->GetFirstColumn ()->GetName ().c_str ());
                    body.Append (";").AppendEOL ();
                    }
                }
            }
        else if (isSelfRelationship)
            {
            for (auto foreignEnd : relationship->ForeignEnd ().GetStorages ())
                {
                auto& builder = const_cast<Storage*>(foreignEnd)->GetTriggerListR ().Create (SqlTriggerBuilder::Type::Delete, SqlTriggerBuilder::Condition::After, false);
                builder.GetNameBuilder ()
                    .Append (foreignEnd->GetTable ().GetName ().c_str ())
                    .Append ("_")
                    .Append (relationship->GetSqlName ())
                    .Append ("_Self_Delete");

                builder.GetOnBuilder ().Append (foreignEnd->GetTable ().GetName ().c_str ());
                auto& body = builder.GetBodyBuilder ();
                body.Append ("--1").Append (relationship->GetRelationshipClassMap ().GetRelationshipClass ().GetFullName ()).AppendEOL ();
                for (auto primaryEnd : relationship->PrimaryEnd ().GetStorages ())
                    {
                    body.Append ("UPDATE ")
                        .AppendEscaped (primaryEnd->GetTable ().GetName ().c_str ())
                        .Append (" SET ")
                        .AppendEscaped (relationship->PrimaryEnd ().GetInstanceId ()->GetFirstColumn ()->GetName ().c_str ())
                        .Append (" = NULL");

                    if (!relationship->To ().GetClassId ()->IsVirtual () && relationship->To ().GetClassId ()->IsMappedToPrimaryTable ())
                        {
                        body.Append (", ")
                            .AppendEscaped (relationship->PrimaryEnd ().GetClassId ()->GetFirstColumn ()->GetName ().c_str ())
                            .Append (" = NULL");
                        }

                    body.Append (" WHERE OLD.")
                        .AppendEscaped (relationship->ForeignEnd ().GetInstanceId ()->GetFirstColumn ()->GetName ().c_str ())
                        .Append (" = ")
                        .AppendEscaped (relationship->PrimaryEnd ().GetInstanceId ()->GetFirstColumn ()->GetName ().c_str ());
                    body.Append (";").AppendEOL ();
                    }
                }


            //if (relationship->RequireCascade ())
            //    {
            //    for (auto foreignEnd : relationship->ForeignEnd ().GetStorages ())
            //        {
            //        auto& builder = const_cast<Storage*>(foreignEnd)->GetTriggerListR ().Create (SqlTriggerBuilder::Type::UpdateOf, SqlTriggerBuilder::Condition::After, false);
            //        builder.GetNameBuilder ()
            //            .Append (foreignEnd->GetTable ().GetName ().c_str ())
            //            .Append ("_")
            //            .Append (relationship->GetSqlName ())
            //            .Append ("_Self_CascadeDelete");

            //        auto keyColumn = relationship->PrimaryEnd ().GetInstanceId ()->GetFirstColumn ()->GetName ().c_str ();
            //        builder.GetOnBuilder ().Append (foreignEnd->GetTable ().GetName ().c_str ());
            //        builder.GetWhenBuilder ().AppendFormatted ("NEW.[%s] IS NULL AND OLD.[%s] IS NOT NULL", keyColumn, keyColumn);
            //        builder.GetUpdateOfColumnsR ().push_back (keyColumn);
            //        auto& body = builder.GetBodyBuilder ();
            //        body.Append ("--6").Append (relationship->GetRelationshipClassMap ().GetRelationshipClass ().GetFullName ()).AppendEOL ();
            //        for (auto primaryEnd : relationship->PrimaryEnd ().GetStorages ())
            //            {
            //            body.Append ("DELETE FROM ")
            //                .AppendEscaped (primaryEnd->GetTable ().GetName ().c_str ());

            //            body.Append (" WHERE ")
            //                .AppendEscaped (relationship->ForeignEnd ().GetInstanceId ()->GetFirstColumn ()->GetName ().c_str ())
            //                .Append (" = OLD.")
            //                .AppendEscaped (relationship->ForeignEnd ().GetInstanceId ()->GetFirstColumn ()->GetName ().c_str ());
            //            body.Append (";").AppendEOL ();
            //            }
            //        }
            //    }

            }
            if (relationship->RequireCascade ())
                {
                for (auto fromStorage : relationship->From ().GetStorages ())
                    {
                    auto& builder = const_cast<Storage*>(fromStorage)->GetTriggerListR ()
                        .Create (SqlTriggerBuilder::Type::Delete, SqlTriggerBuilder::Condition::After, false);

                    builder.GetNameBuilder ()
                        .Append (fromStorage->GetTable ().GetName ().c_str ())
                        .Append ("_")
                        .Append (relationship->GetSqlName ())
                        .Append ("_CascadeDelete");

                    builder.GetOnBuilder ().Append (fromStorage->GetTable ().GetName ().c_str ());
                    auto& body = builder.GetBodyBuilder ();
                    body.Append ("--2 ").Append (relationship->GetRelationshipClassMap ().GetRelationshipClass ().GetFullName ()).AppendEOL ();
                    for (auto toStorage : relationship->To ().GetStorages ())
                        {
                        body
                            .Append ("DELETE FROM ")
                            .AppendEscaped (toStorage->GetTable ().GetName ().c_str ())
                            .Append (" WHERE ");

                        auto toKeyColumn = relationship->To ().GetInstanceId ()->GetFirstColumn ();
                        auto fromKeyColumn = relationship->From ().GetInstanceId ()->GetFirstColumn ();
                        if (toStorage != fromStorage)
                            {
                            //Self join should not be processed here.
                            //This is issue with EndTable our Source/Target key is always in same table. Following should fix that
                            if (persistedInFrom)
                                {
                                fromKeyColumn = toStorage->GetTable ().GetFilteredColumnFirst (ECDbKnownColumns::ECInstanceId);
                                }
                            else
                                {
                                toKeyColumn = fromStorage->GetTable ().GetFilteredColumnFirst (ECDbKnownColumns::ECInstanceId);
                                }
                            }
                        body.AppendFormatted ("([%s] = OLD.[%s])",
                            fromKeyColumn->GetName ().c_str (),
                            toKeyColumn->GetName ().c_str ());

                        if (relationship->IsHolding ())
                            {
                            body.AppendFormatted (" AND (SELECT COUNT (*) FROM " ECDB_HOLDING_VIEW "  WHERE ECInstanceId = OLD.[%s]) = 0", toKeyColumn->GetName ().c_str ());
                            }
                        body.Append (";").AppendEOL ();

                        }
                    }
                }
        }
    }

void ECDbMapAnalyser::Storage::HandleStructArray ()
    {
    if (m_structCascades.empty ())
        return;

    auto& builder = GetTriggerListR ().Create (SqlTriggerBuilder::Type::Delete, SqlTriggerBuilder::Condition::After, false);
    builder.GetNameBuilder ()
        .Append (GetTable ().GetName ().c_str ())
        .Append ("_")
        .Append ("StructArray_Delete");

    builder.GetOnBuilder ().Append (GetTable ().GetName ().c_str ());
    auto& body = builder.GetBodyBuilder (); 
    auto ecInstanceid = GetTable ().GetFilteredColumnFirst (ECDbKnownColumns::ECInstanceId);
    BeAssert (ecInstanceid != nullptr);

    for (auto structClass : m_structCascades)
        {
        for (auto& i : structClass->GetPartitionsR ())
            {
            auto toStorage = i.first;
            auto parentECInstanceId = toStorage->GetTable ().GetFilteredColumnFirst (ECDbKnownColumns::ParentECInstanceId);
            BeAssert (parentECInstanceId != nullptr);
            body
                .Append ("DELETE FROM ")
                .AppendEscaped (toStorage->GetTable ().GetName ().c_str ())
                .Append (" WHERE ")
                .AppendFormatted ("OLD.[%s] = [%s] ", ecInstanceid->GetName ().c_str (), parentECInstanceId->GetName ().c_str ());

            body.Append (";").AppendEOL ();
            }
        }
    }

void ECDbMapAnalyser::Storage::HandleCascadeLinkTable (std::vector<ECDbMapAnalyser::Relationship*> const& relationships)
    {
    for (auto relationship : relationships)
        {
        if (!relationship->RequireCascade ())
            continue;

        auto& builder = GetTriggerListR ().Create (SqlTriggerBuilder::Type::Delete, SqlTriggerBuilder::Condition::After, false);
        builder.GetNameBuilder ()
            .Append (GetTable ().GetName ().c_str ())
            .Append ("_")
            .Append (relationship->GetSqlName ())
            .Append ("_Cascade");
        
        builder.GetOnBuilder ().Append (GetTable ().GetName ().c_str ());
        auto& body = builder.GetBodyBuilder ();
        body.Append ("--3 ").Append (relationship->GetRelationshipClassMap ().GetRelationshipClass ().GetFullName ()).AppendEOL ();
        for (auto storage : relationship->To ().GetStorages ())
            {
            body
                .Append ("DELETE FROM ")
                .AppendEscaped (storage->GetTable ().GetName ().c_str ())
                .Append (" WHERE ");

            auto otherEndPrimaryKey = storage->GetTable ().GetFilteredColumnFirst (ECDbKnownColumns::ECInstanceId);
            body.AppendFormatted ("(OLD.[%s] = [%s])", relationship->To ().GetInstanceId ()->GetFirstColumn ()->GetName ().c_str (), otherEndPrimaryKey->GetName ().c_str ());
            if (relationship->IsHolding ())
                {
                body.AppendFormatted (" AND (SELECT COUNT (*) FROM " ECDB_HOLDING_VIEW "  WHERE ECInstanceId = OLD.[%s]) = 0", relationship->To ().GetInstanceId ()->GetFirstColumn ()->GetName ());
                }
            body.Append (";").AppendEOL ();
            }
        }
    }

void ECDbMapAnalyser::Storage::Generate ()
    {
    std::vector<Relationship*> linkTables;
    for (auto relationship : GetRelationshipsR ())
        {
        if (relationship->IsLinkTable ())
            {
            linkTables.push_back (relationship);
            }
        }

    HandleCascadeLinkTable (linkTables);
    HandleStructArray ();
    }
    
END_BENTLEY_SQLITE_EC_NAMESPACE
