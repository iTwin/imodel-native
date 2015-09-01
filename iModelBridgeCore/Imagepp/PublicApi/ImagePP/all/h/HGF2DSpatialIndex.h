//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DSpatialIndex.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once


#include "HGF2DLiteExtent.h"
#include "HGF2DTemplateExtent.h"
#include "HFCPtr.h"
#include "HGF2DPosition.h"
//#include "HVE2DGenFacet.h"

#include "ZLib.h"

BEGIN_IMAGEPP_NAMESPACE

#define DefaultTreshold (10)
#define MinimalTreshold (3)

template <typename DataType> class FastVector
    {
public:
    //class iterator
    //{
    //    m_currentItem = 0;
    //    m_memory = null;
    //    operator*() return {m_memory[m_currentItem]);

    //}

    FastVector ()
        {
        m_allocatedCount = 0;
        m_count = 0;
        m_memory = NULL;
        }
    ~FastVector()
        {
        if (m_allocatedCount > 0)
            delete m_memory;
        }
    bool reserve(size_t newCount)
        {
        if (newCount > m_allocatedCount)
            {
            void* newMemory = new DataType[newCount];
            if (newMemory == NULL)
                return false;
            memcpy (newMemory, m_memory, sizeof(DataType)*m_count);
            delete [] m_memory;
            m_memory = reinterpret_cast<DataType*>(newMemory);
            m_allocatedCount = newCount;
            }
        return true;
        }
    size_t capacity() const
        {
        return m_allocatedCount;
        }
    bool push_back(const DataType& newObject)
        {
        if((m_allocatedCount <= m_count) && !reserve (m_count + 1))
            return false;

        m_memory[m_count] = newObject;
        m_count++;
        return true;
        }
    bool push_back(DataType* newObjects, size_t count)
        {
        íf (m_allocatedCount <= (m_count + count))
        if (!reserve (m_count + count))
            return false;

        memcpy (&(m_memory[m_count]), newObjects, sizeof(DataType) * count);
//        for (size_t idx = 0 ; idx < count ; idx++)
//        {
//            m_memory[m_count] = newObjects[idx];
//            m_count++;
//        }
        return true;
        }
    DataType& operator[](size_t index) const
        {
        HPRECONDITION(index < m_count);
        return m_memory[index];
        }
    size_t size() const
        {
        return m_count;
        }
    void erase (size_t index)
        {
        if (index < m_count - 1)
            memcpy(&(m_memory[index]), &(m_memory[index+1]), sizeof(DataType) * (m_count - 1 - index));
        m_count--;
        }
    void clear()
        {
        if (m_allocatedCount > 0)
            delete [] m_memory;
        m_count = 0;
        m_allocatedCount = 0;
        }
private:
    DataType* m_memory;
    size_t m_count;
    size_t m_allocatedCount;

    };


template <typename DataType> class FastCompressiblePooledVector;
// This class is meant to become generic. It currently is specialised as a FastCompressiblePooledVector
// for prototyping purposes.
template <typename DataType> class FastVectorCountLimitedPool
    {
public:
    FastVectorCountLimitedPool(size_t countLimit)
        {
        m_countLimit = countLimit;
        m_totalUsedCount = 0;
        }

    void Allocate (size_t Count, const FastCompressiblePooledVector<DataType>* poolItem)
        {
        HPRECONDITION (!poolItem->GetPoolManaged());

        // Check if memory limit attained
        while (m_totalUsedCount + Count > m_countLimit)
            {
            // Not enough memory ... some other must be released
            // Request the least used uncompressed FastCompressiblePooledVector
            // to compress
// Critical section
            m_Pool.back()->Discard();
            // m_Pool.pop_back();

//                      FastCompressiblePooledVector* itemToCompress = *itemIterator;
//                      m_Pool.erase (itemIterator);
//                      itemToCompress->Compress();
// end critical section
            }

        poolItem->SetMemory(new DataType[Count], Count);
        m_totalUsedCount += Count;

        m_Pool.push_front(poolItem);
        poolItem->SetPoolIterator (m_Pool.begin());
        poolItem->SetPoolManaged(true);
        }

    void Reallocate (size_t NewCount, const FastCompressiblePooledVector<DataType>* poolItem)
        {
        // This call will bring item to front and insure item is not discarded during the process
        NotifyAccess (poolItem);

        // Check if memory limit attained
        while (m_totalUsedCount + NewCount - poolItem->GetMemoryCount() > m_countLimit)
            {
            // Not enough memory ... some other must be released
            // Request the least used uncompressed FastCompressiblePooledVector
            // to compress
// Critical section
            m_Pool.back()->Discard();
            // m_Pool.pop_back();
// end critical section
            }

        void* newMemory = new DataType[NewCount];
        memcpy (newMemory, poolItem->GetMemory(), MIN (NewCount, poolItem->GetMemoryCount()) *sizeof(DataType));
        m_totalUsedCount -= poolItem->GetMemoryCount();
        delete [] poolItem->GetMemory();
        poolItem->SetMemory (reinterpret_cast<DataType*>(newMemory), NewCount);
        m_totalUsedCount += NewCount;
        }


    bool Free (const FastCompressiblePooledVector<DataType>* poolItem)
        {
        if (poolItem->GetPoolManaged())
            {
// Critical section
            m_totalUsedCount -= poolItem->GetMemoryCount();
            delete [] poolItem->GetMemory();
            poolItem->SetMemory(NULL, 0);
            m_Pool.erase (poolItem->GetPoolIterator());
            poolItem->SetPoolManaged (false);
            }
// end critical section
        return true;
        }



    bool NotifyAccess (const FastCompressiblePooledVector<DataType>* poolItem)
        {
        HPRECONDITION (poolItem->GetPoolManaged());

        // Check if the item is the one on front
        if (*m_Pool.begin() != *poolItem->GetPoolIterator())
            {
            // Not the one on front ...
// Critical section ...
            m_Pool.erase (poolItem->GetPoolIterator());
            m_Pool.push_front(poolItem);
            poolItem->SetPoolIterator(m_Pool.begin());
// End critical section
            }
        return true;
        }

private:
    list<const FastCompressiblePooledVector<DataType>*> m_Pool;
    size_t m_countLimit;
    size_t m_totalUsedCount;

    };

// The present fast vector is simply a vector class (with limited interface) that can compress
// its data according to memory availability. This memory availability maintenance is provided by given pool
template <typename DataType> class FastCompressiblePooledVector
    {
public:
    //class iterator
    //{
    //    m_currentItem = 0;
    //    m_memory = null;
    //    operator*() return {m_memory[m_currentItem]);

    //}

    FastCompressiblePooledVector (FastVectorCountLimitedPool<DataType>* pool)
        {
        m_allocatedCount = 0;
        m_count = 0;
        m_memory = NULL;
        m_pool = pool;
        m_poolIterator;
        m_poolManaged = false;
        m_compressedMemory = NULL;
        m_compressedAllocatedCount = 0;
        m_accessCount=0;
        }
    ~FastCompressiblePooledVector()
        {
        if (m_allocatedCount > 0)
            m_pool->Free(this);
        }

    bool GetPoolManaged()const
        {
        return m_poolManaged;
        }

    void SetPoolManaged(bool poolManaged) const // Intentionaly const ... only mutable members are modified
        {
        m_poolManaged = poolManaged;
        }

    typename std::list<const FastCompressiblePooledVector<DataType>* >::iterator GetPoolIterator() const
        {
        return m_poolIterator;
        }

    void SetPoolIterator(typename std::list<const FastCompressiblePooledVector<DataType>* >::iterator poolIterator) const // Intentionaly const ... only mutable members are modified
        {
        m_poolIterator = poolIterator;
        }

    FastVectorCountLimitedPool<DataType>* GetPool() const
        {
        return m_pool;
        }

    DataType* GetMemory() const
        {
        return m_memory;
        }

    void SetMemory(DataType* newMemory, size_t memoryCount) const // Intentionaly const ... only mutable members are modified
        {
        m_memory = newMemory;
        m_allocatedCount = memoryCount;
        }

    size_t GetMemoryCount() const
        {
        return m_allocatedCount;
        }

    bool Discard() const // Intentionaly const ... only mutable members are modified
        {
        // Compress memory
        Byte* compressedMemory = (Byte*)malloc (sizeof(DataType)*m_allocatedCount + 100);
        if (compressedMemory == NULL)
            return false;

        uint32_t OutLen = (uint32_t)sizeof(DataType)*m_allocatedCount + 100;
        int err;


        // Use level 5 compression. Varies from 1 to 9, 6 being the default.
        // There seems to be a big difference between 5 and 6 on execution time for
        // big images, with only a small size penalty.
        err = compress2(compressedMemory, &OutLen, (Byte*)m_memory, (uint32_t)((m_count+1)*sizeof(DataType)), 5);

        if(err != Z_OK)
            {
            free(compressedMemory);
            return false;
            }

        m_compressedMemory = (Byte*)malloc (OutLen+1);
        if (m_compressedMemory == NULL)
            {
            free(compressedMemory);
            return false;
            }
        m_compressedMemorySize = OutLen+1;
        m_compressedAllocatedCount = m_allocatedCount;

        memcpy (m_compressedMemory, compressedMemory, OutLen);

        free (compressedMemory);

        // Free memory
        m_pool->Free (this);

        return true;
        }

    bool Inflate() const // Intentionaly const ... only mutable members are modified
        {
        int err;

        HPRECONDITION(m_compressedMemory!= NULL);

        if (m_pool != NULL)
            if (!GetPoolManaged())
                m_pool->Allocate (m_compressedAllocatedCount, this);
            else
                m_pool->Reallocate (m_compressedAllocatedCount, this);
        else
            {
            DataType* newMemory = new DataType[m_compressedAllocatedCount];
            if (newMemory == NULL)
                return false;
            m_memory = reinterpret_cast<DataType*>(newMemory);
            m_allocatedCount = m_compressedAllocatedCount;
            }

        uint32_t OutLen = (uint32_t)m_allocatedCount*sizeof(DataType);
        err = uncompress((Byte*)m_memory, &OutLen, (Byte*)m_compressedMemory, (uint32_t)m_compressedMemorySize);

        if(err != Z_OK)
            return false;

        free(m_compressedMemory);
        m_compressedMemory = NULL;
        return true;

        }

    bool reserve(size_t newCount)
        {
        if (m_compressedMemory != NULL) Inflate();

        if (newCount > m_allocatedCount)
            {
            if (m_pool != NULL)
                if (!GetPoolManaged())
                    m_pool->Allocate (newCount, this);
                else
                    m_pool->Reallocate (newCount, this);
            else
                {
                void* newMemory = new DataType[newCount];
                if (newMemory == NULL)
                    return false;
                memcpy (newMemory, m_memory, sizeof(DataType)*m_count);
                delete [] m_memory;
                m_memory = reinterpret_cast<DataType*>(newMemory);
                m_allocatedCount = newCount;
                }
            }
        return true;
        }
    size_t capacity() const
        {
        return m_allocatedCount;
        }
    bool push_back(const DataType& newObject)
        {
        if (m_compressedMemory != NULL) Inflate();

        if((m_allocatedCount <= m_count) && !reserve (m_count + 1))
            return false;

        m_accessCount++;
        if ((m_accessCount > 100) && (m_pool != NULL))
            {
            m_pool->NotifyAccess(this);
            m_accessCount = 0;
            }

        m_memory[m_count] = newObject;
        m_count++;
        return true;
        }
    bool push_back(DataType* newObjects, size_t count)
        {
        if (m_compressedMemory != NULL) Inflate();

        íf (m_allocatedCount <= (m_count + count))
        if (!reserve (m_count + count))
            return false;

        m_accessCount+=count;
        if ((m_accessCount > 100) && (m_pool != NULL))
            {
            m_pool->NotifyAccess(this);
            m_accessCount = 0;
            }

        memcpy (&(m_memory[m_count]), newObjects, sizeof(DataType) * count);
//        for (size_t idx = 0 ; idx < count ; idx++)
//        {
//            m_memory[m_count] = newObjects[idx];
//            m_count++;
//        }
        return true;
        }
    DataType& operator[](size_t index) const
        {
        HPRECONDITION(index < m_count);
        if (m_compressedMemory != NULL) Inflate();

        m_accessCount++;
        if ((m_accessCount > 100) && (m_pool != NULL))
            {
            m_pool->NotifyAccess(this);
            m_accessCount = 0;
            }

        return m_memory[index];
        }
    size_t size() const
        {
        return m_count;
        }
    void erase (size_t index)
        {
        if (m_compressedMemory != NULL) Inflate();

        if (index < m_count - 1)
            memcpy(&(m_memory[index]), &(m_memory[index+1]), sizeof(DataType) * (m_count - 1 - index));
        m_count--;

        m_accessCount++;
        if ((m_accessCount > 100) && (m_pool != NULL))
            {
            m_pool->NotifyAccess(this);
            m_accessCount = 0;
            }

        }
    void clear()
        {
        if (m_compressedMemory != NULL) Inflate();

        if (m_allocatedCount > 0)
            {
            if (m_pool != NULL)
                {
                if (GetPoolManaged())
                    m_pool->Free(this);
                }
            else
                delete [] m_memory;
            }

        m_count = 0;
        m_allocatedCount = 0;
        }
private:
    mutable DataType* m_memory;
    size_t m_count;
    mutable size_t m_allocatedCount;
    mutable FastVectorCountLimitedPool<DataType>* m_pool;
    mutable typename std::list<const FastCompressiblePooledVector<DataType>* >::iterator m_poolIterator;
    mutable bool m_poolManaged;

    mutable Byte* m_compressedMemory;
    mutable uint32_t m_compressedMemorySize;
    mutable size_t m_compressedAllocatedCount;

    mutable size_t m_accessCount;

    };


//
//template<class TOTO> HGF2DLiteExtent ExtentExtract(const HVE2DGenFacet<TOTO>* spatialObject)
//    {
//    return spatialObject->GetLiteExtent();
//    }

//template<class SPATIAL> HGF2DLiteExtent ExtentExtract(const SPATIAL* spatialObject)
//    {
//    return spatialObject->GetLiteExtent();
//    }


//template<class SPATIAL> HGF2DLiteExtent ExtentExtract(const HFCPtr<SPATIAL> spatialObject)
//    {
//    return spatialObject->GetLiteExtent();
//    }

// General
template<class SPATIAL, class COORD = HGF2DPosition, class EXTENT = HGF2DTemplateExtent<COORD::NumberDataType> > class HGFIndexGeometryOpProvider
    {
public:

    static EXTENT ExtentExtract(const SPATIAL spatialObject)
        {
        return spatialObject.GetLiteExtent();
        }

    static bool IsPointIn(const SPATIAL spatialObject, COORD pi_rCoord)
        {
        return spatialObject.GetLiteExtent().IsPointIn (pi_rCoord);
        }

    static EXTENT MergeExtents (const EXTENT& extent1, const EXTENT& extent2)
        {
        return EXTENT(MIN(extent1.GetXMin(), extent2.GetXMin()),
                      MIN(extent1.GetYMin(), extent2.GetYMin()),
                      MAX(extent1.GetXMax(), extent2.GetXMax()),
                      MAX(extent1.GetYMax(), extent2.GetYMax()));
        }

    static bool IsSpatialInExtent (const SPATIAL& spatial, const EXTENT& extent)
        {
        return ExtentExtract(spatial).Overlaps (extent);
        }

    };

