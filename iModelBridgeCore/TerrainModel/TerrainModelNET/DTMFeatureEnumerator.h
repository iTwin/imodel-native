/*--------------------------------------------------------------------------------------+
|
|     $Source: TerrainModelNET/DTMFeatureEnumerator.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "Bentley.Civil.DTM.h"
#include "DTMHelpers.h"
#include "DTM.h"
BEGIN_BENTLEY_TERRAINMODELNET_NAMESPACE

public value class DTMUserTagRange
    {
    public:
        property DTMUserTag Low;
        property DTMUserTag High;

        static initonly DTMUserTagRange NullRange = DTMUserTagRange (System::Int64::MaxValue, 0);

        DTMUserTagRange (DTMUserTag userTag)
            {
            Low = userTag;
            High = userTag;
            }
        DTMUserTagRange (DTMUserTag low, DTMUserTag high)
            {
            Low = low;
            High = high;
            }
    };

struct DTMFeatureEnumeratorImpl
    {
    Bentley::TerrainModel::DTMFeatureEnumerator&          m_collection;
    Bentley::TerrainModel::DTMFeatureEnumerator::iterator m_current;

    DTMFeatureEnumeratorImpl (Bentley::TerrainModel::DTMFeatureEnumerator& collection) : m_collection (collection), m_current (m_collection.begin ()) {}
    };
public ref class DTMFeatureEnumerator : System::Collections::Generic::IEnumerable <DTMFeatureInfo^>
    {
    ref class Enumerator : System::Collections::Generic::IEnumerator<DTMFeatureInfo^>
        {
        DTMFeatureEnumeratorImpl* m_impl;
        Bentley::TerrainModel::DTMFeatureEnumerator& m_native;
        internal:
            Enumerator (Bentley::TerrainModel::DTMFeatureEnumerator& native) : m_native (native)
                {
                m_native.AddRef ();
                m_impl = nullptr;
                }
            ~Enumerator ()
                {
                if (m_impl)
                    {
                    delete m_impl;
                    m_impl = nullptr;
                    }
                m_native.Release ();
                }
        public:
            virtual bool MoveNext ()
                {
                if (m_impl == nullptr)
                    {
                    m_impl = new DTMFeatureEnumeratorImpl (m_native);
                    return m_impl->m_current != m_impl->m_collection.end ();
                    }
                Bentley::TerrainModel::DTMFeatureEnumerator::iterator endIterator = m_impl->m_collection.end ();
                if (!(m_impl->m_current != endIterator))
                    return false;

                ++m_impl->m_current;
                return m_impl->m_current != endIterator;
                }
        property DTMFeatureInfo^ Current
            {
            virtual DTMFeatureInfo^ get();
            }
        virtual void Reset ()
            {
            m_impl->m_current = m_impl->m_collection.begin ();
            }

        private:
        property System::Object^ CurrentI
            {
            virtual System::Object^ get () sealed = System::Collections::IEnumerator::Current::get
                {
                return Current;
                }
            }
        };

    private:
        DTM^ m_dtm;
        Bentley::TerrainModel::DTMFeatureEnumerator* m_native;

    public:
        DTMFeatureEnumerator (DTM^ dtm);
        ~DTMFeatureEnumerator ()
            {
            m_native->Release();
            }

        property bool Sort
            {
            bool get ();
            void set (bool value);
            }
        property bool ReadSourceFeatures
            {
            bool get ();
            void set (bool value);
            }
            
        property DTMUserTagRange UserTagRange
            {
            DTMUserTagRange get ();
            void set (DTMUserTagRange value);
            }

        // filter by FeatureId
        void IncludeAllFeatures ();
        void ExcludeAllFeatures ();
        void IncludeFeature (DTMFeatureType type);
        void ExcludeFeature (DTMFeatureType type);


        virtual System::Collections::Generic::IEnumerator<DTMFeatureInfo^>^ GetEnumerator ()
            {
            return gcnew Enumerator (*m_native);
            }

    private:
        virtual System::Collections::IEnumerator^ GetEnumeratorI () sealed = System::Collections::IEnumerable::GetEnumerator
            {
            return GetEnumerator ();
            }
    };
#ifdef notdef

//=======================================================================================
/// <summary>
/// Represents feature scan criteria.
/// </summary>
/// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
//=======================================================================================
public ref class DTMFeatureScanCriteria
{
private:

    BGEO::DRange3d      m_Range;
    ArrayList^    m_Criteria;
    bool         m_IsRangeDefined;

internal:

    void ApplyToEnumerator (IBcDTMFeatureEnumerator* pDTMFeatureEnumerator);

public:

    //=======================================================================================
    /// <summary>
    /// Initializes a new instance of the DTMFeatureScanCriteria class.
    /// </summary>                
    /// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
    //=======================================================================================
    DTMFeatureScanCriteria();

    //=======================================================================================
    /// <summary>
    /// Includes a given feature type.
    /// </summary>                
    /// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
    //=======================================================================================
    void IncludeFeatureType (DTMFeatureType featureType);

    //=======================================================================================
    /// <summary>
    /// Excludes all feature types.
    /// </summary>                
    /// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
    //=======================================================================================
    void ExcludeAllFeatureTypes ();

    //=======================================================================================
    /// <summary>
    /// Sets the range of the scan.
    /// </summary>                
    /// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
    //=======================================================================================
    void SetRange(BGEO::DRange3d range);

    //=======================================================================================
    /// <summary>
    /// Gets the range of the scan.
    /// </summary>                
    /// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
    //=======================================================================================
    property BGEO::DRange3d Range
    {
        BGEO::DRange3d get();
    }

    //=======================================================================================
    /// <summary>
    /// Gets whether the range is defined.
    /// </summary>                
    /// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
    //=======================================================================================
    property bool IsRangeDefined
    {
        bool get();
    }
};

#endif

END_BENTLEY_TERRAINMODELNET_NAMESPACE