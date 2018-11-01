/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/DwgDgnV8Host.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConverterInternal.h"
#include <VersionedDgnV8Api/Mstn/RealDWG/DwgPlatformHost.h>
#include <shlwapi.h>
#include <Msi.h>    // MsiGetComponentPath

namespace V8RD = Bentley::RealDwg;

BEGIN_DGNDBSYNC_DGNV8_NAMESPACE


#define RSCID_DWGOpenOptions            1
#define RSCID_DWGSaveOptions            2
#define RTYPE_DWGOpenOptions            RTYPE( 'D','O','O','p' )
#define RTYPE_DWGSaveOptions            RTYPE( 'D','S','O','p' )
#define PROXYSHOW_FromAcadRegistry      3
#define LWDEFAULT_FromAcadRegistry      -1
#define DWGBACKGROUNGCOLOR_IsBlack(r, g, b)    (r < 0x20 && g < 0x20 && b < 0x20)


typedef bvector<BentleyApi::WString>    T_SearchPathList;


// Autodesk provides below key path for finding the registry root key of the installed RealDWG component:
#if defined (_X64_)
static WChar            s_ODBXHOSTAPPREGROOT[] = L"{285CAB69-5CB7-240B-697E-996AA63B6415}";
#elif defined (_X86_)
static WChar            s_ODBXHOSTAPPREGROOT[] = L"{82C5BA96-C57B-42B0-96E7-996AA63B6415}";
#endif

static uint16_t const   s_max_lineweights = 32;
static uint16_t const   s_standardDwgWeights[] = {0, 5, 9, 13, 15, 18, 20, 25, 30, 35, 40, 50, 53, 60, 70, 80, 90, 100, 106, 120, 140, 158, 200, 211 };
static byte             s_standardDwgColors[256][3] =
    {
    { 255, 255, 255 },      // 255
    { 255, 255, 255 },      // 0
    { 255,   0,   0 },      // 1
    { 255, 255,   0 },      // 2
    {   0, 255,   0 },      // 3
    {   0, 255, 255 },      // 4
    {   0,   0, 255 },      // 5
    { 255,   0, 255 },      // 6
    { 255, 255, 255 },      // 7
    { 128, 128, 128 },      // 8
    { 192, 192, 192 },      // 9
    { 255,   0,   0 },      // 10
    { 255, 127, 127 },      // 11
    { 204,   0,   0 },      // 12
    { 204, 102, 102 },      // 13
    { 153,   0,   0 },      // 14
    { 153,  76,  76 },      // 15
    { 127,   0,   0 },      // 16
    { 127,  63,  63 },      // 17
    {  76,   0,   0 },      // 18
    {  76,  38,  38 },      // 19
    { 255,  63,   0 },      // 20
    { 255, 159, 127 },      // 21
    { 204,  51,   0 },      // 22
    { 204, 127, 102 },      // 23
    { 153,  38,   0 },      // 24
    { 153,  95,  76 },      // 25
    { 127,  31,   0 },      // 26
    { 127,  79,  63 },      // 27
    {  76,  19,   0 },      // 28
    {  76,  47,  38 },      // 29
    { 255, 127,   0 },      // 30
    { 255, 191, 127 },      // 31
    { 204, 102,   0 },      // 32
    { 204, 153, 102 },      // 33
    { 153,  76,   0 },      // 34
    { 153, 114,  76 },      // 35
    { 127,  63,   0 },      // 36
    { 127,  95,  63 },      // 37
    {  76,  38,   0 },      // 38
    {  76,  57,  38 },      // 39
    { 255, 191,   0 },      // 40
    { 255, 223, 127 },      // 41
    { 204, 153,   0 },      // 42
    { 204, 178, 102 },      // 43
    { 153, 114,   0 },      // 44
    { 153, 133,  76 },      // 45
    { 127,  95,   0 },      // 46
    { 127, 111,  63 },      // 47
    {  76,  57,   0 },      // 48
    {  76,  66,  38 },      // 49
    { 255, 255,   0 },      // 50
    { 255, 255, 127 },      // 51
    { 204, 204,   0 },      // 52
    { 204, 204, 102 },      // 53
    { 153, 153,   0 },      // 54
    { 153, 153,  76 },      // 55
    { 127, 127,   0 },      // 56
    { 127, 127,  63 },      // 57
    {  76,  76,   0 },      // 58
    {  76, 76,   38 },      // 59
    { 191, 255,   0 },      // 60
    { 223, 255, 127 },      // 61
    { 153, 204,   0 },      // 62
    { 178, 204, 102 },      // 63
    { 114, 153,   0 },      // 64
    { 133, 153,  76 },      // 65
    {  95, 127,   0 },      // 66
    { 111, 127,  63 },      // 67
    {  51,  76,   0 },      // 68
    {  66,  76,  38 },      // 69
    { 127, 255,   0 },      // 70
    { 191, 255, 127 },      // 71
    { 102, 204,   0 },      // 72
    { 153, 204, 102 },      // 73
    {  76, 153,   0 },      // 74
    { 114, 153,  76 },      // 75
    {  63, 127,   0 },      // 76
    {  95, 127,  63 },      // 77
    {  38,  76,   0 },      // 78
    {  57,  76,  38 },      // 79
    {  63, 255,   0 },      // 80
    { 159, 255, 127 },      // 81
    {  51, 204,   0 },      // 82
    { 127, 204, 102 },      // 83
    {  38, 153,   0 },      // 84
    {  95, 153,  76 },      // 85
    {  31, 127,   0 },      // 86
    {  79, 127,  63 },      // 87
    {  19,  76,   0 },      // 88
    {  47,  76,  38 },      // 89
    {   0, 255,   0 },      // 90
    { 127, 255, 127 },      // 91
    {   0, 204,   0 },      // 92
    { 102, 204, 102 },      // 93
    {   0, 153,   0 },      // 94
    {  76, 153,  76 },      // 95
    {   0, 127,   0 },      // 96
    {  63, 127,  63 },      // 97
    {   0,  76,   0 },      // 98
    {  38,  76,  38 },      // 99
    {   0, 255,  63 },      // 100
    { 127, 255, 159 },      // 101
    {   0, 204,  51 },      // 102
    { 102, 204, 127 },      // 103
    {   0, 153,  38 },      // 104
    {  76, 153,  95 },      // 105
    {   0, 127,  31 },      // 106
    {  63, 127,  79 },      // 107
    {   0,  76,  19 },      // 108
    {  38,  76,  47 },      // 109
    {   0, 255, 127 },      // 110
    { 127, 255, 191 },      // 111
    {   0, 204, 102 },      // 112
    { 102, 204, 153 },      // 113
    {   0, 153,  76 },      // 114
    {  76, 153, 114 },      // 115
    {   0, 127,  63 },      // 116
    {  63, 127,  95 },      // 117
    {   0,  76,  38 },      // 118
    {  38,  76,  57 },      // 119
    {   0, 255, 191 },      // 120
    { 127, 255, 223 },      // 121
    {   0, 204, 153 },      // 122
    { 102, 204, 178 },      // 123
    {   0, 153, 114 },      // 124
    {  76, 153, 133 },      // 125
    {   0, 127,  95 },      // 126
    {  63, 127, 111 },      // 127
    {   0,  76,  57 },      // 128
    {  38,  76,  66 },      // 129
    {   0, 255, 255 },      // 130
    { 127, 255, 255 },      // 131
    {   0, 204, 204 },      // 132
    { 102, 204, 204 },      // 133
    {   0, 153, 153 },      // 134
    {  76, 153, 153 },      // 135
    {   0, 127, 127 },      // 136
    {  63, 127, 127 },      // 137
    {   0,  76,  76 },      // 138
    {  38,  76,  76 },      // 139
    {   0, 191, 255 },      // 140
    { 127, 223, 255 },      // 141
    {   0, 153, 204 },      // 142
    { 102, 178, 204 },      // 143
    {   0, 114, 153 },      // 144
    {  76, 133, 153 },      // 145
    {   0,  95, 127 },      // 146
    {  63, 111, 127 },      // 147
    {   0,  57,  76 },      // 148
    {  38,  66,  76 },      // 149
    {   0, 127, 255 },      // 150
    { 127, 191, 255 },      // 151
    {   0, 102, 204 },      // 152
    { 102, 153, 204 },      // 153
    {   0,  76, 153 },      // 154
    {  76, 114, 153 },      // 155
    {   0,  63, 127 },      // 156
    {  63,  95, 127 },      // 157
    {   0,  38,  76 },      // 158
    {  38,  57,  76 },      // 159
    {   0,  63, 255 },      // 160
    { 127, 159, 255 },      // 161
    {   0,  51, 204 },      // 162
    { 102, 127, 204 },      // 163
    {   0,  38, 153 },      // 164
    {  76,  95, 153 },      // 165
    {   0,  31, 127 },      // 166
    {  63,  79, 127 },      // 167
    {   0,  19,  76 },      // 168
    {  38,  47,  76 },      // 169
    {   0,   0, 255 },      // 170
    { 127, 127, 255 },      // 171
    {   0,   0, 204 },      // 172
    { 102, 102, 204 },      // 173
    {   0,   0, 153 },      // 174
    {  76,  76, 153 },      // 175
    {   0,   0, 127 },      // 176
    {  63,  63, 127 },      // 177
    {   0,   0,  76 },      // 178
    {  38,  38,  76 },      // 179
    {  63,   0, 255 },      // 180
    { 159, 127, 255 },      // 181
    {  51,   0, 204 },      // 182
    { 127, 102, 204 },      // 183
    {  38,   0, 153 },      // 184
    {  95,  76, 153 },      // 185
    {  31,   0, 127 },      // 186
    {  79,  63, 127 },      // 187
    {  19,   0,  76 },      // 188
    {  47,  38,  76 },      // 189
    { 127,   0, 255 },      // 190
    { 191, 127, 255 },      // 191
    { 102,   0, 204 },      // 192
    { 153, 102, 204 },      // 193
    {  76,   0, 153 },      // 194
    { 114,  76, 153 },      // 195
    {  63,   0, 127 },      // 196
    {  95,  63, 127 },      // 197
    {  38,   0,  76 },      // 198
    {  57,  38,  76 },      // 199
    { 191,   0, 255 },      // 200
    { 223, 127, 255 },      // 201
    { 153,   0, 204 },      // 202
    { 178, 102, 204 },      // 203
    { 114,   0, 153 },      // 204
    { 133,  76, 153 },      // 205
    {  95,   0, 127 },      // 206
    { 111,  63, 127 },      // 207
    {  57,   0,  76 },      // 208
    {  66,  38,  76 },      // 209
    { 255,   0, 255 },      // 210
    { 255, 127, 255 },      // 211
    { 204,   0, 204 },      // 212
    { 204, 102, 204 },      // 213
    { 153,   0, 153 },      // 214
    { 153,  76, 153 },      // 215
    { 127,   0, 127 },      // 216
    { 127,  63, 127 },      // 217
    {  76,   0,  76 },      // 218
    {  76,  38,  76 },      // 219
    { 255,   0, 191 },      // 220
    { 255, 127, 223 },      // 221
    { 204,   0, 153 },      // 222
    { 204, 102, 178 },      // 223
    { 153,   0, 114 },      // 224
    { 153,  76, 133 },      // 225
    { 127,   0,  95 },      // 226
    { 127,  63, 111 },      // 227
    {  76,   0,  57 },      // 228
    {  76,  38,  66 },      // 229
    { 255,   0, 127 },      // 230
    { 255, 127, 191 },      // 231
    { 204,   0, 102 },      // 232
    { 204, 102, 153 },      // 233
    { 153,   0,  76 },      // 234
    { 153,  76, 114 },      // 235
    { 127,   0,  63 },      // 236
    { 127,  63,  95 },      // 237
    {  76,   0,  38 },      // 238
    {  76,  38,  57 },      // 239
    { 255,   0,  63 },      // 240
    { 255, 127, 159 },      // 241
    { 204,   0,  51 },      // 242
    { 204, 102, 127 },      // 243
    { 153,   0,  38 },      // 244
    { 153,  76,  95 },      // 245
    { 127,   0,  31 },      // 246
    { 127,  63,  79 },      // 247
    {  76,   0,  19 },      // 248
    {  76,  38,  47 },      // 249
    {  51,  51,  51 },      // 250
    {  91,  91,  91 },      // 251
    { 132, 132, 132 },      // 252
    { 173, 173, 173 },      // 253
    { 214, 214, 214 },      // 254
    };  // s_standardDwgColors


