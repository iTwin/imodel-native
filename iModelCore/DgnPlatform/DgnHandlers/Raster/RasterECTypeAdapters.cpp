/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/Raster/RasterECTypeAdapters.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>
#ifdef DGN_IMPORTER_REORG_WIP

static const double MIN_GAMMA = 0.01;
static const double MAX_GAMMA = 256.0;

typedef struct tagPercentFromSignedRange
    {
    int     range;
    double  percent;
    } PercentFromSignedRange;

typedef struct tagPercentFromRange
    {
    unsigned char   range;
    double          percent;
    } PercentFromRange;


/* this table applies to convert from range [0, 255] to percentage [0.0, 100.0] round to nearest .5%*/
static PercentFromRange LookUpPercentFromRange [] =
    {{  0,0.0},
    {  1,0.5}, {  2,0.5}, {  3,1.0}, {  4,1.5}, {  5,2.0},
    {  6,2.5}, {  7,2.5}, {  8,3.0}, {  9,3.5}, { 10,4.0},
    { 11,4.0}, { 12,4.5}, { 13,5.0}, { 14,5.5}, { 15,6.0},
    { 16,6.0}, { 17,6.5}, { 18,7.0}, { 19,7.5}, { 20,8.0},
    { 21,8.0}, { 22,8.5}, { 23,9.0}, { 24,9.5}, { 25,9.5},
    { 26,10.0},{ 27,10.5},{ 28,11.0},{ 29,11.5},{ 30,11.5},
    { 31,12.0},{ 32,12.5},{ 33,13.0},{ 34,13.0},{ 35,13.5},
    { 36,14.0},{ 37,14.5},{ 38,15.0},{ 39,15.0},{ 40,15.5},
    { 41,16.0},{ 42,16.5},{ 43,16.5},{ 44,17.0},{ 45,17.5},
    { 46,18.0},{ 47,18.5},{ 48,18.5},{ 49,19.0},{ 50,19.5},
    { 51,20.0},{ 52,20.5},{ 53,20.5},{ 54,21.0},{ 55,21.5},
    { 56,22.0},{ 57,22.0},{ 58,22.5},{ 59,23.0},{ 60,23.5},
    { 61,24.0},{ 62,24.0},{ 63,24.5},{ 64,25.0},{ 65,25.5},
    { 66,25.5},{ 67,26.0},{ 68,26.5},{ 69,27.0},{ 70,27.5},
    { 71,27.5},{ 72,28.0},{ 73,28.5},{ 74,29.0},{ 75,29.0},
    { 76,29.5},{ 77,30.0},{ 78,30.5},{ 79,31.0},{ 80,31.0},
    { 81,31.5},{ 82,32.0},{ 83,32.5},{ 84,33.0},{ 85,33.0},
    { 86,33.5},{ 87,34.0},{ 88,34.5},{ 89,34.5},{ 90,35.0},
    { 91,35.5},{ 92,36.0},{ 93,36.5},{ 94,36.5},{ 95,37.0},
    { 96,37.5},{ 97,38.0},{ 98,38.0},{ 99,38.5},{100,39.0},
    {101,39.5},{102,40.0},{103,40.0},{104,40.5},{105,41.0},
    {106,41.5},{107,41.5},{108,42.0},{109,42.5},{110,43.0},
    {111,43.5},{112,43.5},{113,44.0},{114,44.5},{115,45.0},
    {116,45.5},{117,45.5},{118,46.0},{119,46.5},{120,47.0},
    {121,47.0},{122,47.5},{123,48.0},{124,48.5},{125,49.0},
    {126,49.0},{127,49.5},{128,50.0},{129,50.5},{130,50.5},
    {131,51.0},{132,51.5},{133,52.0},{134,52.5},{135,52.5},
    {136,53.0},{137,53.5},{138,54.0},{139,54.0},{140,54.5},
    {141,55.0},{142,55.5},{143,56.0},{144,56.0},{145,56.5},
    {146,57.0},{147,57.5},{148,58.0},{149,58.0},{150,58.5},
    {151,59.0},{152,59.5},{153,59.5},{154,60.0},{155,60.5},
    {156,61.0},{157,61.5},{158,61.5},{159,62.0},{160,62.5},
    {161,63.0},{162,63.0},{163,63.5},{164,64.0},{165,64.5},
    {166,65.0},{167,65.0},{168,65.5},{169,66.0},{170,66.5},
    {171,66.5},{172,67.0},{173,67.5},{174,68.0},{175,68.5},
    {176,68.5},{177,69.0},{178,69.5},{179,70.0},{180,70.5},
    {181,70.5},{182,71.0},{183,71.5},{184,72.0},{185,72.0},
    {186,72.5},{187,73.0},{188,73.5},{189,74.0},{190,74.0},
    {191,74.5},{192,75.0},{193,75.5},{194,75.5},{195,76.0},
    {196,76.5},{197,77.0},{198,77.5},{199,77.5},{200,78.0},
    {201,78.5},{202,79.0},{203,79.0},{204,79.5},{205,80.0},
    {206,80.5},{207,81.0},{208,81.0},{209,81.5},{210,82.0},
    {211,82.5},{212,83.0},{213,83.0},{214,83.5},{215,84.0},
    {216,84.5},{217,84.5},{218,85.0},{219,85.5},{220,86.0},
    {221,86.5},{222,86.5},{223,87.0},{224,87.5},{225,88.0},
    {226,88.0},{227,88.5},{228,89.0},{229,89.5},{230,90.0},
    {231,90.0},{232,90.5},{233,91.0},{234,91.5},{235,91.5},
    {236,92.0},{237,92.5},{238,93.0},{239,93.5},{240,93.5},
    {241,94.0},{242,94.5},{243,95.0},{244,95.5},{245,95.5},
    {246,96.0},{247,96.5},{248,97.0},{249,97.0},{250,97.5},
    {251,98.0},{252,98.5},{253,99.0},{254,99.0},{255,100.0}
    };


