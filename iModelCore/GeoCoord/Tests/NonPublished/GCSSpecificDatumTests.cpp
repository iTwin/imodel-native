//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See LICENSE.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------

#include <Bentley/BeTest.h>
#include <Bentley/BeTextFile.h>
#include <GeoCoord/BaseGeoCoord.h>

#include "GeoCoordTestCommon.h"

using namespace ::testing;

using namespace GeoCoordinates;


struct transformPathStep
{
    Utf8String m_targetDatum;
    GenConvertCode m_convertOp;
};

struct datumTransformPaths
{
    Utf8String m_sourceDatum;
    Utf8String m_targetDatum;
    bvector<transformPathStep> m_path;
};

class GCSSpecificDatumTests : public ::testing::TestWithParam< datumTransformPaths >
    {   
    public:
        virtual void SetUp() { GeoCoordTestCommon::Initialize(); };
        virtual void TearDown() {GeoCoordTestCommon::Shutdown();};

        GCSSpecificDatumTests() {};
        ~GCSSpecificDatumTests() {};
    };




bvector<datumTransformPaths> s_pathsToTest{
// Australian tests
{"AGD66", "AGD84",              { {"GDA94", GenConvertCode::GenConvertType_GFILE},
                                    {"WGS84", GenConvertCode::GenConvertType_NONE},
                                    {"GDA94", GenConvertCode::GenConvertType_NONE},
                                    { "AGD84", GenConvertCode::GenConvertType_GFILE}}},
{"AGD66", "ASTRLA66-Grid",      { {"ASTRLA66-Grid", GenConvertCode::GenConvertType_NONE}}},
{"AGD66", "EPSG:6202",          { {"EPSG:6202", GenConvertCode::GenConvertType_NONE}}},
{"AGD66", "WGS84",              { {"GDA94", GenConvertCode::GenConvertType_GFILE},
                                    {"WGS84", GenConvertCode::GenConvertType_NONE}}},
{"AGD66", "GDA94",              { {"GDA94", GenConvertCode::GenConvertType_GFILE}}},
{"AGD66", "EPSG:6283",          { {"GDA94", GenConvertCode::GenConvertType_GFILE},
                                    {"WGS84", GenConvertCode::GenConvertType_NONE},
                                    {"EPSG:6283", GenConvertCode::GenConvertType_NONE}}},
// -------                        
{"ASTRLA66-Grid", "AGD84",      { {"GDA94", GenConvertCode::GenConvertType_GFILE},
                                    {"WGS84", GenConvertCode::GenConvertType_NONE},
                                    {"GDA94", GenConvertCode::GenConvertType_NONE},
                                    { "AGD84", GenConvertCode::GenConvertType_GFILE}}},
{"ASTRLA66-Grid", "AGD66",      { {"AGD66", GenConvertCode::GenConvertType_NONE}}},
{"ASTRLA66-Grid", "EPSG:6202",  { {"EPSG:6202", GenConvertCode::GenConvertType_NONE}}},
{"ASTRLA66-Grid", "WGS84",      { {"GDA94", GenConvertCode::GenConvertType_GFILE},
                                    {"WGS84", GenConvertCode::GenConvertType_NONE}}},
{"ASTRLA66-Grid", "GDA94",      { {"GDA94", GenConvertCode::GenConvertType_GFILE}}},
{"ASTRLA66-Grid", "EPSG:6283",  { {"EPSG:6283", GenConvertCode::GenConvertType_GFILE}}},
// -------
{"AGD84", "AGD66",              { {"GDA94", GenConvertCode::GenConvertType_GFILE},
                                    {"WGS84", GenConvertCode::GenConvertType_NONE},
                                    {"GDA94", GenConvertCode::GenConvertType_NONE},
                                    { "AGD66", GenConvertCode::GenConvertType_GFILE}}},

{"AGD84", "ASTRLA84-Grid",      { {"ASTRLA84-Grid", GenConvertCode::GenConvertType_NONE}}},
{"AGD84", "EPSG:6203",          { {"EPSG:6203", GenConvertCode::GenConvertType_NONE}}},
{"AGD84", "WGS84",              { {"GDA94", GenConvertCode::GenConvertType_GFILE},
                                    {"WGS84", GenConvertCode::GenConvertType_NONE}}},
{"AGD84", "WGS84",              { {"GDA94", GenConvertCode::GenConvertType_GFILE},
                                    {"WGS84", GenConvertCode::GenConvertType_NONE}}},
{"AGD84", "GDA94",              { {"GDA94", GenConvertCode::GenConvertType_GFILE}}},
{"AGD84", "EPSG:6283",          { {"GDA94", GenConvertCode::GenConvertType_GFILE},
                                    {"WGS84", GenConvertCode::GenConvertType_NONE},
                                    {"EPSG:6283", GenConvertCode::GenConvertType_NONE}}},
// -------
{"ASTRLA84-Grid", "AGD66",      { {"GDA94", GenConvertCode::GenConvertType_GFILE},
                                    {"WGS84", GenConvertCode::GenConvertType_NONE},
                                    {"GDA94", GenConvertCode::GenConvertType_NONE},
                                    { "AGD66", GenConvertCode::GenConvertType_GFILE}}},

{"ASTRLA84-Grid", "AGD84",      { {"AGD84", GenConvertCode::GenConvertType_NONE}}},
{"ASTRLA84-Grid", "EPSG:6203",  { {"EPSG:6203", GenConvertCode::GenConvertType_NONE}}},
{"ASTRLA84-Grid", "WGS84",      { {"GDA94", GenConvertCode::GenConvertType_GFILE},
                                    {"WGS84", GenConvertCode::GenConvertType_NONE}}},
{"ASTRLA84-Grid", "WGS84",      { {"GDA94", GenConvertCode::GenConvertType_GFILE},
                                    {"WGS84", GenConvertCode::GenConvertType_NONE}}},
{"ASTRLA84-Grid", "GDA94",      { {"GDA94", GenConvertCode::GenConvertType_GFILE}}},
{"ASTRLA84-Grid", "EPSG:6283",  { {"EPSG:6283", GenConvertCode::GenConvertType_GFILE}}},
// -------
{"GDA94", "AGD84",              { {"AGD84", GenConvertCode::GenConvertType_GFILE}}},
{"GDA94", "AGD66",              { {"AGD66", GenConvertCode::GenConvertType_GFILE}}},
{"GDA94", "EPSG:6283",          { {"EPSG:6283", GenConvertCode::GenConvertType_NONE}}},
{"GDA94", "WGS84",              { {"WGS84", GenConvertCode::GenConvertType_NONE}}},

// -------
{"EPSG:6283", "AGD84",          { {"WGS84", GenConvertCode::GenConvertType_NONE},
                                    {"GDA94", GenConvertCode::GenConvertType_NONE},
                                    {"AGD84", GenConvertCode::GenConvertType_GFILE}}},
{"EPSG:6283", "AGD66",          { {"WGS84", GenConvertCode::GenConvertType_NONE},
                                    {"GDA94", GenConvertCode::GenConvertType_NONE},
                                    {"AGD66", GenConvertCode::GenConvertType_GFILE}}},
{"EPSG:6283", "GDA94",          { {"GDA94", GenConvertCode::GenConvertType_NONE}}},
{"EPSG:6283", "WGS84",          { {"WGS84", GenConvertCode::GenConvertType_NONE}}},
// -------                        
{"GDA94/GSB", "AGD84",          { {"GDA2020", GenConvertCode::GenConvertType_GFILE},
                                    {"WGS84", GenConvertCode::GenConvertType_NONE},
                                    {"GDA94", GenConvertCode::GenConvertType_NONE},
                                    {"AGD84", GenConvertCode::GenConvertType_GFILE}}},
{"GDA94/GSB", "AGD66",          { {"GDA2020", GenConvertCode::GenConvertType_GFILE},
                                    {"WGS84", GenConvertCode::GenConvertType_NONE},
                                    {"GDA94", GenConvertCode::GenConvertType_NONE},
                                    {"AGD66", GenConvertCode::GenConvertType_GFILE}}},
{"GDA94/GSB", "EPSG:6283",      { {"EPSG:6283", GenConvertCode::GenConvertType_NONE}}},
{"GDA94/GSB", "WGS84",          { {"GDA2020", GenConvertCode::GenConvertType_GFILE},
                                    {"WGS84", GenConvertCode::GenConvertType_NONE}}},
{"GDA94/GSB", "GDA2020",        { {"GDA2020", GenConvertCode::GenConvertType_GFILE}}},
// -------
{"GDA2020", "AGD84",          {   {"WGS84", GenConvertCode::GenConvertType_NONE},
                                    {"GDA94", GenConvertCode::GenConvertType_NONE},
                                    {"AGD84", GenConvertCode::GenConvertType_GFILE}}},
{"GDA2020", "AGD66",          {   {"WGS84", GenConvertCode::GenConvertType_NONE},
                                    {"GDA94", GenConvertCode::GenConvertType_NONE},
                                    {"AGD66", GenConvertCode::GenConvertType_GFILE}}},

{"GDA2020", "EPSG:6283",        { {"WGS84", GenConvertCode::GenConvertType_NONE},
                                    {"EPSG:6283", GenConvertCode::GenConvertType_NONE}}},
{"GDA2020", "WGS84",            { {"WGS84", GenConvertCode::GenConvertType_NONE}}},
{"GDA2020", "GDA94",            { {"WGS84", GenConvertCode::GenConvertType_NONE},
                                    {"GDA94", GenConvertCode::GenConvertType_NONE}}},
{"GDA2020", "GDA94/GSB",        { {"GDA94/GSB", GenConvertCode::GenConvertType_GFILE}}},

// South Africa
{"Hartebeesthoek1994", "CAPE-1",   { {"CAPE-1", GenConvertCode::GenConvertType_GEOCTR}}},
{"Hartebeesthoek1994", "CAPE/GSB", { {"CAPE/GSB", GenConvertCode::GenConvertType_GFILE}}},
{"Hartebeesthoek1994", "WGS84",    { {"WGS84", GenConvertCode::GenConvertType_GEOCTR}}},

// Japan
{"EPSG:6612", "JPNGSI-Grid",    { {"JPNGSI-Grid", GenConvertCode::GenConvertType_GFILE}}},
{"EPSG:6612", "Tokyo-Grid",     { {"Tokyo-Grid", GenConvertCode::GenConvertType_GFILE}}},
{"EPSG:6612", "JGD2000",        { {"WGS84", GenConvertCode::GenConvertType_NONE},
                                    {"JGD2000", GenConvertCode::GenConvertType_NONE}}},
{"EPSG:6612", "WGS84",          { {"WGS84", GenConvertCode::GenConvertType_NONE}}},
// -------

{"JGD2000", "JGD2011",          { {"JGD2011", GenConvertCode::GenConvertType_GFILE}}},
{"JGD2000", "JPNGSI-Grid",      { {"JPNGSI-Grid", GenConvertCode::GenConvertType_GFILE}}},
{"JGD2000", "Tokyo-Grid",       { {"Tokyo-Grid", GenConvertCode::GenConvertType_GFILE}}},
{"JGD2000", "WGS84",            { {"WGS84", GenConvertCode::GenConvertType_NONE}}},
// -------
{"JGD2000-7P", "JGD2011",       { {"JGD2011", GenConvertCode::GenConvertType_GFILE}}},
{"JGD2000-7P", "WGS84",         { {"WGS84", GenConvertCode::GenConvertType_7PARM}}},
// -------
{"JGD2011", "JGD2000-7P",       { {"JGD2000-7P", GenConvertCode::GenConvertType_GFILE}}},
{"JGD2011", "JPNGSI-Grid",      { {"JGD2000", GenConvertCode::GenConvertType_GFILE},
                                    {"JPNGSI-Grid", GenConvertCode::GenConvertType_GFILE}}},
{"JGD2011", "Tokyo-Grid",       { {"JGD2000", GenConvertCode::GenConvertType_GFILE},
                                    {"Tokyo-Grid", GenConvertCode::GenConvertType_GFILE}}},
//{"JGD2011", "TOKYO",            { {"WGS84", GenConvertCode::GenConvertType_NONE},
//                                    {"TOKYO", GenConvertCode::GenConvertType_MREG}}},

{"JGD2011", "WGS84",            { {"WGS84", GenConvertCode::GenConvertType_NONE}}},
// ------                         
// {"TOKYO", "JGD2011",            { {"WGS84", GenConvertCode::GenConvertType_MREG},
//                                    {"JGD2011", GenConvertCode::GenConvertType_NONE}} },

{"TOKYO", "WGS84",              { {"WGS84", GenConvertCode::GenConvertType_MREG}}},
// ------
{"Tokyo-Grid", "JGD2000",       { {"JGD2000", GenConvertCode::GenConvertType_GFILE}}},
{"Tokyo-Grid", "JGD2011",       { {"JGD2000", GenConvertCode::GenConvertType_GFILE},
                                    {"JGD2011", GenConvertCode::GenConvertType_GFILE}}},
{"Tokyo-Grid", "WGS84",         { {"JGD2000", GenConvertCode::GenConvertType_GFILE},
                                    {"WGS84", GenConvertCode::GenConvertType_NONE}}},
{"Tokyo-Grid", "EPSG:6612",     { {"EPSG:6612", GenConvertCode::GenConvertType_GFILE}}},

// Canada
{"CSRS", "EPSG:6140",           { {"EPSG:6140", GenConvertCode::GenConvertType_NONE}}},

// USA
{"NAD83/2011", "NAD27",         { {"NSRS07", GenConvertCode::GenConvertType_GFILE},
                                    {"NAD83/HARN-A", GenConvertCode::GenConvertType_GFILE},
                                    {"NAD83", GenConvertCode::GenConvertType_GFILE},
                                    { "NAD27", GenConvertCode::GenConvertType_GFILE}}},
                                  
{"NAD83/2011", "NAD83",         { {"NSRS07", GenConvertCode::GenConvertType_GFILE},
                                    {"NAD83/HARN-A", GenConvertCode::GenConvertType_GFILE},
                                    {"NAD83", GenConvertCode::GenConvertType_GFILE}}},

{"NAD83/2011", "NAD83/HARN-A",  { {"NSRS07", GenConvertCode::GenConvertType_GFILE},
                                    {"NAD83/HARN-A", GenConvertCode::GenConvertType_GFILE}}},

{"NAD83/2011", "NSRS07",        { {"NSRS07", GenConvertCode::GenConvertType_GFILE}}},
// -------

{"NSRS11", "NAD27",             { {"NSRS07", GenConvertCode::GenConvertType_GFILE},
                                    {"NAD83/HARN-A", GenConvertCode::GenConvertType_GFILE},
                                    {"NAD83", GenConvertCode::GenConvertType_GFILE},
                                    { "NAD27", GenConvertCode::GenConvertType_GFILE}}},
                                 
{"NSRS11", "NAD83",             { {"NSRS07", GenConvertCode::GenConvertType_GFILE},
                                    {"NAD83/HARN-A", GenConvertCode::GenConvertType_GFILE},
                                    {"NAD83", GenConvertCode::GenConvertType_GFILE}}},
                                 
{"NSRS11", "NAD83/HARN-A",      { {"NSRS07", GenConvertCode::GenConvertType_GFILE},
                                    {"NAD83/HARN-A", GenConvertCode::GenConvertType_GFILE}}},
                                 
{"NSRS11", "NSRS07",            { {"NSRS07", GenConvertCode::GenConvertType_GFILE}}},
// -------                       
                                 
{"NSRS07", "NAD27",             { {"NAD83/HARN-A", GenConvertCode::GenConvertType_GFILE},
                                    {"NAD83", GenConvertCode::GenConvertType_GFILE},
                                    { "NAD27", GenConvertCode::GenConvertType_GFILE}}},
                                 
{"NSRS07", "NAD83",             { {"NAD83/HARN-A", GenConvertCode::GenConvertType_GFILE},
                                    {"NAD83", GenConvertCode::GenConvertType_GFILE}}},
                                 
{"NSRS07", "NAD83/HARN-A",      { {"NAD83/HARN-A", GenConvertCode::GenConvertType_GFILE}}},
                                 
// -------                       
{"EPSG:6267", "NAD27",          { {"NAD27", GenConvertCode::GenConvertType_NONE}}},
{"EPSG:6267", "NAD83",          { {"NAD83", GenConvertCode::GenConvertType_GFILE}}},
{"EPSG:6267", "WGS84",          { {"NAD83", GenConvertCode::GenConvertType_GFILE},
                                    {"WGS84", GenConvertCode::GenConvertType_NONE}}},
// -------                        
{"NAD27", "EPSG:6267",          { {"EPSG:6267", GenConvertCode::GenConvertType_NONE}}},
{"NAD27", "NAD83",              { {"NAD83", GenConvertCode::GenConvertType_GFILE}}},
{"NAD27", "WGS84",              { {"NAD83", GenConvertCode::GenConvertType_GFILE},
                                    {"WGS84", GenConvertCode::GenConvertType_NONE}}},
// -------                       
{"EPSG:6269", "NAD83",          { {"WGS84", GenConvertCode::GenConvertType_NONE},
                                    {"NAD83", GenConvertCode::GenConvertType_NONE}}},
{"EPSG:6269", "NAD27",          { {"WGS84", GenConvertCode::GenConvertType_NONE},
                                    {"NAD83", GenConvertCode::GenConvertType_NONE},
                                    {"NAD27", GenConvertCode::GenConvertType_GFILE}}},
{"EPSG:6269", "WGS84",          { {"WGS84", GenConvertCode::GenConvertType_NONE}}},

// -------                       
// Puerto Rico                   
{"NAD27", "EPSG:6139",          { {"EPSG:6139", GenConvertCode::GenConvertType_NONE}}},
{"NAD27", "PuertoRico",         { {"PuertoRico", GenConvertCode::GenConvertType_NONE}}},
                                 
// -------                       
{"EPSG:6152", "HPGN",           { {"HPGN", GenConvertCode::GenConvertType_NONE}}},
                                 
// -------                       
{"HPGN", "NAD27",               { {"NAD83", GenConvertCode::GenConvertType_GFILE},
                                    {"NAD27", GenConvertCode::GenConvertType_GFILE}}},
{"HPGN", "EPSG:6152",           { {"EPSG:6152", GenConvertCode::GenConvertType_NONE}}},
{"HPGN", "EPSG:6269",           { {"EPSG:6269", GenConvertCode::GenConvertType_GFILE}}},
{"HPGN", "NAD83",               { {"NAD83", GenConvertCode::GenConvertType_GFILE}}},
                                 
// -------                       
{"NAD83", "PuertoRico",         { {"PuertoRico", GenConvertCode::GenConvertType_GFILE}}},
{"NAD83", "OldHawaiian",        { {"OldHawaiian", GenConvertCode::GenConvertType_GFILE}}},
{"NAD83", "NAD83/HARN",         { {"NAD83/HARN", GenConvertCode::GenConvertType_GFILE}}},
{"NAD83", "HPGN",               { {"HPGN", GenConvertCode::GenConvertType_GFILE}}},
{"NAD83", "HARN/WY",            { {"HARN/WY", GenConvertCode::GenConvertType_GFILE}}},
{"NAD83", "HARN/18",            { {"HARN/18", GenConvertCode::GenConvertType_GFILE}}},
{"NAD83", "EPSG:6152",          { {"EPSG:6152", GenConvertCode::GenConvertType_GFILE}}},
{"NAD83", "EPSG:6140",          { {"EPSG:6140", GenConvertCode::GenConvertType_GFILE}}},
{"NAD83", "CSRS",               { {"CSRS", GenConvertCode::GenConvertType_GFILE}}},
{"NAD83", "NAD27",              { {"NAD27", GenConvertCode::GenConvertType_GFILE}}},
{"NAD83", "NAD27/CGQ77-83",     { {"NAD27/CGQ77-83", GenConvertCode::GenConvertType_GFILE}}},
{"NAD83", "NAD27/1976",         { {"NAD27/1976", GenConvertCode::GenConvertType_GFILE}}},
{"NAD83", "MICHIGAN",           { {"MICHIGAN", GenConvertCode::GenConvertType_GFILE}}},
{"NAD83", "EPSG:6609",          { {"EPSG:6609", GenConvertCode::GenConvertType_GFILE}}},
{"NAD83", "EPSG:6267",          { {"EPSG:6267", GenConvertCode::GenConvertType_GFILE}}},
{"NAD83", "EPSG:6139",          { {"EPSG:6139", GenConvertCode::GenConvertType_GFILE}}},
{"NAD83", "EPSG:6138",          { {"EPSG:6138", GenConvertCode::GenConvertType_GFILE}}},
{"NAD83", "EPSG:6137",          { {"EPSG:6137", GenConvertCode::GenConvertType_GFILE}}},
{"NAD83", "EPSG:6136",          { {"EPSG:6136", GenConvertCode::GenConvertType_GFILE}}},
{"NAD83", "EPSG:6135",          { {"EPSG:6135", GenConvertCode::GenConvertType_GFILE}}},
{"NAD83", "NSRS11",             { {"NAD83/HARN-A", GenConvertCode::GenConvertType_GFILE},
                                    {"NSRS07", GenConvertCode::GenConvertType_GFILE},
                                    {"NSRS11", GenConvertCode::GenConvertType_GFILE}}},
{"NAD83", "NAD83/2011",         { {"NAD83/HARN-A", GenConvertCode::GenConvertType_GFILE},
                                    {"NSRS07", GenConvertCode::GenConvertType_GFILE},
                                    {"NAD83/2011", GenConvertCode::GenConvertType_GFILE}}},
{"NAD83", "WGS84",              { {"WGS84", GenConvertCode::GenConvertType_NONE}}},

// Slovekia
{"Slov/JTSK-NULL", "Slov/JTSK03", { {"Slov/JTSK03", GenConvertCode::GenConvertType_NONE}}},
{"Slov/JTSK-NULL", "WGS84",     { {"WGS84", GenConvertCode::GenConvertType_7PARM}}},
// -------
{"Slov/JTSK03", "Slov/JTSK",    { {"Slov/JTSK", GenConvertCode::GenConvertType_GFILE}}},
{"Slov/JTSK03", "Slov/JTSK-NULL", { {"Slov/JTSK-NULL", GenConvertCode::GenConvertType_NONE}}},
{"Slov/JTSK03", "WGS84",        { {"WGS84", GenConvertCode::GenConvertType_7PARM}}},


// Switzerland
{"CH1903/GSB", "CH1903Plus_2",  { {"CH1903Plus_2", GenConvertCode::GenConvertType_GFILE}}},
{"CH1903/GSB", "WGS84",         { {"CH1903Plus_1", GenConvertCode::GenConvertType_GFILE},
                                    {"CHTRF95", GenConvertCode::GenConvertType_GEOCTR},
                                    {"ETRF89", GenConvertCode::GenConvertType_GEOCTR},
                                    {"WGS84", GenConvertCode::GenConvertType_GEOCTR}}},
{"CH1903/GSB", "CHTRF95",       { {"CH1903Plus_1", GenConvertCode::GenConvertType_GFILE},
                                    {"CHTRF95", GenConvertCode::GenConvertType_GEOCTR}}},
{"CH1903/GSB", "CH1903Plus_1",  { {"CH1903Plus_1", GenConvertCode::GenConvertType_GFILE}}},
                                  
// -------                        
                                  
{"CH1903Plus_1", "CHTRF95",     { {"CHTRF95", GenConvertCode::GenConvertType_GEOCTR}}},
{"CH1903Plus_1", "WGS84",       { {"WGS84", GenConvertCode::GenConvertType_GEOCTR}}},
                                  

{"CH1903Plus_1", "CH1903/GSB",  { {"CH1903/GSB", GenConvertCode::GenConvertType_GFILE}}},
// -------                        
                                  
{"CH1903Plus_2", "CHTRF95",     { {"CHTRF95", GenConvertCode::GenConvertType_GEOCTR}}},
{"CH1903Plus_2", "WGS84",       { {"WGS84", GenConvertCode::GenConvertType_GEOCTR}}},
                                  

{"CH1903Plus_2", "CH1903/GSB",  { {"CH1903/GSB", GenConvertCode::GenConvertType_GFILE}}},
                                  
// -------                        
                                  
{"CHTRF95", "CH1903/GSB",       { {"CH1903Plus_1", GenConvertCode::GenConvertType_GEOCTR},
                                    {"CH1903/GSB", GenConvertCode::GenConvertType_GFILE}}},
{"CHTRF95", "CH1903Plus_1",     { {"CH1903Plus_1", GenConvertCode::GenConvertType_GEOCTR}}},
{"CHTRF95", "CH1903Plus_2",     { {"CH1903Plus_2", GenConvertCode::GenConvertType_GEOCTR}}},
                                  
// France                         
{"EPSG:6171", "EPSG:6275",      { {"EPSG:6275", GenConvertCode::GenConvertType_GFILE}}},
{"EPSG:6171", "NTF",            { {"NTF", GenConvertCode::GenConvertType_GFILE}}},
{"EPSG:6171", "NTF-G-Grid",     { {"NTF-G-Grid", GenConvertCode::GenConvertType_GFILE}}},
{"EPSG:6171", "NTF-G-Grid-ClrkIGN", { {"NTF-G-Grid-ClrkIGN", GenConvertCode::GenConvertType_GFILE}}},
{"EPSG:6171", "WGS84",          { {"WGS84", GenConvertCode::GenConvertType_NONE}}},
{"EPSG:6171", "RGF93",          { {"WGS84", GenConvertCode::GenConvertType_NONE},
                                    {"RGF93", GenConvertCode::GenConvertType_NONE}}},
                                  
// -------                        
{ "RGF93", "EPSG:6275",         { {"EPSG:6275", GenConvertCode::GenConvertType_GFILE}} },
{ "RGF93", "NTF",               { {"NTF", GenConvertCode::GenConvertType_GFILE}} },
{ "RGF93", "NTF-G-Grid",        { {"NTF-G-Grid", GenConvertCode::GenConvertType_GFILE}} },
{ "RGF93", "NTF-G-Grid-ClrkIGN", { {"NTF-G-Grid-ClrkIGN", GenConvertCode::GenConvertType_GFILE}} },
{ "RGF93", "WGS84",             { {"WGS84", GenConvertCode::GenConvertType_NONE}} },
{ "RGF93", "EPSG:6171",         { {"WGS84", GenConvertCode::GenConvertType_NONE},
                                    {"EPSG:6171", GenConvertCode::GenConvertType_NONE}} },
                                  
// -------                        
{"EPSG:6275", "EPSG:6171",      { {"EPSG:6171", GenConvertCode::GenConvertType_GFILE}}},
{"EPSG:6275", "NTF",            { {"NTF", GenConvertCode::GenConvertType_NONE}}},
{"EPSG:6275", "NTF-G-Grid",     { {"NTF-G-Grid", GenConvertCode::GenConvertType_NONE}}},
{"EPSG:6275", "NTF-G-Grid-ClrkIGN", { {"NTF-G-Grid-ClrkIGN", GenConvertCode::GenConvertType_NONE}}},
{"EPSG:6275", "WGS84",          { {"RGF93", GenConvertCode::GenConvertType_GFILE},
                                    {"WGS84", GenConvertCode::GenConvertType_NONE}}},
{"EPSG:6275", "RGF93",          { {"RGF93", GenConvertCode::GenConvertType_GFILE}}},
                                  
// UK                             
                                  
// New Zealand                    
{ "NZGD2000", "EPSG:6272",      { {"EPSG:6272", GenConvertCode::GenConvertType_GFILE}} },
{ "NZGD2000", "NZGD49",         { {"NZGD49", GenConvertCode::GenConvertType_GFILE}} },
{ "NZGD2000", "WGS84",          { {"WGS84", GenConvertCode::GenConvertType_NONE}} },
                                  
{ "NZGD49", "EPSG:6272",        { {"EPSG:6272", GenConvertCode::GenConvertType_NONE}} },
{ "NZGD49", "NZGD2000",         { {"NZGD2000", GenConvertCode::GenConvertType_GFILE}} },
{ "NZGD49", "WGS84",            { {"NZGD2000", GenConvertCode::GenConvertType_GFILE},
                                    {"WGS84", GenConvertCode::GenConvertType_NONE}}},

// South America
{ "SIRGAS2000", "CORREGO-1961-A",       { {"CORREGO-1961-A", GenConvertCode::GenConvertType_GEOCTR}} },
{ "SIRGAS2000", "CORREGO-70-72-BZ/GSB", { {"CORREGO-70-72-BZ/GSB", GenConvertCode::GenConvertType_GFILE}} },
{ "SIRGAS2000", "CORREGO-7072-A",       { {"CORREGO-7072-A", GenConvertCode::GenConvertType_GEOCTR}} },
{ "SIRGAS2000", "CORREGO1961-BZ/GSB",   { {"CORREGO1961-BZ/GSB", GenConvertCode::GenConvertType_GFILE}} },
{ "SIRGAS2000", "SA1969-BZ-A",          { {"SA1969-BZ-A", GenConvertCode::GenConvertType_GEOCTR}} },
{ "SIRGAS2000", "SAD69-BZ/GSB",         { {"SAD69-BZ/GSB", GenConvertCode::GenConvertType_GFILE}} },
{ "SIRGAS2000", "SAD69/96-BZ/GSB",      { {"SAD69/96-BZ/GSB", GenConvertCode::GenConvertType_GFILE}} },
};

