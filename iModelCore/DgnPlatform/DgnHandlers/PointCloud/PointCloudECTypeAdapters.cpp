/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/PointCloud/PointCloudECTypeAdapters.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnHandlers/PointCloudECTypeAdapters.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eric.Paquet                     05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IDgnECTypeAdapterPtr PointCloudViewFlagsAdapter::Create()        
    {
    return new PointCloudViewFlagsAdapter(); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eric.Paquet                     05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool PointCloudViewFlagsAdapter::_Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context)
    {
    if (!v.IsNull())
        {
        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eric.Paquet                     05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool PointCloudViewFlagsAdapter::_ConvertToString (WStringR valueAsString, ECN::ECValueCR inputVal, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter)
    {
    if (!inputVal.IsNull() && inputVal.IsArray())
        {
        ElementHandle eh(context.GetElementRef(),context.GetDgnModel());
        IPointCloudQuery* pQuery = dynamic_cast<IPointCloudQuery*>(&eh.GetHandler());
        if(NULL == pQuery)
            return false; //It is NOT a RasterAttachment!

        PointCloudPropertiesPtr propsP = pQuery->GetPointCloudProperties(eh);
        UInt32 count = inputVal.GetArrayInfo().GetCount();
        for (UInt32 view = 0; view < count; view++)
            {
            if (propsP->GetViewState(view))
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
* @bsimethod                                    Eric.Paquet                     05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool PointCloudViewFlagsAdapter::_ConvertFromString (ECN::ECValueR outVal, WCharCP stringVal, IDgnECTypeAdapterContextCR context)
    {
    // Nothing needs to be done here, since the field is read-only (it is not possible to directly enter view values in the field;
    // one has to use the TypeEditor).
    return false;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eric.Paquet                     05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IDgnECTypeAdapterPtr PointCloudDensityAdapter::Create()        
    {
    return new PointCloudDensityAdapter(); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eric.Paquet                     05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool PointCloudDensityAdapter::_Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context)
    {
    if (!v.IsNull())
        {
        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eric.Paquet                     05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool PointCloudDensityAdapter::_ConvertToString (WStringR valueAsString, ECN::ECValueCR inputVal, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter)
    {
    if (!inputVal.IsNull() && inputVal.IsDouble())
        {
        WChar     formatString[128];
        long density = DataConvert::RoundDoubleToLong (inputVal.GetDouble() * 100);

        BeStringUtilities::Snwprintf(formatString,_countof(formatString),L"%ld", density);
        valueAsString = WString (formatString);
        }

    return !valueAsString.empty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eric.Paquet                     05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool PointCloudDensityAdapter::_ConvertFromString (ECN::ECValueR outVal, WCharCP stringVal, IDgnECTypeAdapterContextCR context)
    {
    if (NULL == stringVal)
        return false;

    Int32 i;
    if (1 == BE_STRING_UTILITIES_SWSCANF (stringVal, L"%d", &i))
        {
        outVal.SetDouble ((double)i/100.0);
        return true;
        }
    
    return false;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eric.Paquet                     05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IDgnECTypeAdapterPtr PointCloudNbPointsAdapter::Create()        
    {
    return new PointCloudNbPointsAdapter(); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eric.Paquet                     05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool PointCloudNbPointsAdapter::_Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context)
    {
    if (!v.IsNull())
        {
        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eric.Paquet                     05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool PointCloudNbPointsAdapter::_ConvertToString (WStringR valueAsString, ECN::ECValueCR inputVal, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter)
    {
    if (!inputVal.IsNull() && inputVal.IsLong())
        {
        WChar     formatString[256];

        UInt64 nbPoints = inputVal.GetLong();
        FormatThousandsSeparator (formatString, nbPoints);
        valueAsString = WString (formatString);
        }

    return !valueAsString.empty();
    }

/*---------------------------------------------------------------------------------**//**
* Format a number with thousands separator (,). 
* Note: it would be simpler if Snwprintf had such formatting capability, but I could not find one...
* @bsimethod                                                    Eric.Paquet     05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudNbPointsAdapter::FormatThousandsSeparator(WCharP pOutValue, UInt64 inValue) const
    {
    WChar  tmpStr[256];
    WChar  outValueStr[256];

    UInt64 remainder = inValue % 1000;
    UInt64 divResult = inValue / 1000;
    BeStringUtilities::Snwprintf (outValueStr, L"%ls", L"");
    do 
        {
        if (divResult > 0)
            BeStringUtilities::Snwprintf (tmpStr, L"%03d%ls", remainder, outValueStr);
        else 
            {
            // Number of points < 1000
            BeStringUtilities::Snwprintf (tmpStr, L"%d%ls", remainder, outValueStr);
            }
        BeStringUtilities::Snwprintf (outValueStr, L"%ls", tmpStr); 
        remainder = divResult % 1000;
        divResult = divResult / 1000;
        if (divResult > 0)
            {
            BeStringUtilities::Snwprintf (tmpStr, L",%ls", outValueStr);
            BeStringUtilities::Snwprintf (outValueStr, L"%ls", tmpStr); 
            }
        else
            {
            if (remainder > 0)
                {
                BeStringUtilities::Snwprintf (tmpStr, L"%d,%ls", remainder, outValueStr);
                BeStringUtilities::Snwprintf (outValueStr, L"%ls", tmpStr); 
                }
            }
        } while (divResult > 0);

    BeStringUtilities::Wcsncpy (pOutValue, 256, outValueStr);
    }