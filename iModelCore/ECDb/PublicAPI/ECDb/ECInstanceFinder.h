/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECDb/ECInstanceFinder.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <ECDb/ECSqlStatement.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

typedef bmultimap<ECN::ECClassId, ECInstanceId> ECInstanceKeyMultiMap;
typedef ECInstanceKeyMultiMap::value_type        ECInstanceKeyMultiMapPair;
typedef ECInstanceKeyMultiMap::iterator          ECInstanceKeyMultiMapIterator;
typedef ECInstanceKeyMultiMap::const_iterator    ECInstanceKeyMultiMapConstIterator;

//======================================================================================
//! Utilty to find instances (keys) starting with one or more seed instances and traversing
//! the graph of relationships. 
//! @remarks The class serves to cache some state (e.g., relevant relationships on a class) 
//! to avoid repeated lookups. 
//! @ingroup ECDbGroup
// @bsiclass                                                 Ramanujam.Raman      03/2013
//+===============+===============+===============+===============+===============+======
struct ECInstanceFinder
{
    /*=================================================================================**//**
    * @bsiclass                                                 Ramanujam.Raman      09/2013
    * @see ECInstanceFinder::FindOptions
    +===============+===============+===============+===============+===============+======*/
    enum RelatedDirection
        {
        RelatedDirection_None               = 0,
        RelatedDirection_Referencing        = (1 << 0),
        RelatedDirection_EmbeddedChildren   = (1 << 1),
        RelatedDirection_EmbeddingParent    = (1 << 2),
        RelatedDirection_HeldChildren       = (1 << 3),
        RelatedDirection_HoldingParents     = (1 << 4),
        RelatedDirection_All                = RelatedDirection_Referencing | RelatedDirection_EmbeddedChildren |
                                              RelatedDirection_EmbeddingParent | RelatedDirection_HeldChildren |
                                              RelatedDirection_HoldingParents
        };

    /*=================================================================================**//**
    //! Options to gather instances and related instances.  
    //! @bsiclass                                              Ramanujam.Raman      09/2013
    +===============+===============+===============+===============+===============+======*/
    struct FindOptions 
        {
        friend ECInstanceFinder;
        private:
            int m_relatedDirections;
            uint8_t m_relationshipDepth;
            ECN::ECClassCP m_ecClass;

        public:
            //! Options to gather instances, related instances, and relationship instances between them 
            //! @param[in] relatedDirectionFlags Identifies directions of related instances to be gathered. 
            //!            See @ref ECInstanceFinder::RelatedDirection for the enumeration flags that can be combined bitwise.
            //! @param[in] relationshipDepth Maximum depth of relationships to traverse. Consider limiting this if performance
            //!            becomes an issue. Pass UINT8_MAX if you need to traverse the entire hierarchy of the specified 
            //!            relationships. 
            //! @param[in] ecClass Identifies the class for which the instance needs to be gathered. This is only applicable 
            //!            if the finder is used with a @ref ECSqlStatement, and when each row returned from the statement can 
            //!            contain columns from multiple classes. Defaults to nullptr. 
            //! @see ECInstanceFinder::RelatedDirection
            ECDB_EXPORT FindOptions 
                        (
                        int relatedDirectionFlags = RelatedDirection_None, 
                        uint8_t relationshipDepth = 0,
                        ECN::ECClassCP ecClass = nullptr
                        );
        };

#if !defined (DOCUMENTATION_GENERATOR)
    //======================================================================================
    // @bsiclass                                                 Ramanujam.Raman      03/2013
    //+===============+===============+===============+===============+===============+======
    struct QueryableRelationship
        {
        private:
            ECN::ECRelationshipClassCP m_relationshipClass;
            ECN::ECClassCP m_thisClass;
            ECN::ECRelationshipEnd m_thisRelationshipEnd;
            RelatedDirection m_relatedDirection;
            std::shared_ptr<ECSqlStatement> m_cachedStatement;

            void CopyFrom (const QueryableRelationship& copyFrom);
            void InitializeRelatedDirection();
            ECSqlStatus PrepareECSqlStatement (ECDbCR ecDb);
        public:
            QueryableRelationship 
                (
                ECN::ECRelationshipClassCR relationshipClass, 
                ECN::ECClassCR thisClass, 
                ECN::ECRelationshipEnd thisRelationshipEnd
                );

            QueryableRelationship (const QueryableRelationship& copyFrom);

            QueryableRelationship& operator= (const QueryableRelationship& copyFrom);

            ECSqlStatus GetPreparedECSqlStatement (std::shared_ptr<ECSqlStatement>& cachedStatement, ECDbCR ecDb);

            Utf8String ToString();

            ECN::ECRelationshipClassCP GetRelationshipClass() const {return m_relationshipClass;}
            RelatedDirection GetRelatedDirection() const {return m_relatedDirection;}
            ECN::ECRelationshipEnd GetRelationshipEnd() const {return m_thisRelationshipEnd;}
        };

