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

    //---------------------------------------------------------------------------------------
    // @bsimethod                                 Affan.Khan                         06/2015
    //---------------------------------------------------------------------------------------
    BentleyStatus SqlGenerator::BuildPropertyExpression (NativeSqlBuilder& viewSql, PropertyMapCR propertyMap, Utf8CP tablePrefix, bool addECPropertyPathAlias, bool nullValue)
        {
        if (auto o = dynamic_cast<PropertyMapToColumnCP>(&propertyMap))
            {
            return BuildPrimitivePropertyExpression (viewSql, *o, tablePrefix, addECPropertyPathAlias, nullValue);
            }
        if (auto o = dynamic_cast<PropertyMapPoint const*>(&propertyMap))
            {
            return BuildPointPropertyExpression (viewSql, *o, tablePrefix, addECPropertyPathAlias, nullValue);
            }
        else if (auto o = dynamic_cast<PropertyMapToInLineStructCP>(&propertyMap))
            {
            return BuildStructPropertyExpression (viewSql, *o, tablePrefix, addECPropertyPathAlias, nullValue);
            }
        else if (auto o = dynamic_cast<PropertyMapToTableCP>(&propertyMap))
            {
            return BentleyStatus::SUCCESS;
            }
        else if (auto o = dynamic_cast<UnmappedPropertyMap const*>(&propertyMap))
            {
            return BentleyStatus::SUCCESS;
            }
        else if (auto o = dynamic_cast<PropertyMapSystem const*>(&propertyMap))
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
             
            BuildColumnExpression (fragments, tablePrefix, columns[0]->GetName ().c_str (), Utf8String (accessString + ".X").c_str (), addECPropertyPathAlias, nullValue);
            BuildColumnExpression (fragments, tablePrefix, columns[1]->GetName ().c_str (), Utf8String (accessString + ".Y").c_str (), addECPropertyPathAlias, nullValue);
            }
        else if (!primitiveProperty->GetIsArray () && primitiveProperty->GetType () == PrimitiveType::PRIMITIVETYPE_Point3D)
            {
            if (columns.size () != 3LL)
                {
                BeAssert (columns.size () == 3LL);
                return BentleyStatus::ERROR;
                }

            BuildColumnExpression (fragments, tablePrefix, columns[0]->GetName ().c_str (), Utf8String (accessString + ".X").c_str (), addECPropertyPathAlias, nullValue);
            BuildColumnExpression (fragments, tablePrefix, columns[1]->GetName ().c_str (), Utf8String (accessString + ".Y").c_str (), addECPropertyPathAlias, nullValue);
            BuildColumnExpression (fragments, tablePrefix, columns[2]->GetName ().c_str (), Utf8String (accessString + ".Z").c_str (), addECPropertyPathAlias, nullValue);
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
    BentleyStatus SqlGenerator::BuildPrimitivePropertyExpression (NativeSqlBuilder& viewSql, PropertyMapToColumnCR propertyMap, Utf8CP tablePrefix, bool addECPropertyPathAlias, bool nullValue)
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
    BentleyStatus SqlGenerator::BuildSystemSelectionClause (NativeSqlBuilder::List& fragments, ClassMapCR const& classMap, Utf8CP tablePrefix, bool addECPropertyPathAlias, bool nullValue)
        {
      
        if (auto column = classMap.GetTable ().GetFilteredColumnFirst (ECDbSystemColumnECInstanceId))
            {
            if (BuildColumnExpression (fragments, tablePrefix, column->GetName ().c_str (), "ECInstanceId", addECPropertyPathAlias, nullValue) != BentleyStatus::SUCCESS)
                return BentleyStatus::ERROR;
            }
        else
            {
            BeAssert (false && "Failed to find ECInstanceId column");
            return BentleyStatus::ERROR;
            }

        if (auto column = classMap.GetTable ().GetFilteredColumnFirst (ECDbSystemColumnECClassId))
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

        if (auto column = classMap.GetTable ().GetFilteredColumnFirst (ECDbSystemColumnParentECInstanceId))
            {
            if (BuildColumnExpression (fragments, tablePrefix, column->GetName ().c_str (), "ParentECInstanceId", addECPropertyPathAlias, nullValue) != BentleyStatus::SUCCESS)
                return BentleyStatus::ERROR;
            }

        if (auto column = classMap.GetTable ().GetFilteredColumnFirst (ECDbSystemColumnECPropertyPathId))
            {
            if (BuildColumnExpression (fragments, tablePrefix, column->GetName ().c_str (), "ECPropertyPathId", addECPropertyPathAlias, nullValue) != BentleyStatus::SUCCESS)
                return BentleyStatus::ERROR;
            }

        if (auto column = classMap.GetTable ().GetFilteredColumnFirst (ECDbSystemColumnECArraryIndex))
            {
            if (BuildColumnExpression (fragments, tablePrefix, column->GetName ().c_str (), "ECArraryIndex", addECPropertyPathAlias, nullValue) != BentleyStatus::SUCCESS)
                return BentleyStatus::ERROR;
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
    BentleyStatus SqlGenerator::BuildSelectionClause (NativeSqlBuilder& viewSql, ECDbMapCR map, ClassMapCR const& baseClassMap, ClassMapCR const& classMap, Utf8CP tablePrefix, bool addECPropertyPathAlias, bool nullValue)
        {
        NativeSqlBuilder::List fragments;

        if (BuildSystemSelectionClause (fragments, classMap, tablePrefix, addECPropertyPathAlias, nullValue) != BentleyStatus::SUCCESS)
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
                viewSql.Append (", ").AppendEOL ();

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
        name.append (Utf8String (ecClass.GetSchema ().GetNamespacePrefix ().c_str ()));
        name.append ("_");
        name.append (Utf8String (ecClass.GetName ().c_str ()));
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
             auto targetECInstanceIdColumn = targetTable.GetFilteredColumnFirst (ECDbSystemColumnECInstanceId);
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
     void SqlGenerator::CollectDerivedEndTableRelationships (std::set<RelationshipClassEndTableMapCP>& childMaps, RelationshipClassMapCR const& classMap)
         {
         if (classMap.GetMapStrategy ().IsEndTableMapping ())
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
     BentleyStatus SqlGenerator::BuildEndTableRelationshipView (NativeSqlBuilder::List& unionList, ECDbMapCR map, RelationshipClassMapCR const& classMap)
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

             if (BuildSelectionClause (sqlBuilder, map, classMap, *endClassMap, tabelPrefix.c_str (), /*addECPropertyPathAlias = */ topLevel, false)
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
    BentleyStatus SqlGenerator::BuildClassView (NativeSqlBuilder& viewSql, ECDbMapCR map, ClassMapCR const& classMap)
        {
        auto& disp = classMap.GetStorageDescription ();
        NativeSqlBuilder::List unionList;
        for (auto& part : disp.GetHorizontalPartitions ())
            {
            if (part.GetTable ().GetPersistenceType () == PersistenceType::Virtual)
                continue;

            NativeSqlBuilder sqlBuilder;

            bool topLevel = (&disp.GetRootHorizontalPartition () == &part);

            sqlBuilder.Append ("SELECT ");
            auto derivedClassId = part.GetClassIds ().front ();
            auto derivedClass = map.GetECDbR ().Schemas ().GetECClass (derivedClassId);

            if (derivedClass == nullptr)
                {
                BeAssert (derivedClass != nullptr);
                return BentleyStatus::ERROR;
                }

            auto derivedClassMap = map.GetClassMap (*derivedClass);
            if (derivedClassMap == nullptr)
                {
                BeAssert (derivedClassMap != nullptr);
                return BentleyStatus::ERROR;
                }

            if (derivedClassMap->GetMapStrategy ().IsEndTableMapping ())
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

            if (BuildSelectionClause (sqlBuilder, map, classMap, static_cast<ClassMapCR>(*derivedClassMap), tabelPrefix.c_str(), /*addECPropertyPathAlias = */ topLevel, false)
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

            if (derivedClassMap->IsRelationshipClassMap())
                {
                auto const relationshipMap = static_cast<RelationshipClassMapCP>(derivedClassMap);
                if (BuildRelationshipJoinIfAny (sqlBuilder, *relationshipMap, ECRelationshipEnd::ECRelationshipEnd_Source, topLevel) != BentleyStatus::SUCCESS)
                    return BentleyStatus::ERROR;

                if (BuildRelationshipJoinIfAny (sqlBuilder, *relationshipMap, ECRelationshipEnd::ECRelationshipEnd_Target, topLevel) != BentleyStatus::SUCCESS)
                    return BentleyStatus::ERROR;
                }

            if (auto classIdcolumn = part.GetTable ().GetFilteredColumnFirst (ECDbSystemColumnECClassId))
                {
                sqlBuilder.Append (" WHERE ");
                sqlBuilder.AppendParenLeft ();
                sqlBuilder.AppendEscaped (classIdcolumn->GetName ().c_str ());
                sqlBuilder.AppendSpace ();
                if (part.NeedsClassIdFilter ())
                    {
                    part.AppendECClassIdFilterSql (sqlBuilder);
                    }
                else
                    {
                    sqlBuilder.Append (" = ");
                    sqlBuilder.Append (derivedClass->GetId ());
                    }
                sqlBuilder.AppendParenRight ();
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

        if (classMap.IsRelationshipClassMap ())
            if (BuildEndTableRelationshipView (unionList, map, static_cast<RelationshipClassMapCR> (classMap)) != BentleyStatus::SUCCESS)
                return BentleyStatus::ERROR;

        if (!unionList.empty ())
            {
            for (auto const& unionEntry : unionList)
                {
                if (&(unionList.front ()) != &(unionEntry))
                    viewSql.Append (" UNION \r\n");

                viewSql.Append (unionEntry);
                }

            return BentleyStatus::SUCCESS;
            }

        //Create null view
        viewSql.Append ("SELECT ");
        if (BuildSelectionClause (viewSql, map, classMap, classMap, nullptr, true, true) != BentleyStatus::SUCCESS)
            {
            return BentleyStatus::ERROR;
            }

        viewSql.Append (" LIMIT 0");
        return BentleyStatus::SUCCESS;
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                 Affan.Khan                         06/2015
    //---------------------------------------------------------------------------------------
    BentleyStatus SqlGenerator::BuildView (NativeSqlBuilder& viewSql, ECDbMapCR map, IClassMap const& classMap)
        {
        return BuildClassView (viewSql, map, static_cast<ClassMapCR>(classMap));
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                 Affan.Khan                         06/2015
    //---------------------------------------------------------------------------------------
    BentleyStatus SqlGenerator::CreateView (ECDbMapR map, IClassMap const& classMap, bool dropViewIfExist)
        {
        NativeSqlBuilder builder;
        auto viewName = SqlGenerator::BuildViewClassName (classMap.GetClass ());
        if (dropViewIfExist)
            map.GetECDbR ().ExecuteSql (SqlPrintfString ("DROP VIEW IF EXISTS %s", viewName.c_str ()));

        builder.Append ("CREATE VIEW ").Append (viewName.c_str ());
        builder.Append ("\r\n  AS \r\n");
        if (BuildView (builder, map, classMap) == BentleyStatus::SUCCESS)
            {
            if (map.GetECDbR ().ExecuteSql (builder.ToString ()) != BE_SQLITE_OK)
                {
                printf ("ERROR : %s \r\n", builder.ToString ());
                printf ("ERROR : %s \r\n", map.GetECDbR ().GetLastError ());
                return BentleyStatus::ERROR;
                }
            }

        return BentleyStatus::SUCCESS;
        }
    BentleyStatus SqlGenerator::BuildDeleteTriggerForStructArrays (NativeSqlBuilder::List& tiggers, ECDbMapCR map, ClassMapCR const& classMap)
        {
        NativeSqlBuilder trigger;
        Utf8String triggerName = BuildSchemaQualifiedClassName (classMap.GetClass ());
        triggerName.append ("_Delete_StructArrays");
        trigger.Append ("CREATE TRIGGER IF NOT EXISTS ");
        trigger.Append (triggerName.c_str ()).AppendEOL ();
        trigger.Append ("INSTEAD OF DELETE").AppendEOL ();
        trigger.Append ("ON ");
        trigger.Append (BuildViewClassName (classMap.GetClass ()).c_str ()).AppendEOL ();
        trigger.Append ("FOR EACH ROW").AppendEOL ();
        trigger.Append ("BEGIN ").AppendEOL ();
        std::map<IClassMap const*, std::vector<PropertyMapToTableCP>> structPropertyMaps;
        classMap.GetPropertyMaps ().Traverse ([&] (TraversalFeedback& feedback, PropertyMapCP propertyMap)
            {
            if (auto mapToTable = dynamic_cast<PropertyMapToTableCP>(propertyMap))
                {
                if (auto associatedClasMap = map.GetClassMap (mapToTable->GetElementType ()))
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
            trigger.Append ("\t-- Struct Properties").AppendEOL ();
            for (auto pair : structPropertyMaps)
                {

                for (auto propertyMap : pair.second)
                    {
                    trigger.Append ("\t-- > ").Append (Utf8String (propertyMap->GetPropertyAccessString ()).c_str ()).AppendEOL ();
                    }

                trigger.Append ("\tDELETE FROM ").AppendEscaped (BuildViewClassName (pair.first->GetClass ()).c_str ());
                trigger.Append (" WHERE ParentECInstanceId = OLD.ECInstanceId;").AppendEOL ();
                }

            trigger.Append ("END ");
            tiggers.push_back (std::move (trigger));
            }
        
        return BentleyStatus::SUCCESS;
        }

    BentleyStatus SqlGenerator::BuildDerivedFilterClause (Utf8StringR filter, ECDb& db, ECClassId baseClassId)
        {
        Utf8CP sql =
            "WITH RECURSIVE "
            "    DerivedClassList (CurrentECClassId, DerivedECClassId) "
            "    AS ( "
            "       SELECT ?1, ?1 FROM ec_Class "
            "    UNION "
            "       SELECT BC.BaseECClassId, BC.ECClassId "
            "           FROM DerivedClassList DCL "
            "       INNER JOIN ec_BaseClass BC ON BC.BaseECClassId = DCL.DerivedECClassId "
            "    ) "
            "SELECT 'IN (' || group_concat(DISTINCT DerivedECClassId) || ')' FROM DerivedClassList ORDER BY DerivedECClassId";

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
    BentleyStatus SqlGenerator::BuildDeleteTriggers (NativeSqlBuilder::List& tiggers, ECDbMapCR map, ClassMapCR const& classMap)
        {
        if (BuildDeleteTriggerForStructArrays (tiggers, map, classMap) != BentleyStatus::SUCCESS)
            return BentleyStatus::ERROR;

        for (auto derivedClassMap : classMap.GetDerivedClassMaps ())
            {
            if (derivedClassMap == nullptr)
                continue;

            NativeSqlBuilder trigger;
            Utf8String triggerName = BuildSchemaQualifiedClassName (classMap.GetClass ());
            triggerName.append ("_Delete_");
            triggerName.append (BuildSchemaQualifiedClassName (derivedClassMap->GetClass ()).c_str ());
            trigger.Append ("CREATE TRIGGER IF NOT EXISTS ");
            trigger.Append (triggerName.c_str ()).AppendEOL ();
            trigger.Append ("INSTEAD OF DELETE").AppendEOL ();
            trigger.Append ("ON ");
            trigger.Append (BuildViewClassName (classMap.GetClass ()).c_str ()).AppendEOL ();
            trigger.Append ("FOR EACH ROW").AppendEOL ();

            Utf8String filterClause;
            if (BuildDerivedFilterClause (filterClause, map.GetECDbR (), derivedClassMap->GetClass ().GetId ()) != BentleyStatus::SUCCESS)
                return BentleyStatus::ERROR;

            if (!filterClause.empty ())
                {
                trigger.Append ("\tWHEN OLD.ECClassId ").Append (filterClause.c_str ()).AppendEOL ();
                }
            trigger.Append ("BEGIN ").AppendEOL ();
            trigger.Append ("\tDELETE FROM ");
            trigger.AppendEscaped (BuildViewClassName (derivedClassMap->GetClass ()).c_str ());
            trigger.Append (" WHERE ECInstanceId = OLD.ECInstanceId;").AppendEOL();
            trigger.Append ("END ");
            tiggers.push_back (std::move (trigger));
            }

        if (classMap.GetTable ().GetPersistenceType () == PersistenceType::Persisted)
            {
            NativeSqlBuilder trigger;
            Utf8String triggerName = BuildSchemaQualifiedClassName (classMap.GetClass ());
            triggerName.append ("_Delete_Me");
            trigger.Append ("CREATE TRIGGER IF NOT EXISTS ");
            trigger.Append (triggerName.c_str ()).AppendEOL ();
            trigger.Append ("INSTEAD OF DELETE").AppendEOL ();
            trigger.Append ("ON ");
            trigger.Append (BuildViewClassName(classMap.GetClass()).c_str()).AppendEOL ();
            trigger.Append ("FOR EACH ROW").AppendEOL ();
            trigger.Append ("\tWHEN OLD.ECClassId = ").Append (classMap.GetClass ().GetId ()).AppendEOL ();


            trigger.Append ("BEGIN ").AppendEOL ();
            trigger.Append ("\tDELETE FROM ");
            trigger.AppendEscaped (classMap.GetTable ().GetName ().c_str ());
            trigger.Append (" WHERE ");

            if (auto column = classMap.GetTable ().GetFilteredColumnFirst (ECDbSystemColumnECInstanceId))
                {
                trigger.AppendEscaped (column->GetName ().c_str ());
                }
            else
                {
                BeAssert (false && "Failed to find ECInstanceId column");
                return BentleyStatus::ERROR;
                }

            trigger.Append (" = ").Append ("OLD.ECInstanceId;").AppendEOL ();
            trigger.Append ("END ");
            tiggers.push_back (std::move (trigger));
            }

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
        
    if (classMap->GetMapStrategy ().IsMapped ())
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

    if (ECDbSchemaPersistence::GetClassesMappedToTable (classesMappedToTable, table, true, map.GetECDbR ()) != BE_SQLITE_DONE)
        return BentleyStatus::ERROR;
 
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
                        where.AppendParenLeft ().AppendQuoted(table.GetName().c_str()).AppendDot().AppendQuoted(classIdColumn->GetName().c_str()).Append (" IN (");
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
                        where.AppendParenLeft ().AppendQuoted (table.GetName ().c_str ()).AppendDot ().AppendQuoted (classIdColumn->GetName ().c_str ()).Append (" NOT IN (");
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
               where.AppendParenLeft ().AppendQuoted (table.GetName ().c_str ()).AppendDot ().AppendQuoted (classIdColumn->GetName ().c_str ()).Append (" = ").Append (firstChildClassMap->GetClass ().GetId ()).AppendParenRight ();
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
    auto primaryECInstanceIdColumn = primaryTable->GetFilteredColumnFirst (ECDbSystemColumnECInstanceId);
    BeAssert (primaryECInstanceIdColumn != nullptr);
    if (!secondaryTables.empty ())
        {
        for (auto secondaryTable : secondaryTables)
            {
            auto secondaryECInstanceIdColumn = secondaryTable->GetFilteredColumnFirst (ECDbSystemColumnECInstanceId);
            BeAssert (secondaryECInstanceIdColumn != nullptr);
            viewSql.Append (" INNER JOIN ").AppendEscaped (secondaryTable->GetName ().c_str ()).Append (" ON ").AppendParenLeft ();
            viewSql.AppendEscaped (secondaryTable->GetName ().c_str ()).AppendDot ().AppendEscaped (secondaryECInstanceIdColumn->GetName ().c_str ());
            viewSql.Append (" = ");
            if (!relationMap.GetSourceECClassIdPropMap ()->IsMappedToPrimaryTable ())
                viewSql.AppendEscaped (primaryTable->GetName ().c_str ()).AppendDot ().AppendEscaped (relationMap.GetSourceECInstanceIdPropMap ()->GetFirstColumn ()->GetName ().c_str ());
            else if (!relationMap.GetTargetECClassIdPropMap ()->IsMappedToPrimaryTable ())
                viewSql.AppendEscaped (primaryTable->GetName ().c_str ()).AppendDot ().AppendEscaped (relationMap.GetTargetECInstanceIdPropMap ()->GetFirstColumn ()->GetName ().c_str ());
            else
                {
                BeAssert (false && "Incorrect case");
                return ERROR;
                }

            viewSql.AppendParenRight ();
            }
        }
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

    //We need to figure out
    //OtherEnd have ECClassId
        //If yes then we need to JOIN otherwise we have a constant value

    //FROM & WHERE
    

    viewSql.Append (" FROM ").AppendEscaped (relationMap.GetTable ().GetName ().c_str ());

    //Append secondary table JOIN
    auto const secondaryTables = relationMap.GetSecondaryTables ();
    auto primaryTable = &relationMap.GetTable ();
    auto primaryECInstanceIdColumn = primaryTable->GetFilteredColumnFirst (ECDbSystemColumnECInstanceId);
    BeAssert (primaryECInstanceIdColumn != nullptr);
    if (!secondaryTables.empty ())
        {
        for (auto secondaryTable : secondaryTables)
            {
            auto secondaryECInstanceIdColumn = secondaryTable->GetFilteredColumnFirst (ECDbSystemColumnECInstanceId);
            BeAssert (secondaryECInstanceIdColumn != nullptr);
            viewSql.Append (" INNER JOIN ").AppendEscaped (secondaryTable->GetName ().c_str ()).Append (" ON ").AppendParenLeft();
            viewSql.AppendEscaped (secondaryTable->GetName ().c_str ()).AppendDot ().AppendEscaped (secondaryECInstanceIdColumn->GetName ().c_str ());
            viewSql.Append (" = ");
            if (!relationMap.GetSourceECClassIdPropMap ()->IsMappedToPrimaryTable ())
                viewSql.AppendEscaped (primaryTable->GetName ().c_str ()).AppendDot ().AppendEscaped (relationMap.GetSourceECInstanceIdPropMap ()->GetFirstColumn()->GetName ().c_str ());
            else if (!relationMap.GetTargetECClassIdPropMap ()->IsMappedToPrimaryTable ())
                viewSql.AppendEscaped (primaryTable->GetName ().c_str ()).AppendDot ().AppendEscaped (relationMap.GetTargetECInstanceIdPropMap ()->GetFirstColumn ()->GetName ().c_str ());
            else
                {
                BeAssert (false && "Incorrect case");
                return ERROR;
                }

            viewSql.AppendParenRight ();
            }
        }

    viewSql.Append (" WHERE ").Append (relationMap.GetOtherEndECInstanceIdPropMap ()->ToNativeSql (nullptr, ECSqlType::Select, false)).Append (" IS NOT NULL");
    return SUCCESS;
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

    if (relationMap.GetMapStrategy().IsUnmapped ())
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

                 auto column = relationMap.GetTable().GetFilteredColumnFirst(ECDbSystemColumnECClassId);
                 if (column != nullptr)
                     {

                     std::vector<ECClassId> classesMappedToTable;
                     if (ECDbSchemaPersistence::GetClassesMappedToTable(classesMappedToTable, relationMap.GetTable(), false, map.GetECDbR()) != BE_SQLITE_DONE)
                         return BentleyStatus::ERROR;

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
            if (status != BentleyStatus::SUCCESS)
                return status;

            auto column = table->GetFilteredColumnFirst (ECDbSystemColumnECClassId);
            if (column != nullptr)
                {

                std::vector<ECClassId> classesMappedToTable;
                if (ECDbSchemaPersistence::GetClassesMappedToTable (classesMappedToTable, *table, false, map.GetECDbR ()) != BE_SQLITE_DONE)
                    return BentleyStatus::ERROR;

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

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khna                        03/2014
//---------------------------------------------------------------------------------------
//static
    BentleyStatus ViewGenerator::CreateSystemView (NativeSqlBuilder& viewSql, SystemViewType systemView, ECDbMapCR map, std::vector<ECClassId> const& classesToInclude, bool polymorphic, ECSqlPrepareContext const& prepareContext)
    {
    std::map<ECClassId, IClassMap const*> viewClasses;

    ECDbSchemaManagerCR schemaManager = map.GetECDbR ().Schemas ();

    if (classesToInclude.empty ())
        {
        Utf8CP classSql = nullptr;
        switch (systemView)
            {
                case SystemViewType::Class: //ECClass and ECStructs (all primary instances)
                    classSql = "SELECT c.Id FROM ec_Class c JOIN ec_ClassMap m ON c.ID = m.ECClassId WHERE (m.MapStrategy  <> 3 AND m.MapStrategy  <> 8) AND c.IsRelationship <> 1"; break;
                case SystemViewType::RelationshipClass: //RelationshipOnly
                    classSql = "SELECT C.Id FROM ec_Class c JOIN ec_ClassMap m ON c.ID = m.ECClassId WHERE (m.MapStrategy  <> 3 AND m.MapStrategy  <> 8) AND c.IsRelationship = 1"; break;
                case SystemViewType::StructArray: //Structs only (all struct array instances)
                    classSql = "SELECT C.Id FROM ec_Class c JOIN ec_ClassMap m ON c.ID = m.ECClassId WHERE (m.MapStrategy  <> 3 AND m.MapStrategy  <> 8) AND c.IsStruct = 1"; break;
            }

        BeAssert (classSql != nullptr);
        Statement stmt;
        auto status = stmt.Prepare (map.GetECDbR (), classSql);
        if (status != BE_SQLITE_OK)
            {
            BeAssert (false && "Failed to prepare statement");
            return BentleyStatus::ERROR;
            }

        while (stmt.Step () == BE_SQLITE_ROW)
            {
            ECClassCP ecClass = schemaManager.GetECClass (stmt.GetValueInt64 (0));
            if (ecClass == nullptr)
                {
                BeAssert (false && "Failed to get ECClass from ECClassId using ECSchemaManager");
                return BentleyStatus::ERROR;
                }

            IClassMap const* classMap = map.GetClassMap (*ecClass);
            if (classMap == nullptr || !ECDbPolicyManager::GetClassPolicy (*classMap, IsValidInECSqlPolicyAssertion::Get ()).IsSupported ())
                {              
                continue;
                }

            viewClasses.insert (std::map<ECClassId, IClassMap const*>::value_type (ecClass->GetId(), classMap));
            }
        }
    else
        {     
        for (auto ecClassId : classesToInclude)
            {       
            ECClassCP ecClass = schemaManager.GetECClass (ecClassId);
            if (ecClass == nullptr)
                {
                BeAssert (false && "Failed to get ECClass from ECClassId using ECSchemaManager");
                return BentleyStatus::ERROR;
                }

            IClassMap const* classMap = map.GetClassMap (*ecClass);
            if (classMap == nullptr || classMap->GetMapStrategy ().IsUnmapped ())
                {
                continue;
                }
            
            if (polymorphic && ecClass->GetRelationshipClassCP () == nullptr)
                LoadDerivedClassMaps (viewClasses, map, classMap);
            else
                viewClasses.insert (std::map<ECClassId, IClassMap const*>::value_type (ecClass->GetId (), classMap));
            }
        }

    std::map<ECDbSqlTable const*, std::vector<IClassMap const*>> tableMap;
    std::map<ECDbSqlTable const*, size_t> tableMapDb;
    std::set<ECDbSqlTable const*> tableToIncludeEntirly;   

    for (auto& pair : viewClasses)
        {
        tableMap[&pair.second->GetTable ()].push_back (pair.second);
        }

    if (!classesToInclude.empty ())
        {
        Statement stmt;
        auto status= stmt.Prepare (map.GetECDbR (), "SELECT COUNT(*) FROM ec_ClassMap GROUP BY MapToDbTable WHERE MapToDbTable = ?");
        if (status != BE_SQLITE_OK)
            {
            BeAssert (false && "Failed to prepare statement");
            return BentleyStatus::ERROR;
            }

        for (auto& pair : tableMap)
            {
            stmt.BindText (1, pair.first->GetName (), Statement::MakeCopy::No);
            if (stmt.Step () != BE_SQLITE_ROW)
                {
                BeAssert (false);
                return BentleyStatus::ERROR;
                }

            if (pair.second.size() == (size_t)stmt.GetValueInt (0))
                tableToIncludeEntirly.insert (pair.first);

            tableMapDb[pair.first] = (size_t)stmt.GetValueInt (0);

            stmt.Reset ();
            stmt.ClearBindings ();
            }
        }
    else
        {
        for (auto& pair : tableMap)
            {
            tableToIncludeEntirly.insert (pair.first);
            }

        for (auto& pair : tableMap)
            {
            tableToIncludeEntirly.insert (pair.first);
            tableMapDb[pair.first] = pair.second.size ();
            }
        }

    if (tableMap.empty ())
        return BentleyStatus::SUCCESS;

    if (SystemViewType::Class == systemView)
        {
        CreateSystemClassView (viewSql, tableMap, tableToIncludeEntirly, false, prepareContext);
        }

    if (SystemViewType::StructArray == systemView)
        {
        CreateSystemClassView (viewSql, tableMap, tableToIncludeEntirly, true, prepareContext);
        }

    if (SystemViewType::RelationshipClass == systemView)
        {
        for (auto& pair : tableMap)
            {
            std::vector<IClassMap const*>& classMaps = pair.second;
            BeAssert (classMaps.size () == 1);

            RelationshipClassMapCP relationshipMap = dynamic_cast<RelationshipClassMapCP>(classMaps[0]);// = static_cast
            BeAssert (relationshipMap != nullptr);

            viewSql.Append (" UNION SELECT ");
            AppendSystemPropMaps (viewSql, map, prepareContext, *relationshipMap);
            viewSql.Append (" FROM ").AppendEscaped (relationshipMap->GetTable ().GetName ().c_str ());

            if (RelationshipClassEndTableMapCP endTableMap = dynamic_cast<RelationshipClassEndTableMapCP>(relationshipMap))
                {                
                viewSql.Append (" WHERE ").Append (endTableMap->GetOtherEndECInstanceIdPropMap ()->ToNativeSql (nullptr, ECSqlType::Select, false)).Append (" IS NOT NULL");
                }
            }
        }
    return BentleyStatus::SUCCESS;
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
        if (classMap == nullptr || classMap->GetMapStrategy ().IsUnmapped ())
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
        ECDbSqlColumn const* ecInstanceIdColumn = classMap->GetPropertyMap (L"ECInstanceId")->GetFirstColumn ();
        ECDbSqlColumn const* ecClassIdColumn = tableP->FindColumnCP ("ECclassId");

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


END_BENTLEY_SQLITE_EC_NAMESPACE
