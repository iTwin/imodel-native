#include "FormsDomainTestFixture.h"
#include "FormsDomain\FormsDomainUtilities.h"
#include <BeJsonCpp/BeJsonUtilities.h>
#include <BeSQLite/BeSQLite.h>
#include <Json/Json.h>
#include <Bentley\BeAssert.h>


#define MODEL_TEST_NAME              "SampleModel"
#define MODEL_TEST_NAME1             "SampleModelAAA"
#define MODEL_TEST_NAME2             "SampleModelBBB"
#define MODEL_TEST_NAME3             "SampleModelCCC"
#define DYNAMIC_SCHEMA_NAME          "SampleDynamic" 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Arturas.Mizaras             11/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FormsDomainTestsFixture, EnsureDomainsAreRegistered)
    {
    ASSERT_TRUE(true);
    DgnDbPtr db = CreateDgnDb();

    ASSERT_TRUE(db.IsValid());

    // //This should create a DGN db with building domain.
    Forms::FormsDomainUtilities::RegisterDomainHandlers();

    DgnDomainCP formDomain = db->Domains().FindDomain(Forms::FormsDomain::GetDomain().GetDomainName());
    ASSERT_TRUE(NULL != formDomain);
    }

BE_JSON_NAME(FormsDomain)

