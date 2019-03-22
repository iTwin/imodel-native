/*--------------------------------------------------------------------------------------+
|
|     $Source: BaseGeoCoord/GeoCoordExceptions.cs $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

namespace Bentley.GeoCoordinatesNET
{
using   System;
using   SRSC  = System.Resources;
using   SREF  = System.Reflection;

/*====================================================================================**/
/// <summary>Class for GeoCoordinate localization.</summary>
/// <author>Barry.Bentley</author>                              <date>06/2004</date>
/*==============+===============+===============+===============+===============+======*/
public class    GeoCoordinateLocalization
{
private static  SRSC.ResourceManager    s_stringResourceManager;

private static  SRSC.ResourceManager    StringResourceManager
    {
    get
        {
        if (null == s_stringResourceManager)
            {
            SREF.Assembly   containingAssembly = SREF.Assembly.GetExecutingAssembly();
            s_stringResourceManager = new SRSC.ResourceManager ("LocalizableStrings", containingAssembly);
            }
        return s_stringResourceManager;
        }
    }

/*------------------------------------------------------------------------------------**/
/// <summary>Get Localized String</summary>
/// <author>Barry.Bentley</author>                              <date>11/2006/</date>
/*--------------+---------------+---------------+---------------+---------------+------*/
private static string           GetLocalizedString
(
SRSC.ResourceManager    resourceManager,
string                  key
)
    {
    string      value;
    return (null != (value = resourceManager.GetString (key))) ? value : String.Format ("XX-{0}-XX", key);
    }

/*------------------------------------------------------------------------------------**/
/// <summary>Get Localized String</summary>
/// <author>Barry.Bentley</author>                              <date>11/2006/</date>
/*--------------+---------------+---------------+---------------+---------------+------*/
public static string           GetLocalizedString
(
string          key
)
    {
    return GetLocalizedString (StringResourceManager, key);
    }

/*------------------------------------------------------------------------------------**/
/// <summary>Get Localized String</summary>
/// <author>Barry.Bentley</author>                              <date>11/2006/</date>
/*--------------+---------------+---------------+---------------+---------------+------*/
public static string           GetLocalizedStringNoSubst
(
string          key
)
    {
    return StringResourceManager.GetString (key);
    }

}


/*====================================================================================**/
/// <summary>Base class for GeoCoordinate-specific exceptions.</summary>
/// <author>Barry.Bentley</author>                              <date>06/2004</date>
/*==============+===============+===============+===============+===============+======*/
public class    GeoCoordinateException : System.ApplicationException
{
/*------------------------------------------------------------------------------------**/
/// <summary>Initialize a new instance of the GeoCoordinateException class.</summary>
/// <param name="message">The localized message that explains the exception.</param>
/// <author>Barry.Bentley</author>                               <date>03/2007</date>
/*--------------+---------------+---------------+---------------+---------------+------*/
public GeoCoordinateException
(
string message
) : base (message)
    {
    }

/*====================================================================================**/
/// <summary>The exception that is thrown when attempting to construct a BaseGeoCoordinateSystem
///  from a keyname when the keyname is not in the coordinate system library.
/// <author>Barry.Bentley</author>                               <date>03/2007</date>
/*==============+===============+===============+===============+===============+======*/
public class    ConstructorFailure : GeoCoordinateException
{
private int     m_csErrorNum;

/*------------------------------------------------------------------------------------**/
/// <summary>Initializes a new instance of the CantConvert class.</summary>
/// <author>John.Gooding</author>                               <date>03/2005</date>
/*--------------+---------------+---------------+---------------+---------------+------*/
public          ConstructorFailure
(
int         errorNum,
string      keyName,
string      csErrorMessage
) : base (String.Format (GeoCoordinateLocalization.GetLocalizedString ("ConstructorFailure"), keyName, csErrorMessage))
    {
    m_csErrorNum = errorNum;
    }

} // ConstructorFailure

}


} // Bentley.GeoCoordinates