template <class SPATIAL, class COORD, class EXTENT> class HGFIndexGeometryOpProvider<SPATIAL*, COORD, EXTENT>
    {

public:


    static EXTENT ExtentExtract(const SPATIAL* spatialObject)
        {
        return spatialObject->GetLiteExtent();
        }

    static bool IsPointIn(const SPATIAL* spatialObject, COORD pi_rCoord)
        {
        return spatialObject->GetLiteExtent().IsPointIn (pi_rCoord);
        }

    static EXTENT MergeExtents (const EXTENT& extent1, const EXTENT& extent2)
        {
        return EXTENT(MIN(extent1.GetXMin(), extent2.GetXMin()),
                      MIN(extent1.GetYMin(), extent2.GetYMin()),
                      MAX(extent1.GetXMax(), extent2.GetXMax()),
                      MAX(extent1.GetYMax(), extent2.GetYMax()));
        }
    static bool IsSpatialInExtent (const SPATIAL* spatial, const EXTENT& extent)
        {
        return ExtentExtract(spatial).Overlaps (extent);
        }
    };

template <class SPATIAL, class COORD, class EXTENT> class HGFIndexGeometryOpProvider<HFCPtr<SPATIAL>, COORD, EXTENT>
    {

public:


    static EXTENT ExtentExtract(const HFCPtr<SPATIAL> spatialObject)
        {
        return spatialObject->GetLiteExtent();
        }

    static bool IsPointIn(const HFCPtr<SPATIAL> spatialObject, COORD pi_rCoord)
        {
        return spatialObject->GetLiteExtent().IsPointIn (pi_rCoord);
        }

    static EXTENT MergeExtents (const EXTENT& extent1, const EXTENT& extent2)
        {
        return EXTENT(MIN(extent1.GetXMin(), extent2.GetXMin()),
                      MIN(extent1.GetYMin(), extent2.GetYMin()),
                      MAX(extent1.GetXMax(), extent2.GetXMax()),
                      MAX(extent1.GetYMax(), extent2.GetYMax()));
        }
    static bool IsSpatialInExtent (const HFCPtr<SPATIAL> spatial, const EXTENT& extent)
        {
        return ExtentExtract(spatial).Overlaps (extent);
        }
    };


template <> class HGFIndexGeometryOpProvider<HGF2DPosition, HGF2DPosition, HGF2DLiteExtent>
    {

public:
    static HGF2DLiteExtent ExtentExtract(const HGF2DPosition spatialObject)
        {
        return HGF2DLiteExtent(spatialObject.GetX(), spatialObject.GetY(), spatialObject.GetX(), spatialObject.GetY());
        }

    static bool IsPointIn(const HGF2DPosition spatialObject, HGF2DPosition pi_rCoord)
        {
        return spatialObject.IsEqualTo (pi_rCoord);
        }

    static HGF2DLiteExtent MergeExtents (const HGF2DLiteExtent& extent1, const HGF2DLiteExtent& extent2)
        {
        return HGF2DLiteExtent(MIN(extent1.GetXMin(), extent2.GetXMin()),
                               MIN(extent1.GetYMin(), extent2.GetYMin()),
                               MAX(extent1.GetXMax(), extent2.GetXMax()),
                               MAX(extent1.GetYMax(), extent2.GetYMax()));
        }
    static bool IsSpatialInExtent (const HGF2DPosition& spatial, const HGF2DLiteExtent& extent)
        {
        return extent.IsPointIn (spatial);
        }

    };

template <> class HGFIndexGeometryOpProvider<HGF2DLiteExtent, HGF2DPosition, HGF2DLiteExtent>
    {

public:
    static HGF2DLiteExtent ExtentExtract(const HGF2DLiteExtent spatialObject)
        {
        return spatialObject;
        }

    static bool IsPointIn(const HGF2DLiteExtent spatialObject, HGF2DPosition pi_rCoord)
        {
        return spatialObject.IsPointIn (pi_rCoord);
        }

    static HGF2DLiteExtent MergeExtents (const HGF2DLiteExtent& extent1, const HGF2DLiteExtent& extent2)
        {
        return HGF2DLiteExtent(MIN(extent1.GetXMin(), extent2.GetXMin()),
                               MIN(extent1.GetYMin(), extent2.GetYMin()),
                               MAX(extent1.GetXMax(), extent2.GetXMax()),
                               MAX(extent1.GetYMax(), extent2.GetYMax()));
        }
    static bool IsSpatialInExtent (const HGF2DLiteExtent& spatial, const HGF2DLiteExtent& extent)
        {
        return spatial.Overlaps (extent);
        }
    };

//template<class SPATIAL> EXTENT ExtentExtract(const SPATIAL* spatialObject)
//    {
//    return spatialObject->GetLiteExtent();
//    }

// Specialization
//template<> EXTENT ExtentExtract(const HGF2DPosition spatialObject)
//    {
//    return EXTENT (spatialObject.GetX(), spatialObject.GetX(), spatialObject.GetY(), spatialObject.GetY());
//    }


/** -----------------------------------------------------------------------------

    This class defines the static interface that must be implemented by all and
    any class that serves as a spatial index finder. Although properly speaking
    static methods cannot be used in the definition of classic interfaces
    the class nevetheless serves the purpose of documenting the behavior of any
    static method class that is used in the template based spatial index class
    defined below.

    -----------------------------------------------------------------------------
*/
template<class SPATIAL, class COORD = HGF2DPosition, class EXTENT = HGF2DLiteExtent> class IHGF2DSpatialIndexFinder
    {
public:

    /** -----------------------------------------------------------------------------

        The HasUniqueSolution() method returns true if the finder class provides access
        to a simgle unitque solution even for multiple solutions query. Such
        finder is useful for indexation of mutually exclusive objects.

        -----------------------------------------------------------------------------
    */
    static bool HasUniqueSolution() = 0;
//            {
//                return false;
//            }


    /** -----------------------------------------------------------------------------

        The IsAProbableSolution() method returns true if the provided spatial object
        is likely to be located in the vicinity of the query extent. This method is most
        useful to obtain the extent of the spatial object and checking if it overlaps
        the query extent.

        -----------------------------------------------------------------------------
    */
    static bool IsAProbableSolution(const SPATIAL pi_rSpatialObject,
                                    const EXTENT& pi_rNodeExtent,
                                    const EXTENT& pi_rGetArea) = 0;
//            {
//                return(pi_rNodeExtent.Overlaps(pi_rpSpatialObject->GetLiteExtent()));
//            }

    /** -----------------------------------------------------------------------------

        The IsAProbableSolution() method returns true if the provided spatial object
        is likely to be located in the vicinity of the query point. This method is most
        useful to obtain the extent of the spatial object and checking if the point
        falls within this extent.

        -----------------------------------------------------------------------------
    */
    static bool IsAProbableSolution(const SPATIAL pi_rSpatialObject,
                                    const EXTENT& pi_rNodeExtent,
                                    const COORD& pi_rCoord) = 0;
//            {
//                return(pi_rSpatialObject->GetLiteExtent().IsPointOutterIn(pi_rCoord));
//            }

    /** -----------------------------------------------------------------------------

        The IsASolution() method returns true if the provided spatial object
        is a solution to the query using an extent. Notice that the method is only called
        if the spatial object has indicated that it is a probable solution. For
        case where the query does not require additional information checking then the method
        can simply return true.

        -----------------------------------------------------------------------------
    */
    static bool IsASolution(const SPATIAL pi_rSpatial,
                            const EXTENT& pi_rNodeExtent,
                            const EXTENT& pi_rGetArea) = 0;
//            {
//                return true;
//            }
//
    /** -----------------------------------------------------------------------------

        The IsASolution() method returns true if the provided spatial object
        is a solution to the query using a point. Notice that the method is only called
        if the spatial object has indicated that it is a probable solution. For
        case where the query does not require additional information checking then the method
        can simply return true. This method is most useful for example when querying a
        spatial index of shape objects and the point must be located within the shapes
        In such case the extent checking performed using IsAProbableSolution() method
        is insufficient to declare the spatial object a solution to the query.

        -----------------------------------------------------------------------------
    */
    static bool IsASolution(const SPATIAL pi_rSpatial,
                            const EXTENT& pi_rNodeExtent,
                            const COORD& pi_rCoord) = 0;
//            {
//                return true;
//            }

private:
    // Desactivated
//        IHGF2DSpatialIndexFinder() {};
//        ~IHGF2DSpatialIndexFinder() {};

    };

/** -----------------------------------------------------------------------------

    This class implements a spatial index default finder. It comforms to the
    IHGFSpatialIndexFinder pseudo interface defined above.
    This default finder
    considers that all spatial objects located in the vicinity of query
    coordinate or query extent are solutions and thus that there are not unique
    solutions. The use of this finder imposes that spatial objects
    provide a GetLiteExtent() method that returs an EXTENT or
    a reference to some internal EXTENT object.

    -----------------------------------------------------------------------------
*/
template<class SPATIAL, class COORD = HGF2DPosition, class EXTENT = HGF2DLiteExtent> class HGF2DSpatialIndexDefaultFinder
    {
public:

    static bool HasUniqueSolution()
        {
        return false;
        }

    static bool IsAProbableSolution(const SPATIAL pi_rSpatialObject,
                                    const EXTENT& pi_rNodeExtent,
                                    const EXTENT& pi_rGetArea)
        {
        return(pi_rNodeExtent.Overlaps(HGFIndexGeometryOpProvider<SPATIAL, COORD, EXTENT>::ExtentExtract(pi_rpSpatialObject)));
        }

    static bool IsAProbableSolution(const SPATIAL pi_rSpatialObject,
                                    const EXTENT& pi_rNodeExtent,
                                    const COORD& pi_rCoord)
        {
        return(HGFIndexGeometryOpProvider<SPATIAL, COORD, EXTENT>::ExtentExtract(pi_rSpatialObject).IsPointOutterIn(pi_rCoord));
        }

    static bool IsASolution(const SPATIAL pi_rSpatial,
                            const EXTENT& pi_rNodeExtent,
                            const EXTENT& pi_rGetArea)
        {
        return true;
        }

    static bool IsASolution(const SPATIAL pi_rSpatial,
                            const EXTENT& pi_rNodeExtent,
                            const COORD& pi_rCoord)
        {
        return true;
        }

private:
    // Desactivated
    HGF2DSpatialIndexDefaultFinder() {};
    ~HGF2DSpatialIndexDefaultFinder() {};

    };

/** -----------------------------------------------------------------------------

    This class implements a spatial index default in point finder. It comforms to the
    IHGFSpatialIndexFinder pseudo interface defined above. This default
    in point finder considers that a single spatial object located in the vicinity of query
    coordinate or query extent is a solutions and thus that there is a unique
    solutions.
    This specific finder requires that spatial objects provide a GetLiteExtent()
    method that returns an EXTENT or a reference to some internal
    EXTENT object. It must also provide a IsPointIn() method to check the presence
    of the point within the spatial object.

    -----------------------------------------------------------------------------
*/
template<class SPATIAL, class COORD, class EXTENT = HGF2DLiteExtent> class HGF2DSpatialIndexUniqueInPointFinder
    {
public:

    static bool HasUniqueSolution()
        {
        return true;
        }

    static bool IsAProbableSolution(const SPATIAL pi_rSpatialObject,
                                    const EXTENT& pi_rNodeExtent,
                                    const EXTENT& pi_rGetArea)
        {
        return(pi_rNodeExtent.Overlaps(HGFIndexGeometryOpProvider<SPATIAL, COORD, EXTENT>::ExtentExtract(pi_rpSpatialObject)));
        }

    static bool IsAProbableSolution(const SPATIAL pi_rSpatialObject,
                                    const EXTENT& pi_rNodeExtent,
                                    const COORD& pi_rCoord)
        {
        return(HGFIndexGeometryOpProvider<SPATIAL, COORD, EXTENT>::ExtentExtract(pi_rSpatialObject).IsPointOutterIn(pi_rCoord));
        }

    static bool IsASolution(const SPATIAL pi_rSpatialObject,
                            const EXTENT& pi_rNodeExtent,
                            const EXTENT& pi_rGetArea)
        {
        return true;
        }

    static bool IsASolution(const SPATIAL pi_rSpatialObject,
                            const EXTENT& pi_rNodeExtent,
                            const COORD& pi_rCoord)
        {
        return(HGFIndexGeometryOpProvider<SPATIAL, COORD, EXTENT>::IsPointIn (pi_rSpatialObject, pi_rCoord));
        }

private:
    // Desactivated
    HGF2DSpatialIndexUniqueInPointFinder() {};
    ~HGF2DSpatialIndexUniqueInPointFinder() {};
    };



/** -----------------------------------------------------------------------------

    This class defines the static interface that must be implemented by all and
    any class that serves as a spatial index finder. Although properly speaking
    static methods cannot be used in the definition of classic interfaces
    the class nevetheless serves the purpose of documenting the behavior of any
    static method class that is used in the template based spatial index class
    defined below.

    -----------------------------------------------------------------------------
*/
template <class SPATIAL> class IHGFSpatialIndexNode
    {
public:


    /**----------------------------------------------------------------------------
     Sets the parent node .. this can only be called for a node that has no parent

     @param pi_rpParentNode - Pointer to parent node
    -----------------------------------------------------------------------------*/
//        void SetParentNode(const HFCPtr<Node>& pi_rpParentNode) = 0;

    /**----------------------------------------------------------------------------
     Returns the parent node

     @return reference to the parent node
    -----------------------------------------------------------------------------*/
//        const HFCPtr<Node>&
//             GetParentNode() const = 0;


    /**----------------------------------------------------------------------------
     Sets the sub nodes ... this can only be for a leaf node and the subnodes must
     exist.

     @param pi_apSubNodes - Array of four sub-nodes pointer that must point to existing
                           nodes
    -----------------------------------------------------------------------------*/
//        void SetSubNodes(HFCPtr<Node> pi_apSubNodes[4]) = 0;

    /**----------------------------------------------------------------------------
     Sets the split treshold value for node

     @param pi_SplitTreshold - The split treshold value used to determine when a
                              leaf node must be split. It is the maximum number of
                              objects referenced by node before a split occurs.
                              This value msut be greater than "MinimalTreshold"
    -----------------------------------------------------------------------------*/
//        void SetSplitTreshold(size_t pi_SplitTreshold) = 0;

    /**----------------------------------------------------------------------------
     Returns the split treshold value for node

    -----------------------------------------------------------------------------*/
//        size_t GetSplitTreshold() const = 0;

    /**----------------------------------------------------------------------------
     Indicates if node is leaf

     @return true if node is a leaf
    -----------------------------------------------------------------------------*/
//        bool IsLeaf() const = 0;

    /**----------------------------------------------------------------------------
     Returns node extent

     @return reference to extent of node
    -----------------------------------------------------------------------------*/
//        const EXTENT&
//             GetExtent() const = 0;


    /**----------------------------------------------------------------------------
     Adds a reference to spatial object in node.
     The spatial object extent must be included (contained) in node extent

     @param Pointer to spatial object to add a reference to in the index
    -----------------------------------------------------------------------------*/
//        void AddItem(const HFCPtr<SPATIAL>& pi_rpSpatialObject) = 0;

    /**----------------------------------------------------------------------------
     Removes a reference to spatial object in node.
     The spatial object extent must be included (contained) in node extent

     @param Pointer to spatial object to remove reference to in the index
    -----------------------------------------------------------------------------*/
//        bool Remove(const HFCPtr<SPATIAL>& pi_rpSpatialObject) = 0;


    /**----------------------------------------------------------------------------
     Gets a list of objects located near given coordinate
     the node will search within subnodes and parent nodes through the whole
     node network except in the direction of the initiator

     @param pi_rCoord The coordinate to obtain objects in the vicinity of.

     @param pio_rListOfObjects The list of objects to which are appended objects in the
                              vicinity of given point

     @param pi_rpInitiator Pointer to node that initiates the call. This node
             will not be called to fetch objects
    -----------------------------------------------------------------------------*/
//       size_t GetAt(const HGF2DPosition& pi_rCoord,
//                   list<HFCPtr<SPATIAL> >& pio_rListOfObjects,
//                   const HFCPtr<Node>& pi_rpInitiator = HFCPtr<Node>(0),
//                   HFCPtr<Node>* po_pLastNode = 0) const = 0;


    /**----------------------------------------------------------------------------
     Gets a list of objects located inside (outter) given extent
     the node will search within subnodes and parent nodes through the whole
     node network except in the direction of the initiator

     @param pi_rExtent The extent to obtain objects in the vicinity of.

     @param pio_rListOfObjects The list of objects to which are appended objects inside
                              the given extent

     @param pi_rpInitiator Pointer to node that initiates the call. This node
             will not be called to fetch objects
    -----------------------------------------------------------------------------*/
//       size_t GetIn(const EXTENT& pi_rExtent,
//                   list<HFCPtr<SPATIAL> >& pio_rListOfObjects,
//                   const HFCPtr<Node>& pi_rpInitiator = HFCPtr<Node>(0)) const = 0;

    };






