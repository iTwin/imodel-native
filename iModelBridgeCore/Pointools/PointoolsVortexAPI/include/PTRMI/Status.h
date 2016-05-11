#pragma once

#include <fstream>
#include <pt/timestamp.h>


namespace PTRMI
{
	class DataBuffer;

	class Status
	{

	public:

		typedef unsigned short		ErrorType;

		static pt::SimpleTimer		timer;

		enum Error
		{
			Status_OK												= 1,
			Status_Error_Failed										= 2,
			Status_Error_Failed_No_Result							= 3,

			Status_Error_Method_Not_Implemented						= 4,

			Status_Error_Bad_Parameter								= 5,
			Status_Error_Memory_Allocation							= 6,

			Status_Server_Class_Invalid								= 7,
			Status_Server_Class_Method_Not_Found					= 8,

			Status_Error_Failed_To_Create_Mutex						= 9,
			Status_Error_Failed_To_Create_Event						= 10,

			Status_Error_Failed_To_Create_Thread					= 11,
			Status_Error_Failed_To_Terminate_Thread					= 12,

			Status_Error_Array_Bounds_Error_Read					= 13,
			Status_Error_Array_Bounds_Error_Write					= 14,

			Status_Error_Array_Read									= 15,
			Status_Error_Array_Write								= 16,

			Status_Client_Stream_Not_Set							= 17,

			Status_Error_Stream_Invalid								= 18,
			Status_Error_Stream_Has_No_Client_Interface				= 19,

			Status_Error_Failed_To_Get_Client_Interface_Stream		= 20,
			Status_Error_Failed_To_Get_Server_Interface_Stream		= 21,

			Status_Error_Failed_To_Find_Server_Interface_Object		= 22,

			Status_Failed_To_Receive_Message						= 23,

			Status_Error_Message_Wait_Timeout						= 24,
			Status_Error_Receive_Message_Signal						= 25,

			Status_Error_Remote_Host_Version_Higher					= 26,

			Status_Error_Send_Header_Failed							= 27,
			Status_Error_Receive_Header_Failed						= 28,

			Status_Error_Send_Invoke_Failed							= 29,

			Status_Error_Remote_Manager_Not_Found					= 30,
			Status_Error_Delete_Remote_Object_Failed				= 31,

			Status_Error_Failed_To_Create_Stream_Pipe				= 32,
			Status_Error_Pipe_Send_Wait_Mutex_Failed				= 33,
			Status_Error_Pipe_Send_Release_Mutex_Failed				= 34,

			Status_Error_Get_Remote_Object_Bad_GUID					= 35,
			Status_Error_Host_GUID_Invalid							= 36,

			Status_Error_Failed_To_Create_Client_Interface			= 37,
			Status_Error_Failed_To_Create_Server_Interface			= 38,

			Status_Error_Failed_To_Find_Server_Interface			= 39,

			Status_Error_Failed_To_Find_Return_Client_Interface		= 40,

			Status_Error_Failed_To_Initialize_Winsock				= 41,
			Status_Error_Failed_To_Create_Listener_Socket			= 42,
			Status_Failed_To_Create_Client_Socket					= 43,
			Status_Error_Failed_To_Bind_Listener_Socket				= 44,
			Status_Error_Socket_Sending								= 45,
			Status_Error_Socket_Receiving							= 46,

			Status_Error_Thread_Resume								= 47,

			Status_Error_Failed_To_Get_Object_Manager				= 48,

			Status_Error_Dispatch_Failed							= 49,
			Status_Error_Dispatch_Receiver_Not_Found				= 50,

			Status_Failed_To_Connect_To_Host						= 51,

			Status_Failed_To_Resolve_Host_URL						= 52,

			Status_Failed_To_Get_Dispatcher							= 53,

			Status_Error_Failed_To_Create_Remote_Object				= 54,

			Status_Remove_This_1									= 55,

			Status_Error_Failed_To_Find_Object_Info					= 56,

			Status_Error_Failed_To_Find_Object_Meta_Interface		= 57,

			Status_Error_External_Call_Function_Not_Defined			= 58,

			Status_Error_Data_Buffer_Read_Pointer_Invalid			= 59,
			Status_Error_Data_Buffer_Write_Pointer_Invalid			= 60,

			Status_Error_File_Size_Larger_Than_Buffer				= 61,
			Status_Error_File_Read									= 62,
			Status_Error_File_Write									= 63,

