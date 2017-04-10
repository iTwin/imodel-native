/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/RealityDataObjects.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/Bentley.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include <RealityPlatform/RealityPlatformAPI.h>
#include <RealityPlatform/RealityDataObjects.h>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

//-------------------------------------------------------------------------------------
// @bsimethod                          Spencer.Mason                            02/2017
//-------------------------------------------------------------------------------------
RealityDataProjectRelationship::RealityDataProjectRelationship(Json::Value jsonInstance)
    {
    if (jsonInstance.isMember("properties"))
        {
        if (jsonInstance["properties"].isMember("RealityDataId") && !jsonInstance["properties"]["RealityDataId"].isNull())
            m_realityDataId = jsonInstance["properties"]["RealityDataId"].asCString();
        if (jsonInstance["properties"].isMember("ProjectId") && !jsonInstance["properties"]["ProjectId"].isNull())
            m_projectId = jsonInstance["properties"]["ProjectId"].asCString();
        }
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            03/2017
//-------------------------------------------------------------------------------------
RealityDataProjectRelationship::RealityDataProjectRelationship()
    {
    }
//-------------------------------------------------------------------------------------
// @bsimethod                          Spencer.Mason                            02/2017
//-------------------------------------------------------------------------------------
Utf8StringCR RealityDataProjectRelationship::GetRealityDataId() const { return m_realityDataId; }

//-------------------------------------------------------------------------------------
// @bsimethod                          Spencer.Mason                            02/2017
//-------------------------------------------------------------------------------------
void RealityDataProjectRelationship::SetRealityDataId(Utf8StringCR realityDataId)  
{
    // Normally the id must comply with sme specific GUID format ... should validate
    m_realityDataId = realityDataId; 
}

//-------------------------------------------------------------------------------------
// @bsimethod                          Spencer.Mason                            02/2017
//-------------------------------------------------------------------------------------
Utf8StringCR RealityDataProjectRelationship::GetProjectId() const { return m_projectId; }


//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            03/2017
//-------------------------------------------------------------------------------------
void RealityDataProjectRelationship::SetProjectId(Utf8StringCR projectId)  
{ 
    // Project id may comply with some naming rules ... check?
    m_projectId = projectId; 
}

//-------------------------------------------------------------------------------------
// @bsimethod                          Spencer.Mason                            02/2017
//-------------------------------------------------------------------------------------
RealityDataDocument::RealityDataDocument(Json::Value jsonInstance)
    {
    if (jsonInstance.isMember("properties"))
        {
        if (jsonInstance["properties"].isMember("ContainerName") && !jsonInstance["properties"]["ContainerName"].isNull())
            m_containerName = jsonInstance["properties"]["ContainerName"].asCString();
        if (jsonInstance["properties"].isMember("Name") && !jsonInstance["properties"]["Name"].isNull())
            m_name = jsonInstance["properties"]["Name"].asCString(); 
        if (jsonInstance["properties"].isMember("Id") && !jsonInstance["properties"]["Id"].isNull())
            m_id = jsonInstance["properties"]["Id"].asCString(); 
        if (jsonInstance["properties"].isMember("FolderId") && !jsonInstance["properties"]["FolderId"].isNull())
            m_folderId = jsonInstance["properties"]["FolderId"].asCString(); 
        if (jsonInstance["properties"].isMember("AccessUrl") && !jsonInstance["properties"]["AccessUrl"].isNull())
            m_accessUrl = jsonInstance["properties"]["AccessUrl"].asCString(); 
        if (jsonInstance["properties"].isMember("RealityDataId") && !jsonInstance["properties"]["RealityDataId"].isNull())
            m_realityDataId = jsonInstance["properties"]["RealityDataId"].asCString(); 
        if (jsonInstance["properties"].isMember("ContentType") && !jsonInstance["properties"]["ContentType"].isNull())
            m_contentType = jsonInstance["properties"]["ContentType"].asCString(); 
        if (jsonInstance["properties"].isMember("Size") && !jsonInstance["properties"]["Size"].isNull())
            m_size = _atoi64(jsonInstance["properties"]["Size"].asCString()); 
        }
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            03/2017
//-------------------------------------------------------------------------------------
RealityDataDocument::RealityDataDocument()
    {
    m_size = 0;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Spencer.Mason                            02/2017
//-------------------------------------------------------------------------------------
Utf8StringCR RealityDataDocument::GetContainerName() const { return m_containerName; }

Utf8StringCR RealityDataDocument::GetName() const { return m_name; }

Utf8StringCR RealityDataDocument::GetId() const { return m_id; }

Utf8StringCR RealityDataDocument::GetFolderId() const { return m_folderId; }

Utf8StringCR RealityDataDocument::GetAccessUrl() const { return m_accessUrl; }

Utf8StringCR RealityDataDocument::GetRealityDataId() const { return m_realityDataId; }

Utf8StringCR RealityDataDocument::GetContentType() const { return m_contentType; }

uint64_t RealityDataDocument::GetSize() const { return m_size; }

//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                           03/2017
//-------------------------------------------------------------------------------------
void RealityDataDocument::SetRealityDataId(Utf8StringCR realityDataId)
{
    // Normally the id must comply with sme specific GUID format ... should validate
    m_realityDataId = realityDataId; 
}

//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                           03/2017
//-------------------------------------------------------------------------------------
void RealityDataDocument::SetId(Utf8StringCR id)
    {
    // Should validate 
    m_id = id;

    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                           03/2017
//-------------------------------------------------------------------------------------
void RealityDataDocument::SetId(Utf8StringCR folderId, Utf8StringCR name)
    {
    // folder id should be validated
    m_id = folderId + "~2F" + name;

    // Result id should be validated
    
    // Keep name and folder id as cache values
    m_name = name;
    m_folderId = folderId;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                           03/2017
//-------------------------------------------------------------------------------------
void RealityDataDocument::SetId(RealityDataFolderCR folder, Utf8StringCR name)
    {
    m_id = folder.GetId() + "~2F" + name;

    // Result id should be validated
    
    // Keep name and folder id as cache values
    m_name = name;
    m_folderId = folder.GetId();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Spencer.Mason                            02/2017
//-------------------------------------------------------------------------------------
RealityDataFolder::RealityDataFolder(Json::Value jsonInstance)
    {
    if (jsonInstance.isMember("properties"))
        {
        if (jsonInstance["properties"].isMember("Name") && !jsonInstance["properties"]["Name"].isNull())
            m_name = jsonInstance["properties"]["Name"].asCString();
        if (jsonInstance["properties"].isMember("RealityDataId") && !jsonInstance["properties"]["RealityDataId"].isNull())
            m_realityDataId = jsonInstance["properties"]["RealityDataId"].asCString();
        if (jsonInstance["properties"].isMember("ParentFolderId") && !jsonInstance["properties"]["ParentFolderId"].isNull())
            m_parentId = jsonInstance["properties"]["ParentFolderId"].asCString();
        }
    }
//-------------------------------------------------------------------------------------
// @bsimethod                          Spencer.Mason                            02/2017
//-------------------------------------------------------------------------------------
Utf8StringCR RealityDataFolder::GetName() const { return m_name; }

Utf8StringCR RealityDataFolder::GetId() const { return m_id; }

Utf8StringCR RealityDataFolder::GetParentId() const { return m_parentId; }

Utf8StringCR RealityDataFolder::GetRealityDataId() const { return m_realityDataId; }

//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                           03/2017
//-------------------------------------------------------------------------------------
void RealityDataFolder::SetRealityDataId(Utf8StringCR realityDataId)
{
    // Normally the id must comply with sme specific GUID format ... should validate
    m_realityDataId = realityDataId; 
}

//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                           03/2017
//-------------------------------------------------------------------------------------
void RealityDataFolder::SetId(Utf8StringCR id)
    {
    // Should validate 
    m_id = id;

    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                           03/2017
//-------------------------------------------------------------------------------------
void RealityDataFolder::SetId(Utf8StringCR parentId, Utf8StringCR name)
    {
    // folder id should be validated
    m_id = parentId + "~2F" + name;

    // Result id should be validated
    
    // Keep name and folder id as cache values
    m_name = name;
    m_parentId = parentId;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                           03/2017
//-------------------------------------------------------------------------------------
void RealityDataFolder::SetId(RealityDataFolderCR parentFolder, Utf8StringCR name)
    {
    m_id = parentFolder.GetId() + "~2F" + name;

    // Result id should be validated
    
    // Keep name and folder id as cache values
    m_name = name;
    m_parentId = parentFolder.GetId();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert         	    4/2016
//-------------------------------------------------------------------------------------
Utf8StringCR RealityDataBase::GetIdentifier() const { return m_identifier; }
void RealityDataBase::SetIdentifier(Utf8CP identifier) { m_identifier = identifier; }

Utf8StringCR RealityDataBase::GetName() const { return m_name; }
void RealityDataBase::SetName(Utf8CP name) { m_name = name; }

Utf8StringCR RealityDataBase::GetResolution() const { return m_resolution; }
void RealityDataBase::SetResolution(Utf8CP res) { m_resolution = res; m_resolutionValueUpToDate = false;}

Utf8StringCR RealityDataBase::GetAccuracy() const { return m_accuracy; }
void RealityDataBase::SetAccuracy(Utf8CP accuracy) { m_accuracy = accuracy; m_accuracyValueUpToDate = false;}

Utf8StringCR RealityDataBase::GetDataset() const { return m_dataset; }
void RealityDataBase::SetDataset(Utf8CP dataset) { m_dataset = dataset; }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert         	    02/2017
//-------------------------------------------------------------------------------------
RealityDataBase::Classification RealityDataBase::GetClassification() const { return m_classification; }
void RealityDataBase::SetClassification(Classification classification) { m_classification = classification; }
Utf8String RealityDataBase::GetClassificationTag() const
    {
    if (Classification::MODEL == m_classification)
        return "Model";
    else if (Classification::TERRAIN == m_classification)
        return "Terrain";
    else if (Classification::IMAGERY == m_classification)
        return "Imagery";
    else if (Classification::PINNED == m_classification)
        return "Pinned";

    return "Undefined";   
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert         	    02/2017
//-------------------------------------------------------------------------------------
//! Static method that converts a classification tag to a classification
StatusInt RealityDataBase::GetClassificationFromTag(RealityDataBase::Classification& returnedClassification, Utf8CP classificationTag)
    {
    Utf8String tag(classificationTag);
    if (tag == "Model")
        returnedClassification = Classification::MODEL;
    else if (tag == "Terrain")
        returnedClassification = Classification::TERRAIN;
    else if (tag == "Imagery")
        returnedClassification = Classification::IMAGERY;
    else if (tag == "Pinned")
        returnedClassification = Classification::PINNED;
    else if (tag == "Undefined")
        returnedClassification = Classification::UNDEFINED_CLASSIF;
    else
        return ERROR;

    return SUCCESS;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason         	    03/2017
//-------------------------------------------------------------------------------------
//! Static method that converts a classification to a classification tag 
Utf8String RealityDataBase::GetTagFromClassification(RealityDataBase::Classification classification)
{
    if (Classification::MODEL == classification)
        return "PUBLIC";
    else if (Classification::TERRAIN == classification)
        return "ENTERPRISE";
    else if (Classification::IMAGERY == classification)
        return "PERMISSION";
    else if (Classification::PINNED == classification)
        return "PRIVATE";

    return "UNDEFINED";
}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert         	    02/2017
//-------------------------------------------------------------------------------------
StatusInt RealityDataBase::SetClassificationByTag(Utf8CP classificationTag)
    {
    return GetClassificationFromTag(m_classification, classificationTag);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason         	    03/2017
//-------------------------------------------------------------------------------------
const bvector<GeoPoint2d>& RealityDataBase::GetFootprint() const 
    { 
    return m_footprint; 
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason         	    03/2017
//-------------------------------------------------------------------------------------
void RealityDataBase::SetFootprint(bvector<GeoPoint2d> const& footprint, Utf8String coordSys) 
    {
    // The given footprint can be empty (remove footprint). If not it must define a shape and clossing point be equal to start point 
    BeAssert((footprint.size() == 0) || 
             ((footprint.size() >= 4) && (footprint[0].latitude == footprint[footprint.size() - 1].latitude) && (footprint[0].longitude == footprint[footprint.size() - 1].longitude)));

    m_coordSys = coordSys; 
    m_footprint = footprint; 
    m_footprintString = ""; 
    m_footprintExtentComputed = false; 
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason         	    03/2017
//-------------------------------------------------------------------------------------
Utf8String RealityDataBase::GetFootprintString() const 
    { 
    if (m_footprintString.length() == 0 && m_footprint.size() > 0)
        {
        m_footprintString = FootprintToString(m_footprint, m_coordSys);
        }
    return m_footprintString;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason         	    03/2017
//-------------------------------------------------------------------------------------
void RealityDataBase::SetFootprintString(Utf8CP footprint) 
    { 
    m_footprintString = footprint; 
    m_footprint.clear(); 
    m_coordSys = ""; 
    m_footprintExtentComputed = false; 

    m_footprint = StringToFootprint(m_footprintString, m_coordSys);
    // The given footprint can be empty (remove footprint). If not it must define a shape and clossing point be equal to start point 
    BeAssert((m_footprint.size() == 0) || 
             ((m_footprint.size() >= 4) && (m_footprint[0].latitude == m_footprint[m_footprint.size() - 1].latitude) && (m_footprint[0].longitude == m_footprint[m_footprint.size() - 1].longitude)));

    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason         	    03/2017
//-------------------------------------------------------------------------------------
Utf8String RealityDataBase::FootprintToString(bvector<GeoPoint2d> footprint, Utf8String coordSys)
    {
    Utf8String filter = "{\\\"points\\\":[";
    for (int i = 0; i < footprint.size() - 1; i++)
        {
        filter.append(Utf8PrintfString("[%f,%f],", footprint[i].longitude, footprint[i].latitude));
        }
    filter.append(Utf8PrintfString("[%f,%f]]", footprint[footprint.size() - 1].longitude, footprint[footprint.size() - 1].latitude));
    if(coordSys.length() > 0)
        filter.append(Utf8PrintfString(", \\\"coordinate_system\\\":\\\"%s\\\"", coordSys));
    filter.append("}");

    return filter;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason         	    03/2017
//-------------------------------------------------------------------------------------
bvector<GeoPoint2d> RealityDataBase::StringToFootprint(Utf8String footprintStr, Utf8String& coordSys)
    {
    coordSys = footprintStr;
    // Extract points.
    footprintStr = footprintStr.substr(footprintStr.find_first_of("["), footprintStr.find_last_of("]") - footprintStr.find_first_of("["));
    size_t delimiterPos = 0;
    while (Utf8String::npos != (delimiterPos = footprintStr.find("[")))
        footprintStr.erase(delimiterPos, 1);
    while (Utf8String::npos != (delimiterPos = footprintStr.find("]")))
        footprintStr.erase(delimiterPos, 1);
    bvector<Utf8String> tokens;
    BeStringUtilities::Split(footprintStr.c_str(), ",", tokens);

    bvector<GeoPoint2d> footprint = bvector<GeoPoint2d>();
    for (size_t i = 0; i < tokens.size(); i += 2)
        {
        GeoPoint2d pt;
        pt.longitude = strtod(tokens[i].c_str(), NULL);
        pt.latitude = strtod(tokens[i + 1].c_str(), NULL);

        footprint.push_back(pt);
        }

    if(coordSys.ContainsI("coordinate_system"))
        {
        coordSys = coordSys.substr(coordSys.find_first_of("c"), coordSys.find_last_of("}") - coordSys.find_first_of("c")); // c-oordinate_system
        coordSys.ReplaceAll("coordinate_system\" : \"", "");
        coordSys.ReplaceAll("\" }", "");
        }
    else
        coordSys = "";
    
    return footprint;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert         	    02/2017
//-------------------------------------------------------------------------------------
DRange2dCR RealityDataBase::GetFootprintExtent() const 
{ 
    if (!m_footprintExtentComputed)
        {
        if (m_footprint.size() > 0)
            {
            m_footprintExtent.low.x = m_footprint[0].longitude;
            m_footprintExtent.low.y = m_footprint[0].latitude;
            m_footprintExtent.high.x = m_footprint[0].longitude;
            m_footprintExtent.high.y = m_footprint[0].latitude;

            for (int index = 1 ; index < m_footprint.size() ; ++index)
                {
                m_footprintExtent.low.x  = min(m_footprint[index].longitude, m_footprintExtent.low.x );
                m_footprintExtent.low.y  = min(m_footprint[index].latitude, m_footprintExtent.low.y );
                m_footprintExtent.high.x = max(m_footprint[index].longitude, m_footprintExtent.high.x);
                m_footprintExtent.high.y = max(m_footprint[index].latitude, m_footprintExtent.high.y);
                }
            }
        else
            {
            m_footprintExtent.low.x = 0.0;
            m_footprintExtent.low.y = 0.0;
            m_footprintExtent.high.x = 0.0;
            m_footprintExtent.high.y = 0.0;
            }
        m_footprintExtentComputed = true;
        }
    return m_footprintExtent; 
}
//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert         	    02/2017
//-------------------------------------------------------------------------------------
bool RealityDataBase::HasApproximateFootprint() const {return m_approximateFootprint;}
void RealityDataBase::SetApproximateFootprint(bool approximateFootprint) {m_approximateFootprint = approximateFootprint;}

Utf8StringCR RealityDataBase::GetDescription() const { return m_description; }
void RealityDataBase::SetDescription(Utf8CP description) { m_description = description; }

RealityDataBase::Visibility RealityDataBase::GetVisibility() const { return m_visibility; }
void RealityDataBase::SetVisibility(RealityDataBase::Visibility visibility) { m_visibility = visibility; }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert         	    02/2017
//-------------------------------------------------------------------------------------
Utf8String RealityDataBase::GetVisibilityTag() const
    {
    return GetTagFromVisibility(m_visibility);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert         	    02/2017
//-------------------------------------------------------------------------------------
//! Static method that converts a classification tag to a classification
StatusInt RealityDataBase::GetVisibilityFromTag(RealityDataBase::Visibility& returnedVisibility, Utf8CP visibilityTag)
    {
    Utf8String tag(visibilityTag);
    if (tag == "PUBLIC")
        returnedVisibility = Visibility::PUBLIC;
    else if (tag == "ENTERPRISE")
        returnedVisibility = Visibility::ENTERPRISE;
    else if (tag == "PERMISSION")
        returnedVisibility = Visibility::PERMISSION;
    else if (tag == "PRIVATE")
        returnedVisibility = Visibility::PRIVATE;
    else if (tag == "UNDEFINED")
        returnedVisibility = Visibility::UNDEFINED_VISIBILITY;
    else
        return ERROR;

    return SUCCESS;
    }


//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert         	    02/2017
//-------------------------------------------------------------------------------------
Utf8String RealityDataBase::GetTagFromVisibility(RealityDataBase::Visibility visibility)
    {
    if (Visibility::PUBLIC == visibility)
        return "PUBLIC";
    else if (Visibility::ENTERPRISE == visibility)
        return "ENTERPRISE";
    else if (Visibility::PERMISSION == visibility)
        return "PERMISSION";
    else if (Visibility::PRIVATE == visibility)
        return "PRIVATE";

    return "UNDEFINED";   
    }


//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert         	    02/2017
//-------------------------------------------------------------------------------------
StatusInt RealityDataBase::SetVisibilityByTag(Utf8CP visibilityTag)
    {
    return GetVisibilityFromTag(m_visibility, visibilityTag);
    }


//-------------------------------------------------------------------------------------
// @bsimethod                                         Alain.Robert           02/2017
//-------------------------------------------------------------------------------------
double RealityDataBase::GetResolutionValue() const
    {
    if (!m_resolutionValueUpToDate)
        {
        bvector<Utf8String> tokens;
        BeStringUtilities::Split(m_resolution.c_str(), "x", tokens);
        BeAssert(2 >= tokens.size());
        if (2 == tokens.size()) 
            {
            // Convert to double.
            double resX = strtod(tokens[0].c_str(), NULL);
            double resY = strtod(tokens[1].c_str(), NULL);

            m_resolutionValue = sqrt(resX * resY);
            m_resolutionValueUpToDate = true;
            }
        else if (1 == tokens.size())
            {
            // Convert to double.
            m_resolutionValue = strtod(tokens[0].c_str(), NULL);

            m_resolutionValueUpToDate = true;
            }
        else 
            return 0.0;
        }

    return m_resolutionValue;
    }



//-------------------------------------------------------------------------------------
// @bsimethod                                         Alain.Robert           02/2017
//-------------------------------------------------------------------------------------
double RealityDataBase::GetAccuracyValue() const
    {
    if (!m_accuracyValueUpToDate)
        {
        bvector<Utf8String> tokens;
        BeStringUtilities::Split(m_accuracy.c_str(), "x", tokens);
        BeAssert(2 >= tokens.size());
        if (2 == tokens.size()) 
            {
            // Convert to double.
            double accurX = strtod(tokens[0].c_str(), NULL);
            double accurY = strtod(tokens[1].c_str(), NULL);

            m_accuracyValue = sqrt(accurX * accurY);
            m_accuracyValueUpToDate = true;
            }
        else if (1 == tokens.size())
            {
            // Convert to double.
            m_accuracyValue = strtod(tokens[0].c_str(), NULL);

            m_accuracyValueUpToDate = true;
            }
        else 
            return 0.0;
        }

    return m_accuracyValue;
    }
//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert         	    02/2017
//-------------------------------------------------------------------------------------
RealityDataBase::RealityDataBase()
    {
    m_footprint = bvector<GeoPoint2d>();
    m_resolutionValueUpToDate = false;
    m_accuracyValueUpToDate = false;
    m_approximateFootprint=false;
    m_visibility = Visibility::UNDEFINED_VISIBILITY;
    m_classification = Classification::UNDEFINED_CLASSIF;
    m_footprintExtentComputed = false;
    }


//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert         	    02/2017
//-------------------------------------------------------------------------------------
RealityDataPtr RealityData::Create()
    {
    return new RealityData();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert         	    02/2017
//-------------------------------------------------------------------------------------
RealityDataPtr RealityData::Create(Utf8StringCR identifier, const DateTime& creationDate, Utf8String const & resolution, const bvector<GeoPoint2d>& footprint, Utf8StringCR name, Utf8StringCR coordSys)
    {
    RealityDataPtr myRealityData = new RealityData();

    myRealityData->SetIdentifier(identifier.c_str());
    myRealityData->SetCreationDateTime(creationDate);
    myRealityData->SetResolution(resolution.c_str());
    myRealityData->SetFootprint(footprint, coordSys);
    myRealityData->SetName(name.c_str());

    return myRealityData;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert         	    02/2017
//-------------------------------------------------------------------------------------
Utf8StringCR RealityData::GetGroup() const { return m_group; }
void RealityData::SetGroup(Utf8CP group) { m_group = group; }

Utf8StringCR RealityData::GetThumbnailDocument() const { return m_thumbnailDocument; }
void RealityData::SetThumbnailDocument(Utf8CP thumbnailDocument) { m_thumbnailDocument = thumbnailDocument; }



//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert         	    02/2017
//-------------------------------------------------------------------------------------
Utf8StringCR RealityData::GetRealityDataType() const { return m_realityDataType; }
void RealityData::SetRealityDataType(Utf8CP realityDataType) { m_realityDataType = realityDataType; }

bool RealityData::IsStreamed() const { return m_streamed; }
void RealityData::SetStreamed(bool streamed) { m_streamed = streamed; }
//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert         	    02/2017
//-------------------------------------------------------------------------------------
DateTimeCR RealityData::GetCreationDateTime() const { return m_creationDate; }
void RealityData::SetCreationDateTime(DateTimeCR creationDate) { m_creationDate = creationDate; }

Utf8StringCR RealityData::GetEnterpriseId() const { return m_enterpriseId; }
void RealityData::SetEnterpriseId(Utf8CP enterpriseId) { m_enterpriseId = enterpriseId; }

Utf8StringCR RealityData::GetContainerName() const { return m_containerName; }
void RealityData::SetContainerName(Utf8CP containerName) { m_containerName = containerName; }

Utf8StringCR RealityData::GetRootDocument() const { return m_rootDocument; }
void RealityData::SetRootDocument(Utf8CP rootDocument) { m_rootDocument = rootDocument; }

Utf8StringCR RealityData::GetMetadataURL() const { return m_metadataURL; }
void RealityData::SetMetadataURL(Utf8CP metadataURL) { m_metadataURL = metadataURL; }

Utf8StringCR RealityData::GetCopyright() const { return m_copyright; }
void RealityData::SetCopyright(Utf8CP copyright) { m_copyright = copyright; }

Utf8StringCR RealityData::GetTermsOfUse() const { return m_termsOfUse; }
void RealityData::SetTermsOfUse(Utf8CP termsOfUse) { m_termsOfUse = termsOfUse; }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert         	    02/2017
//-------------------------------------------------------------------------------------
bool RealityData::IsListable() const { return m_listable; }
void RealityData::SetListable(bool listable) { m_listable = listable; }

DateTime RealityData::GetModifiedDateTime() const { return m_modifiedDate; }
void RealityData::SetModifiedDateTime(DateTime modifiedDate) { m_modifiedDate = modifiedDate; }

Utf8StringCR RealityData::GetOwner() const { return m_owner; }
void RealityData::SetOwner(Utf8CP owner) { m_owner = owner; }

uint64_t RealityData::GetTotalSize() const { return m_totalSize; }
void RealityData::SetTotalSize(uint64_t totalSize) { m_totalSize = totalSize; }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert         	    02/2017
//-------------------------------------------------------------------------------------
RealityData::RealityData()
    {
    m_streamed = true;
    m_listable = true;
    m_totalSize = 0;
    }