/*---------------------------------------------------------------------------------**//**
* test all transformation paths are obtainable from datum to WGS84
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P (GCSSpecificDatumTests, TestTransformPaths)
    {
    datumTransformPaths currentPath = GetParam();

    GeoCoordinates::DatumCP sourceDatum = GeoCoordinates::Datum::CreateDatum(currentPath.m_sourceDatum.c_str());
    GeoCoordinates::DatumCP targetDatum = GeoCoordinates::Datum::CreateDatum(currentPath.m_targetDatum.c_str());

    GeoCoordinates::GeodeticTransformPathCP newPath = GeoCoordinates::GeodeticTransformPath::Create(*sourceDatum, *targetDatum);

    ASSERT_TRUE(nullptr != newPath);

    int scannedIndex = 0;
    for (int stepIndex = 0 ; stepIndex < newPath->GetGeodeticTransformCount() ; stepIndex++)
        {
        GeoCoordinates::GenConvertCode theCode = newPath->GetGeodeticTransform(stepIndex)->GetConvertMethodCode();
        Utf8String targetDatumName = newPath->GetGeodeticTransform(stepIndex)->GetTargetDatumName();
        
        EXPECT_TRUE(scannedIndex < currentPath.m_path.size());
        if (scannedIndex < currentPath.m_path.size())
            {
            GenConvertCode theExpectedCode = currentPath.m_path[scannedIndex].m_convertOp;
            Utf8String expectedTargetDatumName = currentPath.m_path[scannedIndex].m_targetDatum;
            EXPECT_TRUE(theExpectedCode == theCode);
            EXPECT_TRUE(expectedTargetDatumName == targetDatumName);
            }
        scannedIndex++;
        }

    EXPECT_TRUE(currentPath.m_path.size() == scannedIndex);

    sourceDatum->Destroy();
    targetDatum->Destroy();
    newPath->Destroy();
    }


INSTANTIATE_TEST_SUITE_P(GCSSpecificDatumTests_Combined,
    GCSSpecificDatumTests,
    ValuesIn(s_pathsToTest));