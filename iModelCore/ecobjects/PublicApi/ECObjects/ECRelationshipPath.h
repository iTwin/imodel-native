/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECRelationshipPath.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "ECSchema.h"
#include <Bentley/WString.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

//! @addtogroup ECObjectsGroup
//! @beginGroup

//=================================================================================
//! @remarks Classes are stored and retrieved as "SchemaName.ClassName". The string conversion 
//!  utilities convert from "[SchemaName:]ClassName". 
//!  The modified storage format allows for their use in ECSQL as-is. The equivalent strings 
//!  maintain compatibility with older versions of ECObjects.
//  @bsiclass                                                 Ramanujam.Raman      01/2014
// +===============+===============+===============+===============+===============+======
struct ECRelatedClassSpecifier
    {
    friend struct ECRelationshipPath;
    private:
        ECRelationshipClassCP m_relationshipClass;
        ECClassCP m_relatedClass;
        ECRelatedInstanceDirection m_direction;

        mutable Utf8String m_relationshipClassAlias; // Used only in ECSql generation
        mutable Utf8String m_relatedClassAlias; // Used only in ECSql generation

        Utf8StringCR GetRelatedClassAlias() const { return m_relatedClassAlias; }
        void SetRelatedClassAlias(Utf8StringCR relatedClassAlias) const { m_relatedClassAlias = relatedClassAlias; }

        Utf8StringCR GetRelationshipClassAlias() const { return m_relationshipClassAlias; }
        void SetRelationshipClassAlias(Utf8StringCR relationshipClassAlias) const { m_relationshipClassAlias = relationshipClassAlias; }

    public:
        //! Constructor
        ECRelatedClassSpecifier();

        //! Constructor
        ECRelatedClassSpecifier(ECRelationshipClassCR relationshipClass, ECClassCR relatedClass, ECRelatedInstanceDirection direction);

        //! Initializes a related class specifier from the specified string. 
        //! @param relatedClassString [in] Specifier as a string
        //! @param classLocater [in] Class locater to look up classes from class names in the string
        //! @param [in] defaultSchema The schema containing the partly qualified classes in the path. Can be set to nullptr if all the class names 
        //! in relationship path are guaranteed to be fully qualified by the schema name.
        //! @see ToString()
        BentleyStatus InitFromString(Utf8StringCR relatedClassString, IECClassLocaterR classLocater, ECN::ECSchemaCP defaultSchema);

        //! Convert the relationship path to its string equivalent
        Utf8String ToString() const;

        //! Get the related class
        ECClassCP GetRelatedClass() const { return m_relatedClass; }

        //! Get the relationship class
        ECRelationshipClassCP GetRelationshipClass() const { return m_relationshipClass; }

        //! Get the direction the relationship needs to be traversed
        ECRelatedInstanceDirection GetDirection() const { return m_direction; }
    };

//======================================================================================
//! Utility to store paths between items, manipulate these paths by allowing to combine/reverse
//! them, and help generate ECSQL that can query the path. 
// @bsiclass                                                 Ramanujam.Raman      01/2014
//===============+===============+===============+===============+===============+======
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
        Utf8StringCR GetAlias() const { return m_alias; }
        Utf8StringCR GetClassIdExpression() const { return m_classIdExpression; }
        Utf8StringCR GetInstanceIdExpression() const { return m_instanceIdExpression; }
    };

private:
    ECN::ECClassCP m_rootClass;
    bvector<ECRelatedClassSpecifier> m_relatedClassSpecifiers;
    mutable Utf8String m_rootClassAlias; // Used only in ECSql generation

    void Clear();

    void CopyFrom(ECRelationshipPath const& other);
    bool ValidateConstraint(ECN::ECRelationshipConstraintCR constraint, ECN::ECClassCP checkClass) const;
    Utf8String FindNextAvailableAlias(ECClassCR ecClass, bvector<Utf8String>& allClassAliases) const;
    void SetupAliases() const;

    static bool IsAnyClass(ECClassCR);
    static bool AreInSameHierarchy(ECN::ECClassCP& moreDerivedClass, ECN::ECClassCP inClass1, ECN::ECClassCP inClass2);
    static bool FindLastMatchingClass(ECN::ECClassCP& foundClass, bvector<ECRelatedClassSpecifier>::const_iterator& foundIter, ECRelationshipPath const& searchPath, ECN::ECClassCP searchClass);

