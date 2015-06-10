/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECDb/ECRelationshipPath.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#if !defined (DOCUMENTATION_GENERATOR)

ECDB_TYPEDEFS (ECRelationshipPath);

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                                 Ramanujam.Raman      01/2014
* @remarks Classes are stored and retrieved as "SchemaName.ClassName". The string conversion 
  utilities convert from "[SchemaName:]ClassName". 
  The modified storage format allows for their use in ECSql as-is. The equivalent strings 
  maintain compatibility with older versions of ECObjects. 
+===============+===============+===============+===============+===============+======*/
struct ECRelatedClassSpecifier
{
friend struct ECRelationshipPath;
private:
    ECN::ECRelationshipClassCP m_relationshipClass;
    ECN::ECClassCP m_relatedClass;     
    ECN::ECRelatedInstanceDirection m_direction;

    mutable Utf8String m_relationshipClassAlias; // Used only in ECSql generation
    mutable Utf8String m_relatedClassAlias; // Used only in ECSql generation

//__PUBLISH_SECTION_END__

    void Init (ECN::ECRelationshipClassCR relationshipClass, ECN::ECClassCR relatedClass, ECN::ECRelatedInstanceDirection direction);
    
    Utf8StringCR GetRelatedClassAlias() const {return m_relatedClassAlias;}
    void SetRelatedClassAlias(Utf8StringCR relatedClassAlias) const {m_relatedClassAlias = relatedClassAlias;}
    
    Utf8StringCR GetRelationshipClassAlias() const {return m_relationshipClassAlias;}
    void SetRelationshipClassAlias (Utf8StringCR relationshipClassAlias) const {m_relationshipClassAlias = relationshipClassAlias;}

public:
    ECRelatedClassSpecifier() : m_direction (ECN::ECRelatedInstanceDirection::Forward) {}

    ECRelatedClassSpecifier (ECN::ECRelationshipClassCR relationshipClass, ECN::ECClassCR relatedClass, ECN::ECRelatedInstanceDirection direction);

    //! @param [in] defaultSchema The schema containing the partly qualified classes in the path. Can be set to nullptr if all the class names 
    //! in relationship path are guaranteed to be fully qualified by the schema name.
    //! @see ToString()
    StatusInt InitFromString (Utf8StringCR relatedClassString, ECDbCR ecDb, ECN::ECSchemaCP defaultSchema);

    Utf8String ToString () const;

public:
    ECN::ECClassCP GetRelatedClass() const {return m_relatedClass;}
    ECN::ECRelationshipClassCP GetRelationshipClass() const {return m_relationshipClass;}
    ECN::ECRelatedInstanceDirection GetDirection() const {return m_direction;}

//__PUBLISH_SECTION_START__
};

//!======================================================================================
//! @bsiclass                                                 Ramanujam.Raman      01/2014
//! Utility to store paths between items, manipulate these paths by allowing to combine/reverse
//! them, and help generate ECSql that can query the path. 
//!===============+===============+===============+===============+===============+======
struct ECRelationshipPath
{
enum class End
    {
    Root,
    Leaf
    };

enum class SelectOptions
    {
    None,
    InstanceKeyOnly,
    EntireInstance
    };

// Output of ECSql generation
struct GeneratedEndInfo
    {
    friend struct ECRelationshipPath;
    private:
    Utf8String m_alias;
    Utf8String m_classIdExpression;
    Utf8String m_instanceIdExpression;

    public:
       Utf8StringCR GetAlias() const {return m_alias;}
       Utf8StringCR GetClassIdExpression() const {return m_classIdExpression;}
       Utf8StringCR GetInstanceIdExpression() const {return m_instanceIdExpression;}
    };
    
typedef GeneratedEndInfo& GeneratedEndInfoR;

private:
    ECN::ECClassCP m_rootClass;
    bvector<ECRelatedClassSpecifier> m_relatedClassSpecifiers;
    mutable Utf8String m_rootClassAlias; // Used only in ECSql generation

//__PUBLISH_SECTION_END__

private:
    void Clear();
    size_t GetLength() const;

    void CopyFrom (ECRelationshipPath const& other);
    bool ValidateConstraint (ECN::ECRelationshipConstraintCR constraint, ECN::ECClassCP checkClass) const;
    bvector<ECRelatedClassSpecifier>::const_iterator FindLastRelatedClass (const bvector<ECRelatedClassSpecifier>& relatedSpecifiers, ECN::ECClassCP ecClass);
    Utf8String FindNextAvailableAlias (ECN::ECClassCR ecClass, bvector<Utf8String>& allClassAliases) const;
    void SetupAliases() const;

public:
    //! Validates the specified relationship path
    bool Validate() const;

//__PUBLISH_SECTION_START__

public:
    //! Constructs an empty relationship path
    ECRelationshipPath() {}

    void SetEndClass (ECN::ECClassCR ecClass, End end);

