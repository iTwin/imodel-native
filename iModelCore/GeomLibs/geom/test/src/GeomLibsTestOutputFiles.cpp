#include <Geom/GeomApi.h>
#include <Geom/CGWriter.h>
#include <vector>
static size_t sFileCount = 0;

struct StringCounter
{
std::string name; size_t count;
StringCounter (std::string s)
    {
    name = s;
    count = 0;
    }
};

struct StringCountArray : std::vector<StringCounter>
{
void MakeUniqueString (std::string &baseName, std::string &uniqueName)
    {
    uniqueName = baseName;  // New string, use it unchanged.
    for (size_t i = 0; i < size (); i++)
        {
        if (at(i).name == baseName)
            {
            char countString[20];
            sprintf (countString, "%" PRIu64, (uint64_t)++at(i).count);
            uniqueName.append (std::string(countString));
            return;
            }
        }
    push_back (StringCounter (baseName));
    }
};

StringCountArray sFileNames;

FILE *OpenGeomTestOutputFile (char const*cname, char const*extension, char const*access)
    {
    char *geomTestOutputDir =getenv ("GeomTestOutputDir");
    std::string fullName;
    if (NULL == geomTestOutputDir)
        {
        fullName.append (std::string (getenv ("tmp")));
        }
    else
        {
        fullName.append (geomTestOutputDir);
        if (fullName[fullName.size () - 1] != '\\')
            fullName.append (std::string ("\\"));
        }
    std::string uniqueName;
    std::string name (cname);
    sFileNames.MakeUniqueString (name, uniqueName);
    fullName.append (uniqueName);
    if (extension != NULL)
        fullName.append (std::string (extension));
    sFileCount++;
    FILE *file = fopen (fullName.c_str (), access);
    if (file != NULL)
        printf ("<OutputFile>%s</OutputFile>\n", fullName.c_str ());
    else
        printf ("<OutputFileUnableToOpen>%s</OutputFileUnableToOpen>\n", fullName.c_str ());
    return file;
    }


void PrintCurve (MSBsplineCurveCR curve, char *title, FILE *file0)
    {
    if (NULL != title)
        {
        printf ("<Name>%s</Name>\n", title);
        if (NULL == file0)
            {
            FILE *file = OpenGeomTestOutputFile (title, ".xml", "w");
            if (file != NULL)
                {
                CGWriter writer (file);
                writer.EmitCurve (curve);
                fclose (file);
                }
            }
        else
            {
            CGWriter writer (file0);
            writer.EmitCurve (curve);
            }
        }
    }

void PrintCurveVector (CurveVectorCR curve, char *title, FILE *file0)
    {
    if (NULL != title)
        {
        printf ("<Name>%s</Name>\n", title);
        if (NULL == file0)
            {
            FILE *file = OpenGeomTestOutputFile (title, ".xml", "w");
            if (file != NULL)
                {
                CGWriter writer (file);
                writer.EmitCurveVector (curve);
                fclose (file);
                }
            }
        else
            {
            CGWriter writer (file0);
            writer.EmitCurveVector (curve);
            }
        }
    }
