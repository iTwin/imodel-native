/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include <set>
#include <ECObjects/SchemaComparer.h>

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
struct BuildingBridgeSchemaTestFixture : public ECDbTestFixture
    {
    std::vector<Utf8String> m_updatedDbs;
    protected:

        //---------------------------------------------------------------------------------------
        // @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        void CloseReOpenECDb()
            {
            Utf8CP dbFileName = m_ecdb.GetDbFileName();
            BeFileName dbPath(dbFileName);
            m_ecdb.CloseDb();
            ASSERT_FALSE(m_ecdb.IsDbOpen());
            ASSERT_EQ(BE_SQLITE_OK, m_ecdb.OpenBeSQLiteDb(dbPath, ECDb::OpenParams(ECDb::OpenMode::Readonly)));
            ASSERT_TRUE(m_ecdb.IsDbOpen());
            }

        //---------------------------------------------------------------------------------------
        // @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        DbResult OpenBesqliteDb(Utf8CP dbPath) { return m_ecdb.OpenBeSQLiteDb(dbPath, ECDb::OpenParams(ECDb::OpenMode::ReadWrite)); }
    };

#define ASSERT_ECSQL(ECDB_OBJ, PREPARESTATUS, STEPSTATUS, ECSQL)   {\
                                                                    ECSqlStatement stmt;\
                                                                    ASSERT_EQ(PREPARESTATUS, stmt.Prepare(ECDB_OBJ, ECSQL));\
                                                                    if (PREPARESTATUS == ECSqlStatus::Success)\
                                                                        ASSERT_EQ(STEPSTATUS, stmt.Step());\
                                                                   }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(BuildingBridgeSchemaTestFixture, UpdateH2DynamicSchema_CV_BR_OBElementAspect)
    {
    SchemaItem schemaItem(R"schema(<ECSchema schemaName="HS2" alias="HS2" version="01.00.10" displayLabel="HS2" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
    <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA"/>
    <ECCustomAttributes>
        <DynamicSchema xmlns="CoreCustomAttributes.01.00.03"/>
    </ECCustomAttributes>
    <ECEntityClass typeName="ElementAspect"  displayLabel="Element Aspect" modifier="Abstract">
        <ECCustomAttributes>
            <NotSubclassableInReferencingSchemas xmlns="CoreCustomAttributes.01.00.03"/>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="ElementMultiAspect"  displayLabel="Element Multi-Aspect" modifier="Abstract">
        <BaseClass>ElementAspect</BaseClass>
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
            <ShareColumns xmlns="ECDbMap.02.00.00">
                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
            </ShareColumns>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="All_AssetElementAspect" displayLabel="All_Asset">
        <BaseClass>ElementMultiAspect</BaseClass>
        <ECProperty propertyName="UAID" typeName="string" priority="0"/>
        <ECProperty propertyName="OSGB_Easting" typeName="string" priority="0"/>
        <ECProperty propertyName="OSGB_Northing" typeName="string" priority="0"/>
        <ECProperty propertyName="Name" typeName="string" priority="0"/>
        <ECProperty propertyName="Stage" typeName="string" priority="0"/>
        <ECProperty propertyName="Owner" typeName="string" priority="0"/>
        <ECProperty propertyName="Status" typeName="string" priority="0"/>
        <ECProperty propertyName="Offset" typeName="string" priority="0"/>
        <ECProperty propertyName="Reference_chainage" typeName="string" priority="0"/>
        <ECProperty propertyName="URC" typeName="string" priority="0"/>
        <ECProperty propertyName="StartChainage" typeName="string" priority="0"/>
        <ECProperty propertyName="EndChainage" typeName="string" priority="0"/>
        <ECProperty propertyName="Chainage_baseline" typeName="string" displayLabel="Chainage baseline" readOnly="true" priority="0"/>
        <ECProperty propertyName="Classification_Code" typeName="string" displayLabel="Classification Code" priority="0"/>
    </ECEntityClass>
    <ECEntityClass typeName="Primary_AssetElementAspect" displayLabel="Primary_Asset">
        <BaseClass>All_AssetElementAspect</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="CVElementAspect" displayLabel="CV">
        <BaseClass>Primary_AssetElementAspect</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="CV_BRElementAspect" displayLabel="CV_BR">
        <BaseClass>CVElementAspect</BaseClass>
        <ECProperty propertyName="BdgtCntrlAccnt" typeName="string" priority="0"/>
        <ECProperty propertyName="CBSCd" typeName="string" priority="0"/>
        <ECProperty propertyName="CompltnDt" typeName="string" priority="0"/>
        <ECProperty propertyName="CndtnRtng" typeName="string" priority="0"/>
        <ECProperty propertyName="CndtnRtngDt" typeName="string" priority="0"/>
        <ECProperty propertyName="Crtclty" typeName="string" priority="0"/>
        <ECProperty propertyName="DtMntnd" typeName="string" priority="0"/>
        <ECProperty propertyName="DspslRqrmnts" typeName="string" priority="0"/>
        <ECProperty propertyName="GrntPrd" typeName="string" priority="0"/>
        <ECProperty propertyName="HndvrDt" typeName="string" priority="0"/>
        <ECProperty propertyName="InspctnFrqncy" typeName="string" priority="0"/>
        <ECProperty propertyName="InspctnHstry" typeName="string" priority="0"/>
        <ECProperty propertyName="Installation_Date" typeName="string" priority="0"/>
        <ECProperty propertyName="LnghtOpr" typeName="string" priority="0"/>
        <ECProperty propertyName="MntnngOrgnstn" typeName="string" priority="0"/>
        <ECProperty propertyName="MntncFrqncy" typeName="string" priority="0"/>
        <ECProperty propertyName="MntnncHstry" typeName="string" priority="0"/>
        <ECProperty propertyName="MntncUndrtkn" typeName="string" priority="0"/>
        <ECProperty propertyName="Mnfctrr" typeName="string" priority="0"/>
        <ECProperty propertyName="NxtInspctnDt" typeName="string" priority="0"/>
        <ECProperty propertyName="NxtMntnncDt" typeName="string" priority="0"/>
        <ECProperty propertyName="PlnndUs" typeName="string" priority="0"/>
        <ECProperty propertyName="PlnnngArrngmnts" typeName="string" priority="0"/>
        <ECProperty propertyName="RskAssssmntDscrptn" typeName="string" priority="0"/>
        <ECProperty propertyName="SftyRqrmnts" typeName="string" priority="0"/>
        <ECProperty propertyName="SrlBtchNmbr" typeName="string" priority="0"/>
        <ECProperty propertyName="SpclAccssArrngmnts" typeName="string" priority="0"/>
        <ECProperty propertyName="SpclRqrmnts" typeName="string" priority="0"/>
        <ECProperty propertyName="SpcfctnNmbr" typeName="string" priority="0"/>
        <ECProperty propertyName="Supplier" typeName="string" priority="0"/>
        <ECProperty propertyName="TrtmntObjctivs" typeName="string" priority="0"/>
        <ECProperty propertyName="WrrntyExpryDt" typeName="string" priority="0"/>
        <ECProperty propertyName="WrrntyPrd" typeName="string" priority="0"/>
        <ECProperty propertyName="WrrntyStrtDt" typeName="string" priority="0"/>
        <ECProperty propertyName="Grade_of_Material" typeName="string" priority="0"/>
        <ECProperty propertyName="DsgnLf" typeName="string" priority="0"/>
        <ECProperty propertyName="Lngthm" typeName="string" priority="0"/>
        <ECProperty propertyName="WidthAvrgm" typeName="string" priority="0"/>
        <ECProperty propertyName="Mtrl" typeName="string" priority="0"/>
        <ECProperty propertyName="CntrctTyp" typeName="string" priority="0"/>
        <ECProperty propertyName="LtScp" typeName="string" priority="0"/>
        <ECProperty propertyName="MndtryScp" typeName="string" priority="0"/>
        <ECProperty propertyName="AccptblFrUs" typeName="string" priority="0"/>
        <ECProperty propertyName="_1012" typeName="string" priority="0"/>
        <ECProperty propertyName="_1112" typeName="string" priority="0"/>
        <ECProperty propertyName="_1128" typeName="string" priority="0"/>
        <ECProperty propertyName="VlmBrdTnnlsAndShfts" typeName="string" priority="0"/>
        <ECProperty propertyName="VlmGrndTrtmnt" typeName="string" priority="0"/>
        <ECProperty propertyName="VlmOpnCtHghwyWrks" typeName="string" priority="0"/>
        <ECProperty propertyName="VlmOpnCtRlwyWrks" typeName="string" priority="0"/>
        <ECProperty propertyName="VlmTrndCttngsTnnls" typeName="string" priority="0"/>
        <ECProperty propertyName="ChmcllyUARmvl" typeName="string" priority="0"/>
        <ECProperty propertyName="ChmcllyUAFllMtrl" typeName="string" priority="0"/>
        <ECProperty propertyName="_1009" typeName="string" priority="0"/>
        <ECProperty propertyName="ChmcllyUAFllMtrlU2" typeName="string" priority="0"/>
        <ECProperty propertyName="_1108" typeName="string" priority="0"/>
        <ECProperty propertyName="EnrnmntMtErthwrksNd" typeName="string" priority="0"/>
        <ECProperty propertyName="_1109" typeName="string" priority="0"/>
        <ECProperty propertyName="_1005" typeName="string" priority="0"/>
        <ECProperty propertyName="_1002" typeName="string" priority="0"/>
        <ECProperty propertyName="_1006" typeName="string" priority="0"/>
        <ECProperty propertyName="_1001" typeName="string" priority="0"/>
        <ECProperty propertyName="_1007" typeName="string" priority="0"/>
        <ECProperty propertyName="_1008" typeName="string" priority="0"/>
        <ECProperty propertyName="ExcvtMtrlRqrngPrcssn" typeName="string" priority="0"/>
        <ECProperty propertyName="ExcvtdMtrlfrStblstn" typeName="string" priority="0"/>
        <ECProperty propertyName="FtprntAr" typeName="string" priority="0"/>
        <ECProperty propertyName="GtwID" typeName="string" priority="0"/>
        <ECProperty propertyName="_1107" typeName="string" priority="0"/>
        <ECProperty propertyName="ChsvHghPlsctcty" typeName="string" priority="0"/>
        <ECProperty propertyName="ChsvLwPlstcty" typeName="string" priority="0"/>
        <ECProperty propertyName="GrnlrRckOrChlk" typeName="string" priority="0"/>
        <ECProperty propertyName="GtchncllyLndscpFll" typeName="string" priority="0"/>
        <ECProperty propertyName="MxmmHghtAbvRlLvl" typeName="string" priority="0"/>
        <ECProperty propertyName="MthdOfClcltn" typeName="string" priority="0"/>
        <ECProperty propertyName="NtExcvtdVlm" typeName="string" priority="0"/>
        <ECProperty propertyName="PSCAr" typeName="string" priority="0"/>
        <ECProperty propertyName="PrcsstClss6Mtrl" typeName="string" priority="0"/>
        <ECProperty propertyName="SldGlgy" typeName="string" priority="0"/>
        <ECProperty propertyName="SprfclGlgy" typeName="string" priority="0"/>
        <ECProperty propertyName="_1011" typeName="string" priority="0"/>
        <ECProperty propertyName="_1110" typeName="string" priority="0"/>
        <ECProperty propertyName="_1111" typeName="string" priority="0"/>
        <ECProperty propertyName="_1010" typeName="string" priority="0"/>
        <ECProperty propertyName="TtlCrtAwyNmbr" typeName="string" priority="0"/>
        <ECProperty propertyName="TtlCrtAwyUnblkd" typeName="string" priority="0"/>
        <ECProperty propertyName="_2000" typeName="string" priority="0"/>
        <ECProperty propertyName="AvrgCrtclHdrmm" typeName="string" priority="0"/>
        <ECProperty propertyName="ObstclCrssd" typeName="string" priority="0"/>
        <ECProperty propertyName="EnvrnmntTyp" typeName="string" priority="0"/>
        <ECProperty propertyName="ExstngRsdHrtg" typeName="string" priority="0"/>
        <ECProperty propertyName="ExstngRsdStrctr" typeName="string" priority="0"/>
        <ECProperty propertyName="TypOfRtSpprtd" typeName="string" priority="0"/>
        <ECProperty propertyName="CnsrvtnAr" typeName="string" priority="0"/>
        <ECProperty propertyName="EnvrnmntllySnstvAr" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPEA13" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPEA4" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPEA51" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPEA53" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPEB2" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPEB4" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPED1" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPED2" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPFA13" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPFA4" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPFA51" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPFA53" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPFB2" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPFB4" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPFD1" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPFD2" typeName="string" priority="0"/>
        <ECProperty propertyName="APA13" typeName="string" priority="0"/>
        <ECProperty propertyName="APA4" typeName="string" priority="0"/>
        <ECProperty propertyName="APA51" typeName="string" priority="0"/>
        <ECProperty propertyName="APA53" typeName="string" priority="0"/>
        <ECProperty propertyName="APB2" typeName="string" priority="0"/>
        <ECProperty propertyName="APB4" typeName="string" priority="0"/>
        <ECProperty propertyName="APD1" typeName="string" priority="0"/>
        <ECProperty propertyName="APD2" typeName="string" priority="0"/>
        <ECProperty propertyName="EPA13" typeName="string" priority="0"/>
        <ECProperty propertyName="EPA4" typeName="string" priority="0"/>
        <ECProperty propertyName="EPA51" typeName="string" priority="0"/>
        <ECProperty propertyName="EPA53" typeName="string" priority="0"/>
        <ECProperty propertyName="EPB2" typeName="string" priority="0"/>
        <ECProperty propertyName="EPB4" typeName="string" priority="0"/>
        <ECProperty propertyName="EPD1" typeName="string" priority="0"/>
        <ECProperty propertyName="EPD2" typeName="string" priority="0"/>
        <ECProperty propertyName="GWPA13" typeName="string" priority="0"/>
        <ECProperty propertyName="GWPA4" typeName="string" priority="0"/>
        <ECProperty propertyName="GWPA51" typeName="string" priority="0"/>)schema" R"schema(
        <ECProperty propertyName="GWPA53" typeName="string" priority="0"/>
        <ECProperty propertyName="GWPB2" typeName="string" priority="0"/>
        <ECProperty propertyName="GWPB4" typeName="string" priority="0"/>
        <ECProperty propertyName="GWPD1" typeName="string" priority="0"/>
        <ECProperty propertyName="GWPD2" typeName="string" priority="0"/>
        <ECProperty propertyName="MORECMA13" typeName="string" priority="0"/>
        <ECProperty propertyName="MORECMB2" typeName="string" priority="0"/>
        <ECProperty propertyName="MORECMB4" typeName="string" priority="0"/>
        <ECProperty propertyName="MORENMA13" typeName="string" priority="0"/>
        <ECProperty propertyName="MORENMB2" typeName="string" priority="0"/>
        <ECProperty propertyName="MORENMB4" typeName="string" priority="0"/>
        <ECProperty propertyName="MOREUMA13" typeName="string" priority="0"/>
        <ECProperty propertyName="MOREUMB2" typeName="string" priority="0"/>
        <ECProperty propertyName="MOREUMB4" typeName="string" priority="0"/>
        <ECProperty propertyName="MEMA13A5A4" typeName="string" priority="0"/>
        <ECProperty propertyName="FWA13" typeName="string" priority="0"/>
        <ECProperty propertyName="FWA4" typeName="string" priority="0"/>
        <ECProperty propertyName="FWA51" typeName="string" priority="0"/>
        <ECProperty propertyName="FWA53" typeName="string" priority="0"/>
        <ECProperty propertyName="FWB2" typeName="string" priority="0"/>
        <ECProperty propertyName="FWB4" typeName="string" priority="0"/>
        <ECProperty propertyName="FWD1" typeName="string" priority="0"/>
        <ECProperty propertyName="FWD2" typeName="string" priority="0"/>
        <ECProperty propertyName="NHWDA13" typeName="string" priority="0"/>
        <ECProperty propertyName="NHWDA4" typeName="string" priority="0"/>
        <ECProperty propertyName="NHWDA51" typeName="string" priority="0"/>
        <ECProperty propertyName="NHWDA53" typeName="string" priority="0"/>
        <ECProperty propertyName="NHWDB2" typeName="string" priority="0"/>
        <ECProperty propertyName="NHWDD1" typeName="string" priority="0"/>
        <ECProperty propertyName="NHWDD2" typeName="string" priority="0"/>
    </ECEntityClass>
    <ECEntityClass typeName="CV_BR_OBElementAspect" displayLabel="CV_BR_OB">
        <BaseClass>CV_BRElementAspect</BaseClass>
        <ECProperty propertyName="_04000" typeName="string" priority="0"/>
        <ECProperty propertyName="_02000" typeName="string" priority="0"/>
        <ECProperty propertyName="_06000" typeName="string" priority="0"/>
        <ECProperty propertyName="DSAsst" typeName="string" priority="0"/>
        <ECProperty propertyName="Ownr" typeName="string" priority="0"/>
        <ECProperty propertyName="SctrCd" typeName="string" priority="0"/>
        <ECProperty propertyName="Wdthm" typeName="string" priority="0"/>
        <ECProperty propertyName="CM_ISCI" typeName="string" priority="0"/>
        <ECProperty propertyName="CM_CRT" typeName="string" priority="0"/>
        <ECProperty propertyName="CM_StdBL" typeName="string" priority="0"/>
        <ECProperty propertyName="CM_Chg" typeName="string" priority="0"/>
        <ECProperty propertyName="ElvtnAlg" typeName="string" priority="0"/>
        <ECProperty propertyName="AlgnmPln" typeName="string" priority="0"/>
        <ECProperty propertyName="AnglSkw" typeName="string" priority="0"/>
        <ECProperty propertyName="ClrncHrz" typeName="string" priority="0"/>
        <ECProperty propertyName="ClrncVrt" typeName="string" priority="0"/>
        <ECProperty propertyName="CrssngFtrID" typeName="string" priority="0"/>
        <ECProperty propertyName="DpthDck" typeName="string" priority="0"/>
        <ECProperty propertyName="ElmntPssngOvr" typeName="string" priority="0"/>
        <ECProperty propertyName="GrdntMx" typeName="string" priority="0"/>
        <ECProperty propertyName="GrdntMn" typeName="string" priority="0"/>
        <ECProperty propertyName="HghwyAthrty" typeName="string" priority="0"/>
        <ECProperty propertyName="MxmmLdng" typeName="string" priority="0"/>
        <ECProperty propertyName="RdNm" typeName="string" priority="0"/>
        <ECProperty propertyName="RdNmbr" typeName="string" priority="0"/>
        <ECProperty propertyName="RdSrfc" typeName="string" priority="0"/>
        <ECProperty propertyName="SpdLmt" typeName="string" priority="0"/>
        <ECProperty propertyName="SprtTyp" typeName="string" priority="0"/>
        <ECProperty propertyName="SrfcngClr" typeName="string" priority="0"/>
        <ECProperty propertyName="SrvydTrffcFlw" typeName="string" priority="0"/>
        <ECProperty propertyName="ThrdPrty" typeName="string" priority="0"/>
        <ECProperty propertyName="TypFndtn" typeName="string" priority="0"/>
        <ECProperty propertyName="StrctrlT" typeName="string" priority="0"/>
        <ECProperty propertyName="LnghtSpn" typeName="string" priority="0"/>
        <ECProperty propertyName="WdthMx" typeName="string" priority="0"/>
        <ECProperty propertyName="WdthMn" typeName="string" priority="0"/>
        <ECProperty propertyName="CmrRqstFrmID" typeName="string" priority="0"/>
        <ECProperty propertyName="SCP_ScpBkLvl" typeName="string" priority="0"/>
        <ECProperty propertyName="Env_ArOtsndn" typeName="string" priority="0"/>
        <ECProperty propertyName="_1113" typeName="string" priority="0"/>
        <ECProperty propertyName="_1115" typeName="string" priority="0"/>
        <ECProperty propertyName="_1114" typeName="string" priority="0"/>
        <ECProperty propertyName="_1116" typeName="string" priority="0"/>
        <ECProperty propertyName="BlncinDsfnElmntAll" typeName="string" priority="0"/>
        <ECProperty propertyName="BlncInDsngElmntCL4" typeName="string" priority="0"/>
        <ECProperty propertyName="Blncindsgnelmntsbsl" typeName="string" priority="0"/>
        <ECProperty propertyName="BlncDsgnElmntHghCL2" typeName="string" priority="0"/>
        <ECProperty propertyName="BlncinDsgnElmntLwCL2" typeName="string" priority="0"/>
        <ECProperty propertyName="NtsOnErthwrksBlncClc" typeName="string" priority="0"/>
        <ECProperty propertyName="BlncInDsgnElmntCL6" typeName="string" priority="0"/>
        <ECProperty propertyName="BlncInDsgnElmntTpsl" typeName="string" priority="0"/>
        <ECProperty propertyName="BlncinDsgnElmntCL1_3" typeName="string" priority="0"/>
        <ECProperty propertyName="_1120" typeName="string" priority="0"/>
        <ECProperty propertyName="_1013" typeName="string" priority="0"/>
        <ECProperty propertyName="_1017" typeName="string" priority="0"/>
        <ECProperty propertyName="_1016" typeName="string" priority="0"/>
        <ECProperty propertyName="_1015" typeName="string" priority="0"/>
        <ECProperty propertyName="_1014" typeName="string" priority="0"/>
        <ECProperty propertyName="_1124" typeName="string" priority="0"/>
        <ECProperty propertyName="_1018" typeName="string" priority="0"/>
        <ECProperty propertyName="_1123" typeName="string" priority="0"/>
        <ECProperty propertyName="_1122" typeName="string" priority="0"/>
        <ECProperty propertyName="_1121" typeName="string" priority="0"/>
        <ECProperty propertyName="_1118" typeName="string" priority="0"/>
        <ECProperty propertyName="_1117" typeName="string" priority="0"/>
        <ECProperty propertyName="_1119" typeName="string" priority="0"/>
        <ECProperty propertyName="_1126" typeName="string" priority="0"/>
        <ECProperty propertyName="_1125" typeName="string" priority="0"/>
        <ECProperty propertyName="_2001" typeName="string" priority="0"/>
        <ECProperty propertyName="_2002" typeName="string" priority="0"/>
        <ECProperty propertyName="_2003" typeName="string" priority="0"/>
        <ECProperty propertyName="_2004" typeName="string" priority="0"/>
        <ECProperty propertyName="HlthSftyRtng" typeName="string" priority="0"/>)schema" R"schema(
        <ECProperty propertyName="CStrtDt" typeName="string" priority="0"/>
        <ECProperty propertyName="A52" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPEB1" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPEB3" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPEB5" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPEB61" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPEB62" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPEB7" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPEB8" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPEC1" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPEC2" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPEC3" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPEC4" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPFB1" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPFB3" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPFB5" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPFB61" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPFB62" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPFB7" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPFB8" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPFC1" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPFC2" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPFC3" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPFC4" typeName="string" priority="0"/>
        <ECProperty propertyName="APB1" typeName="string" priority="0"/>
        <ECProperty propertyName="APB3" typeName="string" priority="0"/>
        <ECProperty propertyName="APB5" typeName="string" priority="0"/>
        <ECProperty propertyName="APB61" typeName="string" priority="0"/>
        <ECProperty propertyName="APB62" typeName="string" priority="0"/>
        <ECProperty propertyName="APB7" typeName="string" priority="0"/>
        <ECProperty propertyName="APB8" typeName="string" priority="0"/>
        <ECProperty propertyName="APC1" typeName="string" priority="0"/>
        <ECProperty propertyName="APC2" typeName="string" priority="0"/>
        <ECProperty propertyName="APC3" typeName="string" priority="0"/>
        <ECProperty propertyName="APC4" typeName="string" priority="0"/>
        <ECProperty propertyName="EPB1" typeName="string" priority="0"/>
        <ECProperty propertyName="EPB3" typeName="string" priority="0"/>
        <ECProperty propertyName="EPB5" typeName="string" priority="0"/>
        <ECProperty propertyName="EPB61" typeName="string" priority="0"/>
        <ECProperty propertyName="EPB62" typeName="string" priority="0"/>
        <ECProperty propertyName="EPB7" typeName="string" priority="0"/>
        <ECProperty propertyName="EPB8" typeName="string" priority="0"/>
        <ECProperty propertyName="EPC1" typeName="string" priority="0"/>
        <ECProperty propertyName="EPC2" typeName="string" priority="0"/>
        <ECProperty propertyName="EPC3" typeName="string" priority="0"/>
        <ECProperty propertyName="EPC4" typeName="string" priority="0"/>
        <ECProperty propertyName="GWPB1" typeName="string" priority="0"/>
        <ECProperty propertyName="GWPB3" typeName="string" priority="0"/>
        <ECProperty propertyName="GWPB5" typeName="string" priority="0"/>
        <ECProperty propertyName="GWPB61" typeName="string" priority="0"/>
        <ECProperty propertyName="GWPB62" typeName="string" priority="0"/>
        <ECProperty propertyName="GWPB7" typeName="string" priority="0"/>
        <ECProperty propertyName="GWPB8" typeName="string" priority="0"/>
        <ECProperty propertyName="GWPC1" typeName="string" priority="0"/>
        <ECProperty propertyName="GWPC2" typeName="string" priority="0"/>
        <ECProperty propertyName="GWPC3" typeName="string" priority="0"/>
        <ECProperty propertyName="GWPC4" typeName="string" priority="0"/>
        <ECProperty propertyName="HWDA13" typeName="string" priority="0"/>
        <ECProperty propertyName="HWDA4" typeName="string" priority="0"/>
        <ECProperty propertyName="HWDA51" typeName="string" priority="0"/>
        <ECProperty propertyName="HWDA53" typeName="string" priority="0"/>
        <ECProperty propertyName="HWDB1" typeName="string" priority="0"/>
        <ECProperty propertyName="HWDB2" typeName="string" priority="0"/>
        <ECProperty propertyName="HWDB3" typeName="string" priority="0"/>
        <ECProperty propertyName="HWDB4" typeName="string" priority="0"/>
        <ECProperty propertyName="HWDB5" typeName="string" priority="0"/>
        <ECProperty propertyName="HWDB61" typeName="string" priority="0"/>
        <ECProperty propertyName="HWDB62" typeName="string" priority="0"/>
        <ECProperty propertyName="HWDB7" typeName="string" priority="0"/>
        <ECProperty propertyName="HWDB8" typeName="string" priority="0"/>
        <ECProperty propertyName="HWDC1" typeName="string" priority="0"/>
        <ECProperty propertyName="HWDC2" typeName="string" priority="0"/>
        <ECProperty propertyName="HWDC3" typeName="string" priority="0"/>
        <ECProperty propertyName="HWDC4" typeName="string" priority="0"/>
        <ECProperty propertyName="HWDD1" typeName="string" priority="0"/>
        <ECProperty propertyName="HWDD2" typeName="string" priority="0"/>
        <ECProperty propertyName="MORECMB3" typeName="string" priority="0"/>
        <ECProperty propertyName="MORECMB5" typeName="string" priority="0"/>
        <ECProperty propertyName="MORENMB3" typeName="string" priority="0"/>
        <ECProperty propertyName="MORENMB5" typeName="string" priority="0"/>
        <ECProperty propertyName="MOREUMB3" typeName="string" priority="0"/>
        <ECProperty propertyName="MOREUMB5" typeName="string" priority="0"/>
        <ECProperty propertyName="FWB1" typeName="string" priority="0"/>
        <ECProperty propertyName="FWB3" typeName="string" priority="0"/>
        <ECProperty propertyName="FWB5" typeName="string" priority="0"/>
        <ECProperty propertyName="FWB61" typeName="string" priority="0"/>
        <ECProperty propertyName="FWB62" typeName="string" priority="0"/>
        <ECProperty propertyName="FWB7" typeName="string" priority="0"/>
        <ECProperty propertyName="FWB8" typeName="string" priority="0"/>
        <ECProperty propertyName="FWC1" typeName="string" priority="0"/>
        <ECProperty propertyName="FWC2" typeName="string" priority="0"/>
        <ECProperty propertyName="FWC3" typeName="string" priority="0"/>
        <ECProperty propertyName="FWC4" typeName="string" priority="0"/>
        <ECProperty propertyName="NHWDB1" typeName="string" priority="0"/>
        <ECProperty propertyName="NHWDB3" typeName="string" priority="0"/>
        <ECProperty propertyName="NHWDB4" typeName="string" priority="0"/>
        <ECProperty propertyName="NHWDB5" typeName="string" priority="0"/>
        <ECProperty propertyName="NHWDB61" typeName="string" priority="0"/>
        <ECProperty propertyName="NHWDB62" typeName="string" priority="0"/>
        <ECProperty propertyName="NHWDB7" typeName="string" priority="0"/>
        <ECProperty propertyName="NHWDB8" typeName="string" priority="0"/>
        <ECProperty propertyName="NHWDC1" typeName="string" priority="0"/>
        <ECProperty propertyName="NHWDC2" typeName="string" priority="0"/>
        <ECProperty propertyName="NHWDC3" typeName="string" priority="0"/>
        <ECProperty propertyName="NHWDC4" typeName="string" priority="0"/>
        <ECProperty propertyName="ODPA13" typeName="string" priority="0"/>
        <ECProperty propertyName="ODPA4" typeName="string" priority="0"/>
        <ECProperty propertyName="ODPA51" typeName="string" priority="0"/>
        <ECProperty propertyName="ODPA53" typeName="string" priority="0"/>
        <ECProperty propertyName="ODPB1" typeName="string" priority="0"/>
        <ECProperty propertyName="ODPB2" typeName="string" priority="0"/>
        <ECProperty propertyName="ODPB3" typeName="string" priority="0"/>
        <ECProperty propertyName="ODPB4" typeName="string" priority="0"/>
        <ECProperty propertyName="ODPB5" typeName="string" priority="0"/>
        <ECProperty propertyName="ODPB61" typeName="string" priority="0"/>
        <ECProperty propertyName="ODPB62" typeName="string" priority="0"/>
        <ECProperty propertyName="ODPB7" typeName="string" priority="0"/>
        <ECProperty propertyName="ODPB8" typeName="string" priority="0"/>
        <ECProperty propertyName="ODPC1" typeName="string" priority="0"/>
        <ECProperty propertyName="ODPC2" typeName="string" priority="0"/>
        <ECProperty propertyName="ODPC3" typeName="string" priority="0"/>
        <ECProperty propertyName="ODPC4" typeName="string" priority="0"/>
        <ECProperty propertyName="ODPD1" typeName="string" priority="0"/>
        <ECProperty propertyName="ODPD2" typeName="string" priority="0"/>
        <ECProperty propertyName="POCPA13" typeName="string" priority="0"/>
        <ECProperty propertyName="POCPA4" typeName="string" priority="0"/>
        <ECProperty propertyName="POCPA51" typeName="string" priority="0"/>
        <ECProperty propertyName="POCPA53" typeName="string" priority="0"/>
        <ECProperty propertyName="POCPB1" typeName="string" priority="0"/>
        <ECProperty propertyName="POCPB2" typeName="string" priority="0"/>
        <ECProperty propertyName="POCPB3" typeName="string" priority="0"/>
        <ECProperty propertyName="POCPB4" typeName="string" priority="0"/>
        <ECProperty propertyName="POCPB5" typeName="string" priority="0"/>
        <ECProperty propertyName="POCPB61" typeName="string" priority="0"/>
        <ECProperty propertyName="POCPB62" typeName="string" priority="0"/>
        <ECProperty propertyName="POCPB7" typeName="string" priority="0"/>
        <ECProperty propertyName="POCPB8" typeName="string" priority="0"/>
        <ECProperty propertyName="POCPC1" typeName="string" priority="0"/>
        <ECProperty propertyName="POCPC2" typeName="string" priority="0"/>
        <ECProperty propertyName="POCPC3" typeName="string" priority="0"/>
        <ECProperty propertyName="POCPC4" typeName="string" priority="0"/>
        <ECProperty propertyName="POCPD1" typeName="string" priority="0"/>
        <ECProperty propertyName="POCPD2" typeName="string" priority="0"/>
        <ECProperty propertyName="RWDHLA13" typeName="string" priority="0"/>
        <ECProperty propertyName="RWDHLA4" typeName="string" priority="0"/>
        <ECProperty propertyName="RWDHLA51" typeName="string" priority="0"/>
        <ECProperty propertyName="RWDHLB2" typeName="string" priority="0"/>
        <ECProperty propertyName="RWDHLB3" typeName="string" priority="0"/>
        <ECProperty propertyName="RWDHLB4" typeName="string" priority="0"/>
        <ECProperty propertyName="RWDHLB5" typeName="string" priority="0"/>
        <ECProperty propertyName="RWDHLB7" typeName="string" priority="0"/>
        <ECProperty propertyName="RWDHLC1" typeName="string" priority="0"/>
        <ECProperty propertyName="RWDHLC2" typeName="string" priority="0"/>
        <ECProperty propertyName="RWDHLC3" typeName="string" priority="0"/>
        <ECProperty propertyName="RWDHLC4" typeName="string" priority="0"/>
        <ECProperty propertyName="RWDHLD2" typeName="string" priority="0"/>
        <ECProperty propertyName="RWDHLA53" typeName="string" priority="0"/>
        <ECProperty propertyName="RWDHLB1" typeName="string" priority="0"/>
        <ECProperty propertyName="RWDHLB61" typeName="string" priority="0"/>
        <ECProperty propertyName="RWDHLB62" typeName="string" priority="0"/>
        <ECProperty propertyName="RWDHLB8" typeName="string" priority="0"/>
        <ECProperty propertyName="RWDHLD1" typeName="string" priority="0"/>
        <ECProperty propertyName="TMOMA13" typeName="string" priority="0"/>
        <ECProperty propertyName="TMOMB2" typeName="string" priority="0"/>
        <ECProperty propertyName="TMOMB3" typeName="string" priority="0"/>
        <ECProperty propertyName="TMOMB4" typeName="string" priority="0"/>
        <ECProperty propertyName="TMOMB5" typeName="string" priority="0"/>
        <ECProperty propertyName="TMOMD1" typeName="string" priority="0"/>
    </ECEntityClass>
</ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("updateH2DynamicSchema_CV_BR_OBElementAspect.ecdb", schemaItem));

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<ECSchema schemaName="HS2" alias="HS2" version="01.00.11" displayLabel="HS2" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
    <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA"/>
    <ECCustomAttributes>
        <DynamicSchema xmlns="CoreCustomAttributes.01.00.03"/>
    </ECCustomAttributes>
    <ECEntityClass typeName="ElementAspect" displayLabel="Element Aspect" modifier="Abstract">
        <ECCustomAttributes>
            <NotSubclassableInReferencingSchemas xmlns="CoreCustomAttributes.01.00.03"/>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="ElementMultiAspect" displayLabel="Element Multi-Aspect" modifier="Abstract">
        <BaseClass>ElementAspect</BaseClass>
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
            <ShareColumns xmlns="ECDbMap.02.00.00">
                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
            </ShareColumns>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="All_AssetElementAspect" displayLabel="All_Asset">
        <BaseClass>ElementMultiAspect</BaseClass>
        <ECProperty propertyName="UAID" typeName="string" priority="0"/>
        <ECProperty propertyName="OSGB_Easting" typeName="string" priority="0"/>
        <ECProperty propertyName="OSGB_Northing" typeName="string" priority="0"/>
        <ECProperty propertyName="Name" typeName="string" priority="0"/>
        <ECProperty propertyName="Stage" typeName="string" priority="0"/>
        <ECProperty propertyName="Owner" typeName="string" priority="0"/>
        <ECProperty propertyName="Status" typeName="string" priority="0"/>
        <ECProperty propertyName="Offset" typeName="string" priority="0"/>
        <ECProperty propertyName="Reference_chainage" typeName="string" priority="0"/>
        <ECProperty propertyName="URC" typeName="string" priority="0"/>
        <ECProperty propertyName="StartChainage" typeName="string" priority="0"/>
        <ECProperty propertyName="EndChainage" typeName="string" priority="0"/>
        <ECProperty propertyName="Chainage_baseline" typeName="string" displayLabel="Chainage baseline" readOnly="true" priority="0"/>
        <ECProperty propertyName="Classification_Code" typeName="string" displayLabel="Classification Code" priority="0"/>
    </ECEntityClass>
    <ECEntityClass typeName="Primary_AssetElementAspect" displayLabel="Primary_Asset">
        <BaseClass>All_AssetElementAspect</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="CVElementAspect">
        <BaseClass>Primary_AssetElementAspect</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="CV_BRElementAspect">
        <BaseClass>CVElementAspect</BaseClass>
        <ECProperty propertyName="_02000" typeName="string" priority="0"/>
        <ECProperty propertyName="_04000" typeName="string" priority="0"/>
        <ECProperty propertyName="_1001" typeName="string" priority="0"/>
        <ECProperty propertyName="_1002" typeName="string" priority="0"/>
        <ECProperty propertyName="_1005" typeName="string" priority="0"/>
        <ECProperty propertyName="_1006" typeName="string" priority="0"/>
        <ECProperty propertyName="_1007" typeName="string" priority="0"/>
        <ECProperty propertyName="_1008" typeName="string" priority="0"/>
        <ECProperty propertyName="_1009" typeName="string" priority="0"/>
        <ECProperty propertyName="_1010" typeName="string" priority="0"/>
        <ECProperty propertyName="_1011" typeName="string" priority="0"/>
        <ECProperty propertyName="_1012" typeName="string" priority="0"/>
        <ECProperty propertyName="_1107" typeName="string" priority="0"/>
        <ECProperty propertyName="_1108" typeName="string" priority="0"/>
        <ECProperty propertyName="_1109" typeName="string" priority="0"/>
        <ECProperty propertyName="_1110" typeName="string" priority="0"/>
        <ECProperty propertyName="_1111" typeName="string" priority="0"/>
        <ECProperty propertyName="_1112" typeName="string" priority="0"/>
        <ECProperty propertyName="_1128" typeName="string" priority="0"/>
        <ECProperty propertyName="_2000" typeName="string" priority="0"/>
        <ECProperty propertyName="AccptblFrUs" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPEA13" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPEA4" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPEA51" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPEA53" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPEB2" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPEB4" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPED1" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPED2" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPFA13" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPFA4" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPFA51" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPFA53" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPFB2" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPFB4" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPFD1" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPFD2" typeName="string" priority="0"/>
        <ECProperty propertyName="APA13" typeName="string" priority="0"/>
        <ECProperty propertyName="APA4" typeName="string" priority="0"/>
        <ECProperty propertyName="APA51" typeName="string" priority="0"/>
        <ECProperty propertyName="APA53" typeName="string" priority="0"/>
        <ECProperty propertyName="APB2" typeName="string" priority="0"/>
        <ECProperty propertyName="APB4" typeName="string" priority="0"/>
        <ECProperty propertyName="APD1" typeName="string" priority="0"/>
        <ECProperty propertyName="APD2" typeName="string" priority="0"/>
        <ECProperty propertyName="AvrgCrtclHdrmm" typeName="string" priority="0"/>
        <ECProperty propertyName="BdgtCntrlAccnt" typeName="string" priority="0"/>
        <ECProperty propertyName="CBSCd" typeName="string" priority="0"/>
        <ECProperty propertyName="CfAr" typeName="string" priority="0"/>
        <ECProperty propertyName="ChmcllyUAFllMtrl" typeName="string" priority="0"/>
        <ECProperty propertyName="ChmcllyUAFllMtrlU2" typeName="string" priority="0"/>
        <ECProperty propertyName="ChmcllyUARmvl" typeName="string" priority="0"/>
        <ECProperty propertyName="ChsvHghPlsctcty" typeName="string" priority="0"/>
        <ECProperty propertyName="ChsvLwPlstcty" typeName="string" priority="0"/>
        <ECProperty propertyName="CndtnRtng" typeName="string" priority="0"/>
        <ECProperty propertyName="CndtnRtngDt" typeName="string" priority="0"/>
        <ECProperty propertyName="CnsrvtnAr" typeName="string" priority="0"/>
        <ECProperty propertyName="CntrctTyp" typeName="string" priority="0"/>
        <ECProperty propertyName="CompltnDt" typeName="string" priority="0"/>
        <ECProperty propertyName="Crtclty" typeName="string" priority="0"/>)schema" R"schema(
        <ECProperty propertyName="DsgnLf" typeName="string" priority="0"/>
        <ECProperty propertyName="DspslRqrmnts" typeName="string" priority="0"/>
        <ECProperty propertyName="DtMntnd" typeName="string" priority="0"/>
        <ECProperty propertyName="EnrnmntMtErthwrksNd" typeName="string" priority="0"/>
        <ECProperty propertyName="EnvrnmntllySnstvAr" typeName="string" priority="0"/>
        <ECProperty propertyName="EnvrnmntTyp" typeName="string" priority="0"/>
        <ECProperty propertyName="EPA13" typeName="string" priority="0"/>
        <ECProperty propertyName="EPA4" typeName="string" priority="0"/>
        <ECProperty propertyName="EPA51" typeName="string" priority="0"/>
        <ECProperty propertyName="EPA53" typeName="string" priority="0"/>
        <ECProperty propertyName="EPB2" typeName="string" priority="0"/>
        <ECProperty propertyName="EPB4" typeName="string" priority="0"/>
        <ECProperty propertyName="EPD1" typeName="string" priority="0"/>
        <ECProperty propertyName="EPD2" typeName="string" priority="0"/>
        <ECProperty propertyName="ExcvtdMtrlfrStblstn" typeName="string" priority="0"/>
        <ECProperty propertyName="ExcvtMtrlRqrngPrcssn" typeName="string" priority="0"/>
        <ECProperty propertyName="ExstngRsdHrtg" typeName="string" priority="0"/>
        <ECProperty propertyName="ExstngRsdStrctr" typeName="string" priority="0"/>
        <ECProperty propertyName="FtprntAr" typeName="string" priority="0"/>
        <ECProperty propertyName="FWA13" typeName="string" priority="0"/>
        <ECProperty propertyName="FWA4" typeName="string" priority="0"/>
        <ECProperty propertyName="FWA51" typeName="string" priority="0"/>
        <ECProperty propertyName="FWA53" typeName="string" priority="0"/>
        <ECProperty propertyName="FWB2" typeName="string" priority="0"/>
        <ECProperty propertyName="FWB4" typeName="string" priority="0"/>
        <ECProperty propertyName="FWD1" typeName="string" priority="0"/>
        <ECProperty propertyName="FWD2" typeName="string" priority="0"/>
        <ECProperty propertyName="Grade_of_Material" typeName="string" priority="0"/>
        <ECProperty propertyName="GradeofMaterial" typeName="string" priority="0"/>
        <ECProperty propertyName="GrnlrRckOrChlk" typeName="string" priority="0"/>
        <ECProperty propertyName="GrntPrd" typeName="string" priority="0"/>
        <ECProperty propertyName="GtchncllyLndscpFll" typeName="string" priority="0"/>
        <ECProperty propertyName="GtwID" typeName="string" priority="0"/>
        <ECProperty propertyName="GWPA13" typeName="string" priority="0"/>
        <ECProperty propertyName="GWPA4" typeName="string" priority="0"/>
        <ECProperty propertyName="GWPA51" typeName="string" priority="0"/>
        <ECProperty propertyName="GWPA53" typeName="string" priority="0"/>
        <ECProperty propertyName="GWPB2" typeName="string" priority="0"/>
        <ECProperty propertyName="GWPB4" typeName="string" priority="0"/>
        <ECProperty propertyName="GWPD1" typeName="string" priority="0"/>
        <ECProperty propertyName="GWPD2" typeName="string" priority="0"/>
        <ECProperty propertyName="HndvrDt" typeName="string" priority="0"/>
        <ECProperty propertyName="InspctnFrqncy" typeName="string" priority="0"/>
        <ECProperty propertyName="InspctnHstry" typeName="string" priority="0"/>
        <ECProperty propertyName="Installation_Date" typeName="string" priority="0"/>
        <ECProperty propertyName="LnghtOpr" typeName="string" priority="0"/>
        <ECProperty propertyName="Lngthm" typeName="string" priority="0"/>
        <ECProperty propertyName="LtScp" typeName="string" priority="0"/>
        <ECProperty propertyName="MEMA13A5A4" typeName="string" priority="0"/>
        <ECProperty propertyName="MndtryScp" typeName="string" priority="0"/>
        <ECProperty propertyName="Mnfctrr" typeName="string" priority="0"/>
        <ECProperty propertyName="MntncFrqncy" typeName="string" priority="0"/>
        <ECProperty propertyName="MntncUndrtkn" typeName="string" priority="0"/>
        <ECProperty propertyName="MntnncHstry" typeName="string" priority="0"/>
        <ECProperty propertyName="MntnngOrgnstn" typeName="string" priority="0"/>
        <ECProperty propertyName="MORECMA13" typeName="string" priority="0"/>
        <ECProperty propertyName="MORECMB2" typeName="string" priority="0"/>
        <ECProperty propertyName="MORECMB4" typeName="string" priority="0"/>
        <ECProperty propertyName="MORENMA13" typeName="string" priority="0"/>
        <ECProperty propertyName="MORENMB2" typeName="string" priority="0"/>
        <ECProperty propertyName="MORENMB4" typeName="string" priority="0"/>
        <ECProperty propertyName="MOREUMA13" typeName="string" priority="0"/>
        <ECProperty propertyName="MOREUMB2" typeName="string" priority="0"/>
        <ECProperty propertyName="MOREUMB4" typeName="string" priority="0"/>
        <ECProperty propertyName="MthdOfClcltn" typeName="string" priority="0"/>
        <ECProperty propertyName="Mtrl" typeName="string" priority="0"/>
        <ECProperty propertyName="MxmmHghtAbvRlLvl" typeName="string" priority="0"/>
        <ECProperty propertyName="NHWDA13" typeName="string" priority="0"/>
        <ECProperty propertyName="NHWDA4" typeName="string" priority="0"/>
        <ECProperty propertyName="NHWDA51" typeName="string" priority="0"/>
        <ECProperty propertyName="NHWDA53" typeName="string" priority="0"/>
        <ECProperty propertyName="NHWDB2" typeName="string" priority="0"/>
        <ECProperty propertyName="NHWDD1" typeName="string" priority="0"/>)schema" R"schema(
        <ECProperty propertyName="NHWDD2" typeName="string" priority="0"/>
        <ECProperty propertyName="NtExcvtdVlm" typeName="string" priority="0"/>
        <ECProperty propertyName="NxtInspctnDt" typeName="string" priority="0"/>
        <ECProperty propertyName="NxtMntnncDt" typeName="string" priority="0"/>
        <ECProperty propertyName="ObstclCrssd" typeName="string" priority="0"/>
        <ECProperty propertyName="PlnndUs" typeName="string" priority="0"/>
        <ECProperty propertyName="PlnnngArrngmnts" typeName="string" priority="0"/>
        <ECProperty propertyName="PrcsstClss6Mtrl" typeName="string" priority="0"/>
        <ECProperty propertyName="PSCAr" typeName="string" priority="0"/>
        <ECProperty propertyName="RskAssssmntDscrptn" typeName="string" priority="0"/>
        <ECProperty propertyName="ScpBkLvl" typeName="string" priority="0"/>
        <ECProperty propertyName="SftyRqrmnts" typeName="string" priority="0"/>
        <ECProperty propertyName="SldGlgy" typeName="string" priority="0"/>
        <ECProperty propertyName="SpcfctnNmbr" typeName="string" priority="0"/>
        <ECProperty propertyName="SpclAccssArrngmnts" typeName="string" priority="0"/>
        <ECProperty propertyName="SpclRqrmnts" typeName="string" priority="0"/>
        <ECProperty propertyName="SprfclGlgy" typeName="string" priority="0"/>
        <ECProperty propertyName="SrlBtchNmbr" typeName="string" priority="0"/>
        <ECProperty propertyName="Supplier" typeName="string" priority="0"/>
        <ECProperty propertyName="TrtmntObjctivs" typeName="string" priority="0"/>
        <ECProperty propertyName="TtlCrtAwyNmbr" typeName="string" priority="0"/>
        <ECProperty propertyName="TtlCrtAwyUnblkd" typeName="string" priority="0"/>
        <ECProperty propertyName="TypOfRtSpprtd" typeName="string" priority="0"/>
        <ECProperty propertyName="VlmBrdTnnlsAndShfts" typeName="string" priority="0"/>
        <ECProperty propertyName="VlmGrndTrtmnt" typeName="string" priority="0"/>
        <ECProperty propertyName="VlmOpnCtHghwyWrks" typeName="string" priority="0"/>
        <ECProperty propertyName="VlmOpnCtRlwyWrks" typeName="string" priority="0"/>
        <ECProperty propertyName="VlmTrndCttngsTnnls" typeName="string" priority="0"/>
        <ECProperty propertyName="WidthAvrgm" typeName="string" priority="0"/>
        <ECProperty propertyName="WrrntyExpryDt" typeName="string" priority="0"/>
        <ECProperty propertyName="WrrntyPrd" typeName="string" priority="0"/>
        <ECProperty propertyName="WrrntyStrtDt" typeName="string" priority="0"/>
    </ECEntityClass>
    <ECEntityClass typeName="CV_BR_OBElementAspect">
        <BaseClass>CV_BRElementAspect</BaseClass>
        <ECProperty propertyName="_02000" typeName="string" priority="0"/>
        <ECProperty propertyName="_04000" typeName="string" priority="0"/>
        <ECProperty propertyName="_06000" typeName="string" priority="0"/>
        <ECProperty propertyName="_1013" typeName="string" priority="0"/>
        <ECProperty propertyName="_1014" typeName="string" priority="0"/>
        <ECProperty propertyName="_1015" typeName="string" priority="0"/>
        <ECProperty propertyName="_1016" typeName="string" priority="0"/>
        <ECProperty propertyName="_1017" typeName="string" priority="0"/>
        <ECProperty propertyName="_1018" typeName="string" priority="0"/>
        <ECProperty propertyName="_1113" typeName="string" priority="0"/>
        <ECProperty propertyName="_1114" typeName="string" priority="0"/>
        <ECProperty propertyName="_1115" typeName="string" priority="0"/>
        <ECProperty propertyName="_1116" typeName="string" priority="0"/>
        <ECProperty propertyName="_1117" typeName="string" priority="0"/>
        <ECProperty propertyName="_1118" typeName="string" priority="0"/>
        <ECProperty propertyName="_1119" typeName="string" priority="0"/>
        <ECProperty propertyName="_1120" typeName="string" priority="0"/>
        <ECProperty propertyName="_1121" typeName="string" priority="0"/>
        <ECProperty propertyName="_1122" typeName="string" priority="0"/>
        <ECProperty propertyName="_1123" typeName="string" priority="0"/>
        <ECProperty propertyName="_1124" typeName="string" priority="0"/>
        <ECProperty propertyName="_1125" typeName="string" priority="0"/>
        <ECProperty propertyName="_1126" typeName="string" priority="0"/>
        <ECProperty propertyName="_2000" typeName="string" priority="0"/>
        <ECProperty propertyName="_2001" typeName="string" priority="0"/>
        <ECProperty propertyName="_2002" typeName="string" priority="0"/>
        <ECProperty propertyName="_2003" typeName="string" priority="0"/>
        <ECProperty propertyName="_2004" typeName="string" priority="0"/>
        <ECProperty propertyName="A52" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPEB1" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPEB3" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPEB5" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPEB61" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPEB62" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPEB7" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPEB8" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPEC1" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPEC2" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPEC3" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPEC4" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPFB1" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPFB3" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPFB5" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPFB61" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPFB62" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPFB7" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPFB8" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPFC1" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPFC2" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPFC3" typeName="string" priority="0"/>
        <ECProperty propertyName="ADPFC4" typeName="string" priority="0"/>
        <ECProperty propertyName="AlgnmPln" typeName="string" priority="0"/>
        <ECProperty propertyName="AnglSkw" typeName="string" priority="0"/>
        <ECProperty propertyName="APB1" typeName="string" priority="0"/>
        <ECProperty propertyName="APB3" typeName="string" priority="0"/>
        <ECProperty propertyName="APB5" typeName="string" priority="0"/>
        <ECProperty propertyName="APB61" typeName="string" priority="0"/>
        <ECProperty propertyName="APB62" typeName="string" priority="0"/>
        <ECProperty propertyName="APB7" typeName="string" priority="0"/>
        <ECProperty propertyName="APB8" typeName="string" priority="0"/>
        <ECProperty propertyName="APC1" typeName="string" priority="0"/>
        <ECProperty propertyName="APC2" typeName="string" priority="0"/>
        <ECProperty propertyName="APC3" typeName="string" priority="0"/>
        <ECProperty propertyName="APC4" typeName="string" priority="0"/>
        <ECProperty propertyName="ArOtsndn" typeName="string" priority="0"/>
        <ECProperty propertyName="BlncDsgnElmntHghCL2" typeName="string" priority="0"/>
        <ECProperty propertyName="BlncinDsfnElmntAll" typeName="string" priority="0"/>
        <ECProperty propertyName="BlncinDsgnElmntCL1_3" typeName="string" priority="0"/>
        <ECProperty propertyName="BlncInDsgnElmntCL6" typeName="string" priority="0"/>
        <ECProperty propertyName="BlncinDsgnElmntLwCL2" typeName="string" priority="0"/>
        <ECProperty propertyName="Blncindsgnelmntsbsl" typeName="string" priority="0"/>
        <ECProperty propertyName="BlncInDsgnElmntTpsl" typeName="string" priority="0"/>
        <ECProperty propertyName="BlncInDsngElmntCL4" typeName="string" priority="0"/>
        <ECProperty propertyName="ClrncHrz" typeName="string" priority="0"/>
        <ECProperty propertyName="ClrncVrt" typeName="string" priority="0"/>
        <ECProperty propertyName="CM_Chg" typeName="string" priority="0"/>
        <ECProperty propertyName="CM_CRT" typeName="string" priority="0"/>
        <ECProperty propertyName="CM_ISCI" typeName="string" priority="0"/>
        <ECProperty propertyName="CM_StdBL" typeName="string" priority="0"/>
        <ECProperty propertyName="CmrRqstFrmID" typeName="string" priority="0"/>
        <ECProperty propertyName="CrssngFtrID" typeName="string" priority="0"/>
        <ECProperty propertyName="CStrtDt" typeName="string" priority="0"/>
        <ECProperty propertyName="DpthDck" typeName="string" priority="0"/>
        <ECProperty propertyName="DSAsst" typeName="string" priority="0"/>
        <ECProperty propertyName="ElmntPssngOvr" typeName="string" priority="0"/>
        <ECProperty propertyName="ElvtnAlg" typeName="string" priority="0"/>
        <ECProperty propertyName="Env_ArOtsndn" typeName="string" priority="0"/>
        <ECProperty propertyName="EPB1" typeName="string" priority="0"/>
        <ECProperty propertyName="EPB3" typeName="string" priority="0"/>
        <ECProperty propertyName="EPB5" typeName="string" priority="0"/>
        <ECProperty propertyName="EPB61" typeName="string" priority="0"/>
        <ECProperty propertyName="EPB62" typeName="string" priority="0"/>
        <ECProperty propertyName="EPB7" typeName="string" priority="0"/>
        <ECProperty propertyName="EPB8" typeName="string" priority="0"/>
        <ECProperty propertyName="EPC1" typeName="string" priority="0"/>
        <ECProperty propertyName="EPC2" typeName="string" priority="0"/>
        <ECProperty propertyName="EPC3" typeName="string" priority="0"/>
        <ECProperty propertyName="EPC4" typeName="string" priority="0"/>
        <ECProperty propertyName="FWB1" typeName="string" priority="0"/>
        <ECProperty propertyName="FWB3" typeName="string" priority="0"/>
        <ECProperty propertyName="FWB5" typeName="string" priority="0"/>
        <ECProperty propertyName="FWB61" typeName="string" priority="0"/>
        <ECProperty propertyName="FWB62" typeName="string" priority="0"/>
        <ECProperty propertyName="FWB7" typeName="string" priority="0"/>
        <ECProperty propertyName="FWB8" typeName="string" priority="0"/>
        <ECProperty propertyName="FWC1" typeName="string" priority="0"/>
        <ECProperty propertyName="FWC2" typeName="string" priority="0"/>
        <ECProperty propertyName="FWC3" typeName="string" priority="0"/>
        <ECProperty propertyName="FWC4" typeName="string" priority="0"/>
        <ECProperty propertyName="GrdntMn" typeName="string" priority="0"/>
        <ECProperty propertyName="GrdntMx" typeName="string" priority="0"/>
        <ECProperty propertyName="GWPB1" typeName="string" priority="0"/>
        <ECProperty propertyName="GWPB3" typeName="string" priority="0"/>
        <ECProperty propertyName="GWPB5" typeName="string" priority="0"/>
        <ECProperty propertyName="GWPB61" typeName="string" priority="0"/>
        <ECProperty propertyName="GWPB62" typeName="string" priority="0"/>
        <ECProperty propertyName="GWPB7" typeName="string" priority="0"/>
        <ECProperty propertyName="GWPB8" typeName="string" priority="0"/>
        <ECProperty propertyName="GWPC1" typeName="string" priority="0"/>
        <ECProperty propertyName="GWPC2" typeName="string" priority="0"/>
        <ECProperty propertyName="GWPC3" typeName="string" priority="0"/>
        <ECProperty propertyName="GWPC4" typeName="string" priority="0"/>
        <ECProperty propertyName="HghwyAthrty" typeName="string" priority="0"/>
        <ECProperty propertyName="HlthSftyRtng" typeName="string" priority="0"/>
        <ECProperty propertyName="HWDA13" typeName="string" priority="0"/>
        <ECProperty propertyName="HWDA4" typeName="string" priority="0"/>
        <ECProperty propertyName="HWDA51" typeName="string" priority="0"/>
        <ECProperty propertyName="HWDA53" typeName="string" priority="0"/>
        <ECProperty propertyName="HWDB1" typeName="string" priority="0"/>
        <ECProperty propertyName="HWDB2" typeName="string" priority="0"/>
        <ECProperty propertyName="HWDB3" typeName="string" priority="0"/>
        <ECProperty propertyName="HWDB4" typeName="string" priority="0"/>
        <ECProperty propertyName="HWDB5" typeName="string" priority="0"/>
        <ECProperty propertyName="HWDB61" typeName="string" priority="0"/>
        <ECProperty propertyName="HWDB62" typeName="string" priority="0"/>
        <ECProperty propertyName="HWDB7" typeName="string" priority="0"/>
        <ECProperty propertyName="HWDB8" typeName="string" priority="0"/>
        <ECProperty propertyName="HWDC1" typeName="string" priority="0"/>
        <ECProperty propertyName="HWDC2" typeName="string" priority="0"/>
        <ECProperty propertyName="HWDC3" typeName="string" priority="0"/>
        <ECProperty propertyName="HWDC4" typeName="string" priority="0"/>
        <ECProperty propertyName="HWDD1" typeName="string" priority="0"/>)schema" R"schema(
        <ECProperty propertyName="HWDD2" typeName="string" priority="0"/>
        <ECProperty propertyName="LnghtSpn" typeName="string" priority="0"/>
        <ECProperty propertyName="Lngthm" typeName="string" priority="0"/>
        <ECProperty propertyName="MORECMB3" typeName="string" priority="0"/>
        <ECProperty propertyName="MORECMB5" typeName="string" priority="0"/>
        <ECProperty propertyName="MORENMB3" typeName="string" priority="0"/>
        <ECProperty propertyName="MORENMB5" typeName="string" priority="0"/>
        <ECProperty propertyName="MOREUMB3" typeName="string" priority="0"/>
        <ECProperty propertyName="MOREUMB5" typeName="string" priority="0"/>
        <ECProperty propertyName="MxmmLdng" typeName="string" priority="0"/>
        <ECProperty propertyName="NHWDB1" typeName="string" priority="0"/>
        <ECProperty propertyName="NHWDB3" typeName="string" priority="0"/>
        <ECProperty propertyName="NHWDB4" typeName="string" priority="0"/>
        <ECProperty propertyName="NHWDB5" typeName="string" priority="0"/>
        <ECProperty propertyName="NHWDB61" typeName="string" priority="0"/>
        <ECProperty propertyName="NHWDB62" typeName="string" priority="0"/>
        <ECProperty propertyName="NHWDB7" typeName="string" priority="0"/>
        <ECProperty propertyName="NHWDB8" typeName="string" priority="0"/>
        <ECProperty propertyName="NHWDC1" typeName="string" priority="0"/>
        <ECProperty propertyName="NHWDC2" typeName="string" priority="0"/>
        <ECProperty propertyName="NHWDC3" typeName="string" priority="0"/>
        <ECProperty propertyName="NHWDC4" typeName="string" priority="0"/>
        <ECProperty propertyName="NtsOnErthwrksBlncClc" typeName="string" priority="0"/>
        <ECProperty propertyName="ODPA13" typeName="string" priority="0"/>
        <ECProperty propertyName="ODPA4" typeName="string" priority="0"/>
        <ECProperty propertyName="ODPA51" typeName="string" priority="0"/>
        <ECProperty propertyName="ODPA53" typeName="string" priority="0"/>
        <ECProperty propertyName="ODPB1" typeName="string" priority="0"/>
        <ECProperty propertyName="ODPB2" typeName="string" priority="0"/>
        <ECProperty propertyName="ODPB3" typeName="string" priority="0"/>
        <ECProperty propertyName="ODPB4" typeName="string" priority="0"/>
        <ECProperty propertyName="ODPB5" typeName="string" priority="0"/>
        <ECProperty propertyName="ODPB61" typeName="string" priority="0"/>
        <ECProperty propertyName="ODPB62" typeName="string" priority="0"/>
        <ECProperty propertyName="ODPB7" typeName="string" priority="0"/>
        <ECProperty propertyName="ODPB8" typeName="string" priority="0"/>
        <ECProperty propertyName="ODPC1" typeName="string" priority="0"/>
        <ECProperty propertyName="ODPC2" typeName="string" priority="0"/>
        <ECProperty propertyName="ODPC3" typeName="string" priority="0"/>
        <ECProperty propertyName="ODPC4" typeName="string" priority="0"/>
        <ECProperty propertyName="ODPD1" typeName="string" priority="0"/>
        <ECProperty propertyName="ODPD2" typeName="string" priority="0"/>
        <ECProperty propertyName="Ownr" typeName="string" priority="0"/>
        <ECProperty propertyName="POCPA13" typeName="string" priority="0"/>
        <ECProperty propertyName="POCPA4" typeName="string" priority="0"/>
        <ECProperty propertyName="POCPA51" typeName="string" priority="0"/>
        <ECProperty propertyName="POCPA53" typeName="string" priority="0"/>
        <ECProperty propertyName="POCPB1" typeName="string" priority="0"/>
        <ECProperty propertyName="POCPB2" typeName="string" priority="0"/>
        <ECProperty propertyName="POCPB3" typeName="string" priority="0"/>
        <ECProperty propertyName="POCPB4" typeName="string" priority="0"/>
        <ECProperty propertyName="POCPB5" typeName="string" priority="0"/>
        <ECProperty propertyName="POCPB61" typeName="string" priority="0"/>
        <ECProperty propertyName="POCPB62" typeName="string" priority="0"/>
        <ECProperty propertyName="POCPB7" typeName="string" priority="0"/>
        <ECProperty propertyName="POCPB8" typeName="string" priority="0"/>
        <ECProperty propertyName="POCPC1" typeName="string" priority="0"/>
        <ECProperty propertyName="POCPC2" typeName="string" priority="0"/>
        <ECProperty propertyName="POCPC3" typeName="string" priority="0"/>
        <ECProperty propertyName="POCPC4" typeName="string" priority="0"/>
        <ECProperty propertyName="POCPD1" typeName="string" priority="0"/>
        <ECProperty propertyName="POCPD2" typeName="string" priority="0"/>
        <ECProperty propertyName="RdNm" typeName="string" priority="0"/>
        <ECProperty propertyName="RdNmbr" typeName="string" priority="0"/>
        <ECProperty propertyName="RdSrfc" typeName="string" priority="0"/>
        <ECProperty propertyName="RWDHLA13" typeName="string" priority="0"/>
        <ECProperty propertyName="RWDHLA4" typeName="string" priority="0"/>
        <ECProperty propertyName="RWDHLA51" typeName="string" priority="0"/>
        <ECProperty propertyName="RWDHLA53" typeName="string" priority="0"/>
        <ECProperty propertyName="RWDHLB1" typeName="string" priority="0"/>
        <ECProperty propertyName="RWDHLB2" typeName="string" priority="0"/>
        <ECProperty propertyName="RWDHLB3" typeName="string" priority="0"/>
        <ECProperty propertyName="RWDHLB4" typeName="string" priority="0"/>
        <ECProperty propertyName="RWDHLB5" typeName="string" priority="0"/>
        <ECProperty propertyName="RWDHLB61" typeName="string" priority="0"/>
        <ECProperty propertyName="RWDHLB62" typeName="string" priority="0"/>
        <ECProperty propertyName="RWDHLB7" typeName="string" priority="0"/>
        <ECProperty propertyName="RWDHLB8" typeName="string" priority="0"/>
        <ECProperty propertyName="RWDHLC1" typeName="string" priority="0"/>
        <ECProperty propertyName="RWDHLC2" typeName="string" priority="0"/>
        <ECProperty propertyName="RWDHLC3" typeName="string" priority="0"/>
        <ECProperty propertyName="RWDHLC4" typeName="string" priority="0"/>
        <ECProperty propertyName="RWDHLD1" typeName="string" priority="0"/>
        <ECProperty propertyName="RWDHLD2" typeName="string" priority="0"/>
        <ECProperty propertyName="SCP_ScpBkLvl" typeName="string" priority="0"/>
        <ECProperty propertyName="SctrCd" typeName="string" priority="0"/>
        <ECProperty propertyName="SpdLmt" typeName="string" priority="0"/>
        <ECProperty propertyName="SprtTyp" typeName="string" priority="0"/>
        <ECProperty propertyName="SrfcngClr" typeName="string" priority="0"/>
        <ECProperty propertyName="SrvydTrffcFlw" typeName="string" priority="0"/>
        <ECProperty propertyName="StrctrlT" typeName="string" priority="0"/>
        <ECProperty propertyName="StrtDt" typeName="string" priority="0"/>
        <ECProperty propertyName="ThrdPrty" typeName="string" priority="0"/>
        <ECProperty propertyName="TMOMA13" typeName="string" priority="0"/>
        <ECProperty propertyName="TMOMB2" typeName="string" priority="0"/>
        <ECProperty propertyName="TMOMB3" typeName="string" priority="0"/>
        <ECProperty propertyName="TMOMB4" typeName="string" priority="0"/>
        <ECProperty propertyName="TMOMB5" typeName="string" priority="0"/>
        <ECProperty propertyName="TMOMD1" typeName="string" priority="0"/>
        <ECProperty propertyName="TypFndtn" typeName="string" priority="0"/>
        <ECProperty propertyName="Wdthm" typeName="string" priority="0"/>
        <ECProperty propertyName="WdthMn" typeName="string" priority="0"/>
        <ECProperty propertyName="WdthMx" typeName="string" priority="0"/>
    </ECEntityClass>
</ECSchema>
        )schema");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));
    }

END_ECDBUNITTESTS_NAMESPACE
