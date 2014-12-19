//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/tst/HFCStatTest/HFCStatTest.cpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HFCStat.h>
#include <Imagepp/all/h/HFCURLFile.h>

void main(void)
    {
    HFCPtr<HFCURL> pURL1(HFCURL::Instanciate(string("file://c:\\winnt\\profiles\\sebastient\\desktop\\ttt.reg")));
    HFCPtr<HFCURL> pURL2(HFCURL::Instanciate(string("iip://127.0.0.1:1924/images/titus.jpg")));

    HFCStat Reg(pURL1);
    printf("HFCStat.h existence = %lu\n", Reg.IsExistent());
    printf("HFCStat.h modification time = %lu\n", Reg.GetModificationTime());
    Reg.SetModificationTime(Reg.GetModificationTime() - 60);
    printf("HFCStat.h modification time = %lu\n", Reg.GetModificationTime());
    printf("HFCStat.g Read Access = %lu\n", Reg.GetAccessMode().m_HasReadAccess);
    printf("HFCStat.g Write Access = %lu\n", Reg.GetAccessMode().m_HasWriteAccess);
    printf("HFCStat.g Create Access = %lu\n", Reg.GetAccessMode().m_HasCreateAccess);
    printf("HFCStat.g Read Share = %lu\n", Reg.GetAccessMode().m_HasReadShare);
    printf("HFCStat.g Write Share = %lu\n", Reg.GetAccessMode().m_HasWriteShare);

    HFCStat Reg2(pURL2);
    printf("HFCStat.h existence = %lu\n", Reg2.IsExistent());
    printf("HFCStat.h modification time = %lu\n", Reg2.GetModificationTime());
    Reg2.SetModificationTime(Reg2.GetModificationTime() - 60);
    printf("HFCStat.h modification time = %lu\n", Reg2.GetModificationTime());
    printf("HFCStat.g Read Access = %lu\n", Reg2.GetAccessMode().m_HasReadAccess);
    printf("HFCStat.g Write Access = %lu\n", Reg2.GetAccessMode().m_HasWriteAccess);
    printf("HFCStat.g Create Access = %lu\n", Reg2.GetAccessMode().m_HasCreateAccess);
    printf("HFCStat.g Read Share = %lu\n", Reg2.GetAccessMode().m_HasReadShare);
    printf("HFCStat.g Write Share = %lu\n", Reg2.GetAccessMode().m_HasWriteShare);
    }