public:
    //! Constructs an empty relationship path
    ECRelationshipPath() {}

    ~ECRelationshipPath() {}

    //! Copy constructor
    ECOBJECTS_EXPORT ECRelationshipPath(ECRelationshipPath const& other);

    //! Assignment operator
    ECOBJECTS_EXPORT ECRelationshipPath& operator= (ECRelationshipPath const& other);

    //! Initializes a new relationship path from the specified string. 
    //! @param relationshipPathString [in] Path as a string
    //! @param classLocater [in] Class locater to look up classes from class names in the string
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
    ECOBJECTS_EXPORT BentleyStatus InitFromString(Utf8StringCR relationshipPathString, IECClassLocaterR classLocater, ECN::ECSchemaCP defaultSchema);

    //! Checks if the relationship path is empty
    //! @return true if empty. false otherwise. 
    ECOBJECTS_EXPORT bool IsEmpty() const;

    //! Gets the class at the specified end of the path.
    //! @return End class (can be nullptr)
    ECOBJECTS_EXPORT ECClassCP GetEndClass(End end) const;

    //! Generates the ECSql FROM and JOIN clauses to query based on the relationship path
    //! @param fromClause [out] Generated FROM clause
    //! @param joinClause [out] Generated JOIN clause
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
    ECOBJECTS_EXPORT BentleyStatus GenerateECSql(Utf8StringR fromClause, Utf8StringR joinClause, ECRelationshipPath::GeneratedEndInfo& rootInfo, ECRelationshipPath::GeneratedEndInfo& leafInfo, bool isPolymorphic) const;

    //! Creates a string equivalent of the path. 
    //! @return The string equivalent of the path
    //! @remarks All class names in the resulting string are fully qualified with the
    //! schema name.
    ECOBJECTS_EXPORT Utf8String ToString() const;

    //! Reverses the path
    //! @param reversedPath [out] The reversed path. 
    ECOBJECTS_EXPORT void Reverse(ECRelationshipPath& reversedPath) const;

    //! Combines the specified path to the (leaf) end of this path. 
    //! @param pathToCombine [in] Path to combine
    //! @return ERROR if the leaf end of the current path does not match
    //! the root end of the path to be combined. SUCCESS otherwise. 
    ECOBJECTS_EXPORT BentleyStatus Combine(ECRelationshipPath const& pathToCombine);

    //! Set the end class of the relationship path
    ECOBJECTS_EXPORT void SetEndClass(ECClassCR, End);

    //! Validates the specified relationship path
    ECOBJECTS_EXPORT bool Validate() const;

#if 0
    // TODO: Remove these methods after ensuring they are not needed -- wait until Navigator is 
    // completely ported to DgnDb0601Dev. 
    
    //! Determines if there is a "AnyClass" at the specified end
    //! @return true if the end has "AnyClass". false otherwise. 
    ECOBJECTS_EXPORT bool IsAnyClassAtEnd(End end) const;

    //! Get the number of related class specifiers in the path
    size_t GetRelatedClassSpecifierCount() const { return m_relatedClassSpecifiers.size(); }

    //! Gets the related class specifier at the specified index. 
    ECRelatedClassSpecifier const* GetRelatedClassSpecifier(size_t index) const { return (index >= m_relatedClassSpecifiers.size()) ? nullptr : &m_relatedClassSpecifiers[index]; }

    //! Removes leaf entries in the relationship path starting with the specified index
    BentleyStatus TrimLeafEnd(size_t relatedClassSpecifierIndex);

    //! Replaces the class at the end if it's originally specified as AnyClass. 
    //! @return ERROR if the existing path does not have AnyClass at the specified end. 
    //! Returns SUCCESS otherwise. 
    ECOBJECTS_EXPORT BentleyStatus ReplaceAnyClassAtEnd(ECN::ECClassCR replacementClass, End end);
#endif

};

END_BENTLEY_ECOBJECT_NAMESPACE