/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          02/17
+===============+===============+===============+===============+===============+======*/
struct DwgConversionSettings : public V8RD::IDwgConversionSettings
{
// these are persistent V8 resource data structs:
struct RscVersionNumber8
    {
    UInt16  m_release:8;
    UInt16  m_major:8;
    UInt16  m_minor:8;
    UInt16  m_subMinor:8;
    };  // RscVersionNumber8

struct RscDwgOpenFlags
    {
    UInt32  m_openPaperSpaceAs2D:1;
    UInt32  m_openModelSpaceAs2D:1;
    UInt32  m_preserveSeedOrigin:1;
    UInt32  m_disableUnitsDialog:8;
    UInt32  m_setAxisLockFromOrthoMode:1;
    UInt32  m_disallowLogicalNameFromXRefBlockNames:1;      // Applies to save and open.
    UInt32  m_useWeightMap:1;
    UInt32  m_displayWeightsInInches:1;
    UInt32  m_discardInvalidEntities:1;
    UInt32  m_hyperlinkAsEngineeringLink:1;
    UInt32  m_proxyShow:3;
    UInt32  m_proxyView:3;
    UInt32  m_dontCreateDGNMaterials:1;
    UInt32  m_dontCreateDGNLights:1;
    UInt32  m_graphicGroupAttributes:1;
    UInt32  m_customObjectDisplayMode:3;
    UInt32  m_attributesAsTags:1;
    UInt32  m_unused:2;
    };  // RscDwgOpenFlags

struct RscDwgOpenOptions
    {
    public:
    RscVersionNumber8       m_rscVersion;
    Bentley::RgbColorDef    m_designBackgroundColor;
    Bentley::RgbColorDef    m_sheetBackgroundColor;
    Bentley::WChar          m_dgnSeedFile[DGNPLATFORM_RESOURCE_MAXFILELENGTH];
    int                     m_archEngUnitsMode;
    int                     m_decimalUnitsMode;
    int                     m_normalCellMode;
    RscDwgOpenFlags         m_flags;
    UInt8                   m_dgnToDWGLineWeights[32];
    int                     m_defaultLineWeight;
    int                     m_unspecifiedDesignCenterUnitMode;
    int                     m_reserved1[15];
    double                  m_reserved2[20];
    Bentley::WChar          m_reservedStrings[512][4];

public:
    Bentley::RgbColorDef GetDesignBackgroundColor() const { return m_designBackgroundColor; }
    Bentley::RgbColorDef GetSheetBackgroundColor() const { return m_sheetBackgroundColor; }
    void GetDgnSeedFile (Bentley::WStringR filename) const { filename.assign(m_dgnSeedFile); }
    int GetArchEngUnitMode() const { return m_archEngUnitsMode; }
    int GetDecimalUnitMode() const { return m_decimalUnitsMode; }
    int GetUnspecifiedDesignCenterUnitMode() { return m_unspecifiedDesignCenterUnitMode; }
    int GetCustomObjectDisplayView() const { return m_flags.m_proxyView; }
    V8RD::ObjectDisplayMode GetCustomObjectDisplayMode () { return static_cast<V8RD::ObjectDisplayMode>(m_flags.m_customObjectDisplayMode); }
    V8RD::ProxyShowMode GetProxyShowMode () { return static_cast<V8RD::ProxyShowMode>(m_flags.m_proxyShow); }
    bool CreateDGNMaterials() const { return m_flags.m_dontCreateDGNMaterials == 0; }
    bool CreateDGNLights() const { return m_flags.m_dontCreateDGNLights == 0; }
    bool SetAxisLockFromOrthoMode() const { return m_flags.m_setAxisLockFromOrthoMode != 0; }
    bool OpenModelSpaceAs2d() const { return m_flags.m_openModelSpaceAs2D != 0; }
    bool OpenPaperSpaceAs2d() const { return m_flags.m_openPaperSpaceAs2D != 0; }
    bool DiscardInvalidEntities() const { return m_flags.m_discardInvalidEntities != 0; }
    bool HyperlinkAsEngineeringLink() { return m_flags.m_hyperlinkAsEngineeringLink != 0; }
    bool AttributesAsTags() const { return m_flags.m_attributesAsTags != 0; }
    bool DisallowLogicalNameFromXRefBlockNames () const { return m_flags.m_disallowLogicalNameFromXRefBlockNames != 0; }
    bool PreserveSeedOrigin() const { return m_flags.m_preserveSeedOrigin != 0; }
    bool UseWeightMap() const { return m_flags.m_useWeightMap != 0; }
    UInt8 MapDgnWeightToDwg(UInt8 dgnWeight) const { return dgnWeight > 31 ? 0 : m_dgnToDWGLineWeights[dgnWeight]; }
    int GetDefaultLineweight() { return m_defaultLineWeight; }
    };  // RscDwgOpenOptions

private:
    Bentley::WString        m_textStyleNameTemplate;
    Bentley::WString        m_insertLayerName;
    Bentley::WString        m_dictionaryWildcardFilters;
    V8RD::DwgFileVersion    m_saveVersion;
    Converter*              m_v8converter;
    RscFileHandle           m_rscFileHandle;
    // from dwgsettings resource file
    RscDwgOpenOptions*      m_data;
    // from configuration variables
    bool                    m_ignoreXData;


public:
// the constructor
DwgConversionSettings () : m_textStyleNameTemplate(L"Style-$s") , m_saveVersion(V8RD::DwgFileVersion_Max), m_v8converter(nullptr)
    {
    m_rscFileHandle = 0;
    m_data = static_cast<RscDwgOpenOptions*> (::calloc(1, sizeof(RscDwgOpenOptions)));
    m_ignoreXData = true;
    }

// the destructor
~DwgConversionSettings ()
    {
    if (m_data != nullptr)
        ::free (m_data);
    m_data = nullptr;
    m_rscFileHandle = 0;
    m_v8converter = nullptr;
    m_textStyleNameTemplate.clear ();
    m_insertLayerName.clear ();
    m_dictionaryWildcardFilters.clear ();
    m_ignoreXData = true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Don.Fu          02/17
//---------------+---------------+---------------+---------------+---------------+-------
Bentley::StatusInt  Initialize (Converter* v8converter)
    {
    m_v8converter = v8converter;
    
    Bentley::BeFileName settingsFile;
    if (!this->GetRscFileNameFromV8ConfigVars(settingsFile))
        return  BSIERROR;

    Bentley::StatusInt  status = RmgrResource::OpenFile (&m_rscFileHandle, settingsFile, RSC_READONLY);
    if (BSISUCCESS != status)
        return  status;

    status = BSIERROR;

    void* bytesRead = RmgrResourceMT::Load (m_rscFileHandle, RTYPE_DWGOpenOptions, RSCID_DWGOpenOptions);
    if (nullptr != bytesRead)
        {
        UInt32  expectedSize = sizeof *m_data;
        UInt32  numBytes = 0;

        status = RmgrResourceMT::Query (&numBytes, bytesRead, RSC_QRY_SIZE);

        if (BSISUCCESS == status && numBytes == expectedSize)
            {
            ::memcpy (m_data, bytesRead, numBytes);
            
            // validate the Open settings only, for now:
            if (!m_data->m_flags.m_useWeightMap)
                {
                for (int iWeight = 0; iWeight < s_max_lineweights; iWeight++)
                    m_data->m_dgnToDWGLineWeights[iWeight] = (byte) this->GetDwgWeightFromDgnWeight(iWeight);

                m_data->m_flags.m_useWeightMap = true;
                }

            if (0 == m_data->m_unspecifiedDesignCenterUnitMode)
                m_data->m_unspecifiedDesignCenterUnitMode = static_cast <V8RD::DwgOpenUnitMode> (DgnV8Api::StandardUnit::MetricMeters);

            status = BSISUCCESS;
            }
        }

    RmgrResource::CloseFile (m_rscFileHandle);

    m_ignoreXData = ConfigurationManager::IsVariableDefined (L"MS_DWG_SKIP_XDATA");

    return  status;
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Don.Fu          02/17
//---------------+---------------+---------------+---------------+---------------+-------
bool GetRscFileNameFromV8ConfigVars (Bentley::BeFileNameR settingsFile)
    {
    // attempt the V8 workflow to find & read dwgsettings resource file:
    if (BSISUCCESS == DgnV8Api::ConfigurationManager::GetVariable(settingsFile, L"MS_DWGSETTINGSFILE") && BeFileName::DoesPathExist(settingsFile.c_str()))
        return  true;

    if (BSISUCCESS == DgnV8Api::ConfigurationManager::GetVariable(settingsFile, L"MS_DWGSYSTEMDATA") ||
        BSISUCCESS == DgnV8Api::ConfigurationManager::GetVariable(settingsFile, L"MS_DWGDATA"))
        {
        size_t  semicolon = settingsFile.find (L';');
        if (WString::npos != semicolon)
            settingsFile.erase (semicolon);

        settingsFile.AppendToPath (L"dwgsettings.rsc");
        if (Bentley::BeFileName::DoesPathExist(settingsFile.c_str()))
            return  true;
        }

    settingsFile.Clear ();
    return  false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Don.Fu          02/17
//---------------+---------------+---------------+---------------+---------------+-------
bool GetDgnSeedFile (Bentley::WStringR seedFile) const override
    {
    // if user has set a seed file via dwgsettings, use it:
    seedFile.clear ();
    if (m_data != nullptr)
        {
        m_data->GetDgnSeedFile (seedFile);
        if (Bentley::BeFileName::DoesPathExist(seedFile.c_str()))
            return  true;
        }

    // default to seed3d.dgn delivered in V8SDK:
    if (BSISUCCESS != DgnV8Api::ConfigurationManager::GetVariable(seedFile, L"MS_TRANSEED") && Bentley::BeFileName::DoesPathExist(seedFile.c_str()))
        {
        BeAssert ("false && DwgConversionSettings must supply a seed DGN file for DWG!");
        if (nullptr != m_v8converter)
            m_v8converter->ReportError (Converter::IssueCategory::MissingData(), Converter::Issue::MissingSeedFile(), Utf8PrintfString("%ls needed for DWG references", seedFile.c_str()).c_str());
        return  false;
        }

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Don.Fu          02/17
//---------------+---------------+---------------+---------------+---------------+-------
V8RD::DwgOpenUnitMode GetDwgOpenUnitMode (Bentley::WCharCP fileName, V8RD::DwgLinearUnits currentLinearUnits, DgnV8Api::StandardUnit currentDesignCenterUnits, V8RD::DwgFileVersion version) const override
    {
    bool useEnglishUnits = V8RD::DWGLinearUnit_Engineering == currentLinearUnits || V8RD::DWGLinearUnit_Architectural == currentLinearUnits;
    bool isCurrentDesignCenterValid = DgnV8Api::UnitDefinition::GetStandardUnit(currentDesignCenterUnits).IsValid();

    V8RD::DwgOpenUnitMode unitMode = V8RD::DWGOpenUnitMode_SeedFileMasterUnits;
    if (nullptr == m_data)
        {
        // default units
        if (useEnglishUnits)
            unitMode = static_cast <V8RD::DwgOpenUnitMode> (DgnV8Api::StandardUnit::EnglishInches);
        else
            unitMode = isCurrentDesignCenterValid ? static_cast<V8RD::DwgOpenUnitMode>(currentDesignCenterUnits) : static_cast<V8RD::DwgOpenUnitMode>(DgnV8Api::StandardUnit::MetricMeters);
        }
    else
        {
        // attempt V8's way of finding the valid units:
        V8RD::DwgOpenUnitMode   defaultUnits = static_cast <V8RD::DwgOpenUnitMode> (useEnglishUnits ? DgnV8Api::StandardUnit::EnglishInches : DgnV8Api::StandardUnit::MetricMeters);
        int     currentSetting = useEnglishUnits ? m_data->GetArchEngUnitMode() : m_data->GetDecimalUnitMode();
        bool    useDesignCenter = DWG_UNITMODE_UseDesignCenter == currentSetting;

        if (useDesignCenter)
            {
            DgnV8Api::StandardUnit  unspecifiedUnits = static_cast<DgnV8Api::StandardUnit> (m_data->GetUnspecifiedDesignCenterUnitMode());

            if (isCurrentDesignCenterValid)
                unitMode = static_cast <V8RD::DwgOpenUnitMode> (currentDesignCenterUnits);
            else if (DgnV8Api::UnitDefinition::GetStandardUnit(unspecifiedUnits).IsValid())
                unitMode = static_cast <V8RD::DwgOpenUnitMode> (unspecifiedUnits);
            else
                unitMode = defaultUnits;
            }
        else
            {
            unitMode = static_cast <V8RD::DwgOpenUnitMode> (currentSetting);
            }
        }

    return  unitMode;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Don.Fu          02/17
//---------------+---------------+---------------+---------------+---------------+-------
int GetDgnWeightFromDwgWeight (int dwgWeight) const override
    {
    int dgnWeight = 0;
    if (nullptr == m_data || !m_data->UseWeightMap())
        {
        // calculate a DGN weight
        dgnWeight = (int) (0.5 + ((double)dwgWeight / 13.75));
        if (dgnWeight >= s_max_lineweights)
            dgnWeight = s_max_lineweights - 1;
        }
    else
        {
        // estimate a DGN weight from the weight map:
        int     minIndex = 0;
        double  minDelta = 1.0E10;
        for (int iWeight = 0; iWeight < s_max_lineweights; iWeight++)
            {
            // Look for closest - (note may not be ascending...).
            double  delta = abs(m_data->MapDgnWeightToDwg(iWeight) - dwgWeight);
            if (delta < minDelta)
                {
                minDelta = delta;
                minIndex = iWeight;
                }
            }
        dgnWeight = minIndex;
        }
    return  dgnWeight;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Don.Fu          02/17
//---------------+---------------+---------------+---------------+---------------+-------
bool GetDefaultDwgLineWeight (int& value) const override
    {
    if (m_data == nullptr)
        {
        value = 0;
        return  false;
        }

    value = m_data->GetDefaultLineweight ();

    // NEEDSWORK - do we need to check AutoCAD registry for LWDEFAULT?
    if (LWDEFAULT_FromAcadRegistry == value)
        value = 0;

    return  true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Don.Fu          02/17
//---------------+---------------+---------------+---------------+---------------+-------
void GetDisplayColorTable (byte colorTable[768], V8RD::DwgColorTableQuery queryMode) const override
    {
    if (nullptr != m_data)
        {
        // use adjusted color table if modelspace background is NOT the default background color(black):
        bool    useAdjustedColorTable = false;
        switch (queryMode)
            {
            case V8RD::DwgColorTableQuery::DwgColorTable_DefaultColorTableOnly:
                useAdjustedColorTable = false;
                break;
            case V8RD::DwgColorTableQuery::DwgColorTable_AdjustedColorTableOnly:
                useAdjustedColorTable = true;
                break;
            case V8RD::DwgColorTableQuery::DwgColorTable_BasedOnBackgroundColor:
            default:
                {
                // get modelspace background color
                int     red = 0, green = 0, blue = 0;
                this->GetDesignBackgroundColor (red, green, blue);
                useAdjustedColorTable = !DWGBACKGROUNGCOLOR_IsBlack(red, green, blue);
                }
            }

        byte const* dwgColors = DgnV8Api::DgnColorMap::GetRawDwgColorsP (useAdjustedColorTable);

        if (nullptr != dwgColors)
            {
            memcpy (colorTable, dwgColors, 768);
            return;
            }
        }

    // use default colors
    ::memcpy (colorTable, s_standardDwgColors, 768);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Don.Fu          02/17
//---------------+---------------+---------------+---------------+---------------+-------
void GetDesignBackgroundColor (int& red, int& green, int& blue) const override
    {
    if (nullptr != m_data)
        {
        Bentley::RgbColorDef    color = m_data->GetDesignBackgroundColor ();
        red = color.red;
        green = color.green;
        blue = color.blue;
        return;
        }

    red = green = blue = 255;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Don.Fu          02/17
//---------------+---------------+---------------+---------------+---------------+-------
void GetSheetBackgroundColor (int& red, int& green, int& blue) const override
    {
    if (nullptr == m_data)
        {
        Bentley::RgbColorDef    color = m_data->GetSheetBackgroundColor ();
        red = color.red;
        green = color.green;
        blue = color.blue;
        return;
        }
        
    red = green = blue = 255;
    }

int GetCustomObjectDisplayView() const override { return nullptr == m_data ? 0 : m_data->GetCustomObjectDisplayView(); }
V8RD::ObjectDisplayMode GetCustomObjectDisplayMode() const override { return nullptr == m_data ? V8RD::CustomObjectDisplay_Default : m_data->GetCustomObjectDisplayMode(); }
V8RD::ProxyShowMode GetProxyShowMode () const override { return V8RD::ProxyObject_Show; }
bool CreateDGNMaterials() const override { return nullptr == m_data ? true : m_data->CreateDGNMaterials(); }
bool CreateDGNLights() const override { return nullptr == m_data ? true : m_data->CreateDGNLights(); }
bool SetAxisLockFromOrthoMode() const override { return nullptr == m_data ? false : m_data->CreateDGNLights(); }
bool OpenModelSpaceAs2d() const override  { return nullptr == m_data ?  false : m_data->OpenModelSpaceAs2d(); }
bool OpenPaperSpaceAs2d() const override  { return nullptr == m_data ?  false : m_data->OpenPaperSpaceAs2d(); }
bool DiscardInvalidEntities() const override { return nullptr == m_data ? false : m_data->DiscardInvalidEntities(); }
bool HyperlinkAsEngineeringLink() const override { return nullptr == m_data ? false : m_data->HyperlinkAsEngineeringLink(); }
bool AttributesAsTags() const override { return nullptr == m_data ? false : m_data->AttributesAsTags(); }
bool DisallowLogicalNameFromXRefBlockNames() const override { return nullptr == m_data ? false : m_data->DisallowLogicalNameFromXRefBlockNames(); }
bool PreserveSeedOrigin() const override { return nullptr == m_data ? false : m_data->PreserveSeedOrigin(); }
bool AllowPsolidAcisInteropLogging () const override { return  false; }
V8RD::ColumnTextDropMode GetColumnTextDropMode () const override { return V8RD::DropColumnText_SeparateTexts; }
bool GetDefaultXrefTreatAsElement () const override { return true; }
bool IgnoreXData() const override { return m_ignoreXData; }


// SaveAs settings - don't need them so use defaults for all:
bool MapVPortLocateLockToDisplayUnlocked() const override                           { return  false; }
V8RD::ViewportFreezeMode GetViewportFreezeMode () const override                    { return  V8RD::VPFreeze_ViewportsAndGlobal; }
V8RD::SaveReferencePathMode GetSaveReferencePathMode() const override               { return  V8RD::SaveReferencePath_WhenSameDirectory; }
double                  GetLineCodeScale () const override                          { return  0.0; }
double                  GetLineWeightScale () const override                        { return  0.0; }
bool                    GraphicGroupAttributes() const override                     { return  false; }
bool                    SetViewportLayerFromClipElement() const override            { return  false; }
bool                    AllowLeaderHooklineToBeAdded() const override               { return  false; }
bool                    IsZeroZCoordinateEnforced() const override                  { return  false; }
bool                    CreateDWGMaterials() const override                         { return  true;  }
bool                    CreateDWGLights() const override                            { return  true;  }
bool                    CreateExtrusionsFromProjectedSolids() const override        { return  false; }
bool                    CreateSingleBlockFromDuplicateCells() const override        { return  true;  }
bool                    SaveApplicationData() const override                        { return  false; }
bool                    SaveMicroStationSettings() const override                   { return  false; }
bool                    AllowScaledBlocksFromCells() const override                 { return  true;  }
bool                    CreateOverlaysForReferenceAttachments() const override      { return  false; }
bool                    DropUnsupportedLineStyles() const override                  { return  false; }
bool                    DropUnsupportedAreaPatterns() const override                { return  true;  }
bool                    DropUnsupportedDimensions() const override                  { return  false; }
bool                    SetUCSFromCurrentACS() const override                       { return  true;  }
bool                    SaveFrontBackClip() const override                          { return  false; }
bool                    SaveSheetsToSeparateFiles() const override                  { return  false; }
bool                    CreateTrueColorFromDgnIndices() const override              { return  false; }
bool                    DisallowSaveDimensionSettings() const override              { return  false; }
bool                    DisallowBlockNameFromTriForma() const override              { return  false; }
bool                    CreateBlocksFromTriForma () const override                  { return  false; }
bool                    ForcePositiveExtrusionForClockwiseArcs() const override     { return  false; }
bool                    CreatePolylinesFromSplines() const override                 { return  false; }
bool                    CreateBlockDefinitionsOnLayer0() const override             { return  false; }
bool                    CreateBlockDefinitionsWithByBlockColor() const override     { return  false; }
bool                    CreateBlockDefinitionsWithByBlockStyle() const override     { return  false; }
bool                    CreateBlockDefinitionsWithByBlockWeight() const override    { return  false; }
bool                    ConvertEmptyEDFToSpace() const override                     { return  false; }
bool                    OverrideExistingLinetypeDefinitions() const override        { return  false; }
bool                    EnableDimensionRoundoff() const override                    { return  false; }
bool                    UniformlyScaleBlocks () const override                      { return  false; }
bool                    DropNestedCells() const override                            { return  false; }
bool                    DropTriformaCells() const override                          { return  false; }
bool                    AllowFarRefDependency() const override                      { return  false; }
bool                    DropLabelLines() const override                             { return  false; }
bool                    DisableV7RefClipRotation() const override                   { return  false; }
bool                    SaveRasterToSharedCell() const override                     { return  false; }
bool                    CopyRasterToOutputFolder() const override                   { return  false; }
bool                    IsImageFileFormatSupported (WCharCP pi_schemeTypeName, DgnV8Api::ImageFileFormat  imageFileType) const override {return true;}
bool                    UseLevelSymbologyOverrides () const override                { return  false; }
bool                    ConvertReferences () const override                         { return  false; }
int                     GetLevelDisplayView () const override                       { return  0; }
UInt32                  GetMaxOrphanTagsPerSet() const override                     { return  100; }
UInt32                  GetMaxDictionaryItems() const override                      { return  2000; }
Bentley::WStringCR      GetDictionaryWildcardFilters() const override               { return  m_dictionaryWildcardFilters; }
V8RD::ConstructionMapping GetConstructionClassMapping() const override              { return V8RD::Construction_Layer; }
V8RD::PatternMapping    GetPatternClassMapping() const override                     { return V8RD::Pattern_Ignore; }
V8RD::LinearPatternedMapping GetLinearPatternedClassMapping() const override        { return V8RD::LinearPatterned_Omit; }
double                  GetPolyfaceAngleTolerance () const override                 { return 0.5; }
bool                    SaveBlockUnitsFromFileUnits () const override               { return false; }
int                     GetDwgFileCodePage () const override                        { return 1252; }
V8RD::DwgSaveNonDefaultModelMode  GetNonDefaultModelMode () const override          { return V8RD::NonDefaultModels_SeparateFiles; }
V8RD::DwgSaveUnitMode   GetDwgSaveUnitMode () const override    { return V8RD::DWGSaveUnitMode_SeedFileMasterUnits; }
int                     GetDxfSavePrecision () const override   { return 6; }
V8RD::DwgFileVersion    GetDwgSaveVersion () const override     { return m_saveVersion; }
void                    SetDwgSaveVersion (V8RD::DwgFileVersion newVersion) override  { m_saveVersion = newVersion; }
bool                    GetDwgSeedFile (Bentley::WStringR seedFileName) const override       { return false; }
bool                    GetDwgShapeFilePath (Bentley::WStringR shapeFilePath) const override { return false; }
Bentley::WStringCR      GetTextStyleNameTemplate () const override                  { return m_textStyleNameTemplate; }
Bentley::WStringCR      GetInsertLayerName () const override                        { return m_insertLayerName; }
bool GetMergeVisibleEdgeSettings (DgnV8Api::HLineSettings& mveSettings) const override { return false; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Don.Fu          02/17
//---------------+---------------+---------------+---------------+---------------+-------
int GetDwgWeightFromDgnWeight (int dgnWeight) const override
    {
    if (dgnWeight < 0)
        return 0;
    else if (dgnWeight > s_max_lineweights)
        return s_standardDwgWeights[_countof(s_standardDwgWeights)-1];
    else if (m_data != nullptr && m_data->UseWeightMap())
        return m_data->MapDgnWeightToDwg(dgnWeight);

    double  dgnWidthInMM = 0.275 * dgnWeight;
    return this->GetDwgWeightFromWidthInMM (dgnWidthInMM);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Don.Fu          02/17
//---------------+---------------+---------------+---------------+---------------+-------
int GetDwgWeightFromWidthInMM (double dgnWidthInMM) const override
    {
    dgnWidthInMM *= 50.0;
    return  s_standardDwgWeights[this->StandardIndexFromDwgWeight(dgnWidthInMM)];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Don.Fu          02/17
//---------------+---------------+---------------+---------------+---------------+-------
int StandardIndexFromDwgWeight (double dwgWeightInMM) const
    {
    for (int index=1; index < _countof(s_standardDwgWeights); index++)
        {
        if (s_standardDwgWeights[index] > dwgWeightInMM)
            {
            double low = s_standardDwgWeights[index-1], high = s_standardDwgWeights[index];

            return (dwgWeightInMM - low) < (high - dwgWeightInMM) ? index-1 : index;
            }
        }
    return _countof(s_standardDwgWeights) - 1;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Don.Fu          02/17
//---------------+---------------+---------------+---------------+---------------+-------
V8RD::LineStringMapping GetLineStringMapping (bool planar) const override
    {
    if (planar)
        return V8RD::LineString_LWPolyline;
    else
        return V8RD::LineString_3DPolyline;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Don.Fu          02/17
//---------------+---------------+---------------+---------------+---------------+-------
V8RD::SolidSurfaceMapping GetSolidSurfaceMapping (bool isFlatSurfaceOrSolid) const override
    {
    if (isFlatSurfaceOrSolid)
        return V8RD::SolidSurface_Acis;
    else
        return V8RD::SolidSurface_Polyface;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Don.Fu          02/17
//---------------+---------------+---------------+---------------+---------------+-------
V8RD::ClosedMapping GetComplexShapeMapping (bool useFilled, bool threeD) const override
    {
    if (useFilled)
        return V8RD::Closed_Hatch;
    else
        return (threeD ? V8RD::Closed_Region : V8RD::Closed_Polyline);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Don.Fu          02/17
//---------------+---------------+---------------+---------------+---------------+-------
V8RD::ClosedMapping GetTriOrQuadMapping (bool useFilled, bool threeD) const override
    {
    if (useFilled)
        return V8RD::Closed_SolidOrFace;
    else
        return (threeD ? V8RD::Closed_SolidOrFace : V8RD::Closed_Polyline);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Don.Fu          02/17
//---------------+---------------+---------------+---------------+---------------+-------
V8RD::ClosedMapping GetPolygonMapping (bool useFilled, bool threeD) const override
    {
    if (useFilled)
        return V8RD::Closed_Hatch;
    else
        return (threeD ? V8RD::Closed_Polyline : V8RD::Closed_Polyline);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Don.Fu          02/17
//---------------+---------------+---------------+---------------+---------------+-------
V8RD::ClosedMapping GetGroupedHoleMapping (bool useFilled, bool threeD) const override
    {
    if (useFilled)
        return V8RD::Closed_Hatch;
    else
        return (threeD ? V8RD::Closed_Region : V8RD::Closed_Region);
    }
};  // DwgConversionSettings



/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          02/17
+===============+===============+===============+===============+===============+======*/
class V8DwgHost : public V8RD::DwgPlatformHost
{
private:
    DwgConversionSettings*      m_dwgConversionSettings;
    Converter*                  m_v8converter;
    BentleyApi::BeFileName      m_dgnv8Path;
    BentleyApi::BeFileName      m_realdwgPath;
    BentleyApi::BeFileName      m_fallbackShxForShape;
    BentleyApi::BeFileName      m_fallbackShxForText;
    BentleyApi::BeFileName      m_lastUsedShxFontName;
    T_SearchPathList            m_fontSearchPathList;
    T_SearchPathList            m_patternSearchPathList;
    bset<WString>               m_missingFontNames;

public:

// the constructor    
V8DwgHost (BentleyApi::BeFileNameCR v8dir, BentleyApi::BeFileNameCR realdwgDir)
    {
    m_dwgConversionSettings = new DwgConversionSettings ();
    m_v8converter = nullptr;
    m_dgnv8Path.SetName (v8dir);
    m_dgnv8Path.AppendSeparator ();

    m_fallbackShxForShape.SetName (m_dgnv8Path);
    m_fallbackShxForShape.AppendToPath (L"Fonts\\ltypeshp.shx");
    m_fallbackShxForText.SetName (m_dgnv8Path);
    m_fallbackShxForText.AppendToPath (L"Fonts\\simplex.shx");
    m_lastUsedShxFontName.clear ();
    m_fontSearchPathList.clear ();
    m_patternSearchPathList.clear ();
    m_missingFontNames.clear ();

    m_realdwgPath.clear ();
    if (realdwgDir.DoesPathExist())
        {
        // use caller passed in RealDWG directory:
        m_realdwgPath.SetName (realdwgDir);
        }
    else
        {
#ifdef BENTLEY_WIN32
        // honor caller's SetDllDirectory()
        WCHAR   dllDir[2000] = { 0 };
        DWORD   nChars = ::GetDllDirectoryW (2000, dllDir);
        if (nChars > 0 && nChars < 2000)
            m_realdwgPath.assign (dllDir, nChars);
#endif  // BENTLEY_WIN32
        }
    }

// the destructor
~V8DwgHost ()
    {
    if (m_dwgConversionSettings != nullptr)
        delete m_dwgConversionSettings;

    m_dwgConversionSettings = nullptr;
    m_v8converter = nullptr;
    m_dgnv8Path.clear ();
    m_realdwgPath.clear ();
    m_fallbackShxForShape.clear ();
    m_fallbackShxForText.clear ();
    m_lastUsedShxFontName.clear ();
    m_fontSearchPathList.clear ();
    m_patternSearchPathList.clear ();
    m_missingFontNames.clear ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Don.Fu          02/17
//---------------+---------------+---------------+---------------+---------------+-------
Bentley::StatusInt InitializeDwgSettings (Converter* v8converter)
    {
    m_v8converter = v8converter;

    Bentley::StatusInt  status = BSIERROR;
    if (nullptr == v8converter)
        return  status;        

    // initialize dwgsettings
    if (nullptr != m_dwgConversionSettings)
        status = m_dwgConversionSettings->Initialize (v8converter);

    // set up font search paths from the ImportConfig and V8 FontAdmin:
    BentleyApi::WString searchPaths;
    if (nullptr != m_v8converter)
        {
        searchPaths.AssignUtf8 (m_v8converter->_GetWorkspaceFontSearchPaths().c_str());
        searchPaths.ReplaceAll (L"$(AppRoot)$(v8SdkDir)", m_dgnv8Path.c_str());
        }

    Bentley::WString    shxV8, ttfV8;
    DgnV8Api::DgnPlatformLib::GetHost().GetFontAdmin()._GetShxFontPaths(shxV8);
    DgnV8Api::DgnPlatformLib::GetHost().GetFontAdmin()._GetTrueTypeFontPaths(ttfV8);

    BentleyApi::WString workspaceV8;
    if (!shxV8.empty())
        workspaceV8.assign (shxV8.c_str());
    if (!ttfV8.empty())
        {
        workspaceV8 += L";";
        workspaceV8 += ttfV8.c_str();
        }

    if (!workspaceV8.empty() && searchPaths.find(L"$(V8WorkspaceFontPaths)") != BentleyApi::WString::npos)
        searchPaths.ReplaceAll (L"$(V8WorkspaceFontPaths)", workspaceV8.c_str());

    BeStringUtilities::Split (searchPaths.c_str(), L";", nullptr, m_fontSearchPathList);

    // set up pattern search paths from V8 config vars:
    Bentley::WString    path;
    size_t              semicolon;
    if (BSISUCCESS == DgnV8Api::ConfigurationManager::GetVariable(path, L"MS_DATA"))
        {
        if (WString::npos != (semicolon = path.find(L';')))
            path.erase (semicolon);
        if (Bentley::BeFileName::DoesPathExist(path.c_str()))
            m_patternSearchPathList.push_back (BentleyApi::WString(path.c_str()));
        path.clear ();
        }
    if (BSISUCCESS == DgnV8Api::ConfigurationManager::GetVariable(path, L"MS_DWGDATA"))
        {
        if (WString::npos != (semicolon = path.find(L';')))
            path.erase (semicolon);
        if (Bentley::BeFileName::DoesPathExist(path.c_str()))
            m_patternSearchPathList.push_back (BentleyApi::WString(path.c_str()));
        path.clear ();
        }

    return  status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Don.Fu          02/17
//---------------+---------------+---------------+---------------+---------------+-------
bool CopyStringToReturn (Bentley::WCharP outChars, size_t outSize, WStringCR inString)
    {
    size_t  inSize = inString.size ();

    if (inSize < 1)
        return false;
    else if (inSize > outSize)
        inSize = outSize;

    ::wcsncpy (outChars, inString.c_str(), inSize);

    return  true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Don.Fu          02/17
//---------------+---------------+---------------+---------------+---------------+-------
bool _GetPassword (Bentley::WCharCP dwgName, FilePasswordOption options, Bentley::WCharP password, const size_t bufSize) override
    {
    if (nullptr == m_v8converter || nullptr == password || bufSize < 1)
        return  false;

    // retrieve the password from the converter and send it to RealDWG:
    WString pw (m_v8converter->_GetParams().GetPassword().c_str(), true);

    return this->CopyStringToReturn (password, bufSize, pw);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Don.Fu          02/17
//---------------+---------------+---------------+---------------+---------------+-------
Bentley::WCharCP _GetRegistryProductRootKey () override
    {
    if (nullptr == m_realDwgRegistryRootKey)
        {
        Bentley::WString    regkey;

        // try to get RealDWG's HKLM root key from config var V8REALDWG_REGISTRY_ROOTKEY set by a consumer:
        if (BSISUCCESS == ConfigurationManager::GetVariable(regkey, L"V8REALDWG_REGISTRY_ROOTKEY"))
            {
            LOG.tracev ("Found HKLM root registry for RealDWG=%ls", regkey.c_str());

            regkey.ReplaceI (L"HKEY_LOCAL_MACHINE\\", L"");

            HKEY    hkey;
            if (ERROR_SUCCESS == ::RegOpenKeyExW(HKEY_LOCAL_MACHINE, regkey.c_str(), 0, KEY_QUERY_VALUE, &hkey))
                {
                ::RegCloseKey (hkey);
                m_realDwgRegistryRootKey = ::wcsdup (regkey.c_str());
                }
            }

        if (nullptr == m_realDwgRegistryRootKey)
            {
            // 2nd attempt: check MS_PRODUCTCODEGUID set by the installer and find RealDWG root key:
            Bentley::WString productGuid;

            ConfigurationManager::GetVariable (productGuid, L"MS_PRODUCTCODEGUID");

            if (!productGuid.empty())
                {
                WCHAR   productPath[1024];
                DWORD   regLength = _countof (productPath);

                // find the full registry path of the installed RealDWG on the product:
                if (INSTALLSTATE_LOCAL == ::MsiGetComponentPathW(productGuid.c_str(), s_ODBXHOSTAPPREGROOT, productPath, &regLength))
                    {
                    // make it a relative key path to be returned:
                    WCharCP   relativePath = ::wcschr (productPath, '\\');
                    if (nullptr != relativePath && 0 != relativePath[0] && nullptr != ++relativePath && 0 != relativePath[0])
                        m_realDwgRegistryRootKey = ::wcsdup (relativePath);
                    }
                }
            }
        }

    return m_realDwgRegistryRootKey;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Don.Fu          02/17
//---------------+---------------+---------------+---------------+---------------+-------
Bentley::WCharCP _Product () override
    {
    return  L"DgnV8Converter";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Don.Fu          02/17
//---------------+---------------+---------------+---------------+---------------+-------
void _FatalError (Bentley::WCharCP format, ...) override
    {
    va_list     varArgs;
    va_start (varArgs, format);

    WString     errMessage = WPrintfString (format, varArgs);
    va_end (varArgs);

    Utf8String  msg (errMessage.c_str());

    LOG.errorv ("RealDWG fatal error: %ls", msg.c_str());

    if (nullptr != m_v8converter)
        m_v8converter->OnFatalError (Converter::IssueCategory::Unknown(), Converter::Issue::Error(), msg.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Don.Fu          02/17
//---------------+---------------+---------------+---------------+---------------+-------
void _Alert (Bentley::WCharCP message) override
    {
    if (nullptr == message)
        return;

    Utf8String  msg(message);
    LOG.warning (msg.c_str());

    if (nullptr != m_v8converter)
        m_v8converter->ReportIssue (Converter::IssueSeverity::Warning, Converter::IssueCategory::Unknown(), Converter::Issue::Message(), msg.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Don.Fu          02/17
//---------------+---------------+---------------+---------------+---------------+-------
WCHAR* _GetAlternateFontName () override
    {
    static WCHAR*   s_alternateFontName = L"simplex.shx";
    return  s_alternateFontName;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Don.Fu          02/17
//---------------+---------------+---------------+---------------+---------------+-------
bool FindXrefFile (BentleyApi::WStringR outFile, BentleyApi::WCharCP inFile, AcDbDatabaseP dwg)
    {
    // if absolute path exists, return it!
    BeFileName  checkFile(inFile);
    if (checkFile.DoesPathExist())
        return  true;
   
    SpatialConverterBase*   spatialConverter = dynamic_cast<SpatialConverterBase*> (m_v8converter);

    // get base path from the input or the root DWG file name
    const ACHAR*        dwgName = nullptr;
    BentleyApi::WString basePath;
    if (nullptr != dwg && Acad::eOk == dwg->getFilename(dwgName) && nullptr != dwgName)
        basePath = BentleyApi::BeFileName::GetDirectoryName (dwgName);
    else if (nullptr != spatialConverter)
        basePath = BentleyApi::BeFileName::GetDirectoryName (spatialConverter->GetRootFileName().c_str());
    else
        return  false;
    
    // if looks like a relative path, try to resolve it
    if (checkFile.StartsWith(L"..") && BSISUCCESS == BentleyApi::BeFileName::ResolveRelativePath(outFile, inFile, basePath.c_str()))
        return  true;

    // look for the file on the same folder of the base file
    BentleyApi::WString     name, ext;
    checkFile.ParseName (nullptr, nullptr, &name, &ext);

    checkFile.SetName (basePath);
    checkFile.AppendToPath (name.c_str());
    checkFile.AppendExtension (ext.empty() ? L"dwg" : ext.c_str());

    if (checkFile.DoesPathExist())
        {
        outFile.assign (checkFile.c_str());
        return  true;
        }

    return  false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Don.Fu          02/17
//---------------+---------------+---------------+---------------+---------------+-------
bool FindPatternFile (BentleyApi::WStringR outFile, BentleyApi::WCharCP inFile)
    {
    if (nullptr == inFile || m_patternSearchPathList.empty())
        return  false;

    if (BentleyApi::BeFileName::DoesPathExist(inFile))
        {
        outFile.assign (inFile);
        return  true;
        }

    BentleyApi::WString     dev, dir, name, ext;
    BentleyApi::BeFileName::ParseName (&dev, &dir, &name, &ext, inFile);

    // append ".pat", only if pattern name does not already contains the extension name:
    if (0 != ext.CompareToI(L"pat"))
        name += L".pat";

    BentleyApi::BeFileName  foundPath;
    for (auto path : m_patternSearchPathList)
        {
        foundPath.SetName (path);
        foundPath.AppendToPath (name.c_str());

        if (foundPath.DoesPathExist())
            {
            // found the font in a search path - return it to RealDWG:
            outFile.assign (foundPath.c_str());
            return  true;
            }
        }
    return  false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Don.Fu          02/17
//---------------+---------------+---------------+---------------+---------------+-------
bool FindFontFile (BentleyApi::WStringR outFile, BentleyApi::WCharCP inFont, AcadFileType hint)
    {
    // if the full path exists, this is it!
    if (BentleyApi::BeFileName::DoesPathExist(inFont))
        {
        outFile = inFont;
        return  true;
        }

    DgnFontType fontType = ACADFILE_CompiledShapeFile == hint ? DgnFontType::Shx : DgnFontType::TrueType;
    WString     fontName (inFont);
    fontName.Trim ();

    size_t      dot= fontName.find_last_of (L'.');
    if (WString::npos != dot)
        {
        if (ACADFILE_FontFile == hint)
            {
            // This font could be either an shx or a ttf - check for file extension.
            WString ext = fontName.substr (dot + 1, fontName.length() - dot - 1);
            fontType = 0 == ext.CompareToI(L"shx") ? DgnFontType::Shx : DgnFontType::TrueType;
            }
        }
    else if (fontType == DgnFontType::Shx)
        {
        fontName += L".shx";
        }

    // try finding the font from Converter's search paths:
    if (!m_fontSearchPathList.empty())
        {
        BentleyApi::BeFileName  foundPath;
        for (auto path : m_fontSearchPathList)
            {
            foundPath.SetName (path);
            foundPath.AppendToPath (fontName.c_str());

            if (foundPath.DoesPathExist())
                {
                // found the font in a search path - return it to RealDWG:
                outFile.assign (foundPath.c_str());
                return  true;
                }
            }
        }

    if (nullptr != m_v8converter && m_missingFontNames.find(fontName) == m_missingFontNames.end())
        {
        Utf8CP  type = DgnFontType::Shx == fontType ? "SHX" : "TTF";
        m_v8converter->ReportIssueV (Converter::IssueSeverity::Warning, Converter::IssueCategory::MissingData(), Converter::Issue::FontMissing(), nullptr, type, Utf8String(fontName).c_str());
        // only report each missing font once per session:
        m_missingFontNames.insert (fontName);
        }

    // if we get here, we cannot find the font - use an appropriate default font:
    if (DgnFontType::Shx == fontType && m_lastUsedShxFontName.EqualsI(fontName.c_str()))
        {
        /*-------------------------------------------------------------------------------
        A RealDWG/AutoCAD problem:

        The input name fontIn can be an shx for text or an shx for shape.  There is no
        way we can tell which type of the shx it is looking for.  If we return a wrong
        type shx, the caller would enter into an infinite loop calling findFile until a
        right type of shx is returned.  ACAD does that by popping up a dialog box forcing
        user to manually pick a right file.  We are left with few choice.  AutoDesk offers
        no help on how to tell an shx for text vs one for shape, neither they have a way
        to stop the infinite loop!

        As a workaround, here we track and check for shx font name RealDWG has previously
        called for.  That shx file cannot be found, so we had returned our fallback shx
        file for text.  If now findFile is calling us for the same shx font again, we
        assume the shx font we had returned last time to be of a wrong type, so we now
        return an shx font for shape instead.  Not a reliable check but it seems to have
        stopped RealDWG's infinite loop of calling this method.  Ltypeshp.shx is the only
        shx file for shape delivered with ACAD and RealDWG.
        -------------------------------------------------------------------------------*/
        if (m_fallbackShxForShape.DoesPathExist())
            {
            outFile.assign (m_fallbackShxForShape.c_str());
            m_lastUsedShxFontName.clear ();
            return  true;
            }
        }
    else if (m_fallbackShxForText.DoesPathExist())
        {
        // use fallback font
        outFile.assign (m_fallbackShxForText.c_str());

        if (DgnFontType::Shx == fontType)
            m_lastUsedShxFontName.assign (fontName.c_str());

        return  true;
        }

    return  false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Don.Fu          02/17
//---------------+---------------+---------------+---------------+---------------+-------
V8RD::RealDwgStatus _FindFile (Bentley::WCharP fullpathOut, int numChars, Bentley::WCharCP filenameIn, AcDbDatabase* dwg, AcadFileType hint) override
    {
    BentleyApi::WCharP  pExtension = nullptr;
    BentleyApi::WString suggestedPath;

    switch (hint)
        {
        case ACADFILE_FontFile:
        case ACADFILE_CompiledShapeFile:
        case ACADFILE_TrueTypeFontFile:
            {
            if (this->FindFontFile(suggestedPath, filenameIn, hint))
                {
                this->CopyStringToReturn (fullpathOut, numChars, suggestedPath);
                return V8RD::RealDwgStatus::RealDwgSuccess;
                }
            return V8RD::RealDwgStatus::FileNotFound;
            }

        case ACADFILE_FontMapFile:
            {
            pExtension = L".fmp";
            // NEEDSWORK - find font map file
            break;
            }

        case ACADFILE_PatternFile:
            {
            if (this->FindPatternFile(suggestedPath, filenameIn))
                {
                this->CopyStringToReturn (fullpathOut, numChars, suggestedPath);
                return  V8RD::RealDwgSuccess;
                }
            
            return V8RD::RealDwgStatus::FileNotFound;
            }

        case ACADFILE_ARXApplication:
            {
            pExtension = L".dbx";
            if (!m_realdwgPath.empty())
                suggestedPath.assign (m_realdwgPath);
            break;
            }

        case ACADFILE_XRefDrawing:
            {
            if (this->FindXrefFile(suggestedPath, filenameIn, dwg))
                {
                this->CopyStringToReturn (fullpathOut, numChars, suggestedPath);
                return  V8RD::RealDwgStatus::RealDwgSuccess;
                }
            return  V8RD::RealDwgStatus::FileNotFound;
            }

        case ACADFILE_EmbeddedImageFile:
        default:
            {
            pExtension = L"";
            break;
            }
        }

#ifdef BENTLEY_WIN32
    LPCWSTR         fname = static_cast<LPCWSTR> (filenameIn);
    LPCWSTR         extname = static_cast<LPCWSTR> (pExtension);
    LPWSTR          filepart = nullptr;
    WCHAR           found[2000] = { 0 };
    static DWORD    s_maxChars = _countof (found);

    LPCWSTR         searchPath = nullptr;
    if (!suggestedPath.empty())
        searchPath = static_cast <LPCWSTR> (suggestedPath.c_str());

    DWORD   foundChars = ::SearchPathW (searchPath, fname, extname, s_maxChars, found, &filepart);
    if (foundChars > 0 && foundChars < s_maxChars)
        {
        if (foundChars > static_cast<DWORD>(numChars))
            foundChars = numChars;

        ::wcsncpy (fullpathOut, found, foundChars);

        // trace loading the object enabler:
        if (ACADFILE_ARXApplication == hint && nullptr != m_v8converter)
            m_v8converter->SetTaskName (Converter::ProgressMessage::TASK_LOADING_REALDWG(), Utf8String(fullpathOut).c_str());

        return  V8RD::RealDwgSuccess;
        }
#endif

    return  V8RD::RealDwgSuccess;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Don.Fu          02/17
//---------------+---------------+---------------+---------------+---------------+-------
bool _GetDwgConversionSettings (V8RD::IDwgConversionSettings*& settings) override
    {
    settings = m_dwgConversionSettings;
    return  true;
    }

};  // V8DwgHost


static V8DwgHost*   s_dwgHost = nullptr;

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Don.Fu          02/17
//---------------+---------------+---------------+---------------+---------------+-------
bool Converter::IsDwgOrDxfFile (BeFileNameCR filename)
    {
    if (!filename.DoesPathExist())
        return  false;

    ::FILE*   file = ::fopen (Utf8String(filename).c_str(), "r+b");
    if (nullptr == file)
        return false;

    ::fseek (file, 0, SEEK_SET);

    // Sniff first 4K bytes in the file
    char    headerData[4097] = { 0 };
    size_t  headerSize = ::fread (headerData, 1, _countof(headerData) - 1, file);

    headerData[headerSize - 1] = 0;
    ::fclose (file);

    // A DWG file starts with "AC1" or "AC2" in ASCII:
    if ('A' == headerData[0] && 'C' == headerData[1] && ('1' == headerData[2] || '2' == headerData[2]))
        return  true;

    // A complete DXF file should contain $ACADVER followed by a version numer AC10xx in file header:
    char const* asciiString = strstr (headerData, "$ACADVER");
    if (nullptr != asciiString && nullptr != (asciiString = strstr(asciiString, "AC10")))
        return  true;

    /*-----------------------------------------------------------------------------------
    Ideally we want to see at least SECTION and ENTITIES to appear in the string, as the pair
    make a good telltale for a DXF file.  Unfortunately, some crapy DXF files have large data 
    prior to reaching to the ENTITIES section!  There is no point to open entire file just to
    check ENTITIES, because the file can be huge.  TFS's 346565, 633683.
    -----------------------------------------------------------------------------------*/
    if (nullptr != (asciiString = strstr(headerData, "SECTION")))
        return true;

    // A DXB file must start with "AutoCAD Binary DXF":
    if (0 == strncmp(headerData, "AutoCAD Binary DXF", 18))
        return  true;

    // Not a DWG, DXF or DXB file
    return  false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Don.Fu          02/17
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus Converter::InitializeDwgHost (BentleyApi::BeFileNameCR v8dir, BentleyApi::BeFileNameCR realdwgDir)
    {
    if (nullptr != s_dwgHost)
        return  BSISUCCESS;

    if (DgnPlatformLib::QueryHost() != nullptr)
        {
        auto loading = ProgressMessage::GetString (Converter::ProgressMessage::TASK_LOADING_REALDWG());
        LOG.tracev (loading.c_str(), "...");
        }

    s_dwgHost = new V8DwgHost (v8dir, realdwgDir);
    if (nullptr == s_dwgHost)
        return  BSIERROR;

    Bentley::RealDwg::DwgPlatformHost::Initialize (*s_dwgHost, false, false);

    return  BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Don.Fu          02/17
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus Converter::InitializeDwgSettings (Converter* v8converter)
    {
    if (nullptr == s_dwgHost)
        {
        BeAssert (false && "DwgHost must be initialized before initializing DwgSettings!");
        return  BSIERROR;
        }

    if (BSISUCCESS != s_dwgHost->InitializeDwgSettings(v8converter))
        v8converter->ReportIssueV (Converter::IssueSeverity::Warning, Converter::IssueCategory::VisualFidelity(), Converter::Issue::Message(), nullptr, "file dwgsettings.rsc not found for DWG reference files, using defaults");

    return  BSISUCCESS;
    }

END_DGNDBSYNC_DGNV8_NAMESPACE

