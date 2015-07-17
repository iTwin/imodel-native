#include <TerrainModel/Formats/Formats.h>
#include <TerrainModel/Formats/TerrainExporter.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_TERRAINMODEL

void TerrainExporter::SetFeatureInfoCallback (IFeatureInfoCallback* value)
    {
    m_featureInfoCallback = value;
    }

TerrainExporter::IFeatureInfoCallback* TerrainExporter::GetFeatureInfoCallback ()
    {
    return m_featureInfoCallback;
    }

WCharCP TerrainExporter::_GetFileUnitString ()
    {
    switch (GetFileUnit())
        {
        case FileUnit::Millimeter: return L"millimeter";
        case FileUnit::Centimeter: return L"centimeter";
        case FileUnit::Meter: return L"meter";
        case FileUnit::Kilometer: return L"kilometer";
        case FileUnit::Foot: return L"foot";
        case FileUnit::USSurveyFoot: return L"USSurveyFoot";
        case FileUnit::Inch: return L"inch";
        case FileUnit::Mile: return L"mile";

        case FileUnit::Custom: return L"custom";
        }
    return L"unknown";
    }

BENTLEYDTMFORMATS_EXPORT WCharCP TerrainExporter::GetFileUnitString ()
    {
    return _GetFileUnitString ();
    }

BENTLEYDTMFORMATS_EXPORT FileUnit TerrainExporter::GetFileUnit ()
    {
    return _GetFileUnit ();
    }

BENTLEYDTMFORMATS_EXPORT void TerrainExporter::SetFileUnit (FileUnit value)
    {
    _SetFileUnit (value);
    }
