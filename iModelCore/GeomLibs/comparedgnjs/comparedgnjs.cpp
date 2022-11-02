/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <map>
#include <Geom\GeomApi.h>
#include <Bentley\BeFileName.h>
#include <Bentley\BeFile.h>
#include <GeomSerialization\GeomSerializationApi.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include <Bentley\BeDirectoryIterator.h>
#include <json/writer.h>
#include "compareJson.h"
#include "compareGeometry.h"

static const char * s_messagePrefix = "comparedgnjs";
static double compareTol = 1.0e-12;
static int s_echoErrorGeometry = 0;
void messagePrefix (const char* content = nullptr)
    {
    printf ("comparedgnjs: ");
	if (content)
		printf(content);
    }
struct JsonData {
Utf8String m_filename;
bvector<IGeometryPtr> m_geometry;
Json::Value m_value;

JsonData (Utf8String filename)
    : m_filename (filename)
    {
    }
bool Load(bool &canUseGeometry, int &type, int verbose = 0)
{
	m_geometry.clear();
	BeFileName path(m_filename);
	BeFile file;
	// file.Create(path.c_str(), false);
	if (!(BeFileName::DoesPathExist(path.c_str())))
	{
		messagePrefix(); printf("file not found (%ls)\n", path.c_str());
		return false;
	}

	ByteStream entireFile;
	if (BeFileStatus::Success != file.Open(path.c_str(), BeFileAccess::Read))
	{
		messagePrefix(); printf("file.Open failed (%ls)\n", path.c_str());
		return false;
	}

	if (BeFileStatus::Success != file.ReadEntireFile(entireFile))
	{
		messagePrefix(); printf("file.ReadEntireFile failed (%ls)\n", path.c_str());
		return false;
	}
	Utf8String str((Utf8P)entireFile.GetDataP());
	if (verbose > 9)
	{
		messagePrefix(); printf("file %s\n%s", m_filename.c_str(), str.c_str());
	}
	Json::Reader::Parse(str, m_value, false);
	if (type == 1)
		if (!BentleyGeometryJson::TryJsonValueToGeometry(m_value, m_geometry))
		{
			type = 2;
			canUseGeometry = false;
		}
	return true;
}
};
struct TypeCounts {
int numbers;
int arrays;
int objects;
int strings;
int booleans;
int nulls;
};

struct map_cmp {
	bool operator()(Utf8String const a, Utf8String const b) const
	{
		return a.CompareToI(b) < 0;
	}
};

// Type-specific compare methods and handler prototypes
bool compareItems(const double a, const double b);
bool compareItems(const Utf8String, const Utf8String);
bool compareHandler(Json::Value const &a, Json::Value const &b, struct TypeCounts &typeCounts, std::map<Utf8String, int, map_cmp> &propertyCounts, bvector<Utf8String> &errorTracker, int &dif);
bool compareObjects(Json::Value const &a, Json::Value const &b, struct TypeCounts &typeCounts, std::map<Utf8String, int, map_cmp> &propertyCounts, bvector<Utf8String> &errorTracker, int &dif);
bool compareArrays(Json::Value const &a, Json::Value const &b, struct TypeCounts &typeCounts, std::map<Utf8String, int, map_cmp> &propertyCounts, bvector<Utf8String> &errorTracker, int &dif);

bool findProperty(Json::Value const &source, CharCP targetName, Json::Value &value)
{
	for (Json::Value::iterator iter = source.begin(); iter != source.end(); iter++)
	{
		Utf8CP childName = iter.memberName();
		if (0 == BeStringUtilities::Stricmp(targetName, childName))
		{
			value = *iter;
			return true;
		}
	}
	return false;
}

