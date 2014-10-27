// The information contained herein is confidential, proprietary
// to Autodesk, Inc., and considered a trade secret as defined 
// in section 499C of the penal code of the State of California.  
// Use of this information by anyone other than authorized employees
// of Autodesk, Inc. is granted only under a written non-disclosure 
// agreement, expressly prescribing the scope and manner of such use.       
//
// CREATED BY:
//      Norm Olsen
//
// DESCRIPTION:
//

#include <ctype.h>
#include <time.h>
#include <locale.h>

#include "cs_map.h"
#include "cs_Legacy.h"
#include "cs_wkt.h"

#include "csCsvFileSupport.hpp"
#include "csNameMapperSupport.h"
#include "csEpsgStuff.h"

// A structure used in several tests.
struct tst_lst_
{
	char name [24];
	struct cs_Csdef_ *cs_ptr;
	struct cs_Dtdef_ *dt_ptr;
	struct cs_Eldef_ *el_ptr;
};

// Supporting functions.
void usage (bool batch);
void CS_reset (void);
double CStestRN (double low, double high);
int CStestXYZ (double xyz [3],double falseOrg [2],unsigned* sequencer);
int CStestLLH (double llh [2],double cntrlMer,unsigned* sequencer);
double CStstsclk (struct cs_Csprm_ *csprm,double ll [2]);
double CStstsclh (struct cs_Csprm_ *csprm,double ll [2]);
double CStstcnvrg (struct cs_Csprm_ *csprm,double ll [2]);

// Individual test function prototypes.
int CStest1 (bool verbose,bool crypt);
int CStest2 (bool verbose,bool crypt);
int CStest3 (bool verbose,bool crypt);
int CStest4 (bool verbose,char *test_file);
int CStest5 (bool verbose,long32_t duration);
int CStest6 (bool verbose,bool crypt); 
int CStest7 (bool verbose,bool crypt); 
int CStest8 (bool verbose,bool crypt); 
int CStest9 (bool verbose);
int CStestA (bool verbose,char *test_file);
int CStestB (bool verbose,long32_t duration);
int CStestC (bool verbose,long32_t duration);
int CStestD (bool verbose,long32_t duration);
int CStestE (bool verbose,long32_t duration);
int CStestF (bool verbose,long32_t duration);
int CStestG (bool verbose,long32_t duration);
int CStestH (bool verbose,long32_t duration);
int CStestI (bool verbose,long32_t duration);
int CStestJ (bool verbose,long32_t duration);
int CStestK (bool verbose,long32_t duration);
int CStestL (bool verbose,long32_t duration);
int CStestM (const TcsEpsgDataSetV6& epsgV6,bool verbose,long32_t duration);
int CStestN (const TcsEpsgDataSetV6& epsgV6,bool verbose,long32_t duration);
int CStestS (bool verbose);
int CStestT (bool verbose,long32_t duration);
int CStestZ (bool verbose,char *test_file);

enum EcsTestMethod {    testMthNone = 0,
                        testMthCrs2D, 
                        testMthCrs3D,
                        testMthGeoid,
                        testMthUnknown = 999
                    };
enum EcsCrsAuthority {  crsAuthNone = 0,
                        crsAuthCsMap, 
                        crsAuthEpsg, 
                        crsAuthProj4, 
                        crsAuthSrid, 
                        crsAuthUnknown = 999
                    };

class TcsOsGeoTestFile : public TcsCsvFileBase
{
public:
	//=========================================================================
	// Construction, Destruction, and Assignment
	TcsOsGeoTestFile (const wchar_t* dataFilePath);
	TcsOsGeoTestFile (const char* dataFilePath);
	TcsOsGeoTestFile (const TcsOsGeoTestFile& source);
	~TcsOsGeoTestFile (void);
	TcsOsGeoTestFile& operator= (const TcsOsGeoTestFile& rhs);
	//=========================================================================
	// Operator Overrides
	//=========================================================================
	// Public Named Member Functions
	bool IsOk (void) const {return Ok; }
	bool IsComment (unsigned recordNbr);
	bool GetTestName (std::wstring& testName,unsigned recordNbr);
	EcsTestMethod GetTestMethod (unsigned recordNbr);
	EcsCrsAuthority TcsOsGeoTestFile::GetCrsKey (unsigned recordNbr,bool target);
	EcsCrsAuthority GetSrcCrsKey (std::wstring& srcKey,unsigned recordNbr);
	EcsCrsAuthority GetTrgCrsKey (std::wstring& srcKey,unsigned recordNbr);
    bool GetSourceCoordinates (double sourceCoord [3],unsigned recordNbr);
    bool GetTargetCoordinates (double targetCoord [3],unsigned recordNbr);
    bool GetTolerances (double tolerance [3],unsigned recordNbr);
	bool GetDataSource (std::wstring& dataSource,unsigned recordNbr);
	bool GetDataComment (std::wstring& dataComment,unsigned recordNbr);
    bool CompareResults (double results [3],unsigned recordNbr,double& dd);
private:
    bool Ok;
    TcsCsvStatus Status;
};
