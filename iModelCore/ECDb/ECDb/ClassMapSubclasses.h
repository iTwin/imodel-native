/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ClassMapSubclasses.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
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
    // @bsiclass                                                Krischan.Eberle      01/2014
    //+===============+===============+===============+===============+===============+======
    struct NativeSqlConverterImpl : ClassMap::NativeSqlConverterImpl
        {
    private:
        virtual ECSqlStatus _GetWhereClause (NativeSqlBuilder& whereClauseBuilder, ECSqlType ecsqlType, bool isPolymorphicClassExp, Utf8CP tableAlias) const override;

    public:
        explicit NativeSqlConverterImpl (ClassMapCR classMap);
        ~NativeSqlConverterImpl () {}
        };

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
        virtual NativeSqlConverter const& _GetNativeSqlConverter () const override { return m_secondaryTableClassMap.GetNativeSqlConverter (); }
        virtual ECN::ECClassCR _GetClass () const override { return m_secondaryTableClassMap.GetClass (); }
        virtual MapStrategy _GetMapStrategy () const override { return m_secondaryTableClassMap.GetMapStrategy (); }
        virtual ECDbMapCR _GetECDbMap () const override { return m_secondaryTableClassMap.GetECDbMap (); }
        virtual Type _GetClassMapType () const override { return Type::EmbeddedType; };
        virtual ECN::ECClassId _GetParentMapClassId () const override { return m_secondaryTableClassMap.GetParentMapClassId (); }
        virtual ClassDbView const& _GetDbView () const override { return m_secondaryTableClassMap.GetDbView (); }

    public:
        explicit EmbeddedTypeClassMap (ClassMapCR secondaryTableClassMap)
            : m_secondaryTableClassMap (secondaryTableClassMap)
            {}

        ~EmbeddedTypeClassMap () {}

        MapStatus Initialize ();
        };

    std::unique_ptr<EmbeddedTypeClassMap> m_embeddedTypeClassView;

    SecondaryTableClassMap (ECN::ECClassCR ecClass, ECDbMapCR ecDbMap, MapStrategy mapStrategy, bool setIsDirty);

    virtual MapStatus _OnInitialized () override;
    virtual Type _GetClassMapType () const override;
    virtual NativeSqlConverter const& _GetNativeSqlConverter () const override;
    virtual IClassMap const& _GetView (View classView) const override;

public:
    ~SecondaryTableClassMap () {}

    static ClassMapPtr Create (ECN::ECClassCR ecClass, ECDbMapCR ecdbMap, MapStrategy mapStrategy, bool setIsDirty) { return new SecondaryTableClassMap (ecClass, ecdbMap, mapStrategy, setIsDirty); }
    };

//=======================================================================================
//! A class map indicating that the respective ECClass was @b not mapped to a DbTable
// @bsiclass                                                Krischan.Eberle      02/2014
//+===============+===============+===============+===============+===============+======
struct UnmappedClassMap : public ClassMap
    {
private:
    //=======================================================================================
    // @bsiclass                                                Krischan.Eberle      01/2014
    //+===============+===============+===============+===============+===============+======
    struct NativeSqlConverterImpl : NativeSqlConverter
        {
        private:
            virtual ECSqlStatus _GetWhereClause (NativeSqlBuilder& whereClauseBuilder, ECSqlType ecsqlType, bool isPolymorphicClassExp, Utf8CP tableAlias) const override { return ECSqlStatus::ProgrammerError; }

        public:
            explicit NativeSqlConverterImpl () {}
            virtual ~NativeSqlConverterImpl () {}
        };

    virtual MapStatus _InitializePart1 (ClassMapInfoCR classMapInfo, IClassMap const* parentClassMap) override;
    virtual MapStatus _InitializePart2 (ClassMapInfoCR classMapInfo, IClassMap const* parentClassMap) override;
    virtual Type _GetClassMapType () const override { return IClassMap::Type::Unmapped; }
    virtual NativeSqlConverter const& _GetNativeSqlConverter () const override;

    UnmappedClassMap (ECN::ECClassCR ecClass, ECDbMapCR ecdbMap, MapStrategy mapStrategy, bool setIsDirty);

public:
    ~UnmappedClassMap () {}

    static ClassMapPtr Create (ECN::ECClassCR ecClass, ECDbMapCR ecdbMap, MapStrategy mapStrategy, bool setIsDirty) { return new UnmappedClassMap (ecClass, ecdbMap, mapStrategy, setIsDirty); }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
