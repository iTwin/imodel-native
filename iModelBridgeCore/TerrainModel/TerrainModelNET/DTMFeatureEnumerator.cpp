/*--------------------------------------------------------------------------------------+
|
|     $Source: TerrainModelNET/DTMFeatureEnumerator.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "StdAfx.h"
#include ".\dtmfeatureenumerator.h"
#using <mscorlib.dll>

BEGIN_BENTLEY_TERRAINMODELNET_NAMESPACE

ref class DTMFeatureInfoDirect : DTMFeatureInfo
    {
    ::DTMFeatureInfo* m_impl;

    internal:
    DTMFeatureInfoDirect (const ::DTMFeatureInfo& impl) : m_impl (new ::DTMFeatureInfo(impl))
        {
        }

    ~DTMFeatureInfoDirect ()
        {
        delete m_impl;
        }
    public:
        //=======================================================================================
        /// <summary>
        /// Duplicate Point Feature Type
        /// </summary>                
        //=======================================================================================
        property DTMFeatureType DtmFeatureType
            {
            virtual DTMFeatureType get () override
                {
                return (DTMFeatureType)m_impl->FeatureType ();
                }
            }

        //=======================================================================================
        /// <summary>
        /// Duplicate Point Feature Id
        /// </summary>                
        //=======================================================================================
        property DTMFeatureId DtmFeatureId
            {
            virtual DTMFeatureId get () override
                {
                return DTMFeatureId (m_impl->FeatureId ());
                }

            }

        //=======================================================================================
        /// <summary>
        /// Duplicate Point User Tag
        /// </summary>                
        //=======================================================================================
        property ::DTMUserTag DtmUserTag
            {
            virtual ::DTMUserTag get () override
                {
                return m_impl->UserTag ();
                }

            }

        //=======================================================================================
        /// <summary>
        /// Get the points.
        /// </summary>                
        //=======================================================================================
        property array<BGEO::DPoint3d>^ Points
            {
            virtual array<BGEO::DPoint3d> ^ get() override
                {
                bvector<DPoint3d> points;
                m_impl->GetFeaturePoints (points);
                array<BGEO::DPoint3d>^ ret = gcnew array<BGEO::DPoint3d> ((int)points.size ());
                pin_ptr<BGEO::DPoint3d> pRet = &ret[0];

                memcpy (pRet, points.data (), sizeof (DPoint3d)* points.size ());

                return ret;
                }
            }
    };

DTMFeatureEnumerator::DTMFeatureEnumerator (DTM^ dtm) : m_dtm (dtm)
    {
    DTMFeatureEnumeratorPtr native = Bentley::TerrainModel::DTMFeatureEnumerator::Create (*m_dtm->Handle);
    m_native = native.get ();
    m_native->AddRef ();
    }

bool DTMFeatureEnumerator::Sort::get ()
    {
    return m_native->GetSort ();
    }
void DTMFeatureEnumerator::Sort::set (bool value)
    {
    m_native->SetSort (value);
    }

bool DTMFeatureEnumerator::ReadSourceFeatures::get ()
    {
    return m_native->GetReadSourceFeatures ();
    }
void DTMFeatureEnumerator::ReadSourceFeatures::set (bool value)
    {
    m_native->SetReadSourceFeatures (value);
    }

DTMUserTagRange DTMFeatureEnumerator::UserTagRange::get ()
    {
    DTMUserTag low, high;
    m_native->GetUserTagFilterRange (low, high);
    return DTMUserTagRange (low, high);
    }

void DTMFeatureEnumerator::UserTagRange::set (DTMUserTagRange value)
    {
    m_native->SetUserTagFilterRange (value.Low, value.High);
    }

void DTMFeatureEnumerator::IncludeAllFeatures ()
    {
    m_native->IncludeAllFeatures ();
    }

void DTMFeatureEnumerator::ExcludeAllFeatures ()
    {
    m_native->ExcludeAllFeatures ();
    }

void DTMFeatureEnumerator::IncludeFeature (DTMFeatureType type)
    {
    m_native->IncludeFeature ((::DTMFeatureType)type);
    }

void DTMFeatureEnumerator::ExcludeFeature (DTMFeatureType type)
    {
    m_native->ExcludeFeature ((::DTMFeatureType)type);
    }

DTMFeatureInfo^ DTMFeatureEnumerator::Enumerator::Current::get ()
    {
    return gcnew DTMFeatureInfoDirect (*m_impl->m_current);
    }

#ifdef notdef

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
DTMFeatureScanCriteria::DTMFeatureScanCriteria()
    {
    m_Criteria = gcnew ArrayList();
    m_IsRangeDefined = false;
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
BGEO::DRange3d DTMFeatureScanCriteria::Range::get ()
    {
    return m_Range;
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
bool DTMFeatureScanCriteria::IsRangeDefined::get ()
    {
    return m_IsRangeDefined;
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
void DTMFeatureScanCriteria::ApplyToEnumerator(IBcDTMFeatureEnumerator* pDTMFeatureEnumerator)
    {
    for (int iFeature = 0; iFeature < this->m_Criteria->Count; iFeature++)
        {
        long ftType;
        System::Object^ o = this->m_Criteria[iFeature];
        DTMFeatureType currentFeature = (DTMFeatureType)o; //(DTMFeatureType) *dynamic_cast<__box DTMFeatureType)(o);
        DTMHelpers::Copy(ftType, currentFeature);
        pDTMFeatureEnumerator->includeFeatureType (ftType);
        }
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
void DTMFeatureScanCriteria::IncludeFeatureType(DTMFeatureType featureType)
    {
    System::Object^ o = featureType;
    m_Criteria->Add (o);
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
void DTMFeatureScanCriteria::ExcludeAllFeatureTypes()
    {
    m_Criteria->Clear();
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
void DTMFeatureScanCriteria::SetRange(BGEO::DRange3d range)
    {
    this->m_Range = range;
    this->m_IsRangeDefined = true;
    }


#endif
END_BENTLEY_TERRAINMODELNET_NAMESPACE