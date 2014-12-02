/*--------------------------------------------------------------------------------------+
|
|  $Source: DgnHandlers/ElementComparer.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnHandlers/ElementComparer.h>

//Include these?
#include <sstream>
#include <algorithm>

using std::min;

//=======================================================================================
//! @bsimethod                                                     Jeff Linahan     06/11
//======================================================================================= 
bool ElementComparer::SameClass(IECInstanceCR left, IECInstanceCR right) 
    {
    return &left.GetClass() == &right.GetClass();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Jeff Linahan  06/11
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementComparer::IterateProperties(InstanceDiff* record, ECValuesCollectionCR col, IECInstanceCR left, IECInstanceCR right)
    {
    bool same = true;
    FOR_EACH(ECPropertyValueCR p, col)
        {
        ECValueAccessorCR   accessor  = p.GetValueAccessor();

        ECValueCR           leftValue = p.GetValue();
        ECValue             rightValue;
        right.GetValueUsingAccessor (rightValue, accessor);

        if ( ! leftValue.IsStruct() && ! leftValue.Equals(rightValue))
            if (record)
                {
                record->propertyDiffList.push_back(accessor);
                same = false;
                }
            else
                {
                return false;
                }

        if (p.HasChildValues() && IterateProperties(record, *p.GetChildValues(), left, right))
            if (record)
                same = false;
            else
                return false;
        }
    return same;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Jeff Linahan  06/11
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementComparer::CompareInstances (InstanceDiff* record, IECInstanceR left, IECInstanceR right)
    {
    BeAssert(SameClass(left, right));

    if (record)
        {   
        record->left  = &left;
        record->right = &right;
        } 

    ECValuesCollectionPtr col = ECValuesCollection::Create(left);
    return IterateProperties(record, *col, left, right);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Jeff Linahan  06/11
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementComparer::CompareBuckets (BucketDiff* record, Bucket const& b1, Bucket const& b2)
    {
    if (record)
        {
        record->myclass  = &(b1.size() > 0 ? b1[0]->GetClass() : b2[0]->GetClass());
        record->numLeft  = b1.size();
        record->numRight = b2.size();
        }

    bool same = b1.size() == b2.size();

    size_t minsize = min(b1.size(), b2.size());
    for (size_t i = 0; i < minsize; ++i)
        {
        InstanceDiff d;
        if ( ! CompareInstances(record ? &d : NULL, *b1[i], *b2[i]))
            {
            if ( ! record)
                return false;

            same = false;
            record->instanceDiffList.push_back(d);
            }
        }
    return same;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Jeff Linahan  06/11
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementComparer::CompareInstanceLists (TotalDiff* record, InstanceList const& list1, InstanceList const& list2)
    {
    bset<ECClassCP> classesSeen;
    
    // sort left instances into buckets
    BucketMap leftBucketMap;
    FOR_EACH(IECInstancePtr p, list1)
        {
        ECClassCP c = &p->GetClass();
        classesSeen.insert(c);
        leftBucketMap[c].push_back(p);
        }

    // sort right instances into buckets
    BucketMap rightBucketMap;
    FOR_EACH(IECInstancePtr p, list2)
        {
        ECClassCP c = &p->GetClass();
        classesSeen.insert(c);
        rightBucketMap[c].push_back(p);
        }

    // for each class seen, get its two respective buckets and diff them
    bool same = true;
    FOR_EACH(ECClassCP c, classesSeen)
        {
        Bucket const& leftBucket  =  leftBucketMap[c];
        Bucket const& rightBucket = rightBucketMap[c];
       
        BucketDiff d;
        if ( ! CompareBuckets(record ? &d : NULL, leftBucket, rightBucket))
            {
            if ( ! record)
                return false;

            same = false;
            record->bucketDiffList.push_back(d);
            }
        }
    return same;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Jeff Linahan  06/11
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementComparer::CompareElements (TotalDiff* record, ElementHandleCR leftElement, ElementHandleCR rightElement)
    {
    BeAssert(false && "Not implemented in Graphite");
    return false;
#if WIP_DEAD_DGNEC_CODE
    FindInstancesScopePtr   leftScope     = FindInstancesScope::CreateScope (leftElement, true);
    FindInstancesScopePtr   rightScope    = FindInstancesScope::CreateScope (rightElement, true);
    ECQueryPtr              query         = ECQuery::CreateQuery (ECQUERY_INIT_SearchAllClasses);

    InstanceList leftList; 
    FOR_EACH(IECInstancePtr p, DgnECManager::GetManager().FindInstances (*leftScope, *query))
        leftList.push_back(p);

    InstanceList rightList; 
    FOR_EACH(IECInstancePtr p, DgnECManager::GetManager().FindInstances (*rightScope, *query))
        rightList.push_back(p);

    return CompareInstanceLists(record, leftList, rightList);
#endif
    }


