#include <Geom\GeomApi.h>
#include <Bentley\BeFileName.h>
#include <Bentley\BeFile.h>
#include <GeomSerialization\GeomSerializationApi.h>

bool getGeometry (Utf8String &filename, bvector<IGeometryPtr> &geometry)
    {
    geometry.clear ();
    BeFileName path (filename);
    BeFile file;
    file.Create (path.c_str (), false);
    if (!(BeFileName::DoesPathExist (path.c_str ())))
        {
        printf ("  file not found (%ls)\n", path.c_str ());
        return false;
        }

    ByteStream entireFile;
    if (BeFileStatus::Success != file.Open (path.c_str (), BeFileAccess::Read))
        {
        printf ("file.Open failed (%ls)", path.c_str ());
        return false;
        }

    if (BeFileStatus::Success != file.ReadEntireFile (entireFile))
        {
        printf ("file.ReadEntireFile failed (%ls)", path.c_str ());
        return false;
        }
    Utf8String str ((Utf8P)entireFile.GetDataP ());
    return BentleyGeometryJson::TryJsonStringToGeometry (str, geometry);

    }

bool setOptions (int argc, char **argv,
int &verbose,   // numeric value from -v0, -v1, -v2.  Assumed initialized by caller.
bvector<Utf8String> &filenames
)
    {
    filenames.clear ();
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
                printf ("Unrecognized arg (%s)\n", argv[i]);
                errors++;
                }
            }
        else
            {
            filenames.push_back (Utf8String (s));
            }
        }

    if (verbose > 0 || errors > 0)
        {
        printf ("The command line has %d filenames.\n", (int)filenames.size ());
            for (size_t i = 0; i < filenames.size (); i++)
            printf("  input file %d: (%s)\n", (int)i, filenames[i].c_str ());
        }
    return errors == 0;
    }
extern "C"
int main(int argc, char **argv) 
    {
    bvector<Utf8String> filenames;
    int verbose = 0;
    if (!setOptions (argc, argv, verbose, filenames) || filenames.size () != 2)
        {
        printf ("exe name:   %s\n", argv[0]);
        printf ("Usage:  comparedgnjs <fileA> <fileB>\n");
        printf("   Read geometry from each file.  Compare with IGeometry::IsAlmostEqual.\n");
        return 1;
        }
    bvector<IGeometryPtr> geometryA, geometryB;
    bool okA = getGeometry (filenames[0], geometryA);
    bool okB = getGeometry (filenames[1], geometryB);
    if (!okA || !okB)
        return 1;

    if (geometryA.size() != geometryB.size()) 
        {
        printf("Mismatched geometry counts %d != %d\n", (int)geometryA.size (), (int)geometryB.size ());
        return 1;
        }

    for (size_t i = 0; i < geometryA.size(); i++)
        {
        if (!geometryA[i]->IsSameStructureAndGeometry(*geometryB[i]))
            {
            printf("Mismatched geometry at index %d\n", (int)i);
            return 1;
            }
        }
    printf("(%s matches %s)\n", filenames[0].c_str (), filenames[1].c_str());
    return 0;
    }