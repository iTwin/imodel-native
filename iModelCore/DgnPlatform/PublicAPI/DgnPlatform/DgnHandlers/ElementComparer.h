/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/ElementComparer.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

//! @cond DONTINCLUDEINDOC
#if defined (_MSC_VER)
    #pragma managed(push, off)
#endif // defined (_MSC_VER)

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

typedef bvector<ECN::IECInstancePtr> InstanceList;
typedef InstanceList                Bucket;     // a bucket is an instance list where all instances are of the same class
typedef bmap<ECN::ECClassCP, Bucket> BucketMap;

typedef ECN::ECValueAccessor         PropertyDiff;

typedef bvector<PropertyDiff>       PropertyDiffList;

//=======================================================================================
//! @bsistruct                                                     Jeff Linahan     06/11
//=======================================================================================
struct InstanceDiff
{
private:
    ECN::IECInstancePtr left;
    ECN::IECInstancePtr right;
    PropertyDiffList   propertyDiffList;
    friend struct ElementComparer;

public:
    ECN::IECInstancePtr      GetLeft() const              {return left;}
    ECN::IECInstancePtr      GetRight() const             {return right;}
    PropertyDiffList const& GetPropertyDiffList() const  {return propertyDiffList;}
};

typedef bvector<InstanceDiff> InstanceDiffList;

//=======================================================================================
//! @bsistruct                                                     Jeff Linahan     06/11
//=======================================================================================
struct BucketDiff
{
private:
    BucketDiff() : numLeft(0), numRight(0), myclass(NULL) {}

    ECN::ECClassCP    myclass;
    size_t           numLeft;
    size_t           numRight;
    InstanceDiffList instanceDiffList;
    friend struct ElementComparer;

public:
    ECN::ECClassCP           GetClass() const            {return myclass;}
    size_t                  GetNumLeft() const          {return numLeft;}
    size_t                  GetNumRight() const         {return numRight;}
    InstanceDiffList const& GetInstanceDiffList() const {return instanceDiffList;}
};

typedef bvector<BucketDiff> BucketDiffList;

//=======================================================================================
//! @bsistruct                                                     Jeff Linahan     06/11
//=======================================================================================
struct TotalDiff
{
private:
    BucketDiffList bucketDiffList;
    friend struct ElementComparer;

public:
    BucketDiffList GetBucketDiffList() const {return bucketDiffList;}
};

//=======================================================================================
//! @bsistruct                                                     Jeff Linahan     06/11
//=======================================================================================
struct ElementComparer
    {
private:

    static bool SameClass (ECN::IECInstanceCR left, ECN::IECInstanceCR right);
    static bool IterateProperties(InstanceDiff* record, ECN::ECValuesCollectionCR col, ECN::IECInstanceCR left, ECN::IECInstanceCR right);

public:
    DGNPLATFORM_EXPORT static bool CompareInstances     (InstanceDiff* record, ECN::IECInstanceR   left,   ECN::IECInstanceR    right);
    DGNPLATFORM_EXPORT static bool CompareBuckets       (BucketDiff*   record, Bucket const&       b1,    Bucket const&       b2);
    DGNPLATFORM_EXPORT static bool CompareInstanceLists (TotalDiff*    record, InstanceList const& list1, InstanceList const& list2);
    DGNPLATFORM_EXPORT static bool CompareElements      (TotalDiff*    record, ElementHandleCR     elem1, ElementHandleCR     elem2);
    };

//__PUBLISH_SECTION_START__
END_BENTLEY_DGNPLATFORM_NAMESPACE

#if defined (_MSC_VER)
    #pragma managed(pop)
#endif // defined (_MSC_VER)

//! @endcond
