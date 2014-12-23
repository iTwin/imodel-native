
#pragma once

namespace PTRMI
{
	class ObjectInfo
	{

	protected:

		typedef unsigned int	ReferenceCount;

	protected:

		Name					objectName;
		Name					className;

		GUID					ownerClientManager;

		RemotableObject		*	object;

		bool					managed;

		ReferenceCount			referenceCount;

	public:

		ObjectInfo(void)
		{
			clear();
															// Assume there is one initial reference
			setReferenceCounter(0);
		}

		ObjectInfo(const PTRMI::Name &initObjectName, const PTRMI::Name initClassName, const PTRMI::GUID &initClientManager, RemotableObject *initObject, bool initManaged)
		{

															// Set URL name
			setObjectName(initObjectName);
															// Set class name
			setClassName(initClassName);
															// Set the object itself
			setObject(initObject);
															// Set the client's manager GUID to record which remote client owns this object
			setOwnerClientManager(initClientManager);
															// Set whether instantiated by the API
			setManaged(initManaged);
															// Assume there is one initial reference
			setReferenceCounter(0);
		}

		void clear(void)
		{
			setObject(NULL);

			setObjectName(L"");
			setClassName(L"");

			ownerClientManager.clear();

			setManaged(false);

			setReferenceCounter(0);
		}

		void setManaged(bool initManaged)
		{
			managed = initManaged;
		}

		bool getManaged(void)
		{
			return managed;
		}

		void setObjectName(const Name &initName)
		{
			objectName = initName;
		}

		const Name &getObjectName(void)
		{
			return objectName;
		}

		void setClassName(const Name &initClassName)
		{
			className = initClassName;
		}

		const Name &getClassName(void)
		{
			return className;
		}

		void setObject(RemotableObject *initObject)
		{
			object = initObject;
		}

		RemotableObject *getObject(void)
		{
			return object;
		}

		void setOwnerClientManager(const GUID &manager)
		{
			ownerClientManager = manager;
		}

		GUID getOwnerClientManager(void)
		{
			return ownerClientManager;
		}

		ReferenceCount incrementReferenceCounter(void)
		{
			setReferenceCounter(getReferenceCounter() + 1);

			return getReferenceCounter();
		}

		ReferenceCount decrementReferenceCounter(void) 
		{
															// If reference count is valid, continue to decrement
			if(getReferenceCounter() > 0)
			{
				setReferenceCounter(getReferenceCounter() - 1);
			}
			else
			{
															// If count would go sub zero, assert and leave as zero
				assert(getReferenceCounter() > 0);
			}
															// Return the reference counter
			return getReferenceCounter();
		}

		void setReferenceCounter(ReferenceCount value) 
		{
			referenceCount = value;
		}

		ReferenceCount getReferenceCounter(void) 
		{
			return referenceCount;
		}


	};
}