/** -----------------------------------------------------------------------------

    This class implements a spatial index default node. It comforms to the
    IHGFSpatialIndexNode pseudo interface defined above. This default
    node maintains in memory all of the data.

    -----------------------------------------------------------------------------
*/
template <class SPATIAL, class COORD = HGF2DPosition, class EXTENT = HGF2DLiteExtent, class SPATIALFINDER = HGF2DSpatialIndexDefaultFinder<SPATIAL, COORD, EXTENT> > class HGFSpatialIndexDefaultNode : public HFCShareableObject<HGFSpatialIndexDefaultNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> > //, IHGFSpatialIndexNode<SPATIAL>
    {
public:
    /**----------------------------------------------------------------------------
     Constructor for node based upon extent only.

     @param pi_SplitTreshold - The split treshold value used to determine when a
                               leaf node must be split. It is the maximum number of
                               objects referenced by node before a split occurs.
                               This value msut be greater than "MinimalTreshold"

     @param pi_rExtent - The extent of the node
    -----------------------------------------------------------------------------*/
    HGFSpatialIndexDefaultNode(size_t pi_SplitTreshold,
                               const EXTENT& pi_rExtent)
        : m_Extent(pi_rExtent),
          m_IsLeaf(true),
          m_SplitTreshold(pi_SplitTreshold)
        {

        // Do not allow NULL sized extent ...
        if (!(m_Extent.GetXMax() > m_Extent.GetXMin()))
            m_Extent.SetXMax (m_Extent.GetXMin() + MAX (fabs(m_Extent.GetXMin() * HNumeric<double>::EPSILON_MULTIPLICATOR() * 1000.0), 1.0));
        if (!(m_Extent.GetYMax() > m_Extent.GetYMin()))
            m_Extent.SetYMax (m_Extent.GetYMin() + MAX (fabs(m_Extent.GetYMin() * HNumeric<double>::EPSILON_MULTIPLICATOR() * 1000.0), 1.0));

        HINVARIANTS;
        }

    /**----------------------------------------------------------------------------
     Constructor for node based upon parent node and extent.

     @param pi_SplitTreshold - The split treshold value used to determine when a
                              leaf node must be split. It is the maximum number of
                              objects referenced by node before a split occurs.
                              This value msut be greater than "MinimalTreshold"

     @param pi_rExtent - The extent of the node

     @param pi_rpParentNode - Reference to pointer to parent node for created node
    -----------------------------------------------------------------------------*/
    HGFSpatialIndexDefaultNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> (size_t pi_SplitTreshold,
                                                                       const EXTENT& pi_rExtent,
                                                                       const HFCPtr<HGFSpatialIndexDefaultNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> >& pi_rpParentNode)
        : m_Extent(pi_rExtent),
          m_pParentNode(pi_rpParentNode),
          m_IsLeaf(true),
          m_SplitTreshold(pi_SplitTreshold)
        {
        HINVARIANTS;
        }

    /**----------------------------------------------------------------------------
     Copy constructor - This constructor may only be called when the
     given node has no parent

     @param pi_rNode - Reference to node to duplicate. This node must have no parent
    -----------------------------------------------------------------------------*/
    HGFSpatialIndexDefaultNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> (const HGFSpatialIndexDefaultNode<SPATIAL, COORD, EXTENT, SPATIALFINDER>& pi_rNode)
        : m_Extent(pi_rNode.m_rExtent),
          m_pParentNode(0),
          m_IsLeaf(pi_rNode.m_IsLeaf),
          m_ListOfObjects(pi_rNode.m_ListOfObjects),
          m_SplitTreshold(pi_rNode.m_SplitTreshold)
        {
        // A copied node may not have a parent
        HPRECONDITION(pi_rNode.m_pParentNode == 0);

        // Check if there are subnodes
        if (!m_IsLeaf)
            {
            m_apSubNodes[0] = new Node(pi_rNode.m_apSubNodes[0], this);
            m_apSubNodes[1] = new Node(pi_rNode.m_apSubNodes[1], this);
            m_apSubNodes[2] = new Node(pi_rNode.m_apSubNodes[2], this);
            m_apSubNodes[3] = new Node(pi_rNode.m_apSubNodes[3], this);
            }

        HINVARIANTS;
        }

    /**----------------------------------------------------------------------------
     Alternate Copy constructor ... equivalent but parent node is provided as parameter

     @param pi_rNode - Reference to node to duplicate.

     @param pi_rpParentNode - Pointer to parent node
    -----------------------------------------------------------------------------*/
    HGFSpatialIndexDefaultNode<SPATIAL, COORD, EXTENT, SPATIALFINDER>(const HGFSpatialIndexDefaultNode& pi_rNode,
                                                                      const HFCPtr<HGFSpatialIndexDefaultNode>& pi_rpParentNode)
        : m_Extent(pi_rNode.m_rExtent),
          m_pParentNode(pi_rpParentNode),
          m_IsLeaf(pi_rNode.m_IsLeaf),
          m_ListOfObjects(pi_rNode.m_ListOfObjects),
          m_SplitTreshold(pi_rNode.m_SplitTreshold)
        {
        // Check if there are subnodes
        if (!m_IsLeaf)
            {
            m_apSubNodes[0] = new HGFSpatialIndexDefaultNode<SPATIAL, COORD, EXTENT, SPATIALFINDER>(pi_rNode.m_apSubNodes[0], this);
            m_apSubNodes[1] = new HGFSpatialIndexDefaultNode<SPATIAL, COORD, EXTENT, SPATIALFINDER>(pi_rNode.m_apSubNodes[1], this);
            m_apSubNodes[2] = new HGFSpatialIndexDefaultNode<SPATIAL, COORD, EXTENT, SPATIALFINDER>(pi_rNode.m_apSubNodes[2], this);
            m_apSubNodes[3] = new HGFSpatialIndexDefaultNode<SPATIAL, COORD, EXTENT, SPATIALFINDER>(pi_rNode.m_apSubNodes[3], this);
            }

        HINVARIANTS;
        }

    /**----------------------------------------------------------------------------
     Destroyer
    -----------------------------------------------------------------------------*/
    ~HGFSpatialIndexDefaultNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> () {}

    /**----------------------------------------------------------------------------
     Sets the parent node .. this can only be called for a node that has no parent

     @param pi_rpParentNode - Pointer to parent node
    -----------------------------------------------------------------------------*/
    void SetParentNode(const HFCPtr<HGFSpatialIndexDefaultNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> >& pi_rpParentNode)
        {
        HINVARIANTS;

        // The node must be orphan(parentless)
        HPRECONDITION(m_pParentNode == 0);

        m_pParentNode = pi_rpParentNode;
        }

    /**----------------------------------------------------------------------------
     Returns the parent node

     @return reference to the parent node
    -----------------------------------------------------------------------------*/
    const HFCPtr<HGFSpatialIndexDefaultNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> >&
    GetParentNode() const
        {
        return m_pParentNode;
        }


    /**----------------------------------------------------------------------------
     Sets the sub nodes ... this can only be for a leaf node and the subnodes must
     exist.

     @param pi_apSubNodes - Array of four sub-nodes pointer that must point to existing
                           nodes
    -----------------------------------------------------------------------------*/
    void SetSubNodes(HFCPtr<HGFSpatialIndexDefaultNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> > pi_apSubNodes[4])
        {
        HINVARIANTS;

        // Table must be provided and each subnode pointer point to an existing node
        HPRECONDITION(pi_apSubNodes != NULL);
        HPRECONDITION(pi_apSubNodes[0] != 0);
        HPRECONDITION(pi_apSubNodes[1] != 0);
        HPRECONDITION(pi_apSubNodes[2] != 0);
        HPRECONDITION(pi_apSubNodes[3] != 0);

        // Copy new nodes
        m_apSubNodes[0] = pi_apSubNodes[0];
        m_apSubNodes[1] = pi_apSubNodes[1];
        m_apSubNodes[2] = pi_apSubNodes[2];
        m_apSubNodes[3] = pi_apSubNodes[3];

        // Set parent node to all subnodes
        m_apSubNodes[0]->SetParentNode(this);
        m_apSubNodes[1]->SetParentNode(this);
        m_apSubNodes[2]->SetParentNode(this);
        m_apSubNodes[3]->SetParentNode(this);

        // Indicate node is not a leaf anymore
        m_IsLeaf = false;
        }

    /**----------------------------------------------------------------------------
     Sets the split treshold value for node

     @param pi_SplitTreshold - The split treshold value used to determine when a
                              leaf node must be split. It is the maximum number of
                              objects referenced by node before a split occurs.
                              This value msut be greater than "MinimalTreshold"
    -----------------------------------------------------------------------------*/
    void SetSplitTreshold(size_t pi_SplitTreshold)
        {
        HINVARIANTS;

        HPRECONDITION(pi_SplitTreshold >= MinimalTreshold);

        // Set new value
        m_SplitTreshold = pi_SplitTreshold;

        // Check if there are sub-nodes
        if (!m_IsLeaf)
            {
            // Set treshold for all sub-nodes
            m_apSubNodes[0]->SetSplitTreshold(m_SplitTreshold);
            m_apSubNodes[1]->SetSplitTreshold(m_SplitTreshold);
            m_apSubNodes[2]->SetSplitTreshold(m_SplitTreshold);
            m_apSubNodes[3]->SetSplitTreshold(m_SplitTreshold);
            }
        else
            {
            // The node is a leaf ... check if the current number of objects is greater than new treshold
            if (m_ListOfObjects.size() >= m_SplitTreshold)
                {
                // Treshold attained ... we must split
                SplitNode();
                }
            }
        }

    /**----------------------------------------------------------------------------
     Returns the split treshold value for node

    -----------------------------------------------------------------------------*/
    size_t
    GetSplitTreshold() const
        {
        HINVARIANTS;

        return(m_SplitTreshold);
        }


    /**----------------------------------------------------------------------------
     Indicates if node is leaf

     @return true if node is a leaf
    -----------------------------------------------------------------------------*/
    bool IsLeaf() const
        {
        HINVARIANTS;

        return(m_IsLeaf);
        }

    /**----------------------------------------------------------------------------
     Returns node extent

     @return reference to extent of node
    -----------------------------------------------------------------------------*/
    const EXTENT&
    GetExtent() const
        {
        HINVARIANTS;

        return(m_Extent);
        }



    /**----------------------------------------------------------------------------
     Adds a reference to spatial object in node.
     The spatial object extent must be included (contained) in node extent

     @param Pointer to spatial object to add a reference to in the index
    -----------------------------------------------------------------------------*/
    bool AddItemConditional (const SPATIAL pi_rpSpatialObject, bool ExtentFixed)
        {
        HINVARIANTS;

        // Check is spatial extent is in node ...
        if (m_Extent.IsPointIn(HGFIndexGeometryOpProvider<SPATIAL, COORD, EXTENT>::ExtentExtract(pi_rpSpatialObject).GetOrigin()) &&
            m_Extent.IsPointIn(HGFIndexGeometryOpProvider<SPATIAL, COORD, EXTENT>::ExtentExtract(pi_rpSpatialObject).GetCorner()))
            {
            return AddItem (pi_rpSpatialObject);
            }
        // The spatial object is not in extent ... check if we can increase extent (not extent fixed, no parent and no sub-nodes)
        else if (!ExtentFixed && GetParentNode() == NULL && IsLeaf())
            {
            // We can increase the extent ... do it
            m_Extent = HGFIndexGeometryOpProvider<SPATIAL, COORD, EXTENT>::MergeExtents(m_Extent,
                                                                                        HGFIndexGeometryOpProvider<SPATIAL, COORD, EXTENT>::ExtentExtract(pi_rpSpatialObject));
            return AddItem (pi_rpSpatialObject);
            }

        return false;
        }

    /**----------------------------------------------------------------------------
     Adds a reference to spatial object in node.
     The spatial object extent must be included (contained) in node extent

     @param Pointer to spatial object to add a reference to in the index
    -----------------------------------------------------------------------------*/
    virtual bool AddItem(const SPATIAL pi_rpSpatialObject)
        {
        HINVARIANTS;

        // The object must be fully contained in node extent
        HPRECONDITION(m_Extent.IsPointIn(HGFIndexGeometryOpProvider<SPATIAL, COORD, EXTENT>::ExtentExtract(pi_rpSpatialObject).GetOrigin()) &&
                      m_Extent.IsPointIn(HGFIndexGeometryOpProvider<SPATIAL, COORD, EXTENT>::ExtentExtract(pi_rpSpatialObject).GetCorner()));

        // Check if the threshold amount of objects is attained
        if (m_IsLeaf && (m_ListOfObjects.size() + 1 >= m_SplitTreshold))
            {
            // There are too much objects ... need to split current node
            SplitNode();
            }

        // Check if node is still a leaf ...
        if (m_IsLeaf)
            {
            // It is a leaf ... we add reference in list
            m_ListOfObjects.push_back(pi_rpSpatialObject);
            }
        else
            {
            // Attempt to add in one of the subnodes
            bool Added = false;
            for (int i = 0 ; i < 4 && !Added ; ++ i)
                {
                // Check if object is contained in this sub-node
                if (m_apSubNodes[i]->GetExtent().IsPointIn(HGFIndexGeometryOpProvider<SPATIAL, COORD, EXTENT>::ExtentExtract(pi_rpSpatialObject).GetOrigin()) &&
                    m_apSubNodes[i]->GetExtent().IsPointIn(HGFIndexGeometryOpProvider<SPATIAL, COORD, EXTENT>::ExtentExtract(pi_rpSpatialObject).GetCorner()))
                    {
                    // The object is contained ... we add to subnode
                    m_apSubNodes[i]->AddItem(pi_rpSpatialObject);
                    Added = true;
                    }
                }

            // Check if the object was not added in a subnode ...
            if (!Added)
                {
                // The object was not added, evidently because it is too large ...
                // We add it to current node.
                m_ListOfObjects.push_back(pi_rpSpatialObject);
                }
            }
        return true;
        }

    /**----------------------------------------------------------------------------
     Removes a reference to spatial object in node.
     The spatial object extent must be included (contained) in node extent

     @param Pointer to spatial object to remove reference to in the index
    -----------------------------------------------------------------------------*/
    virtual bool Remove(const SPATIAL pi_rpSpatialObject)
        {
        HINVARIANTS;

        // The object must be fully contained in node extent
        HPRECONDITION(m_Extent.IsPointIn(HGFIndexGeometryOpProvider<SPATIAL, COORD, EXTENT>::ExtentExtract(pi_rpSpatialObject).GetOrigin()) &&
                      m_Extent.IsPointIn(HGFIndexGeometryOpProvider<SPATIAL, COORD, EXTENT>::ExtentExtract(pi_rpSpatialObject).GetCorner()));

        bool Removed = false;

        // Check if node is still a leaf ...
        if (m_IsLeaf)
            {
            // It is a leaf ... we add reference in list
            list<SPATIAL>::iterator Itr = m_ListOfObjects.begin();
            while(Itr != m_ListOfObjects.end() && !Removed)
                {
                // Check if current object is the one to remove ...
                if (*Itr == pi_rpSpatialObject)
                    {
                    // We have found it... erase it
                    Itr = m_ListOfObjects.erase(Itr);
                    Removed = true;
                    }
                else
                    {
                    // Not found ... advance to next item
                    ++Itr;
                    }
                }
            }
        else
            {
            // Attempt to remove in one of the subnodes
            bool RemovalShouldHaveBeenPerformed = false;

            for (int i = 0 ; i < 4 && !RemovalShouldHaveBeenPerformed ; ++ i)
                {
                // Check if object is contained in this sub-node
                if (m_apSubNodes[i]->GetExtent().IsPointIn(HGFIndexGeometryOpProvider<SPATIAL, COORD, EXTENT>::ExtentExtract(pi_rpSpatialObject).GetOrigin()) &&
                    m_apSubNodes[i]->GetExtent().IsPointIn(HGFIndexGeometryOpProvider<SPATIAL, COORD, EXTENT>::ExtentExtract(pi_rpSpatialObject).GetCorner()))
                    {
                    // The object is contained ... we add to subnode
                    Removed = m_apSubNodes[i]->Remove(pi_rpSpatialObject);
                    RemovalShouldHaveBeenPerformed = true;
                    }
                }

            // Check if the object was not removed in a subnode ...
            if (!RemovalShouldHaveBeenPerformed)
                {
                // The object was not removed, evidently because it is too large ...
                list<SPATIAL>::iterator Itr = m_ListOfObjects.begin();
                while(Itr != m_ListOfObjects.end() && !Removed)
                    {
                    // Check if current object is the one to remove ...
                    if (*Itr == pi_rpSpatialObject)
                        {
                        // We have found it... erase it
                        Itr = m_ListOfObjects.erase(Itr);
                        Removed = true;
                        }
                    else
                        {
                        // Not found ... advance to next item
                        ++Itr;
                        }
                    }
                }
            }

        return(Removed);
        }


    /**----------------------------------------------------------------------------
     Gets a list of objects located near given coordinate
     the node will search within subnodes and parent nodes through the whole
     node network except in the direction of the initiator

     @param pi_rCoord The coordinate to obtain objects in the vicinity of.

     @param pio_rListOfObjects The list of objects to which are appended objects in the
                              vicinity of given point

     @param pi_rpInitiator Pointer to node that initiates the call. This node
             will not be called to fetch objects

     @return The additional number of objects added to the list
    -----------------------------------------------------------------------------*/
    virtual size_t GetAt(const COORD& pi_rCoord,
                         list<SPATIAL>& pio_rListOfObjects,
                         const HFCPtr<HGFSpatialIndexDefaultNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> >& pi_rpInitiator = HFCPtr<HGFSpatialIndexDefaultNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> >(0),
                         HFCPtr<HGFSpatialIndexDefaultNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> >* po_pLastNode = 0) const
        {
        HINVARIANTS;

        // Save the number of objects currently in list
        size_t InitialNumberOfObjects = pio_rListOfObjects.size();

        // Check if coordinate falls inside node extent
        if (m_Extent.IsPointOutterIn(pi_rCoord))
            {
            // The point is located inside the node ...
            // Obtain objects from subnodes (if any)
            if (!m_IsLeaf)
                {
                // there are sub-nodes ... fetch from each sub-node (if not initiator)
                if (m_apSubNodes[0] != pi_rpInitiator)
                    if ((m_apSubNodes[0]->GetAt(pi_rCoord, pio_rListOfObjects, const_cast<HGFSpatialIndexDefaultNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> *>(this), po_pLastNode) >= 1) && (SPATIALFINDER::HasUniqueSolution()))
                        return(1);

                if (m_apSubNodes[1] != pi_rpInitiator)
                    if ((m_apSubNodes[1]->GetAt(pi_rCoord, pio_rListOfObjects, const_cast<HGFSpatialIndexDefaultNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> *>(this), po_pLastNode) >= 1) && (SPATIALFINDER::HasUniqueSolution()))
                        return(1);

                if (m_apSubNodes[2] != pi_rpInitiator)
                    if ((m_apSubNodes[2]->GetAt(pi_rCoord, pio_rListOfObjects, const_cast<HGFSpatialIndexDefaultNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> *>(this), po_pLastNode) >= 1) && (SPATIALFINDER::HasUniqueSolution()))
                        return(1);

                if (m_apSubNodes[3] != pi_rpInitiator)
                    if ((m_apSubNodes[3]->GetAt(pi_rCoord, pio_rListOfObjects, const_cast<HGFSpatialIndexDefaultNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> *>(this), po_pLastNode) >= 1) && (SPATIALFINDER::HasUniqueSolution()))
                        return(1);
                }

            // Search in present list of objects for current node
            list<SPATIAL>::const_iterator Itr = m_ListOfObjects.begin();
            for ( ; Itr != m_ListOfObjects.end() ; ++Itr)
                {
                // Check if point is in extent of object
                if (SPATIALFINDER::IsAProbableSolution(*Itr, m_Extent, pi_rCoord))
                    {
                    // The point falls inside extent of object .. we add a reference to the list
                    if (SPATIALFINDER::IsASolution(*Itr, m_Extent, pi_rCoord))
                        {
                        pio_rListOfObjects.push_back(*Itr);

                        // If the last node is desired ... set it
                        if (po_pLastNode)
                            *po_pLastNode = const_cast<HGFSpatialIndexDefaultNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> *>(this);

                        if (SPATIALFINDER::HasUniqueSolution())
                            return(1);
                        }
                    }
                }
            }

        // Check if there is a parent node
        if (m_pParentNode != 0)
            {
            // There is a parent ... check if it not the initiator
            if (m_pParentNode != pi_rpInitiator)
                {
                // The parent is not the initiator ... we fetch objects
                if ((m_pParentNode->GetAt(pi_rCoord, pio_rListOfObjects, const_cast<HGFSpatialIndexDefaultNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> *>(this), po_pLastNode) >= 1) && (SPATIALFINDER::HasUniqueSolution()))
                    return(1);
                }
            }

        // Return number of newly found objects
        return(pio_rListOfObjects.size()- InitialNumberOfObjects);
        }


    /**----------------------------------------------------------------------------
     Gets a list of objects located inside (outter) given extent
     the node will search within subnodes and parent nodes through the whole
     node network except in the direction of the initiator

     @param pi_rExtent The extent to obtain objects in the vicinity of.

     @param pio_rListOfObjects The list of objects to which are appended objects inside
                              the given extent

     @param pi_rpInitiator Pointer to node that initiates the call. This node
             will not be called to fetch objects
    -----------------------------------------------------------------------------*/
    virtual size_t GetIn(const EXTENT& pi_rExtent,
                         list<SPATIAL>& pio_rListOfObjects,
                         const HFCPtr<HGFSpatialIndexDefaultNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> >& pi_rpInitiator = HFCPtr<HGFSpatialIndexDefaultNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> >(0)) const
        {
        HINVARIANTS;

        // Save the number of objects currently in list
        size_t InitialNumberOfObjects = pio_rListOfObjects.size();

        // Check if coordinate falls inside node extent
        if (m_Extent.Overlaps(pi_rExtent))
            {
            // The point is located inside the node ...
            // Obtain objects from subnodes (if any)
            if (!m_IsLeaf)
                {
                // there are sub-nodes ... fetch from each sub-node (if not initiator)
                if (m_apSubNodes[0] != pi_rpInitiator)
                    m_apSubNodes[0]->GetIn(pi_rExtent, pio_rListOfObjects, const_cast<HGFSpatialIndexDefaultNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> *>(this));

                if (m_apSubNodes[1] != pi_rpInitiator)
                    m_apSubNodes[1]->GetIn(pi_rExtent, pio_rListOfObjects, const_cast<HGFSpatialIndexDefaultNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> *>(this));

                if (m_apSubNodes[2] != pi_rpInitiator)
                    m_apSubNodes[2]->GetIn(pi_rExtent, pio_rListOfObjects, const_cast<HGFSpatialIndexDefaultNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> *>(this));

                if (m_apSubNodes[3] != pi_rpInitiator)
                    m_apSubNodes[3]->GetIn(pi_rExtent, pio_rListOfObjects, const_cast<HGFSpatialIndexDefaultNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> *>(this));
                }

            // Search in present list of objects for current node
            list<SPATIAL>::const_iterator Itr = m_ListOfObjects.begin();
            for ( ; Itr != m_ListOfObjects.end() ; ++Itr)
                {
                // Check if point is in extent of object
                if (m_Extent.Overlaps(HGFIndexGeometryOpProvider<SPATIAL, COORD, EXTENT>::ExtentExtract(*Itr)))
                    {
                    // The point falls inside extent of object .. we add a reference to the list
                    pio_rListOfObjects.push_back(*Itr);
                    }
                }
            }

        // Check if there is a parent node
        if (m_pParentNode != 0)
            {
            // There is a parent ... check if it not the initiator
            if (m_pParentNode != pi_rpInitiator)
                {
                // The parent is not the initiator ... we fetch objects
                m_pParentNode->GetIn(pi_rExtent, pio_rListOfObjects, const_cast<HGFSpatialIndexDefaultNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> *>(this));
                }
            }

        // Return number of newly found objects
        return(pio_rListOfObjects.size()- InitialNumberOfObjects);
        }


protected:
    void               ValidateInvariants() const
        {
        // The treshold must be valid
        HASSERT(m_SplitTreshold >= MinimalTreshold);

        // If the node is a leaf, then subnodes must not exist
        HASSERT(!m_IsLeaf || ((m_apSubNodes[0] == 0) && (m_apSubNodes[1] == 0) && (m_apSubNodes[2] == 0) && (m_apSubNodes[3] == 0)));

        // If the node is not a leaf, then subnodes must exist
        HASSERT(m_IsLeaf || ((m_apSubNodes[0] != 0) && (m_apSubNodes[1] != 0) && (m_apSubNodes[2] != 0) && (m_apSubNodes[3] != 0)));

        // If the node is a leaf, then split treshold must not be attained
        HASSERT(!m_IsLeaf || m_ListOfObjects.size() < m_SplitTreshold);
        }


    /**----------------------------------------------------------------------------
     PRIVATE METHOD
     Splits the present leaf into subnodes and repositions current object
     references accordingly
     The node must be a leaf to perform.
    -----------------------------------------------------------------------------*/
    virtual void SplitNode()
        {
        HINVARIANTS;

        // The node must be a leaf
        HPRECONDITION(m_IsLeaf);

        double Width = m_Extent.GetWidth();
        double Height = m_Extent.GetHeight();

        m_apSubNodes[0] = new HGFSpatialIndexDefaultNode<SPATIAL, COORD, EXTENT, SPATIALFINDER>(m_SplitTreshold,
                EXTENT(m_Extent.GetXMin(),
                       m_Extent.GetYMin() + (Height / 2),
                       m_Extent.GetXMin() + (Width / 2),
                       m_Extent.GetYMax()),
                this);

        m_apSubNodes[1] = new HGFSpatialIndexDefaultNode<SPATIAL, COORD, EXTENT, SPATIALFINDER>(m_SplitTreshold,
                EXTENT(m_Extent.GetXMin() + (Width / 2),
                       m_Extent.GetYMin() + (Height / 2),
                       m_Extent.GetXMax(),
                       m_Extent.GetYMax()),
                this);

        m_apSubNodes[2] = new HGFSpatialIndexDefaultNode<SPATIAL, COORD, EXTENT, SPATIALFINDER>(m_SplitTreshold,
                EXTENT(m_Extent.GetXMin(),
                       m_Extent.GetYMin(),
                       m_Extent.GetXMin() + (Width / 2),
                       m_Extent.GetYMin() + (Height / 2)),
                this);

        m_apSubNodes[3] = new HGFSpatialIndexDefaultNode<SPATIAL, COORD, EXTENT, SPATIALFINDER>(m_SplitTreshold,
                EXTENT(m_Extent.GetXMin() + (Width / 2),
                       m_Extent.GetYMin(),
                       m_Extent.GetXMax(),
                       m_Extent.GetYMin() + (Height / 2)),
                this);


        // Indicate node is not a leaf anymore
        m_IsLeaf = false;

        // Now attempt to mode current spatial object references to the sub-nodes
        // For every object referenced
        list<SPATIAL>::iterator Itr = m_ListOfObjects.begin();
        while (Itr != m_ListOfObjects.end())
            {
            // Attempt to add in one of the subnodes
            bool Added = false;
            for (int i = 0 ; i < 4 && !Added ; ++ i)
                {
                // Check if object is contained in this sub-node
                if (m_apSubNodes[i]->GetExtent().IsPointIn(HGFIndexGeometryOpProvider<SPATIAL, COORD, EXTENT>::ExtentExtract(*Itr).GetOrigin()) &&
                    m_apSubNodes[i]->GetExtent().IsPointIn(HGFIndexGeometryOpProvider<SPATIAL, COORD, EXTENT>::ExtentExtract(*Itr).GetCorner()))
                    {
                    // The object is contained ... we add to subnode
                    m_apSubNodes[i]->AddItem((*Itr));
                    Added = true;
                    }
                }

            // If the object was added in one of the subnodes ...
            if (Added)
                {
                // ... then we remove it from current node
                Itr = m_ListOfObjects.erase(Itr);

                // Notice that the new Itr points to next object already
                }
            else
                {
                // Since the object was not added we simply advance to next object
                ++Itr;
                }
            }
        }



    bool m_IsLeaf;
    EXTENT m_Extent;

    HFCPtr<HGFSpatialIndexDefaultNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> > m_pParentNode;

    HFCPtr<HGFSpatialIndexDefaultNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> > m_apSubNodes[4];

    list<SPATIAL> m_ListOfObjects;

    size_t m_SplitTreshold;

    };



