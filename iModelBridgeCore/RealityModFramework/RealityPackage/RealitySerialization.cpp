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
    REGISTER_REALITY_SOURCE(OsmDataSource, PACKAGE_ELEMENT_OsmSource);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityDataSourcePtr RealityDataSourceSerializer::Load(RealityPackageStatus& status, BeXmlNodeR node)
    {
    auto iterator = m_creators.find(node.GetName());
    if(iterator == m_creators.end())
        {
        status = RealityPackageStatus::UnknownElementType;
        //We don't know this kind of source. Or maybe it's not a source at all.
        //If the node hold a PACKAGE_SOURCE_ATTRIBUTE_Uri and PACKAGE_SOURCE_ATTRIBUTE_Type we might load 
        // it as generic source. Will it does more harm then good?
        return NULL;
        }

    RealityDataSourcePtr pSource = iterator->second->Create();
    
    status = pSource->_Read(node);
    if(RealityPackageStatus::Success != status)
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

    RealityPackageStatus status = source._Write(*pSourceNode);
    if(RealityPackageStatus::Success != status)
        parentNode.RemoveChildNode(pSourceNode);
        
    return status;      
    }

END_BENTLEY_REALITYPACKAGE_NAMESPACE