    ECDB_EXPORT ECRelationshipPath (ECRelationshipPath const& other);

    ECDB_EXPORT ECRelationshipPath& operator= (ECRelationshipPath const& other);

    //! Initializes a new relationship path from the specified string. 
    //! @param relationshipPathString [in] Path as a string
    //! @param ecDb [in] ECDb
    //! @param defaultSchema [in] The schema containing the partly qualified classes in the path. Can be set to nullptr if 
    //! all the class names in relationship path are guaranteed to be fully qualified by the schema name.
    //! @remarks 
    //! <ul>
    //! <li> The path can be specified as RootClassName[.RelatedClassSpecifier1][.RelatedClassSpecifier2].., where 
    //!      RelatedClassSpecifier = RelationshipClassName:Direction:RelatedClassName, 
    //!      ClassName = [SchemaName:]ClassName, and
    //!      Direction = 0 (forward) or 1 (backward)
    //! <li> Note that the format is slightly different from the standard RelationshipPath format of ECObjects. The 
    //!      string needs to include the root class at the beginning of the path. 
    //! </ul>
    //! @see ToString()
    ECDB_EXPORT StatusInt InitFromString (Utf8StringCR relationshipPathString, ECDbCR ecDb, ECN::ECSchemaCP defaultSchema);

    //! Checks if the relationship path is empty
    //! @return true if empty. false otherwise. 
    ECDB_EXPORT bool IsEmpty() const;
    
    //! Gets the class at the specified end of the path.
    //! @return End class (can be nullptr)
    ECDB_EXPORT ECN::ECClassCP GetEndClass (End end) const;

    //! Determines if there is a "AnyClass" at the specified end
    //! @return true if the end has "AnyClass". false otherwise. 
    ECDB_EXPORT bool IsAnyClassAtEnd (End end) const;

    //! Generates the ECSql FROM and JOIN clauses to query based on the relationship path
    //! @param fromRootClause [out] Generated FROM clause
    //! @param joinClauses [out] Generated JOIN clauses
    //! @param rootInfo [out] Generated additional info on the root end to help with creating the SELECT clause. 
    //! @param leafInfo [in] Generated additional info on the leaf end to help with creating the WHERE clause. 
    //! @param isPolymorphic [in] Pass false to make non-polymorphic queries (i.e., adds "ONLY" to FROM clause). 
    //! @remarks 
    //! <ul>
    //! <li> Always generates ECSql to retrieve FROM the Root class, and JOINs to the other classes in the path
    //!      ending with the leaf class.
    //! <li> Use rootInfo, leafInfo fields to help setup the desired SELECT and WHERE clauses. 
    //! <li> In cases where the ECSQL has self joins, aliases are appropriately setup to resolve any ambiguity.
    //!      The information on the aliases used for the end classes is returned through rootInfo, leafInfo
    //! <li> If the Root class is "AnyClass", the first relationship class is setup in the FROM clause. The information on the       
    //!      expressions to retrieve the root instance keys are returned through rootInfo. 
    //! <li> If the Leaf class is "AnyClass", the last JOIN clause is setup with the last relationship class. The information 
    //!      on the expressions to retrieve the leaf instance keys are returned through leafInfo. 
    //! </ul>
    //!
    //! Example #1
    //! 
    //! Path: dgn:Element.  dgn:ElementHasPrimaryInstance:0:Plant:Pump. Plant:PumpHasSupport:0:Plant:Support
    //! fromRootClause: FROM dgn.Element
    //! joinClauses: 
    //!     JOIN dgn.ElementHasPrimaryInstance ON ElementHasPrimaryInstance.SourceECInstanceId = Element.ECInstanceId
    //!     JOIN Plant.Pump ON ElementHasPrimaryInstance.TargetECInstanceId = Pump.ECInstanceId
    //!     JOIN Plant.PumpHasSupport ON PumpHasSupport.SourceECInstanceId = Pump.ECInstanceId
    //!     JOIN Plant.Support ON PumpHasSupport.TargetECInstanceId = Support.ECInstanceId
    //! rootInfo
    //!     alias: Element
    //!     instanceIdExpression: Element.ECInstanceId
    //!     classIdExpression: Element.GetECClassId()
    //! leafInfo
    //!     alias: Support
    //!     instanceIdExpression: Support.ECInstanceId
    //!     classIdExpression: Support.GetECClassId()
    //! 
    //! Example #2
    //!
    //! Path: bsm:AnyClass. dgn:ElementHasPrimaryInstance:0:bsm:AnyClass. Plant:PumpHasSupport:0:bsm:AnyClass
    //! fromRootClause: FROM dgn.ElementHasPrimaryInstance 
    //! joinClauses: JOIN Plant.PumpHasSupport ON ElementHasPrimaryInstance.TargetECInstanceId = PumpHasSupport.SourceECInstanceId
    //! rootInfo
    //!     alias: ElementHasPrimaryInstance
    //!     instanceIdExpression: ElementHasPrimaryInstance.SourceECInstanceId
    //!     classIdExpression: ElementHasPrimaryInstance.SourceECClassId
    //! leafInfo
    //!     alias: PumpHasSupport
    //!     instanceIdExpression: PumpHasSupport.TargetECInstanceId,
    //!     classIdExpression: PumpHasSupport.TargetECClassId
    //! 
    //! Example #3
    //!
    //! Path: dgn:Element
    //! fromRootClause: FROM dgn.Element
    //! rootInfo
    //!     alias: Element
    //!     instanceIdExpression: Element.ECInstanceId
    //!     classIdExpression: Element.GetECClassId()
    //! leafInfo
    //!     alias: Element
    //!     instanceIdExpression: Element.ECInstanceId
    //!     classIdExpression: Element.GetECClassId()
    //! 
    //! Examples of invalid paths
    //! 
    //! dgn:ElementHasPrimaryInstance:0:Plant:Pump (no root class specified)
    //! dgn:Element.dgn:ElementHasPrimaryInstance:bsm:AnyClass (no direction specifier)
    //! Plant:Pump.dgn:ElementHasPrimaryInstance:1 (no related class)
    //! 
    //! @see Reverse() to flip the direction of traversal.
    ECDB_EXPORT StatusInt GenerateECSql 
        (
        Utf8StringR fromClause, 
        Utf8StringR joinClause,
        ECRelationshipPath::GeneratedEndInfoR rootInfo,
        ECRelationshipPath::GeneratedEndInfoR leafInfo,
        bool isPolymorphic
        ) const;