/* this table applies to convert from range [-128, 127] to percentage [-100.0, 100.0] (round to 1%) */
static PercentFromSignedRange LookUpPercentFromSignedRange [] =
    {
        {-128,-100.0},{-127,-99.0},{-126,-99.0},{-125,-98.0},
        {-124,-97.0}, {-123,-96.0},{-122,-95.0},{-121,-95.0},{-120,-94.0},
        {-119,-93.0}, {-118,-92.0},{-117,-92.0},{-116,-91.0},{-115,-90.0},
        {-114,-89.0}, {-113,-88.0},{-112,-88.0},{-111,-87.0},{-110,-86.0},
        {-109,-85.0}, {-108,-84.0},{-107,-84.0},{-106,-83.0},{-105,-82.0},
        {-104,-81.0}, {-103,-81.0},{-102,-80.0},{-101,-79.0},{-100,-78.0},
        {-99,-77.0},  {-98,-77.0}, {-97,-76.0}, {-96,-75.0}, {-95,-74.0},
        {-94,-74.0},  {-93,-73.0}, {-92,-72.0}, {-91,-71.0}, {-90,-70.0},
        {-89,-70.0},  {-88,-69.0}, {-87,-68.0}, {-86,-67.0}, {-85,-67.0},
        {-84,-66.0},  {-83,-65.0}, {-82,-64.0}, {-81,-63.0}, {-80,-63.0},
        {-79,-62.0},  {-78,-61.0}, {-77,-60.0}, {-76,-59.0}, {-75,-59.0},
        {-74,-58.0},  {-73,-57.0}, {-72,-56.0}, {-71,-56.0}, {-70,-55.0},
        {-69,-54.0},  {-68,-53.0}, {-67,-52.0}, {-66,-52.0}, {-65,-51.0},
        {-64,-50.0},  {-63,-49.0}, {-62,-49.0}, {-61,-48.0}, {-60,-47.0},
        {-59,-46.0},  {-58,-45.0}, {-57,-45.0}, {-56,-44.0}, {-55,-43.0},
        {-54,-42.0},  {-53,-42.0}, {-52,-41.0}, {-51,-40.0}, {-50,-39.0},
        {-49,-38.0},  {-48,-38.0}, {-47,-37.0}, {-46,-36.0}, {-45,-35.0},
        {-44,-34.0},  {-43,-34.0}, {-42,-33.0}, {-41,-32.0}, {-40,-31.0},
        {-39,-31.0},  {-38,-30.0}, {-37,-29.0}, {-36,-28.0}, {-35,-27.0},
        {-34,-27.0},  {-33,-26.0}, {-32,-25.0}, {-31,-24.0}, {-30,-24.0},
        {-29,-23.0},  {-28,-22.0}, {-27,-21.0}, {-26,-20.0}, {-25,-20.0},
        {-24,-19.0},  {-23,-18.0}, {-22,-17.0}, {-21,-17.0}, {-20,-16.0},
        {-19,-15.0},  {-18,-14.0}, {-17,-13.0}, {-16,-13.0}, {-15,-12.0},
        {-14,-11.0},  {-13,-10.0}, {-12,-9.0},  {-11,-9.0},  {-10,-8.0},
        { -9,-7.0},   { -8,-6.0},  { -7,-6.0},  { -6,-5.0},  { -5,-4.0},
        { -4,-3.0},   { -3,-2.0},  { -2,-2.0},  { -1,-1.0},  {  0,0.0},
        {  1,1.0},    {  2,2.0},   {  3,3.0},   {  4,4.0},   {  5,4.0},
        {  6,5.0},    {  7,6.0},   {  8,7.0},   {  9,7.0},   { 10,8.0},
        { 11,9.0},    { 12,10.0},   { 13,11.0},  { 14,11.0},  { 15,12.0},
        { 16,13.0},  { 17,14.0},   { 18,15.0},  { 19,15.0},  { 20,16.0},
        { 21,17.0},  { 22,18.0},   { 23,19.0},  { 24,19.0},  { 25,20.0},
        { 26,21.0},  { 27,22.0},   { 28,22.0},  { 29,23.0},  { 30,24.0},
        { 31,25.0},  { 32,26.0},   { 33,26.0},  { 34,27.0},  { 35,28.0},
        { 36,29.0},  { 37,30.0},   { 38,30.0},  { 39,31.0},  { 40,32.0},
        { 41,33.0},  { 42,33.0},   { 43,34.0},  { 44,35.0},  { 45,36.0},
        { 46,37.0},  { 47,37.0},   { 48,38.0},  { 49,39.0},  { 50,40.0},
        { 51,41.0},  { 52,41.0},   { 53,42.0},  { 54,43.0},  { 55,44.0},
        { 56,44.0},  { 57,45.0},   { 58,46.0},  { 59,47.0},  { 60,48.0},
        { 61,48.0},  { 62,49.0},   { 63,50.0},  { 64,51.0},  { 65,52.0},
        { 66,52.0},  { 67,53.0},   { 68,54.0},  { 69,55.0},  { 70,56.0},
        { 71,56.0},  { 72,57.0},   { 73,58.0},  { 74,59.0},  { 75,59.0},
        { 76,60.0},  { 77,61.0},   { 78,62.0},  { 79,63.0},  { 80,63.0},
        { 81,64.0},  { 82,65.0},   { 83,66.0},  { 84,67.0},  { 85,67.0},
        { 86,68.0},  { 87,69.0},   { 88,70.0},  { 89,70.0},  { 90,71.0},
        { 91,72.0},  { 92,73.0},   { 93,74.0},  { 94,74.0},  { 95,75.0},
        { 96,76.0},  { 97,77.0},   { 98,78.0},  { 99,78.0},  {100,79.0},
        {101,80.0},  {102,81.0},   {103,81.0},  {104,82.0},  {105,83.0},
        {106,84.0},  {107,85.0},   {108,85.0},  {109,86.0},  {110,87.0},
        {111,88.0},  {112,89.0},   {113,89.0},  {114,90.0},  {115,91.0},
        {116,92.0},  {117,93.0},   {118,93.0},  {119,94.0},  {120,95.0},
        {121,96.0},  {122,96.0},   {123,97.0},  {124,98.0},  {125,99.0},
        {126,99.0},  {127,100.0}
    };


