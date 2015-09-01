//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/OCIGeoRasterWrapper.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class OCIGeoRasterWrapper
//-----------------------------------------------------------------------------
#include <ImagePPInternal/hstdcpp.h>
#include <ImagePP/all/h/HFCMacros.h>

#include <oracle/oci.h>

#include <Imagepp/all/h/OCIGeoRasterWrapper.h>

#include <sstream>
using namespace std;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static WString s_ToWString(OraText const* in)
    {
    HPRECONDITION(in!=NULL); 
    WString out;
    BeStringUtilities::Utf16ToWChar (out, (Utf16CP)in);
    return out;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static OraText* s_AllocOraTextP(WStringCR in, ub4& bufLen)
    {
    size_t allocatedBufLen = static_cast<ub4>((in.length() + 1) * sizeof(Utf16Char));
    OraText* pBuffer = new OraText[allocatedBufLen];
    BeStringUtilities::WCharToUtf16(reinterpret_cast<Utf16Char*>(pBuffer),(in.length() + 1),in.c_str());
    ub4 strLen = static_cast<ub4>(in.length());
    //Buffer length returned must be in Nb of Characters but express in byte count without counting the NULL terminated character.
    bufLen = static_cast<ub4>(strLen * sizeof(Utf16Char));
    return pBuffer;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static OraText* s_AllocOraTextP(size_t strLen)
    {
    size_t bufLen = (strLen + 1) * sizeof(Utf16Char);
    OraText* pBuffer = new OraText[bufLen];
    return pBuffer;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void checkerr0(SDOGeoRasterWrapper::OracleError* po_pError, void *handle, ub4 htype, sword status)
    {
    switch (status)
        {
        case OCI_SUCCESS:
            if (po_pError!=NULL)
                {
                po_pError->m_ErrorCode = OCI_SUCCESS;
                }
            break;
        case OCI_SUCCESS_WITH_INFO:
            if (po_pError!=NULL)
                {
                po_pError->m_ErrorCode = OCI_SUCCESS_WITH_INFO;
                po_pError->m_ErrorMsg = L"Error - OCI SUCCESS WITH INFO";
                }
            break;
        case OCI_NEED_DATA:
            if (po_pError!=NULL)
                {
                po_pError->m_ErrorCode = OCI_NEED_DATA;
                po_pError->m_ErrorMsg = L"Error - OCI NEED DATA";
                }
            break;
        case OCI_NO_DATA:
            if (po_pError!=NULL)
                {
                po_pError->m_ErrorCode = OCI_NO_DATA;
                po_pError->m_ErrorMsg = L"Error - OCI NO DATA";
                }
            break;
        case OCI_ERROR:
            if (po_pError!=NULL)
                {
                if (handle)
                    {
                    sb4 errcode=0;
                    OraText errbuf[2048];
                    OCIErrorGet(handle,1,NULL,&errcode,errbuf,(ub4)sizeof(errbuf),htype);
                    po_pError->m_ErrorCode = errcode;
                    po_pError->m_ErrorMsg = s_ToWString(errbuf);
                    }
                else
                    {
                    po_pError->m_ErrorCode = OCI_ERROR;
                    po_pError->m_ErrorMsg = L"Error - Unable to extract detailed diagnostic information";
                    }
                }
            throw OCI_ERROR;
            break;
        case OCI_INVALID_HANDLE:
            if (po_pError!=NULL)
                {
                po_pError->m_ErrorCode = OCI_INVALID_HANDLE;
                po_pError->m_ErrorMsg = L"Error - OCI INVALID HANDLE";
                }
            throw OCI_INVALID_HANDLE;
            break;
        case OCI_STILL_EXECUTING:
            if (po_pError!=NULL)
                {
                po_pError->m_ErrorCode = OCI_STILL_EXECUTING;
                po_pError->m_ErrorMsg = L"Error - OCI STILL EXECUTE";
                }
            break;
        case OCI_CONTINUE:
            if (po_pError!=NULL)
                {
                po_pError->m_ErrorCode = OCI_CONTINUE;
                po_pError->m_ErrorMsg = L"Error - OCI CONTINUE";
                }
            break;
        default:
            break;
        }
    }

#define checkerr(errorHandle, errhp, status) checkerr0((errorHandle), (errhp), OCI_HTYPE_ERROR, (status))
#define checkenv(errorHandle, errhp, status) checkerr0((errorHandle), (errhp), OCI_HTYPE_ENV, (status))



//-----------------------------------------------------------------------------
// OCIConnection
//-----------------------------------------------------------------------------
class OCIConnection : public ImagePP::ImagePPHost::HostObjectBase
    {
    HFC_DECLARE_HOSTOBJECT_SINGLETON(ImagePP::ImageppLib,OCIConnection)

public:
    OCIConnection():m_pRasterId (s_AllocOraTextP(L":RASTERID",m_pRasterIdBufLen)),
                    m_pRESOLUTION (s_AllocOraTextP(L":RESOLUTION",m_pRESOLUTIONBufLen)),
                    m_pBAND (s_AllocOraTextP(L":BAND",m_pBANDBufLen)),
                    m_pPOSX (s_AllocOraTextP(L":POSX",m_pPOSXBufLen)),
                    m_pPOSY (s_AllocOraTextP(L":POSY",m_pPOSYBufLen))
        {
        m_pEnv      = 0;
        m_pError    = 0;
        m_pSvcCtx   = 0;
        }

    ~OCIConnection()
        {
        if (IsConnected())
            Disconnect();
        }

    bool       IsConnected     ();

    bool       Connect         (WStringCR                      pi_rUser,
                                 WStringCR                      pi_rPassword,
                                 WStringCR                      pi_rDatabase,
                                 SDOGeoRasterWrapper::OracleError*  po_pError = 0);

    bool       Connect         (WStringCR                      pi_rConnectionString,
                                 SDOGeoRasterWrapper::OracleError*  po_pError = 0);

    bool       Disconnect      ();

    OracleOCIConnectionP GetConnection();

    OCIEnv*     GetOCIEnv()     {return m_pEnv;}
    OCIError*   GetOCIError()   {return m_pError;}
    OCISvcCtx*  GetOCISvcCtx()  {return m_pSvcCtx;}

    WString CreateGetBlockRequest(WStringCR rasterDataTable)
        {
        //If you change this request, you should also review BindGetBlockVariables below for bind name variables
        wostringstream GetBlockRequest;
        GetBlockRequest << L"select rasterblock from " << rasterDataTable.c_str();
        GetBlockRequest << L" where rasterid= :RASTERID and pyramidlevel= :RESOLUTION";
        GetBlockRequest << L" and bandBlockNumber= :BAND";
        GetBlockRequest << L" and columnblocknumber= :POSX and rowblocknumber= :POSY";
        return WString(GetBlockRequest.str().c_str());
        }

    void BindGetBlockVariables(OCIStmt *stmtp, OraText* pRasterID, sb4 bufLen, unsigned short* pi_Resolution, unsigned short* pi_Band, uint32_t* pi_PosX, uint32_t* pi_PosY)
        {
        SDOGeoRasterWrapper::OracleError OraError;

        //Bind variables
        OCIBind* bind_RASTERID;
        checkerr(&OraError, m_pError, OCIBindByName(stmtp,&bind_RASTERID,m_pError,
            m_pRasterId.get(),m_pRasterIdBufLen,pRasterID,bufLen,SQLT_STR,0,0,0,0,0,OCI_DEFAULT));

        OCIBind* bind_RESOLUTION;
        checkerr(&OraError, m_pError,OCIBindByName(stmtp,&bind_RESOLUTION,m_pError,
            m_pRESOLUTION.get(),m_pRESOLUTIONBufLen,pi_Resolution,(sb4)sizeof(*pi_Resolution),SQLT_UIN,0,0,0,0,0,OCI_DEFAULT));

        OCIBind* bind_BAND;
        checkerr(&OraError,m_pError,OCIBindByName(stmtp,&bind_BAND,m_pError,
            m_pBAND.get(),m_pBANDBufLen,pi_Band,(sb4)sizeof(*pi_Band),SQLT_UIN,0,0,0,0,0,OCI_DEFAULT));

        OCIBind* bind_POSX;
        checkerr(&OraError, m_pError, OCIBindByName(stmtp,&bind_POSX,m_pError,
            m_pPOSX.get(),m_pPOSXBufLen,pi_PosX,(sb4)sizeof(*pi_PosX),SQLT_UIN,0,0,0,0,0,OCI_DEFAULT));

        OCIBind* bind_POSY;
        checkerr(&OraError, m_pError,OCIBindByName(stmtp,&bind_POSY,m_pError,
            m_pPOSY.get(),m_pPOSYBufLen,pi_PosY,(sb4)sizeof(*pi_PosY),SQLT_UIN,0,0,0,0,0,OCI_DEFAULT));
        }


private:
    OCIEnv*                             m_pEnv;
    OCIError*                           m_pError;
    OCISvcCtx*                          m_pSvcCtx;

    unique_ptr<OraText>                   m_pRasterId;
    unique_ptr<OraText>                   m_pRESOLUTION;
    unique_ptr<OraText>                   m_pBAND;
    unique_ptr<OraText>                   m_pPOSX;
    unique_ptr<OraText>                   m_pPOSY;
    ub4                                 m_pRasterIdBufLen;
    ub4                                 m_pRESOLUTIONBufLen;
    ub4                                 m_pBANDBufLen;
    ub4                                 m_pPOSXBufLen;
    ub4                                 m_pPOSYBufLen;
    };

//-----------------------------------------------------------------------------
// public
// IsConnected
//-----------------------------------------------------------------------------
OracleOCIConnectionP OCIConnection::GetConnection()
    {
    return this;
    }

//-----------------------------------------------------------------------------
// public
// IsConnected
//-----------------------------------------------------------------------------
bool OCIConnection::IsConnected()
    {
    return m_pEnv != 0 &&  m_pSvcCtx != 0;
    }


//-----------------------------------------------------------------------------
// public
// Connect
//-----------------------------------------------------------------------------
bool OCIConnection::Connect(WStringCR                           pi_rUser,
                            WStringCR                           pi_rPassword,
                            WStringCR                           pi_rDatabase,
                            SDOGeoRasterWrapper::OracleError*   po_pError)
    {
    try
        {
        ub4               userBufLen;
        unique_ptr<OraText> pUser (s_AllocOraTextP(pi_rUser,userBufLen));
        ub4               passwordBufLen;
        unique_ptr<OraText> pPassword (s_AllocOraTextP(pi_rPassword,passwordBufLen));
        ub4               databaseBufLen;
        unique_ptr<OraText> pDatabase (s_AllocOraTextP(pi_rDatabase,databaseBufLen));
        
        //Initilize the mode to be the threaded and object environment
        checkenv(po_pError,m_pEnv,OCIEnvNlsCreate(&m_pEnv,OCI_THREADED|OCI_OBJECT,NULL,NULL,NULL,NULL,(size_t)0,(void**)NULL,OCI_UTF16ID,OCI_UTF16ID));

        //allocation an error handle
        checkerr(po_pError, GetOCIError(), OCIHandleAlloc(m_pEnv, (void**)&m_pError, OCI_HTYPE_ERROR, 0, (void**)0));

        //NOTICE: If an application uses this logon method, the service context, server, and user session handles will all be read-only;
        //the application cannot switch session or transaction by changing the appropriate attributes of the service context handle by means of an OCIAttrSet() call.
        //This is what we support for now...
        checkerr(po_pError, GetOCIError(), OCILogon2(m_pEnv,m_pError,&m_pSvcCtx,pUser.get(), userBufLen, pPassword.get(), passwordBufLen, pDatabase.get(), databaseBufLen, OCI_LOGON2_STMTCACHE));
        }
    catch (...)
        {
        Disconnect();
        }
    return IsConnected();
    }

//-----------------------------------------------------------------------------
// public
// Connect
//-----------------------------------------------------------------------------
bool OCIConnection::Connect(WStringCR pi_rConnectionString,
                              SDOGeoRasterWrapper::OracleError* po_pError)
    {
    WString User;
    WString Password;
    WString Database;
    WString::size_type Pos = pi_rConnectionString.find(L'/');
    WString::size_type Pos2;
    if (Pos != WString::npos)
        {
        User = pi_rConnectionString.substr(0, Pos);
        ++Pos;

        if ((Pos2 = pi_rConnectionString.find(L'@', Pos)) != WString::npos)
            {
            Password = pi_rConnectionString.substr(Pos, Pos2 - Pos);
            Database = pi_rConnectionString.substr(Pos2 + 1);
            }
        else
            Password = pi_rConnectionString.substr(Pos);
        }
    else if ((Pos = pi_rConnectionString.find(L'@')) != WString::npos)
        {
        User = pi_rConnectionString.substr(0, Pos);
        Database = pi_rConnectionString.substr(Pos + 1);
        }
    else
        {
        User = pi_rConnectionString;
        }
    return Connect(User, Password, Database, po_pError);
    }


//-----------------------------------------------------------------------------
// public
// Disconnect
//-----------------------------------------------------------------------------
bool OCIConnection::Disconnect()
    {
    HPRECONDITION(IsConnected());

    if (m_pSvcCtx != 0)
        {
        OCILogoff(m_pSvcCtx,m_pError);
        m_pSvcCtx=0;
        }
    if (m_pError!=0)
        {
        OCIHandleFree(m_pError,OCI_HTYPE_ERROR);
        m_pError = 0;
        }
    if (m_pEnv!=0)
        {
        OCITerminate(OCI_DEFAULT);
        m_pEnv = 0;
        }

    return true;
    }

//-----------------------------------------------------------------------------
// static variable
//-----------------------------------------------------------------------------
HFC_IMPLEMENT_HOSTOBJECT_SINGLETON(ImagePP::ImageppLib,OCIConnection)

//-----------------------------------------------------------------------------
// static method
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// public
// IsConnected
//-----------------------------------------------------------------------------
bool OCIGeoRasterWrapper::IsConnected()
    {
    return OCIConnection::GetInstance()->IsConnected();
    }


//-----------------------------------------------------------------------------
// public
// Connect
//-----------------------------------------------------------------------------
bool OCIGeoRasterWrapper::Connect(WStringCR       pi_rUser,
                                    WStringCR       pi_rPassword,
                                    WStringCR       pi_rDatabase,
                                    OracleError*        po_pError)
    {
    return OCIConnection::GetInstance()->Connect(pi_rUser, pi_rPassword, pi_rDatabase, po_pError);
    }

//-----------------------------------------------------------------------------
// public
// Connect
//-----------------------------------------------------------------------------
bool OCIGeoRasterWrapper::Connect(WStringCR  pi_rConnectionString,
                                    OracleError*    po_pError)
    {
    return OCIConnection::GetInstance()->Connect(pi_rConnectionString, po_pError);
    }


//-----------------------------------------------------------------------------
// public
// Disconnect
//-----------------------------------------------------------------------------
bool OCIGeoRasterWrapper::Disconnect()
    {
    return OCIConnection::GetInstance()->Disconnect();
    }

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
OCIGeoRasterWrapper::OCIGeoRasterWrapper(WStringCR  pi_rTableName,
                                           WStringCR  pi_rColumnName,
                                           WStringCR  pi_rImageID,
                                           WStringCR  pi_rRasterDataTableName,
                                           const Utf16Char*   pi_pXMLGeoRasterHeader,
                                           size_t         pi_XMLSize)
: SDOGeoRasterWrapper(pi_rTableName,
                        pi_rColumnName,
                        pi_rImageID,
                        pi_rRasterDataTableName,
                        pi_pXMLGeoRasterHeader,
                        pi_XMLSize),
m_pConnection(OCIConnection::GetInstance()->GetConnection())
    {
    HPRECONDITION(m_pConnection != 0);
    HPRECONDITION(pi_pXMLGeoRasterHeader == 0 || !pi_rRasterDataTableName.empty());

    if (pi_pXMLGeoRasterHeader == 0)
        {
        // query the header
        WString Request;

        Request = L"select t." + pi_rColumnName + L".rasterdatatable, t." + pi_rColumnName + L".metadata.getClobVal() from ";
        Request += pi_rTableName + L" t where t." + pi_rColumnName + L".rasterid = " + pi_rImageID;

        ub4               requestBufLen;
        unique_ptr<OraText> pRequest (s_AllocOraTextP(Request,requestBufLen));

        //Prepare the statement
        SDOGeoRasterWrapper::OracleError OraError;
        OCIStmt      *stmtp;
        checkerr(&OraError, m_pConnection->GetOCIError(), OCIHandleAlloc(m_pConnection->GetOCIEnv(), (void**)&stmtp, OCI_HTYPE_STMT, 0, (void**)0));
        checkerr(&OraError, m_pConnection->GetOCIError(), OCIStmtPrepare(stmtp,m_pConnection->GetOCIError(),pRequest.get(),requestBufLen,OCI_NTV_SYNTAX,OCI_DEFAULT));

        //Execute statement
        checkerr(&OraError, m_pConnection->GetOCIError(),OCIStmtExecute (m_pConnection->GetOCISvcCtx(), stmtp, m_pConnection->GetOCIError(), 0, 0, (OCISnapshot *)0, (OCISnapshot *)0, OCI_DEFAULT));

        unique_ptr<OraText> pRasterDataTable;

        // Request a parameter descriptor for position 1 in the select-list 
        OCIParam     *mypard = (OCIParam *) 0;
        sb4          parm_status;
        parm_status = OCIParamGet((void *)stmtp, OCI_HTYPE_STMT, m_pConnection->GetOCIError(),(void **)&mypard, (ub4) 1);
        if (parm_status==OCI_SUCCESS)
            {
        //Use for debug only
#if 0
            /* Retrieve the datatype attribute */
            ub2          dtype;
            checkerr(&OraError, m_pConnection->GetOCIError(), OCIAttrGet( mypard, (ub4) OCI_DTYPE_PARAM,  &dtype,(ub4 *) 0, (ub4) OCI_ATTR_DATA_TYPE, m_pConnection->GetOCIError()));

            /* Retrieve the column name attribute */
            ub4     col_name_len = 0;
            OraText* col_name;
            checkerr(&OraError, m_pConnection->GetOCIError(), OCIAttrGet( mypard, (ub4) OCI_DTYPE_PARAM, (void**) &col_name, (ub4 *) &col_name_len, (ub4) OCI_ATTR_NAME, m_pConnection->GetOCIError()));
#endif
            /* Retrieve the length semantics for the column */
            ub4 char_semantics = 0;
            checkerr(&OraError, m_pConnection->GetOCIError(), OCIAttrGet( mypard, (ub4) OCI_DTYPE_PARAM,  &char_semantics,(ub4 *) 0, (ub4) OCI_ATTR_CHAR_USED, m_pConnection->GetOCIError()));
            ub2 col_width = 0;
            ub4 sizelen=0;
            if (char_semantics)
                /* Retrieve the column width in characters */
                checkerr(&OraError, m_pConnection->GetOCIError(), OCIAttrGet( mypard, (ub4) OCI_DTYPE_PARAM, &col_width, &sizelen, (ub4) OCI_ATTR_CHAR_SIZE, m_pConnection->GetOCIError()));
            else
                /* Retrieve the column width in bytes */
                checkerr(&OraError, m_pConnection->GetOCIError(), OCIAttrGet( mypard, (ub4) OCI_DTYPE_PARAM, &col_width, &sizelen, (ub4) OCI_ATTR_DATA_SIZE,m_pConnection->GetOCIError()));

            //define placeholder
            OCIDefine    *defn1p=NULL;
            pRasterDataTable = unique_ptr<OraText>(s_AllocOraTextP(col_width));
            checkerr(&OraError, m_pConnection->GetOCIError(),OCIDefineByPos(stmtp,&defn1p,m_pConnection->GetOCIError(),1,pRasterDataTable.get(),col_width+1,SQLT_STR,0,0,0,OCI_DEFAULT));
            }

        //Define CLOB to read using locator
        OCILobLocator    *lob_loc=NULL;
        OCIDefine    *defn2p=NULL;
        checkerr(&OraError, m_pConnection->GetOCIError(), OCIDescriptorAlloc(m_pConnection->GetOCIEnv(), (void **)&lob_loc,  (ub4)OCI_DTYPE_LOB, (size_t)0, (void**)0));
        checkerr(&OraError, m_pConnection->GetOCIError(), OCIDefineByPos(stmtp,&defn2p,m_pConnection->GetOCIError(),2,(void *) &lob_loc, (sb4)0 ,SQLT_CLOB,0,0,0,OCI_DEFAULT));

        //Fetch and process data
        checkerr(&OraError, m_pConnection->GetOCIError(),OCIStmtFetch(stmtp, m_pConnection->GetOCIError(), 1, OCI_FETCH_NEXT, OCI_DEFAULT));

        m_RasterDataTable = s_ToWString(pRasterDataTable.get());

        // create query to read the raster block data (see also GetBlock and BindGetBlockVariables for name binding)
        m_getBlockRequest = m_pConnection->CreateGetBlockRequest(m_RasterDataTable);

        //read header CLOB
        ub4 loblen = 0;
        checkerr(&OraError, m_pConnection->GetOCIError(),OCILobGetLength(m_pConnection->GetOCISvcCtx(), m_pConnection->GetOCIError(), lob_loc, &loblen));
        ub4               amtByteInLob = (loblen+1) * sizeof(Utf16Char);
        unique_ptr<OraText> pXMLHeader;

        pXMLHeader = unique_ptr<OraText>(s_AllocOraTextP(loblen));
        memset(pXMLHeader.get(),0,amtByteInLob);
        oraub8 byte_amt(amtByteInLob);
        oraub8 char_amt(loblen+1);
        checkerr(&OraError, m_pConnection->GetOCIError(),OCILobRead2(m_pConnection->GetOCISvcCtx(), m_pConnection->GetOCIError(),lob_loc,&byte_amt,&char_amt,1,pXMLHeader.get(),amtByteInLob,OCI_ONE_PIECE,0,0,OCI_UTF16ID,SQLCS_IMPLICIT));
        WString XMLHeaderStr(s_ToWString(pXMLHeader.get()));
        
        m_XMLHeaderSize = XMLHeaderStr.length() * sizeof(Utf16Char);
        m_pXMLHeader    = new Utf16Char[XMLHeaderStr.length()+1];
        BeStringUtilities::WCharToUtf16 (m_pXMLHeader.get(), XMLHeaderStr.length(), XMLHeaderStr.c_str());

        OCIHandleFree(stmtp,OCI_HTYPE_STMT);
        OCIDescriptorFree(lob_loc,OCI_DTYPE_LOB);
        }
    }



//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
OCIGeoRasterWrapper::~OCIGeoRasterWrapper()
    {
    }


//-----------------------------------------------------------------------------
// public
// GetBlock
//
// note : this method is normally use when the image is not compressed. The
//        output buffer must be allocated be the caller
//-----------------------------------------------------------------------------
void OCIGeoRasterWrapper::GetBlock(unsigned short pi_Resolution,
                                    unsigned short pi_Band,
                                    uint32_t   pi_PosX,
                                    uint32_t   pi_PosY,
                                    Byte*     po_pBuffer,
                                    size_t     pi_BufferSize)
    {
    SDOGeoRasterWrapper::OracleError OraError;
    OCIStmt      *stmtp;
    ub4 KeyBufLen;
    unique_ptr<OraText> pKey (s_AllocOraTextP(L"GetBlock",KeyBufLen));

    ub4               requestBufLen;
    unique_ptr<OraText> pRequest (s_AllocOraTextP(m_getBlockRequest,requestBufLen));

    //Prepare the statement
    checkerr(&OraError, m_pConnection->GetOCIError(), OCIStmtPrepare2(m_pConnection->GetOCISvcCtx(), &stmtp, m_pConnection->GetOCIError(),pRequest.get(),requestBufLen,pKey.get(),KeyBufLen,OCI_NTV_SYNTAX,OCI_DEFAULT));

    //Bind variables
    ub4 bufLen;
    unique_ptr<OraText> pRasterIdValue (s_AllocOraTextP(m_RasterID,bufLen));
    bufLen += sizeof(Utf16Char);//Must add 1 more character for the NULL terminated character.

    m_pConnection->BindGetBlockVariables(stmtp,pRasterIdValue.get(),bufLen,&pi_Resolution,&pi_Band,&pi_PosX,&pi_PosY);

    //Execute statement
    checkerr(&OraError, m_pConnection->GetOCIError(),OCIStmtExecute (m_pConnection->GetOCISvcCtx(), stmtp, m_pConnection->GetOCIError(), 0, 0, (OCISnapshot *)0, (OCISnapshot *)0, OCI_DEFAULT));

    //Define Lob buffer data
    OCIDefine    *defn1p=NULL;
    checkerr(&OraError, m_pConnection->GetOCIError(),OCIDefineByPos(stmtp,&defn1p,m_pConnection->GetOCIError(),1,po_pBuffer,(sb4)pi_BufferSize,SQLT_BIN,0,0,0,OCI_DEFAULT));

    //Fetch and process data
    checkerr(&OraError, m_pConnection->GetOCIError(),OCIStmtFetch(stmtp, m_pConnection->GetOCIError(), 1, OCI_FETCH_NEXT, OCI_DEFAULT));
    
    //Cleanup
    OCIStmtRelease (stmtp,m_pConnection->GetOCIError(),pKey.get(),KeyBufLen,OCI_DEFAULT);
    }


//-----------------------------------------------------------------------------
// public
// GetBlock
//
// note : this method can be use when the image is compressed, the method will
//        allocate the output buffer with the right size
//-----------------------------------------------------------------------------
void OCIGeoRasterWrapper::GetBlock(unsigned short pi_Resolution,
                                    unsigned short pi_Band,
                                    uint32_t   pi_PosX,
                                    uint32_t   pi_PosY,
                                    Byte**    po_ppBuffer,
                                    size_t*    po_pBufferSize)
    {
    SDOGeoRasterWrapper::OracleError OraError;
    OCIStmt      *stmtp;
    ub4 KeyBufLen;
    unique_ptr<OraText> pKey (s_AllocOraTextP(L"GetBlock",KeyBufLen));

    ub4               requestBufLen;
    unique_ptr<OraText> pRequest (s_AllocOraTextP(m_getBlockRequest,requestBufLen));

    //Prepare the statement
    checkerr(&OraError, m_pConnection->GetOCIError(), OCIStmtPrepare2(m_pConnection->GetOCISvcCtx(), &stmtp, m_pConnection->GetOCIError(),pRequest.get(),requestBufLen,pKey.get(),KeyBufLen,OCI_NTV_SYNTAX,OCI_DEFAULT));

    //Bind variables
    ub4 bufLen;
    unique_ptr<OraText> pRasterIdValue (s_AllocOraTextP(m_RasterID,bufLen));
    bufLen += sizeof(Utf16Char);//Must add 1 more character for the NULL terminated character.

    m_pConnection->BindGetBlockVariables(stmtp,pRasterIdValue.get(),bufLen,&pi_Resolution,&pi_Band,&pi_PosX,&pi_PosY);

    //Execute statement
    checkerr(&OraError, m_pConnection->GetOCIError(),OCIStmtExecute (m_pConnection->GetOCISvcCtx(), stmtp, m_pConnection->GetOCIError(), 0, 0, (OCISnapshot *)0, (OCISnapshot *)0, OCI_DEFAULT));

    //Define BLOB to read using locator
    OCILobLocator    *lob_loc=NULL;
    OCIDefine    *defn1p=NULL;
    checkerr(&OraError, m_pConnection->GetOCIError(), OCIDescriptorAlloc(m_pConnection->GetOCIEnv(), (void **)&lob_loc,  (ub4)OCI_DTYPE_LOB, (size_t)0, (void**)0));
    checkerr(&OraError, m_pConnection->GetOCIError(), OCIDefineByPos(stmtp,&defn1p,m_pConnection->GetOCIError(),1,(void *) &lob_loc, (sb4)0 ,SQLT_BLOB,0,0,0,OCI_DEFAULT));

    //Fetch and process data
    checkerr(&OraError, m_pConnection->GetOCIError(),OCIStmtFetch(stmtp, m_pConnection->GetOCIError(), 1, OCI_FETCH_NEXT, OCI_DEFAULT));

    //get BLOB length
    ub4 loblen = 0;
    checkerr(&OraError, m_pConnection->GetOCIError(),OCILobGetLength(m_pConnection->GetOCISvcCtx(), m_pConnection->GetOCIError(), lob_loc, &loblen));

    // copy the raster data into the memory buffer
    *po_pBufferSize = loblen;
    *po_ppBuffer = new Byte [loblen];
    if (*po_ppBuffer != 0)
        {
        oraub8 byte_amt(loblen);
        oraub8 char_amt(loblen);
        checkerr(&OraError, m_pConnection->GetOCIError(),OCILobRead2(m_pConnection->GetOCISvcCtx(), m_pConnection->GetOCIError(),lob_loc,&byte_amt,&char_amt,1,*po_ppBuffer,loblen,OCI_ONE_PIECE,0,0,0,SQLCS_IMPLICIT));
        }

    //Cleanup
    OCIDescriptorFree(lob_loc,OCI_DTYPE_LOB);
    OCIStmtRelease (stmtp,m_pConnection->GetOCIError(),pKey.get(),KeyBufLen,OCI_DEFAULT);
    }


//-----------------------------------------------------------------------------
// public
// SetBlock
//-----------------------------------------------------------------------------
bool OCIGeoRasterWrapper::SetBlock(unsigned short pi_Resolution,
                                    unsigned short pi_Band,
                                    uint32_t     pi_PosX,
                                    uint32_t     pi_PosY,
                                    const Byte* pi_pBuffer,
                                    size_t       pi_BufferSize)
    {
    // not yet supported
    return false;
    }


//-----------------------------------------------------------------------------
// public
// GetWkt
//-----------------------------------------------------------------------------
bool OCIGeoRasterWrapper::GetWkt(uint32_t pi_SRID, WStringR po_rWKT)
    {
    bool IsWKTFound = false;
    // create query to read the raster block data
    wostringstream Request;
    Request << "SELECT WKTEXT FROM MDSYS.CS_SRS WHERE SRID=" << pi_SRID;

    ub4               requestBufLen;
    unique_ptr<OraText> pRequest (s_AllocOraTextP(Request.str().c_str(),requestBufLen));

    //Prepare the statement
    SDOGeoRasterWrapper::OracleError OraError;
    OCIStmt      *stmtp;
    checkerr(&OraError, m_pConnection->GetOCIError(),OCIHandleAlloc(m_pConnection->GetOCIEnv(), (void**)&stmtp, OCI_HTYPE_STMT, 0, (void**)0));
    checkerr(&OraError, m_pConnection->GetOCIError(), OCIStmtPrepare(stmtp,m_pConnection->GetOCIError(),pRequest.get(),requestBufLen,OCI_NTV_SYNTAX,OCI_DEFAULT));

    //Execute statement
    checkerr(&OraError, m_pConnection->GetOCIError(),OCIStmtExecute (m_pConnection->GetOCISvcCtx(), stmtp, m_pConnection->GetOCIError(), 0, 0, (OCISnapshot *)0, (OCISnapshot *)0, OCI_DEFAULT));

    unique_ptr<OraText> pRasterDataTable;

    // Request a parameter descriptor for position 1 in the select-list 
    OCIParam     *mypard = (OCIParam *) 0;
    sb4          parm_status;
    parm_status = OCIParamGet((void *)stmtp, OCI_HTYPE_STMT, m_pConnection->GetOCIError(),(void **)&mypard, (ub4) 1);
    if (parm_status==OCI_SUCCESS)
        {
//Use for debug only
#if 0
        /* Retrieve the datatype attribute */
        ub2          dtype;
        checkerr(&OraError, m_pConnection->GetOCIError(), OCIAttrGet( mypard, (ub4) OCI_DTYPE_PARAM,  &dtype,(ub4 *) 0, (ub4) OCI_ATTR_DATA_TYPE, m_pConnection->GetOCIError()));

        /* Retrieve the column name attribute */
        ub4     col_name_len = 0;
        OraText* col_name;
        checkerr(&OraError, m_pConnection->GetOCIError(), OCIAttrGet( mypard, (ub4) OCI_DTYPE_PARAM, (void**) &col_name, (ub4 *) &col_name_len, (ub4) OCI_ATTR_NAME, m_pConnection->GetOCIError()));
#endif
        /* Retrieve the length semantics for the column */
        ub4 char_semantics = 0;
        checkerr(&OraError, m_pConnection->GetOCIError(), OCIAttrGet( mypard, (ub4) OCI_DTYPE_PARAM,  &char_semantics,(ub4 *) 0, (ub4) OCI_ATTR_CHAR_USED, m_pConnection->GetOCIError()));
        ub2 col_width = 0;
        ub4 sizelen=0;
        if (char_semantics)
            /* Retrieve the column width in characters */
            checkerr(&OraError, m_pConnection->GetOCIError(), OCIAttrGet( mypard, (ub4) OCI_DTYPE_PARAM, &col_width, &sizelen, (ub4) OCI_ATTR_CHAR_SIZE, m_pConnection->GetOCIError()));
        else
            /* Retrieve the column width in bytes */
            checkerr(&OraError, m_pConnection->GetOCIError(), OCIAttrGet( mypard, (ub4) OCI_DTYPE_PARAM, &col_width, &sizelen, (ub4) OCI_ATTR_DATA_SIZE,m_pConnection->GetOCIError()));

        //define placeholder
        OCIDefine    *defn1p=NULL;
        pRasterDataTable = unique_ptr<OraText>(s_AllocOraTextP(col_width));
        checkerr(&OraError, m_pConnection->GetOCIError(),OCIDefineByPos(stmtp,&defn1p,m_pConnection->GetOCIError(),1,pRasterDataTable.get(),col_width+1,SQLT_STR,0,0,0,OCI_DEFAULT));
        }

    //Fetch and process data
    checkerr(&OraError, m_pConnection->GetOCIError(),OCIStmtFetch(stmtp, m_pConnection->GetOCIError(), 1, OCI_FETCH_NEXT, OCI_DEFAULT));

    if (OraError.m_ErrorCode==OCI_SUCCESS)
        {
        po_rWKT = s_ToWString(pRasterDataTable.get());
        IsWKTFound = true;
        }

    //Cleanup
    OCIHandleFree(stmtp,OCI_HTYPE_STMT);

    return IsWKTFound;
    }


