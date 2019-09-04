/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <WebServices/iModelHub/Common.h>
#include <WebServices/iModelHub/Client/Result.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE

USING_NAMESPACE_BENTLEY_WEBSERVICES

DEFINE_POINTER_SUFFIX_TYPEDEFS(iModelBaseInfo);
//=======================================================================================
//@bsiclass                                      Vilius.Kazlauskas             08/2019
//=======================================================================================
struct iModelBaseInfo : RefCountedBase
{
friend struct iModelInfo;
friend struct iModelCreateInfo;
private:
    static const int s_extentSize = 4;
    static const int s_latitudeLimit = 90;
    static const int s_longitudeLimit = 180;

    Utf8String m_name;
    Utf8String m_description;
    Utf8String m_template;
    bvector<double> m_extent;

    iModelBaseInfo() {}
    iModelBaseInfo(Utf8StringCR name, Utf8StringCR description, bvector<double> extent = bvector<double>(), Utf8StringCR imodelTemplate = "")
        : m_name(name), m_description(description), m_extent(extent), m_template(imodelTemplate) {}

    StatusResult ValidateExtent() const;
    StatusResult ValidateCoordinate(double coordinate, int limit) const;
public:
    Utf8StringCR GetName() const { return m_name; }
    Utf8StringCR GetDescription() const { return m_description; }
    Utf8StringCR GetTemplate() const { return m_template; }
    bvector<double> GetExtent() const { return m_extent; }

    void SetName(Utf8StringCR name) { m_name = name; }
    void SetDescription(Utf8StringCR description) { m_description = description; }
    void SetTemplate(Utf8StringCR imodelTemplate) { m_template = imodelTemplate; }
    void SetExtent(bvector<double> extent) { m_extent = extent; }

    StatusResult Validate() const;
};
END_BENTLEY_IMODELHUB_NAMESPACE