// ========================================
/** -----------------------------------------------------------------------------

    This class implements a spatial index default node. It comforms to the
    IHGFSpatialIndexNode pseudo interface defined above. This default
    node maintains in memory all of the data.

    -----------------------------------------------------------------------------
*/
template <class SPATIAL, class COORD = HGF2DPosition, class EXTENT = HGF2DLiteExtent, class SPATIALFINDER = HGF2DSpatialIndexDefaultFinder<SPATIAL, COORD, EXTENT> > class HGFSpatialIndexPointNode : public HFCShareableObject<HGFSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> >
    {
public:

    /**----------------------------------------------------------------------------
     Constructor for node based upon extent only.

     @param pi_SplitTreshold - The split treshold value used to determine when a
                               leaf node must be split. It is the maximum number of
                               objects referenced by node before a split occurs.
                               This value msut be greater than "MinimalTreshold"

     @param pi_rExtent - The extent of the node
    -----------------------------------------------------------------------------*/
    HGFSpatialIndexPointNode(size_t pi_SplitTreshold,
                             const EXTENT& pi_rExtent)
        : m_Extent(pi_rExtent),
          m_IsLeaf(true),
          m_SplitTreshold(pi_SplitTreshold)
        {
        // Do not allow NULL sized extent ...
        if (!(m_Extent.GetXMax() > m_Extent.GetXMin()))
            m_Extent.SetXMax (m_Extent.GetXMin() + MAX (fabs(m_Extent.GetXMin() * HNumeric<double>::EPSILON_MULTIPLICATOR() * 1000.0), 1.0));
        if (!(m_Extent.GetYMax() > m_Extent.GetYMin()))
            m_Extent.SetYMax (m_Extent.GetYMin() + MAX (fabs(m_Extent.GetYMin() * HNumeric<double>::EPSILON_MULTIPLICATOR() * 1000.0), 1.0));

        HINVARIANTS;
        }

    /**----------------------------------------------------------------------------
     Constructor for node based upon parent node and extent.

     @param pi_SplitTreshold - The split treshold value used to determine when a
                              leaf node must be split. It is the maximum number of
                              objects referenced by node before a split occurs.
                              This value msut be greater than "MinimalTreshold"

     @param pi_rExtent - The extent of the node

     @param pi_rpParentNode - Reference to pointer to parent node for created node
    -----------------------------------------------------------------------------*/
    HGFSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> (size_t pi_SplitTreshold,
                                                                     const EXTENT& pi_rExtent,
                                                                     const HFCPtr<HGFSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> >& pi_rpParentNode)
        : m_Extent(pi_rExtent),
          m_pParentNode(pi_rpParentNode),
          m_IsLeaf(true),
          m_SplitTreshold(pi_SplitTreshold)
        {
        HINVARIANTS;
        }

    /**----------------------------------------------------------------------------
     Copy constructor - This constructor may only be called when the
     given node has no parent

     @param pi_rNode - Reference to node to duplicate. This node must have no parent
    -----------------------------------------------------------------------------*/
    HGFSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> (const HGFSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER>& pi_rNode)
        : m_Extent(pi_rNode.m_rExtent),
          m_pParentNode(0),
          m_IsLeaf(pi_rNode.m_IsLeaf),
          m_ListOfObjects(pi_rNode.m_ListOfObjects),
          m_SplitTreshold(pi_rNode.m_SplitTreshold)
        {
        // A copied node may not have a parent
        HPRECONDITION(pi_rNode.m_pParentNode == 0);

        // Check if there are subnodes
        if (!m_IsLeaf)
            {
            m_apSubNodes[0] = new Node(pi_rNode.m_apSubNodes[0], this);
            m_apSubNodes[1] = new Node(pi_rNode.m_apSubNodes[1], this);
            m_apSubNodes[2] = new Node(pi_rNode.m_apSubNodes[2], this);
            m_apSubNodes[3] = new Node(pi_rNode.m_apSubNodes[3], this);
            }

        HINVARIANTS;
        }

    /**----------------------------------------------------------------------------
     Alternate Copy constructor ... equivalent but parent node is provided as parameter

     @param pi_rNode - Reference to node to duplicate.

     @param pi_rpParentNode - Pointer to parent node
    -----------------------------------------------------------------------------*/
    HGFSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER>(const HGFSpatialIndexPointNode& pi_rNode,
                                                                    const HFCPtr<HGFSpatialIndexPointNode>& pi_rpParentNode)
        : m_Extent(pi_rNode.m_rExtent),
          m_pParentNode(pi_rpParentNode),
          m_IsLeaf(pi_rNode.m_IsLeaf),
          m_ListOfObjects(pi_rNode.m_ListOfObjects),
          m_SplitTreshold(pi_rNode.m_SplitTreshold)
        {
        // Check if there are subnodes
        if (!m_IsLeaf)
            {
            m_apSubNodes[0] = new HGFSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER>(pi_rNode.m_apSubNodes[0], this);
            m_apSubNodes[1] = new HGFSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER>(pi_rNode.m_apSubNodes[1], this);
            m_apSubNodes[2] = new HGFSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER>(pi_rNode.m_apSubNodes[2], this);
            m_apSubNodes[3] = new HGFSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER>(pi_rNode.m_apSubNodes[3], this);
            }

        HINVARIANTS;
        }

    /**----------------------------------------------------------------------------
     Destroyer
    -----------------------------------------------------------------------------*/
    ~HGFSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> () {}


    /**----------------------------------------------------------------------------
     Sets the parent node .. this can only be called for a node that has no parent

     @param pi_rpParentNode - Pointer to parent node
    -----------------------------------------------------------------------------*/
    void SetParentNode(const HFCPtr<HGFSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> >& pi_rpParentNode)
        {
        HINVARIANTS;

        // The node must be orphan(parentless)
        HPRECONDITION(m_pParentNode == 0);

        m_pParentNode = pi_rpParentNode;
        }

    /**----------------------------------------------------------------------------
     Returns the parent node

     @return reference to the parent node
    -----------------------------------------------------------------------------*/
    const HFCPtr<HGFSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> >&
    GetParentNode() const
        {
        return m_pParentNode;
        }

    /**----------------------------------------------------------------------------
     Sets the sub nodes ... this can only be for a leaf node and the subnodes must
     exist.

     @param pi_apSubNodes - Array of four sub-nodes pointer that must point to existing
                           nodes
    -----------------------------------------------------------------------------*/
    void SetSubNodes(HFCPtr<HGFSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> > pi_apSubNodes[4])
        {
        HINVARIANTS;

        // Table must be provided and each subnode pointer point to an existing node
        HPRECONDITION(pi_apSubNodes != NULL);
        HPRECONDITION(pi_apSubNodes[0] != 0);
        HPRECONDITION(pi_apSubNodes[1] != 0);
        HPRECONDITION(pi_apSubNodes[2] != 0);
        HPRECONDITION(pi_apSubNodes[3] != 0);

        // Copy new nodes
        m_apSubNodes[0] = pi_apSubNodes[0];
        m_apSubNodes[1] = pi_apSubNodes[1];
        m_apSubNodes[2] = pi_apSubNodes[2];
        m_apSubNodes[3] = pi_apSubNodes[3];

        // Set parent node to all subnodes
        m_apSubNodes[0]->SetParentNode(this);
        m_apSubNodes[1]->SetParentNode(this);
        m_apSubNodes[2]->SetParentNode(this);
        m_apSubNodes[3]->SetParentNode(this);

        // Indicate node is not a leaf anymore
        m_IsLeaf = false;
        }

    /**----------------------------------------------------------------------------
     Sets the split treshold value for node

     @param pi_SplitTreshold - The split treshold value used to determine when a
                              leaf node must be split. It is the maximum number of
                              objects referenced by node before a split occurs.
                              This value msut be greater than "MinimalTreshold"
    -----------------------------------------------------------------------------*/
    void SetSplitTreshold(size_t pi_SplitTreshold)
        {
        HINVARIANTS;

        HPRECONDITION(pi_SplitTreshold >= MinimalTreshold);

        // Set new value
        m_SplitTreshold = pi_SplitTreshold;

        // Check if there are sub-nodes
        if (!m_IsLeaf)
            {
            // Set treshold for all sub-nodes
            m_apSubNodes[0]->SetSplitTreshold(m_SplitTreshold);
            m_apSubNodes[1]->SetSplitTreshold(m_SplitTreshold);
            m_apSubNodes[2]->SetSplitTreshold(m_SplitTreshold);
            m_apSubNodes[3]->SetSplitTreshold(m_SplitTreshold);
            }
        else
            {
            // The node is a leaf ... check if the current number of objects is greater than new treshold
            if (m_VectorOfPoints.size() >= m_SplitTreshold)
                {
                // Treshold attained ... we must split
                SplitNode();
                }
            }
        }

    /**----------------------------------------------------------------------------
     Returns the split treshold value for node

    -----------------------------------------------------------------------------*/
    size_t
    GetSplitTreshold() const
        {
        HINVARIANTS;

        return(m_SplitTreshold);
        }


    /**----------------------------------------------------------------------------
     Indicates if node is leaf

     @return true if node is a leaf
    -----------------------------------------------------------------------------*/
    bool IsLeaf() const
        {
        HINVARIANTS;

        return(m_IsLeaf);
        }

    /**----------------------------------------------------------------------------
     Returns node extent

     @return reference to extent of node
    -----------------------------------------------------------------------------*/
    const EXTENT&
    GetExtent() const
        {
        HINVARIANTS;

        return(m_Extent);
        }


    /**----------------------------------------------------------------------------
     Adds a reference to spatial object in node.
     The spatial object extent must be included (contained) in node extent

     @param Pointer to spatial object to add a reference to in the index
    -----------------------------------------------------------------------------*/
    bool AddItemConditional (const SPATIAL pi_rpSpatialObject, bool ExtentFixed)
        {
        HINVARIANTS;

        // Check is spatial extent is in node ...
        if (HGFIndexGeometryOpProvider<SPATIAL, COORD, EXTENT>::IsSpatialInExtent(pi_rpSpatialObject, m_Extent))

//            if (m_Extent.IsPointIn(HGFIndexGeometryOpProvider<SPATIAL, COORD, EXTENT>::ExtentExtract(pi_rpSpatialObject).GetOrigin()) &&
//                m_Extent.IsPointIn(HGFIndexGeometryOpProvider<SPATIAL, COORD, EXTENT>::ExtentExtract(pi_rpSpatialObject).GetCorner()))
            {
            return AddItem (pi_rpSpatialObject);
            }
        // The spatial object is not in extent ... check if we can increase extent (not extent fixed, no parent and no sub-nodes)
        else if (!ExtentFixed && GetParentNode() == NULL && IsLeaf())
            {
            // We can increase the extent ... do it
            m_Extent = HGFIndexGeometryOpProvider<SPATIAL, COORD, EXTENT>::MergeExtents(m_Extent,
                                                                                        HGFIndexGeometryOpProvider<SPATIAL, COORD, EXTENT>::ExtentExtract(pi_rpSpatialObject));
            return AddItem (pi_rpSpatialObject);
            }

        return false;
        }


    /**----------------------------------------------------------------------------
     Adds a reference to spatial object in node.
     The spatial object extent must be included (contained) in node extent

     @param Pointer to spatial object to add a reference to in the index
    -----------------------------------------------------------------------------*/
    virtual bool AddItem(const SPATIAL pi_rpSpatialObject)
        {
        HINVARIANTS;

        // The object must be fully contained in node extent
        HPRECONDITION((HGFIndexGeometryOpProvider<SPATIAL, COORD, EXTENT>::IsSpatialInExtent(pi_rpSpatialObject, m_Extent)));

        // Check if the threshold amount of objects is attained
        if (m_IsLeaf && (m_VectorOfPoints.size() + 1 >= m_SplitTreshold))
            {
            // There are too much objects ... need to split current node
            SplitNode();
            }

        // Check if node is still a leaf ...
        if (m_IsLeaf)
            {
            if (m_VectorOfPoints.size() + 1 >= m_VectorOfPoints.capacity())
                m_VectorOfPoints.reserve (m_VectorOfPoints.size() + GetSplitTreshold() / 10.0);

            // It is a leaf ... we add reference in list
            m_VectorOfPoints.push_back(pi_rpSpatialObject);
            }
        else
            {
            // Attempt to add in one of the subnodes
            bool Added = false;
            for (int i = 0 ; i < 4 && !Added ; ++ i)
                {
                // Check if object is contained in this sub-node
                if (HGFIndexGeometryOpProvider<SPATIAL, COORD, EXTENT>::IsSpatialInExtent(pi_rpSpatialObject, m_apSubNodes[i]->GetExtent()))
                    {
                    // The object is contained ... we add to subnode
                    m_apSubNodes[i]->AddItem(pi_rpSpatialObject);
                    Added = true;
                    }
                }

            // Check if the object was not added in a subnode ...
            if (!Added)
                {
                // The object was not added, evidently because it is too large ...
                // We add it to current node.
                if (m_VectorOfPoints.size()+ 1 >= m_VectorOfPoints.capacity())
                    m_VectorOfPoints.reserve (m_VectorOfPoints.size() + GetSplitTreshold() / 10.0);

                m_VectorOfPoints.push_back(pi_rpSpatialObject);
                }
            }
        return true;
        }

    /**----------------------------------------------------------------------------
     Removes a reference to spatial object in node.
     The spatial object extent must be included (contained) in node extent

     @param Pointer to spatial object to remove reference to in the index
    -----------------------------------------------------------------------------*/
    virtual bool Remove(const SPATIAL pi_rpSpatialObject)
        {
        HINVARIANTS;

        // The object must be fully contained in node extent
        HPRECONDITION(m_Extent.IsPointIn(HGFIndexGeometryOpProvider<SPATIAL, COORD, EXTENT>::ExtentExtract(pi_rpSpatialObject).GetOrigin()) &&
                      m_Extent.IsPointIn(HGFIndexGeometryOpProvider<SPATIAL, COORD, EXTENT>::ExtentExtract(pi_rpSpatialObject).GetCorner()));

        bool Removed = false;

        // Check if node is still a leaf ...
        if (m_IsLeaf)
            {
#if (1)
            // It is a leaf ... we add reference in list
            for (size_t currentIndex = 0 ; !Removed && currentIndex < m_VectorOfPoints.size(); currentIndex++)
                {
                // Check if current object is the one to remove ...
                if (m_VectorOfPoints[currentIndex] == pi_rpSpatialObject)
                    {
                    // We have found it... erase it
                    m_VectorOfPoints.erase(currentIndex);
                    Removed = true;
                    }

                }
#else
            // The object was not removed, evidently because it is too large ...
            vector<SPATIAL>::iterator Itr = m_VectorOfPoints.begin();
            while(Itr != m_VectorOfPoints.end() && !Removed)
                {
                // Check if current object is the one to remove ...
                if (*Itr == pi_rpSpatialObject)
                    {
                    // We have found it... erase it
                    Itr = m_VectorOfPoints.erase(Itr);
                    Removed = true;
                    }
                else
                    {
                    // Not found ... advance to next item
                    ++Itr;
                    }
                }
#endif
            }
        else
            {
            // Attempt to remove in one of the subnodes
            bool RemovalShouldHaveBeenPerformed = false;

            for (int i = 0 ; i < 4 && !RemovalShouldHaveBeenPerformed ; ++ i)
                {
                // Check if object is contained in this sub-node
                if (HGFIndexGeometryOpProvider<SPATIAL, COORD, EXTENT>::IsSpatialInExtent(pi_rpSpatialObject, m_apSubNodes[i]->GetExtent()))
                    {
                    // The object is contained ... we add to subnode
                    Removed = m_apSubNodes[i]->Remove(pi_rpSpatialObject);
                    RemovalShouldHaveBeenPerformed = true;
                    }
                }

            // Check if the object was not removed in a subnode ...
            if (!RemovalShouldHaveBeenPerformed)
                {
#if (1)
                // The object was not removed, evidently because it is too large ...
                for (size_t currentIndex = 0 ; !Removed && currentIndex < m_VectorOfPoints.size(); currentIndex++)
                    {
                    // Check if current object is the one to remove ...
                    if ( m_VectorOfPoints[currentIndex] == pi_rpSpatialObject)
                        {
                        // We have found it... erase it
                        m_VectorOfPoints.erase(currentIndex);
                        Removed = true;
                        }
                    }
#else
                // The object was not removed, evidently because it is too large ...
                vector<SPATIAL>::iterator Itr = m_VectorOfPoints.begin();
                while(Itr != m_VectorOfPoints.end() && !Removed)
                    {
                    // Check if current object is the one to remove ...
                    if (*Itr == pi_rpSpatialObject)
                        {
                        // We have found it... erase it
                        Itr = m_VectorOfPoints.erase(Itr);
                        Removed = true;
                        }
                    else
                        {
                        // Not found ... advance to next item
                        ++Itr;
                        }
                    }
#endif
                }
            }

        return(Removed);
        }


    /**----------------------------------------------------------------------------
     Gets a list of objects located near given coordinate
     the node will search within subnodes and parent nodes through the whole
     node network except in the direction of the initiator

     @param pi_rCoord The coordinate to obtain objects in the vicinity of.

     @param pio_rListOfObjects The list of objects to which are appended objects in the
                              vicinity of given point

     @param pi_rpInitiator Pointer to node that initiates the call. This node
             will not be called to fetch objects

     @return The additional number of objects added to the list
    -----------------------------------------------------------------------------*/
    virtual size_t GetAt(const COORD& pi_rCoord,
                         list<SPATIAL>& pio_rListOfObjects,
                         const HFCPtr<HGFSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> >& pi_rpInitiator = HFCPtr<HGFSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> >(0),
                         HFCPtr<HGFSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> >* po_pLastNode = 0) const
        {
        HINVARIANTS;

        // Save the number of objects currently in list
        size_t InitialNumberOfObjects = pio_rListOfObjects.size();

        // Check if coordinate falls inside node extent
        if (m_Extent.IsPointOutterIn(pi_rCoord))
            {
            // The point is located inside the node ...
            // Obtain objects from subnodes (if any)
            if (!m_IsLeaf)
                {
                // there are sub-nodes ... fetch from each sub-node (if not initiator)
                if (m_apSubNodes[0] != pi_rpInitiator)
                    if ((m_apSubNodes[0]->GetAt(pi_rCoord, pio_rListOfObjects, const_cast<HGFSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> *>(this), po_pLastNode) >= 1) && (SPATIALFINDER::HasUniqueSolution()))
                        return(1);

                if (m_apSubNodes[1] != pi_rpInitiator)
                    if ((m_apSubNodes[1]->GetAt(pi_rCoord, pio_rListOfObjects, const_cast<HGFSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> *>(this), po_pLastNode) >= 1) && (SPATIALFINDER::HasUniqueSolution()))
                        return(1);

                if (m_apSubNodes[2] != pi_rpInitiator)
                    if ((m_apSubNodes[2]->GetAt(pi_rCoord, pio_rListOfObjects, const_cast<HGFSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> *>(this), po_pLastNode) >= 1) && (SPATIALFINDER::HasUniqueSolution()))
                        return(1);

                if (m_apSubNodes[3] != pi_rpInitiator)
                    if ((m_apSubNodes[3]->GetAt(pi_rCoord, pio_rListOfObjects, const_cast<HGFSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> *>(this), po_pLastNode) >= 1) && (SPATIALFINDER::HasUniqueSolution()))
                        return(1);
                }

            // Search in present list of objects for current node
            for (size_t currentIndex = 0 ; currentIndex < m_VectorOfPoints.size(); currentIndex++)
                {
                // Check if point is in extent of object
                if (SPATIALFINDER::IsAProbableSolution(m_VectorOfPoints[currentIndex], m_Extent, pi_rCoord))
                    {
                    // The point falls inside extent of object .. we add a reference to the list
                    if (SPATIALFINDER::IsASolution(m_VectorOfPoints[currentIndex], m_Extent, pi_rCoord))
                        {
                        pio_rListOfObjects.push_back(m_VectorOfPoints[currentIndex]);

                        // If the last node is desired ... set it
                        if (po_pLastNode)
                            *po_pLastNode = const_cast<HGFSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> *>(this);

                        if (SPATIALFINDER::HasUniqueSolution())
                            return(1);
                        }
                    }
                }
            }

        // Check if there is a parent node
        if (m_pParentNode != 0)
            {
            // There is a parent ... check if it not the initiator
            if (m_pParentNode != pi_rpInitiator)
                {
                // The parent is not the initiator ... we fetch objects
                if ((m_pParentNode->GetAt(pi_rCoord, pio_rListOfObjects, const_cast<HGFSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> *>(this), po_pLastNode) >= 1) && (SPATIALFINDER::HasUniqueSolution()))
                    return(1);
                }
            }

        // Return number of newly found objects
        return(pio_rListOfObjects.size()- InitialNumberOfObjects);
        }


    /**----------------------------------------------------------------------------
     Gets a list of objects located inside (outter) given extent
     the node will search within subnodes and parent nodes through the whole
     node network except in the direction of the initiator

     @param pi_rExtent The extent to obtain objects in the vicinity of.

     @param pio_rListOfObjects The list of objects to which are appended objects inside
                              the given extent

     @param pi_rpInitiator Pointer to node that initiates the call. This node
             will not be called to fetch objects
    -----------------------------------------------------------------------------*/
    virtual size_t GetIn(const EXTENT& pi_rExtent,
                         list<SPATIAL>& pio_rListOfObjects,
                         const HFCPtr<HGFSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> >& pi_rpInitiator = HFCPtr<HGFSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> >(0)) const
        {
        HINVARIANTS;

        // Save the number of objects currently in list
        size_t InitialNumberOfObjects = pio_rListOfObjects.size();

        // Check if coordinate falls inside node extent
        if (m_Extent.Overlaps(pi_rExtent))
            {
            // The point is located inside the node ...
            // Obtain objects from subnodes (if any)
            if (!m_IsLeaf)
                {
                // there are sub-nodes ... fetch from each sub-node (if not initiator)
                if (m_apSubNodes[0] != pi_rpInitiator)
                    m_apSubNodes[0]->GetIn(pi_rExtent, pio_rListOfObjects, const_cast<HGFSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> *>(this));

                if (m_apSubNodes[1] != pi_rpInitiator)
                    m_apSubNodes[1]->GetIn(pi_rExtent, pio_rListOfObjects, const_cast<HGFSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> *>(this));

                if (m_apSubNodes[2] != pi_rpInitiator)
                    m_apSubNodes[2]->GetIn(pi_rExtent, pio_rListOfObjects, const_cast<HGFSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> *>(this));

                if (m_apSubNodes[3] != pi_rpInitiator)
                    m_apSubNodes[3]->GetIn(pi_rExtent, pio_rListOfObjects, const_cast<HGFSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> *>(this));
                }

            // Search in present list of objects for current node
            for (size_t currentIndex = 0 ; currentIndex < m_VectorOfPoints.size(); currentIndex++)
                {
                // Check if point is in extent of object
                if (m_Extent.Overlaps(HGFIndexGeometryOpProvider<SPATIAL, COORD, EXTENT>::ExtentExtract(m_VectorOfPoints[currentIndex])))
                    {
                    // The point falls inside extent of object .. we add a reference to the list
                    pio_rListOfObjects.push_back(m_VectorOfPoints[currentIndex]);
                    }
                }
            }

        // Check if there is a parent node
        if (m_pParentNode != 0)
            {
            // There is a parent ... check if it not the initiator
            if (m_pParentNode != pi_rpInitiator)
                {
                // The parent is not the initiator ... we fetch objects
                m_pParentNode->GetIn(pi_rExtent, pio_rListOfObjects, const_cast<HGFSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> *>(this));
                }
            }

        // Return number of newly found objects
        return(pio_rListOfObjects.size()- InitialNumberOfObjects);
        }


protected:
    void               ValidateInvariants() const
        {
        // The treshold must be valid
        HASSERT(m_SplitTreshold >= MinimalTreshold);

        // If the node is a leaf, then subnodes must not exist
        HASSERT(!m_IsLeaf || ((m_apSubNodes[0] == 0) && (m_apSubNodes[1] == 0) && (m_apSubNodes[2] == 0) && (m_apSubNodes[3] == 0)));

        // If the node is not a leaf, then subnodes must exist
        HASSERT(m_IsLeaf || ((m_apSubNodes[0] != 0) && (m_apSubNodes[1] != 0) && (m_apSubNodes[2] != 0) && (m_apSubNodes[3] != 0)));

        // If the node is a leaf, then split treshold must not be attained
        HASSERT(!m_IsLeaf || m_VectorOfPoints.size() < m_SplitTreshold);
        }


    /**----------------------------------------------------------------------------
     PRIVATE METHOD
     Splits the present leaf into subnodes and repositions current object
     references accordingly
     The node must be a leaf to perform.
    -----------------------------------------------------------------------------*/
    virtual void SplitNode()
        {
        HINVARIANTS;

        // The node must be a leaf
        HPRECONDITION(m_IsLeaf);

        double Width = m_Extent.GetWidth();
        double Height = m_Extent.GetHeight();

        m_apSubNodes[0] = new HGFSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER>(m_SplitTreshold,
                EXTENT(m_Extent.GetXMin(),
                       m_Extent.GetYMin() + (Height / 2),
                       m_Extent.GetXMin() + (Width / 2),
                       m_Extent.GetYMax()),
                this);

        m_apSubNodes[1] = new HGFSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER>(m_SplitTreshold,
                EXTENT(m_Extent.GetXMin() + (Width / 2),
                       m_Extent.GetYMin() + (Height / 2),
                       m_Extent.GetXMax(),
                       m_Extent.GetYMax()),
                this);

        m_apSubNodes[2] = new HGFSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER>(m_SplitTreshold,
                EXTENT(m_Extent.GetXMin(),
                       m_Extent.GetYMin(),
                       m_Extent.GetXMin() + (Width / 2),
                       m_Extent.GetYMin() + (Height / 2)),
                this);

        m_apSubNodes[3] = new HGFSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER>(m_SplitTreshold,
                EXTENT(m_Extent.GetXMin() + (Width / 2),
                       m_Extent.GetYMin(),
                       m_Extent.GetXMax(),
                       m_Extent.GetYMin() + (Height / 2)),
                this);


        // Indicate node is not a leaf anymore
        m_IsLeaf = false;

        // Now attempt to mode current spatial object references to the sub-nodes
        // For every object referenced
        for (size_t currentIndex = 0 ; currentIndex < m_VectorOfPoints.size(); currentIndex++)
            {
            // Attempt to add in one of the subnodes
            bool Added = false;
            for (int i = 0 ; i < 4 && !Added ; ++ i)
                {
                // Check if object is contained in this sub-node
                if (HGFIndexGeometryOpProvider<SPATIAL, COORD, EXTENT>::IsSpatialInExtent((m_VectorOfPoints[currentIndex]), m_apSubNodes[i]->GetExtent()))
                    {
                    if (m_apSubNodes[i]->m_VectorOfPoints.size() + 1 >= m_apSubNodes[i]->m_VectorOfPoints.capacity())
                        m_apSubNodes[i]->m_VectorOfPoints.reserve (m_apSubNodes[i]->m_VectorOfPoints.size() + m_apSubNodes[i]->GetSplitTreshold() / 10.0);

                    // The object is contained ... we add to subnode ... unchecked!
                    m_apSubNodes[i]->m_VectorOfPoints.push_back (m_VectorOfPoints[currentIndex]);
                    Added = true;
                    }
                }


            }
        m_VectorOfPoints.clear();
        }


    bool m_IsLeaf;
    EXTENT m_Extent;

    HFCPtr<HGFSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> > m_pParentNode;

    HFCPtr<HGFSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> > m_apSubNodes[4];

#if (0)
    vector<SPATIAL> m_VectorOfPoints;
#else
    FastVector<SPATIAL> m_VectorOfPoints;
    // FastCompressiblePooledVector<SPATIAL> m_VectorOfPoints;
#endif

    size_t m_SplitTreshold;
    };