#ifdef UNUSED_FUNCTION
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static double  GetPercentFromRange
(            /*<= percentage [0.0, 100.0] (round to 0.5)*/
int pi_range /*=> range [0, 255]*/
)
    {
    //mdlSystem_enterDebug ();
    BeAssert((pi_range>=0) && (pi_range<=255));
    return LookUpPercentFromRange[pi_range].percent;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static double  GetPercentFromSignedRange
(                  /*<= percentage [-100.0, 100.0] (round to 0.5)*/
int pi_signedRange /*=> range [-128, 127]*/
)
    {
    BeAssert((pi_signedRange>=-128) && (pi_signedRange<=127));
    return LookUpPercentFromSignedRange[pi_signedRange + 128].percent;
    }

#ifdef UNUSED_FUNCTION
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static unsigned char GetRangeFromPercent
(                  /* <= range [0, 255] */
double pi_percent  /* => percentage [0.0, 100.0] */
)
    {
    int i;

    BeAssert((pi_percent>=0.0) && (pi_percent<=100));

    if (pi_percent >= 100)
        return(255);

    if (pi_percent <= 0)
        return(0);

    for (i=1;i<256;i++ )
        {
        if (LookUpPercentFromRange[i].percent > pi_percent)
            {
            // TR 152509 return the percentage which is the closest to the input percentage
            if (LookUpPercentFromRange[i].percent - pi_percent < pi_percent - LookUpPercentFromRange[i-1].percent)
                return LookUpPercentFromRange[i].range;
            else /*The previous range was the one we searching for*/
                return LookUpPercentFromRange[i-1].range;
            }
        }
    return(0);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static char GetSignedRangeFromPercent
(                 /* <= range [-128, 127] */
double pi_percent /* => percentage [-100.0, 100.0] */
)
    {
    int i;

    BeAssert((pi_percent>=-100.0) && (pi_percent<=100));

    if (pi_percent >= 100)
        return(127);

    if (pi_percent <= -100)
        return(-128);

    for (i=1;i<256;i++ )
        {
        if (LookUpPercentFromSignedRange[i].percent > pi_percent)
            {
            /*The previous range was the one we searching for*/
            return LookUpPercentFromSignedRange[i-1].range;
            }
        }
    return(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IDgnECTypeAdapterPtr RangeToPercentageAdapter::Create()        
    {
    return new RangeToPercentageAdapter(); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool RangeToPercentageAdapter::_Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context)
    {
    if (!v.IsNull() && v.IsInteger())
        {
        Int32 range = v.GetInteger();

        if (range<-128 || range>127)
            return false;

        return true;           
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool RangeToPercentageAdapter::_ConvertToString (WStringR valueAsString, ECN::ECValueCR inputVal, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter)
    {
    double percent(GetPercentFromSignedRange(inputVal.GetInteger()));

    WChar    buffer [100];
    BeStringUtilities::Snwprintf (buffer, _countof(buffer), L"%3.0lf %%", percent);
    valueAsString = WString(buffer);

    return !valueAsString.empty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool RangeToPercentageAdapter::_ConvertFromString (ECN::ECValueR outVal, WCharCP stringVal, IDgnECTypeAdapterContextCR context)
    {
    if (NULL == stringVal)
        return false;

    double percent;
    if (1 == BE_STRING_UTILITIES_SWSCANF (stringVal, L"%lf", &percent))
        {
        Int32 range(GetSignedRangeFromPercent(percent));
        outVal.SetInteger (range);
        return true;
        }

    return false;
    }

/////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IDgnECTypeAdapterPtr PageNumberTypeAdapter::Create()        
    {
    return new PageNumberTypeAdapter(); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool PageNumberTypeAdapter::_Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context)
    {
    //Displayed value varies from 1 to NbPage
    //Internal valid values are from 0 to NbPage-1
    if (!v.IsNull() && v.IsInteger())
        {
        Int32 page = v.GetInteger();

        if (page<0)
            return false;

        return true;           
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool PageNumberTypeAdapter::_ConvertToString (WStringR valueAsString, ECN::ECValueCR inputVal, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter)
    {
    //We want to display page 0 as 1
    WChar    buffer [100];
    BeStringUtilities::Snwprintf (buffer, _countof(buffer), L"%d", (inputVal.GetInteger()+1));
    valueAsString = WString(buffer);

    return !valueAsString.empty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool PageNumberTypeAdapter::_ConvertFromString (ECN::ECValueR outVal, WCharCP stringVal, IDgnECTypeAdapterContextCR context)
    {
    if (NULL == stringVal)
        return false;

    Int32 i;
    if (1 == BE_STRING_UTILITIES_SWSCANF (stringVal, L"%d", &i))
        {
        //We want to display page 0 as 1
        outVal.SetInteger (i-1);
        return true;
        }

    return false;
    }


/////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IDgnECTypeAdapterPtr ColorModeTypeAdapter::Create()        
    {
    return new ColorModeTypeAdapter(); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ColorModeTypeAdapter::_Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context)
    {
    //Displayed value varies from 1 to NbPage
    //Internal valid values are from 0 to NbPage-1
    if (!v.IsNull() && v.IsInteger())
        {
        Int32 ColorMode = v.GetInteger();

        if (ColorMode == (Int32)DgnPlatform::ImageColorMode::Unknown         ||
            ColorMode == (Int32)DgnPlatform::ImageColorMode::Any             ||
            ColorMode == (Int32)DgnPlatform::ImageColorMode::RGB             ||
            ColorMode == (Int32)DgnPlatform::ImageColorMode::Palette16       ||
            ColorMode == (Int32)DgnPlatform::ImageColorMode::Palette256      ||
            ColorMode == (Int32)DgnPlatform::ImageColorMode::GreyScale       ||
            ColorMode == (Int32)DgnPlatform::ImageColorMode::Monochrome      ||
            ColorMode == (Int32)DgnPlatform::ImageColorMode::RGBA            ||
            ColorMode == (Int32)DgnPlatform::ImageColorMode::Palette256Alpha ||
            ColorMode == (Int32)DgnPlatform::ImageColorMode::GreyScale16     ||
            ColorMode == (Int32)DgnPlatform::ImageColorMode::Palette2          )
            return true;           
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ColorModeTypeAdapter::_ConvertToString (WStringR valueAsString, ECN::ECValueCR inputVal, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter)
    {
    Int32 ColorMode = inputVal.GetInteger();

    switch (ColorMode)
    {
    case DgnPlatform::ImageColorMode::Unknown:
    case DgnPlatform::ImageColorMode::Any:
        {
        valueAsString = ConfigurationManager::GetString (DGNHANDLERS_STRINGS, DgnHandlersMessage::MSGID_ECTYPEADAPTER_ColorMode_Unknown);
        }
        break;
    case DgnPlatform::ImageColorMode::RGB:
        {
        valueAsString = ConfigurationManager::GetString (DGNHANDLERS_STRINGS, DgnHandlersMessage::MSGID_ECTYPEADAPTER_ColorMode_RGB);
        }
        break;
    case DgnPlatform::ImageColorMode::Palette16:
        {
        valueAsString = ConfigurationManager::GetString (DGNHANDLERS_STRINGS, DgnHandlersMessage::MSGID_ECTYPEADAPTER_ColorMode_Palette16);
        }
        break;
    case DgnPlatform::ImageColorMode::Palette256:
        {
        valueAsString = ConfigurationManager::GetString (DGNHANDLERS_STRINGS, DgnHandlersMessage::MSGID_ECTYPEADAPTER_ColorMode_Palette256);
        }
        break;
    case DgnPlatform::ImageColorMode::GreyScale:
        {
        valueAsString = ConfigurationManager::GetString (DGNHANDLERS_STRINGS, DgnHandlersMessage::MSGID_ECTYPEADAPTER_ColorMode_Greyscale);
        }
        break;
    case DgnPlatform::ImageColorMode::Monochrome:
        {
        valueAsString = ConfigurationManager::GetString (DGNHANDLERS_STRINGS, DgnHandlersMessage::MSGID_ECTYPEADAPTER_ColorMode_Monochrome);
        }
        break;
    case DgnPlatform::ImageColorMode::RGBA:
        {
        valueAsString = ConfigurationManager::GetString (DGNHANDLERS_STRINGS, DgnHandlersMessage::MSGID_ECTYPEADAPTER_ColorMode_RGBA);
        }
        break;
    case DgnPlatform::ImageColorMode::Palette256Alpha:
        {
        valueAsString = ConfigurationManager::GetString (DGNHANDLERS_STRINGS, DgnHandlersMessage::MSGID_ECTYPEADAPTER_ColorMode_Palette256Alpha);
        }
        break;
    case DgnPlatform::ImageColorMode::GreyScale16:
        {
        valueAsString = ConfigurationManager::GetString (DGNHANDLERS_STRINGS, DgnHandlersMessage::MSGID_ECTYPEADAPTER_ColorMode_Greyscale16);
        }
        break;
    case DgnPlatform::ImageColorMode::Palette2:
        {
        valueAsString = ConfigurationManager::GetString (DGNHANDLERS_STRINGS, DgnHandlersMessage::MSGID_ECTYPEADAPTER_ColorMode_Palette2);
        }
        break;
    default:
        {
        valueAsString = ConfigurationManager::GetString (DGNHANDLERS_STRINGS, DgnHandlersMessage::MSGID_ECTYPEADAPTER_ColorMode_Unknown);
        }
        break;
    }

    return !valueAsString.empty();
    }



/////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IDgnECTypeAdapterPtr RasterViewFlagsAdapter::Create()        
    {
    return new RasterViewFlagsAdapter(); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterViewFlagsAdapter::_Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context)
    {
    if (!v.IsNull())
        {
        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterViewFlagsAdapter::_ConvertToString (WStringR valueAsString, ECN::ECValueCR inputVal, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter)
    {
    if (!inputVal.IsNull() && inputVal.IsArray())
        {
        ElementHandle eh(context.GetElementRef(),context.GetDgnModel());
        RasterFrameHandler* pQuery = dynamic_cast<RasterFrameHandler*>(&eh.GetHandler());
        if(NULL == pQuery)
            return false; //It is NOT a RasterAttachment!

        UInt32 count = inputVal.GetArrayInfo().GetCount();
        for (UInt32 view = 0; view < count; view++)
            {
            if (pQuery->GetViewState(eh,view))
                {
                WChar     formatString[128];

                BeStringUtilities::Snwprintf(formatString,_countof(formatString),L"%d",view+1);
                valueAsString += WString (formatString);
                }
            else
                {
                valueAsString += WString(L"  ");
                }
            if (view < 7)
                valueAsString += WString(L"-");
            }
        }


    return !valueAsString.empty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterViewFlagsAdapter::_ConvertFromString (ECN::ECValueR outVal, WCharCP stringVal, IDgnECTypeAdapterContextCR context)
    {
    if (NULL == stringVal)
        return false;

    return false;
    }

/////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IDgnECTypeAdapterPtr GeotiffUnitTypeAdapter::Create()        
    {
    return new GeotiffUnitTypeAdapter(); 
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static void AddAllUnits(IDgnECTypeAdapter::StandardValuesCollection& values)
    {
    int         iSystem;
    UnitSystem  unitSystem[] = {UnitSystem::Metric, UnitSystem::English, UnitSystem::USSurvey};

    for (iSystem = 0; iSystem < _countof (unitSystem); iSystem++)
        {
        UnitIteratorOptions options;
        options.SetAllowSingleSystem (unitSystem[iSystem]);

        StandardUnitCollection collection (options);

        FOR_EACH (StandardUnitCollection::Entry const& standardUnit,  collection)
            {
            WString unitName(standardUnit.GetName(false));
            values.push_back(unitName);
            }
        }
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeotiffUnitTypeAdapter::_Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context)
    {
    if (!v.IsNull() && v.IsInteger())
        {
        Int32 unitValue = v.GetInteger();
        if (unitValue==0)
            return true;

        UnitDefinition unitDef(UnitDefinition::GetStandardUnit(static_cast<StandardUnit>(unitValue)));

        return unitDef.IsValid();
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeotiffUnitTypeAdapter::_ConvertToString (WStringR valueAsString, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter)
    {
    Int32 unitValue = v.GetInteger();

    if (unitValue == StandardUnit::None)
        {
        valueAsString = ConfigurationManager::GetString (DGNHANDLERS_STRINGS, DgnHandlersMessage::MSGID_ECTYPEADAPTER_WorkingUnits);
        }
    else
        {
        valueAsString = UnitDefinition::GetStandardName(static_cast<StandardUnit>(unitValue),false);
        }

    return !valueAsString.empty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeotiffUnitTypeAdapter::_ConvertFromString (ECN::ECValueR outVal, WCharCP stringVal, IDgnECTypeAdapterContextCR context)
    {
    if (NULL == stringVal)
        return false;
    
    if (0 == wcscmp (stringVal, ConfigurationManager::GetString (DGNHANDLERS_STRINGS, DgnHandlersMessage::MSGID_ECTYPEADAPTER_WorkingUnits).c_str()))
        {
        //It's UORs - set it to 0
        outVal.SetInteger ((Int32)StandardUnit::None);
        }
    else
        {
        UnitDefinition unitDef(UnitDefinition::GetStandardUnitByName(stringVal));
        outVal.SetInteger ((Int32)unitDef.IsStandardUnit());
        }

    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeotiffUnitTypeAdapter::_GetStandardValues (StandardValuesCollection& values, IDgnECTypeAdapterContextCR context)
    {
    //Add UORs
    WString WorkingUnits = ConfigurationManager::GetString (DGNHANDLERS_STRINGS, DgnHandlersMessage::MSGID_ECTYPEADAPTER_WorkingUnits);
    values.push_back(WorkingUnits);

    AddAllUnits(values);

    return true;
    }

/////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IDgnECTypeAdapterPtr WorldFileUnitTypeAdapter::Create()        
    {
    return new WorldFileUnitTypeAdapter(); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool WorldFileUnitTypeAdapter::_Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context)
    {
    if (!v.IsNull() && v.IsInteger())
        {
        Int32 unitValue = v.GetInteger();

        UnitDefinition unitDef(UnitDefinition::GetStandardUnit(static_cast<StandardUnit>(unitValue)));

        return unitDef.IsValid();
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool WorldFileUnitTypeAdapter::_ConvertToString (WStringR valueAsString, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter)
    {
    Int32 unitValue = v.GetInteger();

    valueAsString = UnitDefinition::GetStandardName(static_cast<StandardUnit>(unitValue),false);

    return !valueAsString.empty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool WorldFileUnitTypeAdapter::_ConvertFromString (ECN::ECValueR outVal, WCharCP stringVal, IDgnECTypeAdapterContextCR context)
    {
    if (NULL == stringVal)
        return false;
    
    UnitDefinition unitDef(UnitDefinition::GetStandardUnitByName(stringVal));
    outVal.SetInteger ((Int32)unitDef.IsStandardUnit());

    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool WorldFileUnitTypeAdapter::_GetStandardValues (StandardValuesCollection& values, IDgnECTypeAdapterContextCR context)
    {
    AddAllUnits(values);

    return true;
    }


/////////////////////////////////////////////////////
#ifdef DGNV10FORMAT_CHANGES_WIP

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IDgnECTypeAdapterPtr RasterGeopriorityTypeAdapter::Create()        
    {
    return new RasterGeopriorityTypeAdapter(); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterGeopriorityTypeAdapter::_Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context)
    {
    if (!v.IsNull() && v.IsInteger())
        {
        Int32 geopriorityValues = v.GetInteger();

        if (geopriorityValues == (Int32)DgnPlatform::GeoreferencePriority_Attachment ||
            geopriorityValues == (Int32)DgnPlatform::GeoreferencePriority_RasterFile ||
            geopriorityValues == (Int32)DgnPlatform::GeoreferencePriority_SisterFile   )
            return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterGeopriorityTypeAdapter::_ConvertToString (WStringR valueAsString, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter)
    {
    Int32 geopriorityValues = v.GetInteger();

    if (geopriorityValues == (Int32)DgnPlatform::GeoreferencePriority_Attachment)
        {
        valueAsString = ConfigurationManager::GetString (DGNHANDLERS_STRINGS, DgnHandlersMessage::MSGID_ECTYPEADAPTER_GeopriorityAttachment);
        }
    else if (geopriorityValues == (Int32)DgnPlatform::GeoreferencePriority_RasterFile)
        {
        valueAsString = ConfigurationManager::GetString (DGNHANDLERS_STRINGS, DgnHandlersMessage::MSGID_ECTYPEADAPTER_GeopriorityHeader);
        }
    else if (geopriorityValues == (Int32)DgnPlatform::GeoreferencePriority_SisterFile)
        {
        valueAsString = ConfigurationManager::GetString (DGNHANDLERS_STRINGS, DgnHandlersMessage::MSGID_ECTYPEADAPTER_GeoprioritySisterFile);
        }

    return !valueAsString.empty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterGeopriorityTypeAdapter::_ConvertFromString (ECN::ECValueR outVal, WCharCP stringVal, IDgnECTypeAdapterContextCR context)
    {
    return false;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterGeopriorityTypeAdapter::_HasStandardValues() const 
    {
    return false; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterGeopriorityTypeAdapter::_GetStandardValues (StandardValuesCollection& values, IDgnECTypeAdapterContextCR context)
    {
    return false;
    }
#endif
/////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IDgnECTypeAdapterPtr RasterDisplayPriorityPlaneTypeAdapter::Create()        
    {
    return new RasterDisplayPriorityPlaneTypeAdapter(); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterDisplayPriorityPlaneTypeAdapter::_Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context)
    {
    if (!v.IsNull() && v.IsInteger())
        {
        Int32 displayPriorityValue = v.GetInteger();

        if (displayPriorityValue == 0 || //For backward compatibility only
            displayPriorityValue == (Int32)DgnPlatform::DisplayPriority_BackPlane ||
            displayPriorityValue == (Int32)DgnPlatform::DisplayPriority_DesignPlane ||
            displayPriorityValue == (Int32)DgnPlatform::DisplayPriority_FrontPlane   )
            return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterDisplayPriorityPlaneTypeAdapter::_ConvertToString (WStringR valueAsString, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter)
    {
    RasterDisplayPriorityPlane geopriorityValues = (RasterDisplayPriorityPlane)v.GetInteger();

    if (geopriorityValues == DgnPlatform::DisplayPriority_BackPlane)
        {
        valueAsString = ConfigurationManager::GetString (DGNHANDLERS_STRINGS, DgnHandlersMessage::MSGID_ECTYPEADAPTER_DisplayPriority_BackPlane);
        }
    else if (geopriorityValues == DgnPlatform::DisplayPriority_DesignPlane)
        {
        valueAsString = ConfigurationManager::GetString (DGNHANDLERS_STRINGS, DgnHandlersMessage::MSGID_ECTYPEADAPTER_DisplayPriority_DesignPlane);
        }
    else if (geopriorityValues == DgnPlatform::DisplayPriority_FrontPlane)
        {
        valueAsString = ConfigurationManager::GetString (DGNHANDLERS_STRINGS, DgnHandlersMessage::MSGID_ECTYPEADAPTER_DisplayPriority_FrontPlane);
        }

    return !valueAsString.empty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterDisplayPriorityPlaneTypeAdapter::_ConvertFromString (ECN::ECValueR outVal, WCharCP stringVal, IDgnECTypeAdapterContextCR context)
    {
    if (NULL == stringVal)
        return false;

    if (0 == wcscmp (stringVal, ConfigurationManager::GetString (DGNHANDLERS_STRINGS, DgnHandlersMessage::MSGID_ECTYPEADAPTER_DisplayPriority_BackPlane).c_str()))
        {
        outVal.SetInteger ((Int32)DgnPlatform::DisplayPriority_BackPlane);
        return true;
        }
    else if (0 == wcscmp (stringVal, ConfigurationManager::GetString (DGNHANDLERS_STRINGS, DgnHandlersMessage::MSGID_ECTYPEADAPTER_DisplayPriority_DesignPlane).c_str()))
        {
        outVal.SetInteger ((Int32)DgnPlatform::DisplayPriority_DesignPlane);
        return true;
        }
    else if (0 == wcscmp (stringVal, ConfigurationManager::GetString (DGNHANDLERS_STRINGS, DgnHandlersMessage::MSGID_ECTYPEADAPTER_DisplayPriority_FrontPlane).c_str()))
        {
        outVal.SetInteger ((Int32)DgnPlatform::DisplayPriority_FrontPlane);
        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterDisplayPriorityPlaneTypeAdapter::_GetStandardValues (StandardValuesCollection& values, IDgnECTypeAdapterContextCR context)
    {
    WString   displayPriorityPlane = ConfigurationManager::GetString (DGNHANDLERS_STRINGS, DgnHandlersMessage::MSGID_ECTYPEADAPTER_DisplayPriority_BackPlane);
    values.push_back (displayPriorityPlane);

    displayPriorityPlane = ConfigurationManager::GetString (DGNHANDLERS_STRINGS, DgnHandlersMessage::MSGID_ECTYPEADAPTER_DisplayPriority_DesignPlane);
    values.push_back (displayPriorityPlane);

    displayPriorityPlane = ConfigurationManager::GetString (DGNHANDLERS_STRINGS, DgnHandlersMessage::MSGID_ECTYPEADAPTER_DisplayPriority_FrontPlane);
    values.push_back (displayPriorityPlane);

    return true;
    }

/////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IDgnECTypeAdapterPtr RasterDisplayOrderTypeAdapter::Create()        
    {
    return new RasterDisplayOrderTypeAdapter(); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterDisplayOrderTypeAdapter::_Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context)
    {
    if (!v.IsNull() && v.IsInteger())
        {
        //Int32 displayOrderValue = v.GetInteger();

        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterDisplayOrderTypeAdapter::_ConvertToString (WStringR valueAsString, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter)
    {
    Int32 displayOrderValue = v.GetInteger();

    RasterDisplayPriorityPlane geopriorityValues(DgnPlatform::DisplayPriority_DesignPlane);
    if (displayOrderValue>0)
        geopriorityValues = DgnPlatform::DisplayPriority_FrontPlane;
    else if (displayOrderValue<0)
        geopriorityValues = DgnPlatform::DisplayPriority_BackPlane;

    if (geopriorityValues == DgnPlatform::DisplayPriority_BackPlane)
        {
        valueAsString = ConfigurationManager::GetString (DGNHANDLERS_STRINGS, DgnHandlersMessage::MSGID_ECTYPEADAPTER_DisplayPriority_BackPlane);
        }
    else if (geopriorityValues == DgnPlatform::DisplayPriority_DesignPlane)
        {
        valueAsString = ConfigurationManager::GetString (DGNHANDLERS_STRINGS, DgnHandlersMessage::MSGID_ECTYPEADAPTER_DisplayPriority_DesignPlane);
        }
    else if (geopriorityValues == DgnPlatform::DisplayPriority_FrontPlane)
        {
        valueAsString = ConfigurationManager::GetString (DGNHANDLERS_STRINGS, DgnHandlersMessage::MSGID_ECTYPEADAPTER_DisplayPriority_FrontPlane);
        }

    WChar     formatString[256];
    BeStringUtilities::Snwprintf(formatString,_countof(formatString),L"%d [%ls]",displayOrderValue,valueAsString.c_str());

    valueAsString = formatString;

    return !valueAsString.empty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterDisplayOrderTypeAdapter::_ConvertFromString (ECN::ECValueR outVal, WCharCP stringVal, IDgnECTypeAdapterContextCR context)
    {
    if (NULL == stringVal)
        return false;

    int orderValue;
    if (1 == BE_STRING_UTILITIES_SWSCANF (stringVal, L"%ld", &orderValue))
        {
        outVal.SetInteger (orderValue);
        return true;
        }

    return false;
    }

#endif