//
// Inspect the command line arguments.
// <ul>
// <li> -v0, -v1 etc are verbose settings.
//   <ul>
//   <li>  0: no output unless errors.
//   <li>  1: filenames at end
//   <li>  2: filenames beginning and end.
//	 <li>  3: verbose json compare (only has an effect if used together with -js)
//   <li> 10: output file contents.  This is a packed json string.
//   </ul>
// <li> -n<n> is a setting that allows a certain number of numeric differences in json compare.
// <li> -t<n> is a setting that changes the tolerance for numeric compares
// <li> -j forces JSON string compare
// <li> -g forces geometric compare
// <li> -e echo error geometry
// <li>Anything not beginning with a dash is loaded as the filename in a JsonData structure.
// <li> If given two files, compares them normally. If given two directories, compares the directories. If given a file and directory, check if file exists in directory.
// </ul>
bool parseCommandLine (int argc, char **argv,
int &verbose,   // numeric value from -v0, -v1, -v2.  Assumed initialized by caller.
int &userType,		// numeric value assigned from -js. Assumed to be 1 by caller (compare geometries), otherwise, compares json
int &dif,
int &echoError, // 1==> echo error (mismatch)
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
			double dValue;
            if (1 == Utf8String::Sscanf_safe (s, "-v%d", &value))
                {
                verbose = value;
                }
			else if (1 == Utf8String::Sscanf_safe (s, "-n%d", &value))
				{
				dif = value;
				}
			else if (1 == Utf8String::Sscanf_safe(s, "-t%lf", &dValue))
				{
				compareTol = dValue;
				}
			else if (s[1] == 'j' && s[2] == '\0')
				{
				userType = 2;
				}
			else if (s[1] == 'g' && s[2] == '\0')
				{
				userType = 1;
				}
            else if (s[1] == 'e' && s[2] == '\0')
                {
                echoError = 1;
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

    if (verbose == 2 || errors > 0)
        {
        messagePrefix (); printf ("The command line has %d filenames.\n", (int)data.size ());
        for (size_t i = 0; i < data.size (); i++)
            {
            messagePrefix (); printf("  input file %d: (%s)\n", (int)i, data[i].m_filename.c_str ());
            }
        }
    return errors == 0;
    }



bool compareGeometry(bvector<JsonData> allData, int verbose)
	{
    EvolvingComparison stats;
	size_t n0 = allData[0].m_geometry.size();
	size_t n1 = allData[1].m_geometry.size();
	if (verbose > 1)
	{
		messagePrefix(); printf("Top level geometry arrays sizes: (%d) and (%d)\n", (int)n0, (int)n1);
	}
	if (n0 != n1)
	{
		messagePrefix(); printf("     GEOMETRY COMPARE FAIL: Mismatched geometry counts %d != %d\n", (int)n0, (int)n1);
		return 1;
	}

	for (size_t i = 0; i < n0; i++)
	    {
		int numNull = 0;
		for (size_t k = 0; k < 2; k++)
		    {
			    if (!allData[k].m_geometry[i].IsValid())
			    {
				    messagePrefix(); printf("geometry[%d] in file %d is null\n", (int)i, (int)k);
				    numNull++;
			    }
		    }
		// single null is error ..
		if (numNull == 1)
			return false;
		// 2 nulls is implied ok
		// 0 nulls needs comparison
		if (numNull == 0)
		    {
            if (allData[0].m_geometry[i]->IsSameStructureAndGeometry (*allData[1].m_geometry[i], compareTol))
                {
                if (!stats.isCompatibleEqual ())
                    return false;
                }
            else if (secondaryCompare (allData[0].m_geometry[i], allData[1].m_geometry[i], stats, compareTol))
                {
                // compatible translations so far . . ..
                }
            else
                {
			    messagePrefix(); printf("     GEOMETRY COMPARE FAIL: Mismatched geometry at index %d\n", (int)i);
                if (s_echoErrorGeometry)
                    {
                    Utf8String s0, s1;
                    if (BentleyGeometryJson::TryGeometryToJsonString (s0, *allData[0].m_geometry[i], true))
                        BentleyGeometryJson::DumpJson (s0);
                    if (BentleyGeometryJson::TryGeometryToJsonString (s1, *allData[1].m_geometry[i], true))
                        BentleyGeometryJson::DumpJson (s1);
                    }
                return false;
                }
		    }
	    }

    // Fall through if all equal or all translated ...
    if (stats.numMatchedMoments > 0)
        {
        messagePrefix ();
        printf ("     GEOMETRY COMPARE WARNING: %d geoemtry objects have some difference but matched moments\n",
                (int)stats.numMatchedMoments);
        }
    if (stats.numMatchingFirstTranslate > 0)
        {
        auto vector = stats.firstTranslate.Value ();
        messagePrefix (); printf ("     GEOMETRY COMPARE SUCCESS: Files are equal except for translation (%g,%g,%g)\n",
                vector.x, vector.y, vector.z
                );
        }
    else
        {
    	messagePrefix(); printf("     GEOMETRY COMPARE SUCCESS: Files are equal!\n");
        }
	return true;
	}
// Function that iterates through an ARRAY and appends to errorTracker
void appendArrayToErrorTracker(Json::Value const &item, bvector<Utf8String> &errorTracker) {
	int n = item.size();
	int counter = 1;
	Utf8String lastName;
	for (int i = 0; i < n; i++)
	{
		if (item[i].isObject())
		{
			unsigned int index = 0;
			for (Json::Value::iterator iter = item[i].begin(); iter != item[i].end(); iter++)
			{
				Utf8String currName = iter.memberName();
				if (strcmp(currName.c_str(), lastName.c_str()) == 0)	// Compare to last property name found
				{
					counter++;

					if (!(i == n - 1 && index < item[i].size()))	// If very last item of array and inner object, must continue on to printing step
						continue;
				}

				// Add property
				if (counter > 1)	// Print with number value
				{
					char toInsert[100];
					snprintf(toInsert, sizeof(toInsert), "%s(x%d)", lastName.c_str(), counter);
					errorTracker.insert(errorTracker.begin(), toInsert);
				}
				else
				{
					if (!(i == 0 && iter == item[i].begin()))	// If lastName has not yet been set, force insertion
						errorTracker.insert(errorTracker.begin(), lastName);
					if (i == n - 1 && index == item[i].size())	// If very last item for entire array, force insertion
						errorTracker.insert(errorTracker.begin(), currName);
				}
				counter = 1;
				index++;
				lastName = iter.memberName();
			}
		}
	}
}
// Function that iterates through an OBJECT and appends to errorTracker
void appendPropertiesToErrorTracker(Json::Value const &item, bvector<Utf8String> &errorTracker) {
	for (Json::Value::iterator iter = item.begin(); iter != item.end(); iter++)
	{
		errorTracker.insert(errorTracker.begin(), iter.memberName());
	}
}
bool compareItems(double a, double b, int &dif)
{
	double absA = (a < 0) ? a * -1 : a;
	double absB = (b < 0) ? b * -1 : b;
	double bMinusA = b - a;
	bMinusA = (bMinusA < 0) ? bMinusA * -1 : bMinusA;
	if (bMinusA < compareTol * (1 + absA + absB))
	{
		return true;
	}
	else
	{
		if (dif > 0)
		{
			dif--;
			return true;
		}
		else
		{
			messagePrefix(); printf("     JSON COMPARE FAIL: Mismatched number values [%.12f and %.12f]\n", a, b);
			return false;
		}
	}
}
bool compareItems(const char* a, const char* b)		// Does not take into account capitalization
{
	if (!BeStringUtilities::Stricmp(a, b))
	{
		return true;
	}
	else
	{
		messagePrefix("     JSON COMPARE FAIL: Mismatched string values\n");
		return false;
	}
}
bool compareArrays(Json::Value const &a, Json::Value const &b, struct TypeCounts &typeCounts, std::map<Utf8String, int, map_cmp> &propertyCounts, bvector<Utf8String> &errorTracker, int &dif)
	{
	if (a.size() != b.size())	// If not equal... must trace down to the root of the problem in order to provide some form of tracking
	{
		errorTracker.insert(errorTracker.begin(), "]");
		// Append object to tracker that counts the properties of each array element (which is an object) in b, ONLY AT THIS LEVEL
		appendArrayToErrorTracker(b, errorTracker);
		errorTracker.insert(errorTracker.begin(), "[");
		// Now do the same for a
		errorTracker.insert(errorTracker.begin(), "]");
		appendArrayToErrorTracker(a, errorTracker);
		errorTracker.insert(errorTracker.begin(), "[");

		messagePrefix(); printf("     JSON COMPARE FAIL: Mismatched array lengths file 1: [%u] file 2: [%u]\n", a.size(), b.size());
		return false;
	}

	// Keep track of result for each element of array
	bool toReturn = true;
	int n = a.size();
	for (int i = 0; i < n; i++)
	{
		toReturn = toReturn && compareHandler(a[i], b[i], typeCounts, propertyCounts, errorTracker, dif);
		// If false, break loop and return immediately
		if (!toReturn)
		{
			char toInsert[18];
			snprintf(toInsert, sizeof(toInsert), "[%d]", i);
			errorTracker.insert(errorTracker.begin(), toInsert);
			break;
		}
	}
	return toReturn;
	}
bool compareObjects(Json::Value const &a, Json::Value const &b, struct TypeCounts &typeCounts, std::map<Utf8String, int, map_cmp> &propertyCounts, bvector<Utf8String> &errorTracker, int &dif)
	{
	if (a.size() != b.size())
	{
		// Loop through properties and add to error tracker
		errorTracker.insert(errorTracker.begin(), "}");
		appendPropertiesToErrorTracker(b, errorTracker);
		errorTracker.insert(errorTracker.begin(), "{");
		// same for first file
		errorTracker.insert(errorTracker.begin(), "}");
		appendPropertiesToErrorTracker(a, errorTracker);
		errorTracker.insert(errorTracker.begin(), "{");

		messagePrefix(); printf("     JSON COMPARE FAIL: Mismatched property list lengths file 1: [%u] file 2: [%u]\n", a.size(), b.size());
		return false;
	}

	// Keep track of result for each property in object
	bool toReturn = true;
	for (Json::Value::iterator iter = a.begin(); iter != a.end(); iter++)
	{

		// Add each property found to the propertyCounts
		if (propertyCounts.count(iter.memberName()) == 0)
			propertyCounts[iter.memberName()] = 1;
		else
			propertyCounts[iter.memberName()]++;

		Json::Value bProp;
		if (!findProperty(b, iter.memberName(), bProp)) {
			// Add object contents to errorTracker
			errorTracker.insert(errorTracker.begin(), "}");
			appendPropertiesToErrorTracker(b, errorTracker);
			errorTracker.insert(errorTracker.begin(), "{");
			// same for first file
			errorTracker.insert(errorTracker.begin(), "}");
			appendPropertiesToErrorTracker(a, errorTracker);
			errorTracker.insert(errorTracker.begin(), "{");

			messagePrefix(); printf("     JSON COMPARE FAIL: Property %s of file 1 not in file 2\n", iter.memberName());
			return false;
		}

		toReturn = toReturn && compareHandler(*iter, bProp, typeCounts, propertyCounts, errorTracker, dif);
		// If toReturn becomes false at any point, push the property and break (will cause a chain reaction up the stack)
		if (!toReturn)
		{
			errorTracker.insert(errorTracker.begin(), iter.memberName());
			return false;
		}
	}
	return toReturn;
	}
bool compareHandler(Json::Value const &a, Json::Value const &b, struct TypeCounts &typeCounts, std::map<Utf8String, int, map_cmp> &propertyCounts, bvector<Utf8String> &errorTracker, int &dif)
	{
	if (a.type() != b.type())
	{
		messagePrefix(); printf("     JSON COMPARE FAIL: Type mismatch (%d != %d)\n", (int) a.type(), (int) b.type());
		return false;
	}

	// handle various cases and increment corresponding counters
	if (a.isDouble() || a.isInt())
	{
		typeCounts.numbers++;
		return compareItems(a.asDouble(), b.asDouble(), dif);
	}
	else if (a.isArray()) {		// Array and object compare function calls must take the typeCounts propertyCounts, errorTracker, & tol with them...they may call back on the handler
		typeCounts.arrays++;
		return compareArrays(a, b, typeCounts, propertyCounts, errorTracker, dif);
	}
	else if (a.isObject()) {
		typeCounts.objects++;
		return compareObjects(a, b, typeCounts, propertyCounts, errorTracker, dif);
	}
	else if (a.isString())
	{
		typeCounts.strings++;
		return compareItems(a.asCString(), b.asCString());
	}
	else if (a.isBool())
	{
		typeCounts.booleans++;
		return a.asBool() == b.asBool();
	}
	else if (a.isNull())
	{
		typeCounts.nulls++;
		// as long as both are null/undefined, is okay
		return true;
	}
	else
	{
		// unsupported type
		return false;
	}
	}
bool compareJSON(bvector<JsonData> const &allData, int verbose, int &dif) // initial call for entire json objects
	{
	Json::Value a = allData[0].m_value;
	Json::Value b = allData[1].m_value;
	struct TypeCounts typeCounts = { 0, 0, 0, 0, 0, 0 };
	std::map<Utf8String, int, map_cmp> propertyCounts;
	bvector<Utf8String> errorTracker;

	bool retVal = compareHandler(a, b, typeCounts, propertyCounts, errorTracker, dif);
	if (!retVal) {
		if (verbose == 3)
		{
			// If failed, print out the results of error tracing
			messagePrefix("-------------------- Error Tracking --------------------\n");
			size_t vectorSize = errorTracker.size();
			bool needMessagePrefix = true;
			for (size_t i = 0; i < vectorSize; i++)
			{
				if (needMessagePrefix)
					messagePrefix();
				if (i < vectorSize - 1 && errorTracker[i + 1].c_str()[0] == '[')	// Is an array that failed at a specific index; include index on same line
				{
					printf("%s", errorTracker[i].c_str());
					needMessagePrefix = false;
				}
				else
				{
					printf("%s\n", errorTracker[i].c_str());
					needMessagePrefix = true;
				}
			}
		}
	}
	else
	{
		messagePrefix(); printf("     JSON COMPARE SUCCESS: Files are equal!\n");
	}
	if (verbose == 3)
	{
		// Print out counts of each type
		messagePrefix("-------------------- Type Counts (up to failure/completion) --------------------\n");
		messagePrefix(); printf("Numbers: %d\n", typeCounts.numbers);
		messagePrefix(); printf("Arrays: %d\n", typeCounts.arrays);
		messagePrefix(); printf("Objects: %d\n", typeCounts.objects);
		messagePrefix(); printf("Strings: %d\n", typeCounts.strings);
		messagePrefix(); printf("Booleans: %d\n", typeCounts.booleans);
		messagePrefix(); printf("Nulls: %d\n", typeCounts.nulls);
		// Print out counts of each property
		messagePrefix("-------------------- Property Counts (up to failure/completion) --------------------\n");
		for (auto& x : propertyCounts)
		{
			messagePrefix(); printf("%s: %d\n", x.first.c_str(), x.second);
		}
	}
	return retVal;
	}
int launchCompare(bvector<JsonData> allData, bool &canUseGeometry, int &type, int &verbose, int &userType, int &dif)
	{
	// load json and geometry objects from each file.
	for (size_t i = 0; i < allData.size(); i++)
	{
		if (!allData[i].Load(canUseGeometry, type, verbose))
		{
			messagePrefix(); printf("	Unable to read from (%s)\n", allData[i].m_filename.c_str());
			return 1;
		}
		if (userType == 1 && !canUseGeometry)
		{
			messagePrefix(); printf("	Unable to parse json into geometry from (%s)\n", allData[i].m_filename.c_str());
			return 1;
		}
	}

	if (userType != 0 && userType != type)
		type = userType;

	if (type == 1)
	{	// Type 1 comparison: geometric compare
		if (compareGeometry(allData, verbose))
			return 0;
		else
			return 1;
	}
	if (type == 2)
	{	// Type 2 comparison: json objects
		if (compareJSON(allData, verbose, dif)) {
			return 0;
		}
		else {
			return 1;
		}
	}

	// Type should always be a 1 or 2... error
	return 1;
	}
extern "C"
int main(int argc, char **argv)
    {
    bvector<JsonData> allData;
    int verbose = 0;    // enables final filename echos
	int type = 1;		// acts as the default type (is changed depending on what type of file comes in)
	int userType = 0;	// user-declared type of comparison (0 for default... 1 for geometric compare... 2 for json property compare)
	int dif = 0;
	bool canUseGeometry = true;
    if (!parseCommandLine (argc, argv, verbose, userType, dif, s_echoErrorGeometry, allData) || allData.size () != 2)
        {
        messagePrefix (); printf ("exe name:   %s\n", argv[0]);
        messagePrefix ("	Usage:  comparedgnjs [-g || -j] [-vNN] [-nNN] [-tNN] <fileA || directoryA> <fileB || directoryB>\n");
		messagePrefix("		If given two files, compares them with one of two compare methods...\n");
		messagePrefix("		If given two directories, compares the contents of the directories individually...\n");
		messagePrefix("		If given a file and a directory, check if file exists in directory...\n");
		messagePrefix ("  Note: Geometry compare uses IGeometry::IsAlmostEqual.\n");
        messagePrefix ("    -v0         no output except errors.\n");
        messagePrefix ("    -v1         final confirmation of matched filenames (geometry compare only)\n");
        messagePrefix ("    -v2         beginning and end confirmation of filenames, Top level geometry counts. (geometry compare only)\n");
		messagePrefix ("    -v3		  verbose json compare (JSON string compare mode only)\n");
		messagePrefix("    -v11         all of above plus echo complete file contents (packed strings; geometry compare only)\n");
		messagePrefix ("    -t<N>		  sets the tolerance to <N> when comparing numeric values (default is 1.0e-12)\n");
		messagePrefix ("    -n<N>		  allows <N> number of differences when comparing numeric values (JSON string compare mode only)\n");
        messagePrefix ("    -e         echo geometry with errors.\n");
		messagePrefix ("    -j         force a JSON string compare, rather than the determined default\n");
		messagePrefix ("    -g         force a geometric compare, rather than the determined default\n");
        return 1;
        }

	BeFileName folderPath1(allData[0].m_filename.c_str());
	BeFileName folderPath2(allData[1].m_filename.c_str());

	if (BeFileName::IsDirectory(folderPath1) && BeFileName::IsDirectory(folderPath2))	// Comparing two directories
		{
		verbose = 0;	// Ignore verbosity for every file (just print out success or fail, so user may do specific comparison after)
		messagePrefix(); printf("*** Verbosity has been turned off for directory comparison\n");

		// grab all files from each directory
		bvector<WString> files1;
		bvector<WString> files2;
		BeFileName sourceDir1(folderPath1);
		BeFileName sourceDir2(folderPath2);
		BeFileName filename;
		bool isDir;
		for (BeDirectoryIterator dir(sourceDir1); dir.GetCurrentEntry(filename, isDir, true) == SUCCESS; dir.ToNext())
			{
			// only grab file name itself
			if (!filename.IsDirectory())
				files1.push_back(filename.GetFileNameWithoutExtension().append(L".").append(filename.GetExtension()));
			}
		for (BeDirectoryIterator dir(sourceDir2); dir.GetCurrentEntry(filename, isDir, true) == SUCCESS; dir.ToNext())
			{
			if (!filename.IsDirectory())
				files2.push_back(filename.GetFileNameWithoutExtension().append(L".").append(filename.GetExtension()));
			}

		// reveal files that are missing from one directory to another
		bvector<WString> onlyFolder1;
		bvector<WString> onlyFolder2;

		std::remove_copy_if(files1.begin(), files1.end(), std::back_inserter(onlyFolder1),
			[&files2](const WString& arg)
			{ return (std::find(files2.begin(), files2.end(), arg) != files2.end()); });
		std::remove_copy_if(files2.begin(), files2.end(), std::back_inserter(onlyFolder2),
			[&files1](const WString& arg)
			{ return (std::find(files1.begin(), files1.end(), arg) != files1.end()); });
		for (size_t i = 0; i < onlyFolder1.size(); i++)
			files1.erase(std::remove(files1.begin(), files1.end(), onlyFolder1[i]), files1.end());
		for (size_t i = 0; i < onlyFolder2.size(); i++)
			files2.erase(std::remove(files2.begin(), files2.end(), onlyFolder2[i]), files2.end());

		if (onlyFolder1.size() > 0)
		{
			messagePrefix(); printf("-------------------- Files in (%s) not in (%s) --------------------\n", allData[0].m_filename.c_str(), allData[1].m_filename.c_str());
			for (size_t i = 0; i < onlyFolder1.size(); i++)
			{
				messagePrefix(); printf("%s\n", Utf8String(onlyFolder1[i]).c_str());
			}
			printf("\n");
		}
		if (onlyFolder2.size() > 0)
		{
			messagePrefix(); printf("-------------------- Files in (%s) not in (%s) --------------------\n", allData[1].m_filename.c_str(), allData[0].m_filename.c_str());
			for (size_t i = 0; i < onlyFolder2.size(); i++)
			{
				messagePrefix(); printf("%s\n", Utf8String(onlyFolder2[i]).c_str());
			}
			printf("\n");
		}


		// compare matched files one by one (all items in files1 vector should also be in files2 vector and vise-versa by now)
		int retVal = 0;
		for (size_t i = 0; i < files1.size(); i++)
			{
			bvector<JsonData> fileData;
			// file 1 full path
			WString filePath1 = folderPath1.GetName();
			filePath1.append(L"\\").append(files1[i]);
			// file 2 full path
			WString filePath2 = folderPath2.GetName();
			filePath2.append(L"\\").append(files2[i]);
			// set up vector and pass to comparison functions
			fileData.push_back(JsonData(Utf8String(filePath1.c_str())));
			fileData.push_back(JsonData(Utf8String(filePath2.c_str())));
			messagePrefix(); printf("%ws:\n", files1[i].c_str());
			retVal += launchCompare(fileData, canUseGeometry, type, verbose, userType, dif);
			}

		return retVal;
		}
	else if (!BeFileName::IsDirectory(folderPath1) && !BeFileName::IsDirectory(folderPath2))	// Given two files to compare
		{
		return launchCompare(allData, canUseGeometry, type, verbose, userType, dif);
		}
	else	// Check if file exists in directory
		{
		WString fullPath;
		if (BeFileName::IsDirectory(folderPath1))
			{
			BeFileName file(allData[1].m_filename.c_str());
			fullPath = folderPath1.GetName();
			if (fullPath[fullPath.size() - 1] != '\\')
				fullPath.append(L"\\");
			fullPath.append(file.GetName());
			}
		else
			{
			BeFileName file(allData[0].m_filename.c_str());
			fullPath = folderPath2.GetName();
			if (fullPath[fullPath.size() - 1] != '\\')
				fullPath.append(L"\\");
			fullPath.append(file.GetName());
			}

		if (!(BeFileName::DoesPathExist(fullPath.c_str())))
		{
			messagePrefix(); printf("file not found (%ls)\n", fullPath.c_str());
			return 1;
		}
		else
		{
			messagePrefix(); printf("successfully found file at location (%ls)\n", fullPath.c_str());
			return 0;
		}
		}
	}