// ======================================

// ========================================
/** -----------------------------------------------------------------------------

    This class implements a spatial index default node. It comforms to the
    IHGFSpatialIndexNode pseudo interface defined above. This default
    node maintains in memory all of the data.

    -----------------------------------------------------------------------------
*/
template <class SPATIAL, class COORD = HGF2DPosition, class EXTENT = HGF2DLiteExtent, class SPATIALFINDER = HGF2DSpatialIndexDefaultFinder<SPATIAL, COORD, EXTENT> > class HGFPooledSpatialIndexPointNode : public HFCShareableObject<HGFSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> >
    {
public:

    /**----------------------------------------------------------------------------
     Constructor for node based upon extent only.

     @param pi_SplitTreshold - The split treshold value used to determine when a
                               leaf node must be split. It is the maximum number of
                               objects referenced by node before a split occurs.
                               This value msut be greater than "MinimalTreshold"

     @param pi_rExtent - The extent of the node
    -----------------------------------------------------------------------------*/
    HGFPooledSpatialIndexPointNode(size_t pi_SplitTreshold,
                                   const EXTENT& pi_rExtent,
                                   FastVectorCountLimitedPool<SPATIAL>* pool)
        : m_Extent(pi_rExtent),
          m_IsLeaf(true),
          m_SplitTreshold(pi_SplitTreshold),
          m_VectorOfPoints (pool)
        {
        // Do not allow NULL sized extent ...
        if (!(m_Extent.GetXMax() > m_Extent.GetXMin()))
            m_Extent.SetXMax (m_Extent.GetXMin() + MAX (fabs(m_Extent.GetXMin() * HNumeric<double>::EPSILON_MULTIPLICATOR() * 1000.0), 1.0));
        if (!(m_Extent.GetYMax() > m_Extent.GetYMin()))
            m_Extent.SetYMax (m_Extent.GetYMin() + MAX (fabs(m_Extent.GetYMin() * HNumeric<double>::EPSILON_MULTIPLICATOR() * 1000.0), 1.0));

        HINVARIANTS;
        }

    /**----------------------------------------------------------------------------
     Constructor for node based upon parent node and extent.

     @param pi_SplitTreshold - The split treshold value used to determine when a
                              leaf node must be split. It is the maximum number of
                              objects referenced by node before a split occurs.
                              This value msut be greater than "MinimalTreshold"

     @param pi_rExtent - The extent of the node

     @param pi_rpParentNode - Reference to pointer to parent node for created node
    -----------------------------------------------------------------------------*/
    HGFPooledSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> (size_t pi_SplitTreshold,
                                                                           const EXTENT& pi_rExtent,
                                                                           const HFCPtr<HGFPooledSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> >& pi_rpParentNode)
        : m_Extent(pi_rExtent),
          m_pParentNode(pi_rpParentNode),
          m_IsLeaf(true),
          m_SplitTreshold(pi_SplitTreshold),
          m_VectorOfPoints(pi_rpParentNode->GetPool())
        {
        HINVARIANTS;
        }

    /**----------------------------------------------------------------------------
     Copy constructor - This constructor may only be called when the
     given node has no parent

     @param pi_rNode - Reference to node to duplicate. This node must have no parent
    -----------------------------------------------------------------------------*/
    HGFPooledSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> (const HGFPooledSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER>& pi_rNode)
        : m_Extent(pi_rNode.m_rExtent),
          m_pParentNode(0),
          m_IsLeaf(pi_rNode.m_IsLeaf),
          m_ListOfObjects(pi_rNode.m_ListOfObjects),
          m_SplitTreshold(pi_rNode.m_SplitTreshold),
          m_VectorOfPoints (pi_rNode.GetPool())
        {
        // A copied node may not have a parent
        HPRECONDITION(pi_rNode.m_pParentNode == 0);

        // Check if there are subnodes
        if (!m_IsLeaf)
            {
            m_apSubNodes[0] = new Node(pi_rNode.m_apSubNodes[0], this);
            m_apSubNodes[1] = new Node(pi_rNode.m_apSubNodes[1], this);
            m_apSubNodes[2] = new Node(pi_rNode.m_apSubNodes[2], this);
            m_apSubNodes[3] = new Node(pi_rNode.m_apSubNodes[3], this);
            }

        HINVARIANTS;
        }

    /**----------------------------------------------------------------------------
     Alternate Copy constructor ... equivalent but parent node is provided as parameter

     @param pi_rNode - Reference to node to duplicate.

     @param pi_rpParentNode - Pointer to parent node
    -----------------------------------------------------------------------------*/
    HGFPooledSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER>(const HGFPooledSpatialIndexPointNode& pi_rNode,
                                                                          const HFCPtr<HGFPooledSpatialIndexPointNode>& pi_rpParentNode)
        : m_Extent(pi_rNode.m_rExtent),
          m_pParentNode(pi_rpParentNode),
          m_IsLeaf(pi_rNode.m_IsLeaf),
          m_ListOfObjects(pi_rNode.m_ListOfObjects),
          m_SplitTreshold(pi_rNode.m_SplitTreshold),
          m_VectorOfPoints(pi_rParentNode.GetPool())
        {
        // Check if there are subnodes
        if (!m_IsLeaf)
            {
            m_apSubNodes[0] = new HGFPooledSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER>(pi_rNode.m_apSubNodes[0], this);
            m_apSubNodes[1] = new HGFPooledSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER>(pi_rNode.m_apSubNodes[1], this);
            m_apSubNodes[2] = new HGFPooledSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER>(pi_rNode.m_apSubNodes[2], this);
            m_apSubNodes[3] = new HGFPooledSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER>(pi_rNode.m_apSubNodes[3], this);
            }

        HINVARIANTS;
        }

    /**----------------------------------------------------------------------------
     Destroyer
    -----------------------------------------------------------------------------*/
    ~HGFPooledSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> () {}


    /**----------------------------------------------------------------------------
     Returns the pool

     @return the pool
    -----------------------------------------------------------------------------*/
    FastVectorCountLimitedPool<SPATIAL>*
    GetPool() const
        {
        return m_VectorOfPoints.GetPool();
        }

    /**----------------------------------------------------------------------------
     Sets the parent node .. this can only be called for a node that has no parent

     @param pi_rpParentNode - Pointer to parent node
    -----------------------------------------------------------------------------*/
    void SetParentNode(const HFCPtr<HGFPooledSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> >& pi_rpParentNode)
        {
        HINVARIANTS;

        // The node must be orphan(parentless)
        HPRECONDITION(m_pParentNode == 0);

        m_pParentNode = pi_rpParentNode;
        }

    /**----------------------------------------------------------------------------
     Returns the parent node

     @return reference to the parent node
    -----------------------------------------------------------------------------*/
    const HFCPtr<HGFPooledSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> >&
    GetParentNode() const
        {
        return m_pParentNode;
        }

    /**----------------------------------------------------------------------------
     Sets the sub nodes ... this can only be for a leaf node and the subnodes must
     exist.

     @param pi_apSubNodes - Array of four sub-nodes pointer that must point to existing
                           nodes
    -----------------------------------------------------------------------------*/
    void SetSubNodes(HFCPtr<HGFPooledSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> > pi_apSubNodes[4])
        {
        HINVARIANTS;

        // Table must be provided and each subnode pointer point to an existing node
        HPRECONDITION(pi_apSubNodes != NULL);
        HPRECONDITION(pi_apSubNodes[0] != 0);
        HPRECONDITION(pi_apSubNodes[1] != 0);
        HPRECONDITION(pi_apSubNodes[2] != 0);
        HPRECONDITION(pi_apSubNodes[3] != 0);

        // Copy new nodes
        m_apSubNodes[0] = pi_apSubNodes[0];
        m_apSubNodes[1] = pi_apSubNodes[1];
        m_apSubNodes[2] = pi_apSubNodes[2];
        m_apSubNodes[3] = pi_apSubNodes[3];

        // Set parent node to all subnodes
        m_apSubNodes[0]->SetParentNode(this);
        m_apSubNodes[1]->SetParentNode(this);
        m_apSubNodes[2]->SetParentNode(this);
        m_apSubNodes[3]->SetParentNode(this);

        // Indicate node is not a leaf anymore
        m_IsLeaf = false;
        }

    /**----------------------------------------------------------------------------
     Sets the split treshold value for node

     @param pi_SplitTreshold - The split treshold value used to determine when a
                              leaf node must be split. It is the maximum number of
                              objects referenced by node before a split occurs.
                              This value msut be greater than "MinimalTreshold"
    -----------------------------------------------------------------------------*/
    void SetSplitTreshold(size_t pi_SplitTreshold)
        {
        HINVARIANTS;

        HPRECONDITION(pi_SplitTreshold >= MinimalTreshold);

        // Set new value
        m_SplitTreshold = pi_SplitTreshold;

        // Check if there are sub-nodes
        if (!m_IsLeaf)
            {
            // Set treshold for all sub-nodes
            m_apSubNodes[0]->SetSplitTreshold(m_SplitTreshold);
            m_apSubNodes[1]->SetSplitTreshold(m_SplitTreshold);
            m_apSubNodes[2]->SetSplitTreshold(m_SplitTreshold);
            m_apSubNodes[3]->SetSplitTreshold(m_SplitTreshold);
            }
        else
            {
            // The node is a leaf ... check if the current number of objects is greater than new treshold
            if (m_VectorOfPoints.size() >= m_SplitTreshold)
                {
                // Treshold attained ... we must split
                SplitNode();
                }
            }
        }

    /**----------------------------------------------------------------------------
     Returns the split treshold value for node

    -----------------------------------------------------------------------------*/
    size_t
    GetSplitTreshold() const
        {
        HINVARIANTS;

        return(m_SplitTreshold);
        }


    /**----------------------------------------------------------------------------
     Indicates if node is leaf

     @return true if node is a leaf
    -----------------------------------------------------------------------------*/
    bool IsLeaf() const
        {
        HINVARIANTS;

        return(m_IsLeaf);
        }

    /**----------------------------------------------------------------------------
     Returns node extent

     @return reference to extent of node
    -----------------------------------------------------------------------------*/
    const EXTENT&
    GetExtent() const
        {
        HINVARIANTS;

        return(m_Extent);
        }


    /**----------------------------------------------------------------------------
     Adds a reference to spatial object in node.
     The spatial object extent must be included (contained) in node extent

     @param Pointer to spatial object to add a reference to in the index
    -----------------------------------------------------------------------------*/
    bool AddItemConditional (const SPATIAL pi_rpSpatialObject, bool ExtentFixed)
        {
        HINVARIANTS;

        // Check is spatial extent is in node ...
        if (HGFIndexGeometryOpProvider<SPATIAL, COORD, EXTENT>::IsSpatialInExtent(pi_rpSpatialObject, m_Extent))

//            if (m_Extent.IsPointIn(HGFIndexGeometryOpProvider<SPATIAL, COORD, EXTENT>::ExtentExtract(pi_rpSpatialObject).GetOrigin()) &&
//                m_Extent.IsPointIn(HGFIndexGeometryOpProvider<SPATIAL, COORD, EXTENT>::ExtentExtract(pi_rpSpatialObject).GetCorner()))
            {
            return AddItem (pi_rpSpatialObject);
            }
        // The spatial object is not in extent ... check if we can increase extent (not extent fixed, no parent and no sub-nodes)
        else if (!ExtentFixed && GetParentNode() == NULL && IsLeaf())
            {
            // We can increase the extent ... do it
            m_Extent = HGFIndexGeometryOpProvider<SPATIAL, COORD, EXTENT>::MergeExtents(m_Extent,
                                                                                        HGFIndexGeometryOpProvider<SPATIAL, COORD, EXTENT>::ExtentExtract(pi_rpSpatialObject));
            return AddItem (pi_rpSpatialObject);
            }

        return false;
        }


    /**----------------------------------------------------------------------------
     Adds a reference to spatial object in node.
     The spatial object extent must be included (contained) in node extent

     @param Pointer to spatial object to add a reference to in the index
    -----------------------------------------------------------------------------*/
    virtual bool AddItem(const SPATIAL pi_rpSpatialObject)
        {
        HINVARIANTS;

        // The object must be fully contained in node extent
        HPRECONDITION((HGFIndexGeometryOpProvider<SPATIAL, COORD, EXTENT>::IsSpatialInExtent(pi_rpSpatialObject, m_Extent)));

        // Check if the threshold amount of objects is attained
        if (m_IsLeaf && (m_VectorOfPoints.size() + 1 >= m_SplitTreshold))
            {
            // There are too much objects ... need to split current node
            SplitNode();
            }

        // Check if node is still a leaf ...
        if (m_IsLeaf)
            {
            if (m_VectorOfPoints.size() + 1 >= m_VectorOfPoints.capacity())
                m_VectorOfPoints.reserve (m_VectorOfPoints.size() + GetSplitTreshold() / 10.0);

            // It is a leaf ... we add reference in list
            m_VectorOfPoints.push_back(pi_rpSpatialObject);
            }
        else
            {
            // Attempt to add in one of the subnodes
            bool Added = false;
            for (int i = 0 ; i < 4 && !Added ; ++ i)
                {
                // Check if object is contained in this sub-node
                if (HGFIndexGeometryOpProvider<SPATIAL, COORD, EXTENT>::IsSpatialInExtent(pi_rpSpatialObject, m_apSubNodes[i]->GetExtent()))
                    {
                    // The object is contained ... we add to subnode
                    m_apSubNodes[i]->AddItem(pi_rpSpatialObject);
                    Added = true;
                    }
                }

            // Check if the object was not added in a subnode ...
            if (!Added)
                {
                // The object was not added, evidently because it is too large ...
                // We add it to current node.
                if (m_VectorOfPoints.size()+ 1 >= m_VectorOfPoints.capacity())
                    m_VectorOfPoints.reserve (m_VectorOfPoints.size() + GetSplitTreshold() / 10.0);

                m_VectorOfPoints.push_back(pi_rpSpatialObject);
                }
            }
        return true;
        }

    /**----------------------------------------------------------------------------
     Removes a reference to spatial object in node.
     The spatial object extent must be included (contained) in node extent

     @param Pointer to spatial object to remove reference to in the index
    -----------------------------------------------------------------------------*/
    virtual bool Remove(const SPATIAL pi_rpSpatialObject)
        {
        HINVARIANTS;

        // The object must be fully contained in node extent
        HPRECONDITION(m_Extent.IsPointIn(HGFIndexGeometryOpProvider<SPATIAL, COORD, EXTENT>::ExtentExtract(pi_rpSpatialObject).GetOrigin()) &&
                      m_Extent.IsPointIn(HGFIndexGeometryOpProvider<SPATIAL, COORD, EXTENT>::ExtentExtract(pi_rpSpatialObject).GetCorner()));

        bool Removed = false;

        // Check if node is still a leaf ...
        if (m_IsLeaf)
            {
#if (1)
            // It is a leaf ... we add reference in list
            for (size_t currentIndex = 0 ; !Removed && currentIndex < m_VectorOfPoints.size(); currentIndex++)
                {
                // Check if current object is the one to remove ...
                if (m_VectorOfPoints[currentIndex] == pi_rpSpatialObject)
                    {
                    // We have found it... erase it
                    m_VectorOfPoints.erase(currentIndex);
                    Removed = true;
                    }

                }
#else
            // The object was not removed, evidently because it is too large ...
            vector<SPATIAL>::iterator Itr = m_VectorOfPoints.begin();
            while(Itr != m_VectorOfPoints.end() && !Removed)
                {
                // Check if current object is the one to remove ...
                if (*Itr == pi_rpSpatialObject)
                    {
                    // We have found it... erase it
                    Itr = m_VectorOfPoints.erase(Itr);
                    Removed = true;
                    }
                else
                    {
                    // Not found ... advance to next item
                    ++Itr;
                    }
                }
#endif
            }
        else
            {
            // Attempt to remove in one of the subnodes
            bool RemovalShouldHaveBeenPerformed = false;

            for (int i = 0 ; i < 4 && !RemovalShouldHaveBeenPerformed ; ++ i)
                {
                // Check if object is contained in this sub-node
                if (HGFIndexGeometryOpProvider<SPATIAL, COORD, EXTENT>::IsSpatialInExtent(pi_rpSpatialObject, m_apSubNodes[i]->GetExtent()))
                    {
                    // The object is contained ... we add to subnode
                    Removed = m_apSubNodes[i]->Remove(pi_rpSpatialObject);
                    RemovalShouldHaveBeenPerformed = true;
                    }
                }

            // Check if the object was not removed in a subnode ...
            if (!RemovalShouldHaveBeenPerformed)
                {
#if (1)
                // The object was not removed, evidently because it is too large ...
                for (size_t currentIndex = 0 ; !Removed && currentIndex < m_VectorOfPoints.size(); currentIndex++)
                    {
                    // Check if current object is the one to remove ...
                    if ( m_VectorOfPoints[currentIndex] == pi_rpSpatialObject)
                        {
                        // We have found it... erase it
                        m_VectorOfPoints.erase(currentIndex);
                        Removed = true;
                        }
                    }
#else
                // The object was not removed, evidently because it is too large ...
                vector<SPATIAL>::iterator Itr = m_VectorOfPoints.begin();
                while(Itr != m_VectorOfPoints.end() && !Removed)
                    {
                    // Check if current object is the one to remove ...
                    if (*Itr == pi_rpSpatialObject)
                        {
                        // We have found it... erase it
                        Itr = m_VectorOfPoints.erase(Itr);
                        Removed = true;
                        }
                    else
                        {
                        // Not found ... advance to next item
                        ++Itr;
                        }
                    }
#endif
                }
            }

        return(Removed);
        }


    /**----------------------------------------------------------------------------
     Gets a list of objects located near given coordinate
     the node will search within subnodes and parent nodes through the whole
     node network except in the direction of the initiator

     @param pi_rCoord The coordinate to obtain objects in the vicinity of.

     @param pio_rListOfObjects The list of objects to which are appended objects in the
                              vicinity of given point

     @param pi_rpInitiator Pointer to node that initiates the call. This node
             will not be called to fetch objects

     @return The additional number of objects added to the list
    -----------------------------------------------------------------------------*/
    virtual size_t GetAt(const COORD& pi_rCoord,
                         list<SPATIAL>& pio_rListOfObjects,
                         const HFCPtr<HGFPooledSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> >& pi_rpInitiator = HFCPtr<HGFSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> >(0),
                         HFCPtr<HGFPooledSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> >* po_pLastNode = 0) const
        {
        HINVARIANTS;

        // Save the number of objects currently in list
        size_t InitialNumberOfObjects = pio_rListOfObjects.size();

        // Check if coordinate falls inside node extent
        if (m_Extent.IsPointOutterIn(pi_rCoord))
            {
            // The point is located inside the node ...
            // Obtain objects from subnodes (if any)
            if (!m_IsLeaf)
                {
                // there are sub-nodes ... fetch from each sub-node (if not initiator)
                if (m_apSubNodes[0] != pi_rpInitiator)
                    if ((m_apSubNodes[0]->GetAt(pi_rCoord, pio_rListOfObjects, const_cast<HGFPooledSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> *>(this), po_pLastNode) >= 1) && (SPATIALFINDER::HasUniqueSolution()))
                        return(1);

                if (m_apSubNodes[1] != pi_rpInitiator)
                    if ((m_apSubNodes[1]->GetAt(pi_rCoord, pio_rListOfObjects, const_cast<HGFPooledSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> *>(this), po_pLastNode) >= 1) && (SPATIALFINDER::HasUniqueSolution()))
                        return(1);

                if (m_apSubNodes[2] != pi_rpInitiator)
                    if ((m_apSubNodes[2]->GetAt(pi_rCoord, pio_rListOfObjects, const_cast<HGFPooledSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> *>(this), po_pLastNode) >= 1) && (SPATIALFINDER::HasUniqueSolution()))
                        return(1);

                if (m_apSubNodes[3] != pi_rpInitiator)
                    if ((m_apSubNodes[3]->GetAt(pi_rCoord, pio_rListOfObjects, const_cast<HGFPooledSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> *>(this), po_pLastNode) >= 1) && (SPATIALFINDER::HasUniqueSolution()))
                        return(1);
                }

            // Search in present list of objects for current node
            for (size_t currentIndex = 0 ; currentIndex < m_VectorOfPoints.size(); currentIndex++)
                {
                // Check if point is in extent of object
                if (SPATIALFINDER::IsAProbableSolution(m_VectorOfPoints[currentIndex], m_Extent, pi_rCoord))
                    {
                    // The point falls inside extent of object .. we add a reference to the list
                    if (SPATIALFINDER::IsASolution(m_VectorOfPoints[currentIndex], m_Extent, pi_rCoord))
                        {
                        pio_rListOfObjects.push_back(m_VectorOfPoints[currentIndex]);

                        // If the last node is desired ... set it
                        if (po_pLastNode)
                            *po_pLastNode = const_cast<HGFPooledSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> *>(this);

                        if (SPATIALFINDER::HasUniqueSolution())
                            return(1);
                        }
                    }
                }
            }

        // Check if there is a parent node
        if (m_pParentNode != 0)
            {
            // There is a parent ... check if it not the initiator
            if (m_pParentNode != pi_rpInitiator)
                {
                // The parent is not the initiator ... we fetch objects
                if ((m_pParentNode->GetAt(pi_rCoord, pio_rListOfObjects, const_cast<HGFPooledSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> *>(this), po_pLastNode) >= 1) && (SPATIALFINDER::HasUniqueSolution()))
                    return(1);
                }
            }

        // Return number of newly found objects
        return(pio_rListOfObjects.size()- InitialNumberOfObjects);
        }


    /**----------------------------------------------------------------------------
     Gets a list of objects located inside (outter) given extent
     the node will search within subnodes and parent nodes through the whole
     node network except in the direction of the initiator

     @param pi_rExtent The extent to obtain objects in the vicinity of.

     @param pio_rListOfObjects The list of objects to which are appended objects inside
                              the given extent

     @param pi_rpInitiator Pointer to node that initiates the call. This node
             will not be called to fetch objects
    -----------------------------------------------------------------------------*/
    virtual size_t GetIn(const EXTENT& pi_rExtent,
                         list<SPATIAL>& pio_rListOfObjects,
                         const HFCPtr<HGFPooledSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> >& pi_rpInitiator = HFCPtr<HGFPooledSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> >(0)) const
        {
        HINVARIANTS;

        // Save the number of objects currently in list
        size_t InitialNumberOfObjects = pio_rListOfObjects.size();

        // Check if coordinate falls inside node extent
        if (m_Extent.Overlaps(pi_rExtent))
            {
            // The point is located inside the node ...
            // Obtain objects from subnodes (if any)
            if (!m_IsLeaf)
                {
                // there are sub-nodes ... fetch from each sub-node (if not initiator)
                if (m_apSubNodes[0] != pi_rpInitiator)
                    m_apSubNodes[0]->GetIn(pi_rExtent, pio_rListOfObjects, const_cast<HGFPooledSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> *>(this));

                if (m_apSubNodes[1] != pi_rpInitiator)
                    m_apSubNodes[1]->GetIn(pi_rExtent, pio_rListOfObjects, const_cast<HGFPooledSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> *>(this));

                if (m_apSubNodes[2] != pi_rpInitiator)
                    m_apSubNodes[2]->GetIn(pi_rExtent, pio_rListOfObjects, const_cast<HGFPooledSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> *>(this));

                if (m_apSubNodes[3] != pi_rpInitiator)
                    m_apSubNodes[3]->GetIn(pi_rExtent, pio_rListOfObjects, const_cast<HGFPooledSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> *>(this));
                }

            // Search in present list of objects for current node
            for (size_t currentIndex = 0 ; currentIndex < m_VectorOfPoints.size(); currentIndex++)
                {
                // Check if point is in extent of object
                if (m_Extent.Overlaps(HGFIndexGeometryOpProvider<SPATIAL, COORD, EXTENT>::ExtentExtract(m_VectorOfPoints[currentIndex])))
                    {
                    // The point falls inside extent of object .. we add a reference to the list
                    pio_rListOfObjects.push_back(m_VectorOfPoints[currentIndex]);
                    }
                }
            }

        // Check if there is a parent node
        if (m_pParentNode != 0)
            {
            // There is a parent ... check if it not the initiator
            if (m_pParentNode != pi_rpInitiator)
                {
                // The parent is not the initiator ... we fetch objects
                m_pParentNode->GetIn(pi_rExtent, pio_rListOfObjects, const_cast<HGFPooledSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> *>(this));
                }
            }

        // Return number of newly found objects
        return(pio_rListOfObjects.size()- InitialNumberOfObjects);
        }


protected:
    void               ValidateInvariants() const
        {
        // The treshold must be valid
        HASSERT(m_SplitTreshold >= MinimalTreshold);

        // If the node is a leaf, then subnodes must not exist
        HASSERT(!m_IsLeaf || ((m_apSubNodes[0] == 0) && (m_apSubNodes[1] == 0) && (m_apSubNodes[2] == 0) && (m_apSubNodes[3] == 0)));

        // If the node is not a leaf, then subnodes must exist
        HASSERT(m_IsLeaf || ((m_apSubNodes[0] != 0) && (m_apSubNodes[1] != 0) && (m_apSubNodes[2] != 0) && (m_apSubNodes[3] != 0)));

        // If the node is a leaf, then split treshold must not be attained
        HASSERT(!m_IsLeaf || m_VectorOfPoints.size() < m_SplitTreshold);
        }


    /**----------------------------------------------------------------------------
     PRIVATE METHOD
     Splits the present leaf into subnodes and repositions current object
     references accordingly
     The node must be a leaf to perform.
    -----------------------------------------------------------------------------*/
    virtual void SplitNode()
        {
        HINVARIANTS;

        // The node must be a leaf
        HPRECONDITION(m_IsLeaf);

        double Width = m_Extent.GetWidth();
        double Height = m_Extent.GetHeight();

        m_apSubNodes[0] = new HGFPooledSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER>(m_SplitTreshold,
                EXTENT(m_Extent.GetXMin(),
                       m_Extent.GetYMin() + (Height / 2),
                       m_Extent.GetXMin() + (Width / 2),
                       m_Extent.GetYMax()),
                this);

        m_apSubNodes[1] = new HGFPooledSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER>(m_SplitTreshold,
                EXTENT(m_Extent.GetXMin() + (Width / 2),
                       m_Extent.GetYMin() + (Height / 2),
                       m_Extent.GetXMax(),
                       m_Extent.GetYMax()),
                this);

        m_apSubNodes[2] = new HGFPooledSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER>(m_SplitTreshold,
                EXTENT(m_Extent.GetXMin(),
                       m_Extent.GetYMin(),
                       m_Extent.GetXMin() + (Width / 2),
                       m_Extent.GetYMin() + (Height / 2)),
                this);

        m_apSubNodes[3] = new HGFPooledSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER>(m_SplitTreshold,
                EXTENT(m_Extent.GetXMin() + (Width / 2),
                       m_Extent.GetYMin(),
                       m_Extent.GetXMax(),
                       m_Extent.GetYMin() + (Height / 2)),
                this);


        // Indicate node is not a leaf anymore
        m_IsLeaf = false;

        // Now attempt to mode current spatial object references to the sub-nodes
        // For every object referenced
        for (size_t currentIndex = 0 ; currentIndex < m_VectorOfPoints.size(); currentIndex++)
            {
            // Attempt to add in one of the subnodes
            bool Added = false;
            for (int i = 0 ; i < 4 && !Added ; ++ i)
                {
                // Check if object is contained in this sub-node
                if (HGFIndexGeometryOpProvider<SPATIAL, COORD, EXTENT>::IsSpatialInExtent((m_VectorOfPoints[currentIndex]), m_apSubNodes[i]->GetExtent()))
                    {
                    if (m_apSubNodes[i]->m_VectorOfPoints.size() + 1 >= m_apSubNodes[i]->m_VectorOfPoints.capacity())
                        m_apSubNodes[i]->m_VectorOfPoints.reserve (m_apSubNodes[i]->m_VectorOfPoints.size() + m_apSubNodes[i]->GetSplitTreshold() / 10.0);

                    // The object is contained ... we add to subnode ... unchecked!
                    m_apSubNodes[i]->m_VectorOfPoints.push_back (m_VectorOfPoints[currentIndex]);
                    Added = true;
                    }
                }


            }
        m_VectorOfPoints.clear();
        }


    bool m_IsLeaf;
    EXTENT m_Extent;

    HFCPtr<HGFPooledSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> > m_pParentNode;

    HFCPtr<HGFPooledSpatialIndexPointNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> > m_apSubNodes[4];

#if (0)
    vector<SPATIAL> m_VectorOfPoints;
#else
    // FastVector<SPATIAL> m_VectorOfPoints;
    FastCompressiblePooledVector<SPATIAL> m_VectorOfPoints;
#endif

    size_t m_SplitTreshold;
    };