			Status_File_Not_Ext_Data								= 64,
			Status_Error_File_Ext_Data_Newer_Version				= 65,
			Status_Error_File_Name_Bad								= 66,

			Status_Error_Failed_To_Set_Client_Interface_Ext_Data	= 67,

			Status_Error_Data_Buffer_Read_Source_To_Short			= 68,
			Status_Error_Failed_To_Write_Ext_Data					= 69,

			Status_Error_Stream_Has_No_Send_Buffer					= 70,
			Status_Error_Stream_Has_No_Receive_Buffer				= 71,

			Status_Error_Failed_To_Find_Ext_Manager					= 72,

			Status_Error_Failed_To_Write_To_Buffer_From_Buffer		= 73,

			Status_Error_Failed_To_Advance_Buffer_Read_Ptr			= 74,

			Status_Error_Failed_To_Resize_Internal_Buffer			= 75,

			Status_Error_Bad_Buffer_Size							= 76,

			Status_Error_Failed_To_Advance_Buffer_Write_Ptr			= 77,

			Status_Error_Failed_To_Set_Read_Parameters				= 78,
			Status_Error_Failed_To_Set_Write_Parameters				= 79,

			Status_Error_Copy_Ext_Bufffer_State_Invalid_Mode		= 80,

			Status_Error_Failed_To_Find_Pipe_Receive_Buffer			= 81,

			Status_Warning_Data_Buffer_Read_Null_Parameter			= 82,

			Status_Warning_Data_Buffer_Read_No_Pipe_For_Data		= 83,

			Status_Error_Data_Buffer_Read_Allocate_Failed			= 84,

			Status_Error_Failed_To_Find_Pipe_Pipe_Manager			= 85,

			Status_Error_Failed_To_Find_Pipe_For_Invoke				= 86,

			Status_Warning_Failed_To_Get_Client_Side_File_Path		= 87,

			Status_Error_Invalid_Dispatch_Type						= 88,

			Status_Error_File_Type_Incorrect						= 89,
			Status_Error_File_Newer_Version							= 90,

			Status_Error_File_Open_For_Read							= 91,
			Status_Error_File_Open_For_Write						= 92,
			Status_Error_File_Open_For_Read_Write					= 93,
			Status_Error_File_Move									= 94,

			Status_Error_File_Move_Pointer							= 95,

			Status_Error_Invalid_GUID								= 96,

			Status_Error_RMI_Invoke_Failed							= 97,
			Status_Error_Pipe_Failed								= 98,

			Status_Error_Ext_Data_Bad								= 99,
			Status_Error_Ext_Data_Out_Of_Date						= 100,

			Status_Error_Send_Header_Size_Too_Large					= 101,
			Status_Error_Receive_Header_Size_Too_Large				= 102,

			Status_Error_Cache_Full_File_Data_Source_Not_Found		= 103,
			Status_Error_Cache_File_Data_Source_Not_Found			= 104,
			Status_Error_Cache_Read_Set_Failed						= 105,
			Status_Error_Cache_Write_Failed							= 106,

			Status_Error_Client_Interface_Lock_Timeout				= 107,
			Status_Error_Client_Interface_Not_Locked				= 108,

			Status_Error_File_Size_Zero								= 109,

			Status_Error_Client_Manager_Server_Interface_Exists		= 110,
			Status_Error_Failed_To_Find_Client_Manager				= 111,

			Status_Error_Failed_To_Create_Error_Handler				= 112,

			Status_Error_Failed_To_Find_Object_Lock					= 113,
			Status_Warning_Failed_To_Lock_Object					= 114,
			Status_Error_Object_Lock_Exists							= 115,
			Status_Error_Failed_To_Release_Object_Lock				= 116,
			Status_Warning_Failed_To_Delete_Object_Lock				= 117,

			Status_Error_Failed_To_Find_Pipe_For_Deletion			= 118,
			Status_Error_Failed_To_Delete_Pipe						= 119,
			Status_Error_Failed_To_Find_Pipe_Protocol_Manager		= 120,
			Status_Error_Failed_To_Find_Pipe_For_Discard			= 121,
			Status_Error_Failed_To_Find_Pipe_For_Removal			= 122,

			Status_Error_Failed_To_Lock_Reference_Counter			= 123,
			Status_Error_Failed_To_Release_Reference_Counter		= 124,