    //! Creates a string equivalent of the path. 
    //! @return The string equivalent of the path
    //! @remarks All class names in the resulting string are fully qualified with the
    //! schema name.
    ECDB_EXPORT Utf8String ToString() const;

    //! Reverses the path
    //! @param reversedPath [out] The reversed path. 
    ECDB_EXPORT void Reverse (ECRelationshipPath& reversedPath) const;

    //! Combines the specified path to the (leaf) end of this path. 
    //! @param pathToCombine [in] Path to ccmbine
    //! @return ERROR if the leaf end of the current path does not match
    //! the root end of the path to be combined. SUCCESS otherwise. 
    ECDB_EXPORT StatusInt Combine (ECRelationshipPath const& pathToCombine);

    //! Replaces the class at the end if it's originally specified as AnyClass. 
    //! @return ERROR if the existing path does not have AnyClass at the specified end. 
    //! Returns SUCCESS otherwise. 
    ECDB_EXPORT StatusInt ReplaceAnyClassAtEnd (ECN::ECClassCR replacementClass, End end);
};

//__PUBLISH_SECTION_END__

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

typedef bvector<ECRelationshipPath> ECRelationshipPathVector;
typedef const ECRelationshipPathVector& ECRelationshipPathVectorCR;
typedef ECRelationshipPathVector& ECRelationshipPathVectorR;
typedef bmap<ECN::ECClassCP, ECRelationshipPathVector> ECRelationshipPathVectorByClass;
typedef bpair<ECN::ECClassCP, ECRelationshipPathVector> ECRelationshipPathVectorByClassPair;

//======================================================================================
// @bsiclass                                             Ramanujam.Raman      01 / 2014
//===============+===============+===============+===============+===============+======
struct ECRelatedItemsDisplaySpecificationsCache : public BeSQLite::DbAppData
{
private:
    ECDbCR m_ecDb;
    ECRelationshipPathVectorByClass m_pathsByClass;

    ECRelatedItemsDisplaySpecificationsCache (ECDbCR ecDb) : m_ecDb (ecDb) {}

    virtual void _OnCleanup (BeSQLite::DbR host) override {delete this;}

    static BeSQLite::DbAppData::Key const& GetKey() {static BeSQLite::DbAppData::Key s_key; return s_key;}

    static ECN::ECClassCP GetRelatedSpecificationsClass (ECDbCR ecDb);

    BentleyStatus ExtractFromCustomAttribute
        (
        ECN::IECInstanceCR customAttributeSpecification,
        ECN::ECSchemaCR customAttributeContainerSchema
        );

    BentleyStatus Initialize ();
    void AddPathToCache (ECRelationshipPathCR path);

public:
    virtual ~ECRelatedItemsDisplaySpecificationsCache () {}

    static ECRelatedItemsDisplaySpecificationsCache* GetCache (ECDbCR host);

    bool TryGetRelatedPathsFromClass (ECRelationshipPathVectorR relationshipPathVec, ECN::ECClassCR ecClass) const;
};

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

//__PUBLISH_SECTION_START__

END_BENTLEY_SQLITE_EC_NAMESPACE

#endif
//__PUBLISH_SECTION_END__

