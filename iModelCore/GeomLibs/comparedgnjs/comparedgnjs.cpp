/*--------------------------------------------------------------------------------------+
|
|     $Source: comparedgnjs/comparedgnjs.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Geom\GeomApi.h>
#include <Bentley\BeFileName.h>
#include <Bentley\BeFile.h>
#include <GeomSerialization\GeomSerializationApi.h>
#include <BeJsonCpp/BeJsonUtilities.h>

static const char * s_messagePrefix = "comparedgnjs";
void messagePrefix ()
    {
    printf ("comparedgnjs: ");
    }
struct JsonData {
Utf8String m_filename;
bvector<IGeometryPtr> m_geometry;
Json::Value m_value;

JsonData (Utf8String filename)
    : m_filename (filename)
    {
    }
bool Load (int verbose = 0)
    {
    m_geometry.clear ();
    BeFileName path (m_filename);
    BeFile file;
    file.Create (path.c_str (), false);
    if (!(BeFileName::DoesPathExist (path.c_str ())))
        {
        messagePrefix (); printf ("  file not found (%ls)\n", path.c_str ());
        return false;
        }

    ByteStream entireFile;
    if (BeFileStatus::Success != file.Open (path.c_str (), BeFileAccess::Read))
        {
        messagePrefix (); printf ("file.Open failed (%ls)", path.c_str ());
        return false;
        }

    if (BeFileStatus::Success != file.ReadEntireFile (entireFile))
        {
        messagePrefix (); printf ("file.ReadEntireFile failed (%ls)", path.c_str ());
        return false;
        }
    Utf8String str ((Utf8P)entireFile.GetDataP ());
    if (verbose > 9)
        {
        messagePrefix (); printf ("file %s\n%s", m_filename.c_str (), str.c_str());
        }
    Json::Reader::Parse (str, m_value, false);
    return BentleyGeometryJson::TryJsonValueToGeometry (m_value, m_geometry);
    }
};
//
// Inspect the command line arguments.
// <ul>
// <li> -v0, -v1 etc are verbose settings.
//   <ul>
//   <li>  0: no output unless errors.
//   <li>  1: filenames at end
//   <li>  2: filenames beginning and end.
//   <li> 10: output file contents.  This is a packed json string.
//   </ul>
// <li>Anything not beginning with a dash is loaded as the filename in a JsonData structure.
// </ul>
bool parseCommandLine (int argc, char **argv,
int &verbose,   // numeric value from -v0, -v1, -v2.  Assumed initialized by caller.
bvector<JsonData> &data
)
    {
    data.clear ();
    int errors = 0;
    if (argc < 3)
        errors++;
    for (int i = 1; i < argc; i++)
        {
        auto s = argv[i];
        if (s[0] == '-')
            {
            int value;
            if (1 == sscanf (s, "-v%d", &value))
                {
                verbose = value;
                }
            else
                {
                messagePrefix (); printf ("Unrecognized arg (%s)\n", argv[i]);
                errors++;
                }
            }
        else
            {
            data.push_back (JsonData (Utf8String (s)));
            }
        }

    if (verbose > 1 || errors > 0)
        {
        messagePrefix (); printf ("The command line has %d filenames.\n", (int)data.size ());
        for (size_t i = 0; i < data.size (); i++)
            {
            messagePrefix (); printf("  input file %d: (%s)\n", (int)i, data[i].m_filename.c_str ());
            }
        }
    return errors == 0;
    }
extern "C"
int main(int argc, char **argv) 
    {
    bvector<JsonData> allData;
    int verbose = 1;    // enables final filename echos
    if (!parseCommandLine (argc, argv, verbose, allData) || allData.size () != 2)
        {
        messagePrefix (); printf ("exe name:   %s\n", argv[0]);
        messagePrefix (); printf ("Usage:  comparedgnjs [-vNN] <fileA> <fileB>\n");
        messagePrefix (); printf ("   -v0         no output except errors.\n");
        messagePrefix (); printf ("   -v1         final confirmation of matched filenames.\n");
        messagePrefix (); printf ("   -v2         beginning and end confirmation of filenames, Top level geometry counts.\n");
        messagePrefix (); printf ("   -v11         all of above plus echo complete file contents (packed strings)\n");
        messagePrefix (); printf("   Read geometry from each file.  Compare with IGeometry::IsAlmostEqual.\n");
        return 1;
        }

    // load json and geometry objects from each file.
    int errors = 0;
    for (size_t i = 0; i < allData.size (); i++)
        {
        if (!allData[i].Load (verbose))
            {
            messagePrefix (); printf ("Unable to read or parse json from (%s)\n", allData[i].m_filename.c_str ());
            errors++;
            }
        }
    if (errors > 0)
        return 1;

    // compare geometry
    size_t n0 = allData[0].m_geometry.size ();
    size_t n1 = allData[1].m_geometry.size ();
    if (verbose > 1)
        {
        messagePrefix (); printf ("Top level geometry arrays sizes: (%d) and (%d)\n", (int)n0, (int)n1);
        }
    if (n0 != n1)
        {
        messagePrefix (); printf("FAIL: Mismatched geometry counts %d != %d\n", (int)n0, (int)n1);
        return 1;
        }

    for (size_t i = 0; i < n0; i++)
        {
        int numNull = 0;
        for (size_t k = 0; k < 2; k++)
            {
            if (!allData[k].m_geometry[i].IsValid ())
                {
                messagePrefix (); printf ("geometry[%d] in file %d is null\n", (int)i, (int)k);
                numNull++;
                }
            }
        // single null is error ..
        if (numNull == 1)
            return 1;
        // 2 nulls is implied ok
        // 0 nulls needs comparison
        if (numNull == 0 &&!allData[0].m_geometry[i]->IsSameStructureAndGeometry(*allData[1].m_geometry[i]))
            {
            messagePrefix (); printf("FAIL: Mismatched geometry at index %d\n", (int)i);
            return 1;
            }
        }
    if (verbose > 0)
        messagePrefix (); printf("SUCCESS: geometry comparison: (%s) isAlmostEqual (%s)\n", allData[0].m_filename.c_str (), allData[1].m_filename.c_str());
    return 0;
    }