// ======================================

/** -----------------------------------------------------------------------------

    This class implements a spatial index base ... it provides functionality
    related to NODEs

    -----------------------------------------------------------------------------
*/
template<class COORD, class EXTENT, class NODE> class HGF2DSpatialIndexBase
    {

public:

    // Primary methods
    HGF2DSpatialIndexBase(size_t SplitTreshold = DefaultTreshold);
    HGF2DSpatialIndexBase(EXTENT const& pi_rMaxExtent, size_t SplitTreshold = DefaultTreshold);
    HGF2DSpatialIndexBase(const HGF2DSpatialIndexBase&   pi_rSpatialObject);
    virtual             ~HGF2DSpatialIndexBase();

    HGF2DSpatialIndexBase&
    operator=(const HGF2DSpatialIndexBase& pi_rObj);

protected:

    size_t              GetSplitTreshold() const;
    void                SetSplitTreshold(size_t pi_SplitTreshold);

    bool                HasMaxExtent () const;
    EXTENT              GetMaxExtent () const;

    void                ValidateInvariants() const
        {
        // The treshold must be valid
        HASSERT(m_SplitTreshold >= MinimalTreshold);

        // Notice that even if we have strong aggregation we do not check invariants of root node
        };




    mutable HFCPtr<NODE> m_pLastNode;
    HFCPtr<NODE> m_pRootNode;

    size_t m_SplitTreshold;

    EXTENT m_MaxExtent;
    bool            m_HasMaxExtent;


private:

    };



