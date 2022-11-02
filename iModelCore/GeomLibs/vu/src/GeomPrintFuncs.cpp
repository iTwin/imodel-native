/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


static GeomPrintFuncsP s_defaultPrintFuncs = NULL;

void GeomPrintFuncs::SetDefault (GeomPrintFuncs *funcs)
    {
    s_defaultPrintFuncs = funcs;
    }

GeomPrintFuncsP GeomPrintFuncs::GetDefault ()
    {
    if (s_defaultPrintFuncs != NULL)
        return s_defaultPrintFuncs;
    s_defaultPrintFuncs = new  GeomPrintToBeConsole ();
    return s_defaultPrintFuncs;
    }

// Default implementations of the lowest level EmitChar does nothing -- derived class must override and output to actual device.
void GeomPrintFuncs::EmitChar (char ch)
    {
    }
        // Default implementations reduce to EmitChar ...
void GeomPrintFuncs::EmitString (char const *pString)
    {
    for (int i = 0; pString[i] != 0; i++)
        EmitChar (pString[i]);
    }

void GeomPrintFuncs::EmitInt (int value)
    {
    char buffer[100];
    sprintf (buffer, "%i", value);
    EmitString (buffer);
    }

void GeomPrintFuncs::EmitDouble (double value)
    {
    char buffer[100];
    sprintf (buffer, "%g", value);
    EmitString (buffer);
    }

void GeomPrintFuncs::EmitHex (int value)
    {
    char buffer[100];
    sprintf (buffer, "%x", value);
    EmitString (buffer);
    }

void GeomPrintFuncs::EmitLineBreak ()
    {
    EmitChar ('\n');
    }

void GeomPrintFuncs::EmitAttributeDouble (const char *name, double value)
    {
    EmitString (" ");
    EmitString (name);
    EmitString ("=\"");
    EmitDouble (value);
    EmitString ("\"");
    }
        // Default implementations produce xml tag with type as tag, members as attributes, optional description as content.
void GeomPrintFuncs::EmitTag (DPoint4dCR value, const char *description)
    {
    EmitString ("<DPoint4d");
    EmitAttributeDouble ("x", value.x);
    EmitAttributeDouble ("y", value.y);
    EmitAttributeDouble ("z", value.x);
    EmitAttributeDouble ("e", value.w);
    if (description != NULL)
        EmitString ("/>");
    else
        {
        EmitString (">");
        EmitString (description);
        EmitString("</DPoint4d>");
        }
    }

void GeomPrintFuncs::EmitTag (DPoint3dCR value, const char *description)
    {
    EmitString ("<DPoint3d");
    EmitAttributeDouble ("x", value.x);
    EmitAttributeDouble ("y", value.y);
    EmitAttributeDouble ("z", value.x);
    if (description != NULL)
        EmitString ("/>");
    else
        {
        EmitString (">");
        EmitString (description);
        EmitString("</DPoint3d>");
        }
    }

void GeomPrintFuncs::EmitTag (DPoint2dCR value, const char *description)
    {
    EmitString ("<DPoint2d");
    EmitAttributeDouble ("x", value.x);
    EmitAttributeDouble ("y", value.y);
    if (description != NULL)
        EmitString ("/>");
    else
        {
        EmitString (">");
        EmitString (description);
        EmitString("</DPoint2d>");
        }
    }

void GeomPrintFuncs::EmitTag (DVec3dCR value, const char *description)
    {
    EmitString ("<DVec3d");
    EmitAttributeDouble ("x", value.x);
    EmitAttributeDouble ("y", value.y);
    EmitAttributeDouble ("z", value.x);
    if (description != NULL)
        EmitString ("/>");
    else
        {
        EmitString (">");
        EmitString (description);
        EmitString("</DVec3d>");
        }
    }

void GeomPrintFuncs::EmitTag (DVec2dCR value, const char *description)
    {
    EmitString ("<DVec2d");
    EmitAttributeDouble ("x", value.x);
    EmitAttributeDouble ("y", value.y);
    if (description != NULL)
        EmitString ("/>");
    else
        {
        EmitString (">");
        EmitString (description);
        EmitString("</DVec2d>");
        }
    }

void GeomPrintFuncs::EmitTag (GraphicsPointCR value, const char *description)
    {
    }

void GeomPrintFuncs::EmitTag (DEllipse3dCR value, const char *description)
    {
    }

void GeomPrintFuncs::EmitTag (RotMatrixCR value, const char *description)
    {
    }

void GeomPrintFuncs::EmitTag (TransformCR value, const char *description)
    {
    }

void GeomPrintFuncs::EmitTagRadians (double radians, const char *description)
    {
    }

void GeomPrintFuncs::EmitTagDegrees (double degrees, const char *description)
    {
    }

void GeomPrintFuncs::EmitTagDouble (double value, const char *description)
    {
    }

void GeomPrintFuncs::EmitTagInt32 (int32_t value, const char *description)
    {
    }

void GeomPrintFuncs::EmitTagInt64 (int64_t value, const char *description)
    {
    }

void GeomPrintFuncs::EmitTagString (const char *value, const char *description)
    {
    }

void GeomPrintFuncs::EmitTagPointer (void const *pointer, const char *description)
    {
    }


static bvector<WString> s_enabledSources;
// Enable/disable output to default printfuncs by specified source
void GeomPrintFuncs::EnableDefault (wchar_t const*name, bool enable)
    {
    for (size_t i = 0, n = s_enabledSources.size (); i < n; i++)
        {
        if (s_enabledSources[i] == name)
            {if (!enable)
                s_enabledSources.erase (s_enabledSources.begin () + i);
            return;
            }
        }
    // Not there.   Add it.
    if (enable)
        s_enabledSources.push_back (WString (name));
    }

// Get the default printfuncs for specified source.
GeomPrintFuncsP GeomPrintFuncs::GetDefault (wchar_t const*name)
    {
    for (size_t i = 0, n = s_enabledSources.size (); i < n; i++)
        {
        if (s_enabledSources[i] == name)
            return GetDefault ();
        }
    return NULL;
    }

void GeomPrintToBeConsole::EmitChar (char ch)
    {
#ifdef GEOMAPI_PRINTF
    GEOMAPI_PRINTF ("%c", ch);
#else
    BeConsole::Printf ("%c", ch);
#endif
    }
void GeomPrintToBeConsole::EmitString (char const *pString)
    {
#ifdef GEOMAPI_PRINTF
    GEOMAPI_PRINTF  ("%s", pString);
#else
    BeConsole::Printf ("%s", pString);
#endif
    }


END_BENTLEY_GEOMETRY_NAMESPACE
