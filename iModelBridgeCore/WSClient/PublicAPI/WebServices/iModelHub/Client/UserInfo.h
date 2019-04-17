/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <WebServices/iModelHub/Common.h>
#include <WebServices/iModelHub/Client/Result.h>
#include <WebServices/Client/Response/WSObjectsReader.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE
typedef RefCountedPtr<struct UserInfo> UserInfoPtr;
DEFINE_POINTER_SUFFIX_TYPEDEFS(UserInfo);
DEFINE_TASK_TYPEDEFS(UserInfoPtr, UserInfo);
DEFINE_TASK_TYPEDEFS(bvector<UserInfoPtr>, UsersInfo);

//=======================================================================================
//! Information about user.
//@bsiclass                                      Paulius.Valiunas             01/2017
//=======================================================================================
struct UserInfo : RefCountedBase
{
private:
	friend struct UserInfoManager;
    friend struct iModelInfo;
    friend struct StatisticsInfo;
	Utf8String m_id;
	Utf8String m_name;
	Utf8String m_surname;
	Utf8String m_email;

	UserInfo(Utf8String id, Utf8String name, Utf8String surname, Utf8String email)
		: m_id(id), m_name(name), m_surname(surname), m_email(email) {}

	bool operator==(UserInfoCR user) const { return user.GetId() == GetId(); }
	static UserInfoPtr ParseRapidJson(RapidJsonValueCR properties);
	static UserInfoPtr Parse(WebServices::WSObjectsReader::Instance instance);
    static UserInfoPtr ParseFromRelated(WebServices::WSObjectsReader::Instance *instance);
public:
	Utf8String GetId() const {return m_id;}
	Utf8String GetName() const {return m_name;}
	Utf8String GetSurname() const {return m_surname;}
	Utf8String GetEmail() const {return m_email;}
};
END_BENTLEY_IMODELHUB_NAMESPACE