/** -----------------------------------------------------------------------------

    This class implements a spatial index

    -----------------------------------------------------------------------------
*/
template<class SPATIAL, class COORD = HGF2DPosition, class EXTENT = HGF2DLiteExtent, class SPATIALFINDER = HGF2DSpatialIndexDefaultFinder<SPATIAL, COORD, EXTENT>, class NODE = HGFSpatialIndexDefaultNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> > class HGF2DSpatialIndex : HGF2DSpatialIndexBase<COORD, EXTENT, NODE>
    {
    typedef list<SPATIAL> ItemList;

public:

    // Primary methods
    HGF2DSpatialIndex(size_t SplitTreshold = DefaultTreshold);
    HGF2DSpatialIndex(EXTENT const& pi_rMaxExtent, size_t SplitTreshold = DefaultTreshold);
    HGF2DSpatialIndex(const HGF2DSpatialIndex&   pi_rSpatialObject);
    virtual             ~HGF2DSpatialIndex();

    HGF2DSpatialIndex&
    operator=(const HGF2DSpatialIndex& pi_rObj);


    bool                AddItem(const SPATIAL pi_pSpatialObject);
    bool                RemoveItem(const SPATIAL pi_pSpatialObject);

    size_t              GetAt(const COORD& pi_rCoord,
                              list<SPATIAL>& pio_rListOfObjects) const;

    size_t              GetIn(const EXTENT& pi_rExtent,
                              list<SPATIAL>& pio_rListOfObjects) const;
    size_t              GetAll(list<SPATIAL>& pio_rListOfObjects) const;

//        void                Query       (double pi_x, double pi_y, ItemList & pio_rItems) const;
//        SPATIAL             QuerySingle (double pi_x, double pi_y) const;

    void                Dump(ofstream& outStream) const;

    void                            PushRootDown(const EXTENT& pi_rObjectExtent);
protected:

    void                ValidateInvariants() const
        {
        };

private:

    };