			Status_Error_Failed_To_Lock_Pipe						= 125,
			Status_Error_Failed_To_Release_Pipe						= 126,

			Status_Error_Failed_To_Lock_Stream_Caller				= 127,
			Status_Error_Failed_To_Release_Stream_Caller			= 128,

			Status_Error_Server_Interface_Invoke_Invalid			= 129,

			Status_Warning_Pipe_Has_Host_GUID						= 130,

			Status_Error_Failed_To_Create_Group						= 131,
			Status_Warning_Failed_To_Find_Group						= 132,
			Status_Warning_Failed_To_Find_Item						= 133,

			Status_Warning_Failed_To_Find_Host						= 134,
			Status_Error_Failed_To_Lock_Host						= 135,

			Status_Error_Failed_To_Find_Host_Manager				= 136,

			Status_Error_Failed_To_Find_Stream_Pipe					= 137,

			Status_Error_Failed_To_Swap_Manager_Info				= 138,

			Status_Error_Failed_To_Initialize_Host					= 139,
			Status_Error_Failed_To_Find_Host_Pipe					= 140,

			Status_Error_Failed_To_Recover_Remote_Object			= 141,
			Status_Error_Remote_Object_Not_Recoverable				= 142,

			Status_Error_Host_Exists								= 143,

			Status_Error_Failed_To_Find_Pipe_Manager				= 144,

			Status_Error_Failed_To_Add_Pipe_Manager_Pipe			= 145,
			Status_Error_Failed_To_Remove_Pipe_Manager_Pipe			= 146,

			Status_Error_Failed_To_Find_Pipe_For_Initialization		= 147,
			Status_Error_Failed_To_Initialize_Pipe					= 148,

			Status_Error_Failed_To_Release_Host						= 149,

			Status_Error_Reading_Multi_Read_Set						= 150,
			Status_Error_Writing_Multi_Read_Set						= 151,

			Status_Error_Failed_To_Lock_Server_Interface			= 152,
			Status_Error_Server_Interface_Invalid					= 153,
			Status_Error_Failed_To_Find_ReadSet						= 154,
			Status_Error_Failed_To_Find_MultiRead					= 155,

			Status_Error_External_Call_Function_Failed				= 156,

			Status_Error_Failed_To_Lock_Stream_Manager				= 157,
			Status_Error_Failed_To_Release_Stream_Manager			= 158,

			Status_Error_Object_Ping_Failed							= 159,

			Status_Warning_Failed_To_Spin_Lock_Object				= 160,

			Status_Warning_Item_Exists								= 161,

			Status_Error_Failed_To_Lock_Object_Manager				= 162,
			Status_Error_Failed_To_Lock_Manager						= 163,

			Status_Count
		};

	protected:

		Error					error;

		static bool				logEnabled;
		static std::wstring		logFile;
		
	public:

		Status(void)
		{
			set(Status_OK);
		}

		Status(Error value, bool report = true)
		{
			set(value, report);
		}

		static void setLogFile(const wchar_t *filePath)
		{
			if(filePath)
			{
				if(wcslen(filePath) > 0)
				{
					logFile = filePath;
					setLogEnabled(true);

#ifndef NO_DATA_SOURCE_SERVER
                    BeFileName::BeDeleteFile(filePath);
					log(L"Log started ", L"");
#endif
				}
			}
			else
			{
				logFile = L"";
				setLogEnabled(false);
			}
		}

		static void setLogEnabled(bool enabled)
		{
			logEnabled = enabled;
		}

		static bool getLogEnabled(void)
		{
			return logEnabled;
		}

		void set(Error value, bool report = true);

		Error get(void) const
		{
			return error;
		}

		bool is(Error value)
		{
			return get() == value;
		}

		bool is(Status &status)
		{
			return get() == status.get();
		}

		bool isOK(void)
		{
			return (get() == Status_OK);
		}

		bool isFailed(void)
		{
			return (get() != Status_OK);
		}

		static void			writeTime			(void);

		static void			log					(const wchar_t *message1, const wchar_t *message2);

		static void			log					(const wchar_t *message1, unsigned int value);

		Status				read				(PTRMI::DataBuffer &buffer);
		Status				write				(PTRMI::DataBuffer &buffer) const;
		Status				write				(void *buffer) const;

		static unsigned int	getMaxWriteSize		(void);

	};


}