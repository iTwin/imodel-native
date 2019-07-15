/*--------------------------------------------------------------------------------------+
|
|     $Source: FormatsNET/TwelvedXML.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "TwelvedXML.h"

BEGIN_BENTLEY_TERRAINMODELNET_FORMATS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                Cecilio.Shirakawa  05/2019
//---------------------------------------------------------------------------------------
TwelvedXMLImporter^ TwelvedXMLImporter::Create (System::String^ filename)
    {
    pin_ptr<const wchar_t> p = PtrToStringChars (filename);
    TwelvedXMLImporterPtr importer = BENTLEY_NAMESPACE_NAME::TerrainModel::TwelvedXMLImporter::Create (p);
    if (importer.IsValid ())
        return gcnew TwelvedXMLImporter (importer.get ());
    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Cecilio.Shirakawa  05/2019
//---------------------------------------------------------------------------------------
TwelvedXMLImporter::TwelvedXMLImporter (BENTLEY_NAMESPACE_NAME::TerrainModel::TwelvedXMLImporter* importer) : TerrainImporter (importer)
    {
    m_importer = importer;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Cecilio.Shirakawa  05/2019
//---------------------------------------------------------------------------------------
TwelvedXMLOptions TwelvedXMLImporter::Options::get ()
    {
    return (TwelvedXMLOptions)m_importer->GetOptions ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Cecilio.Shirakawa  05/2019
//---------------------------------------------------------------------------------------
void TwelvedXMLImporter::Options::set (TwelvedXMLOptions value)
    {
    m_importer->SetOptions ((Bentley::TerrainModel::TwelvedXMLOptions)value);
    }

END_BENTLEY_TERRAINMODELNET_FORMATS_NAMESPACE