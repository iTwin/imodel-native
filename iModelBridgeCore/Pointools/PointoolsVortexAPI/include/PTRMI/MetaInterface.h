
#pragma once

namespace PTRMI
{
	class MetaInterfaceBase
	{
	protected:

		Name							objectClassName;

	public:

		virtual						   ~MetaInterfaceBase			(void) {}

		virtual	RemotableObject		*	newObject					(void) = 0;
		virtual bool					deleteObject				(RemotableObject *object) = 0;

		virtual ClientInterfaceBase	*	newClientInterface			(void) = 0;
		virtual ServerInterfaceBase *	newServerInterface			(void) = 0;

		virtual ServerInterfaceBase *	newObjectAndServerInterface	(void) = 0;
	};


	template<typename Obj> class MetaInterface : public MetaInterfaceBase
	{
	public:

		typedef typename ObjClientInfo<Obj>::I		ClientInterfaceType;
		typedef typename ObjServerInfo<Obj>::I		ServerInterfaceType;

	public:

		MetaInterface(const PTRMI::Name &initObjectClassName)
		{
			objectClassName = initObjectClassName;
		}

		RemotableObject *newObject(void)
		{
			return new Obj;
		}

		Obj *newObjectTyped(void)
		{
			return new Obj;
		}


		bool deleteObject(RemotableObject *object)
		{
			if(object)
			{
				Obj *objectTyped = reinterpret_cast<Obj *>(object);

				delete objectTyped;

				return true;
			}

			return false;
		}


		ClientInterfaceBase	*newClientInterface	(void)
		{
			return new ClientInterfaceType;
		}

		ServerInterfaceBase *newServerInterface(void)
		{
			return new ServerInterfaceType;
		}

		ServerInterfaceBase *newObjectAndServerInterface(void)
		{
			Obj					*	newObject;
			ServerInterfaceBase	*	serverInterface;
															// Create a new object
			if((newObject = newObjectTyped()) == NULL)
			{
				return NULL;
			}
															// Create a new server interface for the object
			if((serverInterface = newServerInterface()) == NULL)
			{
															// Delete allocated object
				deleteObject(newObject);
															// Return failed
				return NULL;
			}
															// No need to lock server interface because it is not in use yet
															// Bind ServerInterface to new object
			serverInterface->setObject(newObject);
															// Return server interface
			return serverInterface;
		}
	};

}