    typedef bvector<QueryableRelationship> QueryableRelationshipVector;
    typedef bvector<QueryableRelationship>* QueryableRelationshipVectorP;
    typedef bvector<QueryableRelationship>& QueryableRelationshipVectorR;
    typedef const bvector<QueryableRelationship>& QueryableRelationshipVectorCR;
    typedef bmap<ECN::ECClassId, QueryableRelationshipVector> QueryableRelationshipsByClass;
#endif
private:
    QueryableRelationshipsByClass m_queryableRelationshipsByClass;

    static void FindEndClasses (bset<ECN::ECClassId>& endClassIds, ECN::ECClassId relationshipClassId, ECN::ECRelationshipEnd relationshipEnd, ECDbCR ecDb);
    static DbResult FindRelationshipsOnEnd (QueryableRelationshipVector& queryableRelationships, ECN::ECClassId thisEndClassId, ECDbCR ecDb);
    DbResult GetRelationshipsOnEnd (QueryableRelationshipVectorP &queryableRelationships, ECN::ECClassId thisEndClassId);

    BentleyStatus FindInstancesRecursive 
        (
        ECInstanceKeyMultiMap& instanceKeyMap, 
        const ECInstanceKeyMultiMap& seedInstanceKeyMap, 
        ECInstanceFinder::FindOptions findOptions,
        uint8_t& currentDepth
        );
    static void DumpInstanceKeyMap (const ECInstanceKeyMultiMap& instanceKeyMultiMap, ECDbCR ecDb);

private:
    ECDbCR m_ecDb;
    ECInstanceFinder (ECInstanceFinder const& other) : m_ecDb (other.m_ecDb) {}
    ECInstanceFinder& operator= (ECInstanceFinder const&) {return *this;}

public:
    //! Creates a new instance of ECInstanceFinder
    //! @remarks Holds some cached state to speed up future queries, including prepared statements to traverse
    //! relationships. 
    //! @see Finalize
    ECDB_EXPORT explicit ECInstanceFinder (ECDbCR ecDb);

    //! Gathers instances starting with the seed instances, recursively traversing any specified relationships
    //! @param[out] instanceKeyMap All instances found organized as a multi-map of ECClassId-s & ECInstanceId-s. 
    //! Any existing entries are cleared first. 
    //! @param[in] seedInstanceKeyMap Input seed instances
    //! @param[in] findOptions Options to find instances and the related instances
    //! @return SUCCESS or ERROR 
    ECDB_EXPORT BentleyStatus FindInstances
        (
        ECInstanceKeyMultiMap& instanceKeyMap, 
        const ECInstanceKeyMultiMap& seedInstanceKeyMap, 
        FindOptions findOptions
        );

    //! Finds instances related to the specified seed instances. 
    //! @param[out] relatedInstanceKeyMap (Optional) Related instances organized as a multi-map of ECClassId-s & ECInstanceId-s. 
    //! Any existing entries are cleared first. 
    //! @param[out] relationshipInstanceKeyMap (Optional) Corresponding relationship instances organized as a multi-map of ECClassId-s & ECInstanceId-s. 
    //! Any existing entries are cleared first. 
    //! @param[in] seedInstanceKeyMap Input seed instances
    //! @param[in] findRelatedDirections Identifies directions of related instances to be gathered. 
    //!            See @ref ECInstanceFinder::RelatedDirection for the enumeration flags that can be combined bitwise.
    //! @return SUCCESS or ERROR
    ECDB_EXPORT BentleyStatus FindRelatedInstances 
        (
        ECInstanceKeyMultiMap* relatedInstanceKeyMap, 
        ECInstanceKeyMultiMap* relationshipInstanceKeyMap, 
        const ECInstanceKeyMultiMap& seedInstanceKeyMap,
        int findRelatedDirections
        );

    //! Finds instances related to the specified seed instance. 
    //! @param[out] relatedInstanceKeyMap (Optional) Related instances organized as a multi-map of ECClassId-s & ECInstanceId-s. 
    //! Any existing entries are cleared first. 
    //! @param[out] relationshipInstanceKeyMap (Optional) Corresponding relationship instances organized as a multi-map of ECClassId-s & ECInstanceId-s. 
    //! Any existing entries are cleared first. 
    //! @param[in] seedInstanceKey Input seed instance
    //! @param[in] findRelatedDirections Identifies directions of related instances to be gathered. 
    //!            See @ref ECInstanceFinder::RelatedDirection for the enumeration flags that can be combined bitwise.
    //! @return SUCCESS or ERROR
    ECDB_EXPORT BentleyStatus FindRelatedInstances 
        (
        ECInstanceKeyMultiMap* relatedInstanceKeyMap, 
        ECInstanceKeyMultiMap* relationshipInstanceKeyMap, 
        const ECInstanceKey seedInstanceKey,
        int findRelatedDirections
        );

    //! Finalizes all the held prepared statements. 
    //! Note that finalization needs to be done before the Db is closed. Destroying the ECInstanceFinder automatically 
    //! causes finalization. 
    ECDB_EXPORT void Finalize();

};

END_BENTLEY_SQLITE_EC_NAMESPACE
