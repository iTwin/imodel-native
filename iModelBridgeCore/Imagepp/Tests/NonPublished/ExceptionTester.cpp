//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------

#include "../imagepptestpch.h"
#include "ExceptionTester.h"

#define EXPECT_SUBSTR(expected, actual) \
    EXPECT_PRED_FORMAT2(testing::IsSubstring, expected, actual)
        

ExceptionTester::ExceptionTester()
{
        HPASourcePos StartPos;
        HPASourcePos EndPos;

        StartPos.m_Line      = 10;
        StartPos.m_Column = 11;

        EndPos.m_Line    = 10;
        EndPos.m_Column = 12;

        m_TestNode = new HPANode(0, StartPos, EndPos, 0);
}
/* Disabled
TEST_F (ExceptionTester, HFCGetMessageTest)
{
    HFCUnknownException _HFCUnknownException;
    EXPECT_SUBSTR(L"Unknown exception", _HFCUnknownException.GetExceptionMessage().c_str());

    HFCCorruptedFileException _HFCCorruptedFileException(L"CorruptedFileName");
    EXPECT_SUBSTR(L"Corrupted file", _HFCCorruptedFileException.GetExceptionMessage().c_str());
    EXPECT_STREQ(L"CorruptedFileName", _HFCCorruptedFileException.GetFileName().c_str());
    
    HFCNoDiskSpaceLeftException _HFCNoDiskSpaceLeftException(L"DeviceName");
    EXPECT_SUBSTR(L"enough disk space left", _HFCNoDiskSpaceLeftException.GetExceptionMessage().c_str());
    EXPECT_STREQ(L"DeviceName", _HFCNoDiskSpaceLeftException.GetDeviceName().c_str());
    
    HFCInternetConnectionException _HFCInternetConnectionException(L"Test Device", HFCInternetConnectionException::UNKNOWN);
    EXPECT_SUBSTR(L"Internet connection", _HFCInternetConnectionException.GetExceptionMessage().c_str());
    EXPECT_SUBSTR(L"Test Device", _HFCInternetConnectionException.GetDeviceName().c_str());
    EXPECT_EQ(HFCInternetConnectionException::UNKNOWN, _HFCInternetConnectionException.GetErrorType());
    
    HFCCannotCreateSynchroObjException _HFCCannotCreateSynchroObjException(HFCCannotCreateSynchroObjException::MUTEX);
    EXPECT_SUBSTR(L"Cannot create a synchronization object", _HFCCannotCreateSynchroObjException.GetExceptionMessage().c_str());
    EXPECT_EQ(HFCCannotCreateSynchroObjException::MUTEX, _HFCCannotCreateSynchroObjException.GetSynchroObject());

    HFCErrorCode errCode;
    HFCErrorCodeException _HFCErrorCodeException(errCode);
    EXPECT_SUBSTR(L"An error code triggered an exception", _HFCErrorCodeException.GetExceptionMessage().c_str());
}

TEST_F (ExceptionTester, HPAGetMessageTest)
{
    HPACannotResolveStartRuleException _HPACannotResolveStartRuleException(m_TestNode);
    EXPECT_SUBSTR(L"The parser cannot resolve the starting rule", _HPACannotResolveStartRuleException.GetExceptionMessage().c_str());
    
    HPAGenericException _HPAGenericException(m_TestNode , L"Message");
    EXPECT_SUBSTR(L"The following generic parser exception occurred : Message", _HPAGenericException.GetExceptionMessage().c_str());
    EXPECT_STREQ(L"Message", _HPAGenericException.GetMessage().c_str());
}

TEST_F (ExceptionTester, HGFGetMessageTest)
{
    HGFDomainException _HGFDomainException;
    EXPECT_SUBSTR(L"The requested is outside the grid model's domain", _HGFDomainException.GetExceptionMessage().c_str());
}

TEST_F (ExceptionTester, HFSGetMessageTest)
{
    HFSCannotEnumNextNetResourceException _HFSCannotEnumNextNetResourceException;
    EXPECT_SUBSTR(L"Cannot enumerate the next network resource", _HFSCannotEnumNextNetResourceException.GetExceptionMessage().c_str());

    HFSGenericInvalidPathException _HFSGenericInvalidPathException(L"CorruptedFileName");
    EXPECT_SUBSTR(L"CorruptedFileName is an invalid file server path", _HFSGenericInvalidPathException.GetExceptionMessage().c_str());
    
    HFSHIBPInvalidResponseException _HFSHIBPInvalidResponseException(L"Request");
    EXPECT_SUBSTR(L"Invalid IBP response", _HFSHIBPInvalidResponseException.GetExceptionMessage().c_str());

    HFSHIBPErrorException _HFSHIBPErrorException(L"Message");
    EXPECT_SUBSTR(L"The following IBP error occurred : Message", _HFSHIBPErrorException.GetExceptionMessage().c_str());
}


TEST_F (ExceptionTester, HPSGetMessageTest)
{
    HPSAlphaPaletteNotSupportedException _HPSAlphaPaletteNotSupportedException(m_TestNode);
    EXPECT_SUBSTR(L"The alpha palette specified in the PSS is not supported  (line : 10, column : 12)", _HPSAlphaPaletteNotSupportedException.GetExceptionMessage().c_str());
   
    HPSTypeMismatchException _HPSTypeMismatchException(m_TestNode, HPSTypeMismatchException::STRING);
    EXPECT_SUBSTR(L"Type mismatch in the PSS (line : 10, column : 12)", _HPSTypeMismatchException.GetExceptionMessage().c_str());
    EXPECT_EQ(HPSTypeMismatchException::STRING, _HPSTypeMismatchException.GetExpectedType());
    
    HPSOutOfRangeException _HPSOutOfRangeException(m_TestNode, 0,0);
    EXPECT_SUBSTR(L"Out of range exception in the PSS (line : 10, column : 12)", _HPSOutOfRangeException.GetExceptionMessage().c_str());
    EXPECT_EQ(0, _HPSOutOfRangeException.GetLower());
    EXPECT_EQ(0, _HPSOutOfRangeException.GetUpper());

    HPSAlreadyDefinedException _HPSAlreadyDefinedException(m_TestNode,  L"Name");
    EXPECT_SUBSTR(L"Redefinition found in the PSS (line : 10, column : 12)", _HPSAlreadyDefinedException.GetExceptionMessage().c_str());
    EXPECT_STREQ(L"Name", _HPSAlreadyDefinedException.GetName().c_str());
}
TEST_F (ExceptionTester, HCDGetMessageTest)
{
    HCDIJGLibraryErrorException _HCDIJGLibraryErrorException;
    EXPECT_SUBSTR(L"A JPEG error occurred", _HCDIJGLibraryErrorException.GetExceptionMessage().c_str());

    HCDIJLErrorException _HCDIJLErrorException(0);
    EXPECT_SUBSTR(L"An error occurred in the JPEG library", _HCDIJLErrorException.GetExceptionMessage().c_str());
    EXPECT_EQ(0, _HCDIJLErrorException.GetErrorCode());
}
*/
