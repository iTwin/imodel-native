/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPackage/RealitySerialization.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <RealityPackage/RealityPackage.h>
#include <RealityPackage/RealityDataSource.h>
#include <BeXml/BeXml.h>
#include "RealitySerialization.h"


BEGIN_BENTLEY_REALITYPACKAGE_NAMESPACE

#define REGISTER_REALITY_SOURCE(name, ElementName) \
    static struct name##Creator : public IDataSourceCreate \
        {\
        virtual RealityDataSourcePtr Create() const override {return new name();} \
        }s_reality##name; \
        m_creators.Insert(ElementName, &s_reality##name);

//=======================================================================================
//                              RealityDataSourceSerializer
//=======================================================================================

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityDataSourceSerializer::RealityDataSourceSerializer()
    {
    REGISTER_REALITY_SOURCE(RealityDataSource, PACKAGE_ELEMENT_Source);
    REGISTER_REALITY_SOURCE(WmsDataSource, PACKAGE_ELEMENT_WmsSource);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityDataSourcePtr RealityDataSourceSerializer::Load(BeXmlNodeR node)
    {
    auto iterator = m_creators.find(node.GetName());
    if(iterator == m_creators.end())
        {
        //We don't know this kind of source. Or maybe it's not a source at all.
        //If the node hold a PACKAGE_SOURCE_ATTRIBUTE_Uri and PACKAGE_SOURCE_ATTRIBUTE_Type we might load 
        // it as generic source. Will it does more harm then good?
        return NULL;
        }

    RealityDataSourcePtr pSource = iterator->second->Create();

    if(RealityPackageStatus::Success != pSource->_Read(node))
        return NULL;

    return pSource;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSourceSerializer::Store(RealityDataSourceCR source, BeXmlNodeR parentNode)
    {
    BeXmlNodeP pSourceNode = parentNode.AddEmptyElement(source._GetElementName());
    if(NULL == pSourceNode)
        return RealityPackageStatus::UnknownError;

    if(RealityPackageStatus::Success != source._Write(*pSourceNode))
        {
        parentNode.RemoveChildNode(pSourceNode);
        return RealityPackageStatus::UnknownError;
        }

    return RealityPackageStatus::Success;      
    }

END_BENTLEY_REALITYPACKAGE_NAMESPACE