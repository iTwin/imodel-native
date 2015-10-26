/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ClassMapSubclasses.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ClassMap.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! ClassMap for ECClasses which can be used as domain class or embedded types. They are
//! mapped to a secondary table
// @bsiclass                                                    Krischan.Eberle   02/2014
//+===============+===============+===============+===============+===============+======
struct SecondaryTableClassMap : ClassMap
    {
private:
    //=======================================================================================
    //! ClassMap that represents the 'embedded type' view of the SecondaryTableClassMap
    // @bsiclass                                                    Krischan.Eberle   02/2014
    //+===============+===============+===============+===============+===============+======
    struct EmbeddedTypeClassMap : IClassMap
        {
    private:
        ClassMapCR m_secondaryTableClassMap;
        PropertyMapCollection m_embeddedClassViewPropMaps;
        virtual IClassMap const& _GetView (View classView) const override { return *this; };
        virtual PropertyMapCollection const& _GetPropertyMaps () const override { return m_embeddedClassViewPropMaps; }
        virtual ECDbSqlTable& _GetTable () const override { return m_secondaryTableClassMap.GetTable (); }
        virtual ECN::ECClassCR _GetClass () const override { return m_secondaryTableClassMap.GetClass (); }
        virtual ECDbMapStrategy const& _GetMapStrategy () const override { return m_secondaryTableClassMap.GetMapStrategy (); }
        virtual ECDbMapCR _GetECDbMap () const override { return m_secondaryTableClassMap.GetECDbMap (); }
        virtual Type _GetClassMapType () const override { return Type::EmbeddedType; };
        virtual ECN::ECClassId _GetParentMapClassId () const override { return m_secondaryTableClassMap.GetParentMapClassId (); }
        virtual ClassDbView const& _GetDbView () const override { return m_secondaryTableClassMap.GetDbView (); }
    public:
        explicit EmbeddedTypeClassMap (ClassMapCR secondaryTableClassMap) : IClassMap(), m_secondaryTableClassMap (secondaryTableClassMap) {}

        ~EmbeddedTypeClassMap () {}

        MapStatus Initialize ();
        };

    std::unique_ptr<EmbeddedTypeClassMap> m_embeddedTypeClassView;

    SecondaryTableClassMap (ECN::ECClassCR ecClass, ECDbMapCR ecDbMap, ECDbMapStrategy mapStrategy, bool setIsDirty);

    virtual MapStatus _OnInitialized () override;
    virtual Type _GetClassMapType () const override;
    virtual IClassMap const& _GetView (View classView) const override;
    //virtual BentleyStatus _Save (std::set<ClassMap const*>& savedGraph);
    virtual BentleyStatus _Load (std::set<ClassMap const*>& loadGraph, ECDbClassMapInfo const& mapInfo, IClassMap const* parentClassMap) override
        {
        auto a =  ClassMap::_Load (loadGraph, mapInfo, parentClassMap);
        m_embeddedTypeClassView->Initialize ();
        return a;
        }
public:
    ~SecondaryTableClassMap () {}

    static ClassMapPtr Create (ECN::ECClassCR ecClass, ECDbMapCR ecdbMap, ECDbMapStrategy mapStrategy, bool setIsDirty) { return new SecondaryTableClassMap (ecClass, ecdbMap, mapStrategy, setIsDirty); }
    };

//=======================================================================================
//! A class map indicating that the respective ECClass was @b not mapped to a DbTable
// @bsiclass                                                Krischan.Eberle      02/2014
//+===============+===============+===============+===============+===============+======
struct UnmappedClassMap : public ClassMap
    {
private:
     virtual MapStatus _InitializePart1 (SchemaImportContext*, ClassMapInfo const& classMapInfo, IClassMap const* parentClassMap) override;
     virtual MapStatus _InitializePart2(SchemaImportContext*, ClassMapInfo const& classMapInfo, IClassMap const* parentClassMap) override { return MapStatus::Success; }
    virtual Type _GetClassMapType () const override { return IClassMap::Type::Unmapped; }
    UnmappedClassMap (ECN::ECClassCR ecClass, ECDbMapCR ecdbMap, ECDbMapStrategy mapStrategy, bool setIsDirty);
    virtual BentleyStatus _Load (std::set<ClassMap const*>& loadGraph, ECDbClassMapInfo const& mapInfo, IClassMap const* parentClassMap) override
        {
        auto nullTable = GetECDbMap ().GetSQLManager ().GetNullTable ();
        SetTable (const_cast<ECDbSqlTable*> (nullTable));
        return BentleyStatus::SUCCESS;
        }

public:
    ~UnmappedClassMap () {}

    static ClassMapPtr Create (ECN::ECClassCR ecClass, ECDbMapCR ecdbMap, ECDbMapStrategy mapStrategy, bool setIsDirty) { return new UnmappedClassMap (ecClass, ecdbMap, mapStrategy, setIsDirty); }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