/** -----------------------------------------------------------------------------

    This class implements a spatial index

    -----------------------------------------------------------------------------
*/
template<class SPATIAL, class COORD = HGF2DPosition, class EXTENT = HGF2DLiteExtent, class SPATIALFINDER = HGF2DSpatialIndexDefaultFinder<SPATIAL, COORD, EXTENT>, class NODE = HGFSpatialIndexDefaultNode<SPATIAL, COORD, EXTENT, SPATIALFINDER> > class HGF2DPooledSpatialIndex : HGF2DSpatialIndexBase<COORD, EXTENT, NODE>
    {
    typedef list<SPATIAL> ItemList;

public:

    // Primary methods
    HGF2DPooledSpatialIndex(FastVectorCountLimitedPool<SPATIAL>* pool, size_t SplitTreshold = DefaultTreshold);
    HGF2DPooledSpatialIndex(EXTENT const& pi_rMaxExtent, FastVectorCountLimitedPool<SPATIAL>* pool, size_t SplitTreshold = DefaultTreshold);
    HGF2DPooledSpatialIndex(const HGF2DPooledSpatialIndex&   pi_rSpatialObject);
    virtual             ~HGF2DPooledSpatialIndex();

    HGF2DPooledSpatialIndex&
    operator=(const HGF2DPooledSpatialIndex& pi_rObj);

    FastVectorCountLimitedPool<SPATIAL>* GetPool() const;

    bool                AddItem(const SPATIAL pi_pSpatialObject);
    bool                RemoveItem(const SPATIAL pi_pSpatialObject);

    size_t              GetAt(const COORD& pi_rCoord,
                              list<SPATIAL>& pio_rListOfObjects) const;

    size_t              GetIn(const EXTENT& pi_rExtent,
                              list<SPATIAL>& pio_rListOfObjects) const;
    size_t              GetAll(list<SPATIAL>& pio_rListOfObjects) const;

//        void                Query       (double pi_x, double pi_y, ItemList & pio_rItems) const;
//        SPATIAL             QuerySingle (double pi_x, double pi_y) const;

    void                Dump(ofstream& outStream) const;
    void                            PushRootDown(const EXTENT& pi_rObjectExtent);
protected:

    void                ValidateInvariants() const
        {
        };

private:
    FastVectorCountLimitedPool<SPATIAL>* m_pool;
    };

END_IMAGEPP_NAMESPACE

#include "HGF2DSpatialIndex.hpp"

