#ifndef __OpenViBE_AcquisitionServer_CSettingsHelper_H__
#define __OpenViBE_AcquisitionServer_CSettingsHelper_H__

#include <ovas_base.h>

#include <sstream>
#include <map>
#include <iostream>

namespace OpenViBEAcquisitionServer
{
	/**
	  * \class Property
	  * \author Jussi T. Lindgren (Inria)
	  * \date 2013-11
	  * \brief Base class for properties. A property is essentially a <name,value> pair. 
	  *
	  * \note This class is intended for typed inheritance. It carries no data.
	  */
	class Property
	{
	public:
		Property(const OpenViBE::CString& name) 
			: m_name(name)
		{ }
		virtual ~Property() {}

		const OpenViBE::CString getName(void) const { return m_name; }

		virtual std::ostream& toStream(std::ostream& out) const = 0;
		virtual std::istream& fromStream(std::istream& in) = 0;

	private:
		OpenViBE::CString m_name;

	};

	// Helper operators to map base class << and >> to those of the derived classes. These can not be members.
	inline std::ostream& operator<< (std::ostream& out, const OpenViBEAcquisitionServer::Property& obj) {
		return obj.toStream(out);
	}

	inline std::istream& operator>> (std::istream& in, OpenViBEAcquisitionServer::Property& obj) {
		return obj.fromStream(in);
	}

	/*
	 * \class TypedProperty
	 * \author Jussi T. Lindgren (Inria)
	 * \date 2013-11
	 * \brief A property with a typed data pointer.
	 *
	 * \note The class does NOT own the data pointer, but may allow modification of its contents via replaceData().
	 */
	template<typename T> class TypedProperty : public Property
	{
	public:
		TypedProperty (const OpenViBE::CString& name, T* data)
			: Property(name), m_data(data) { };

		// Access data
		const T* getData(void) const { return m_data; };

		// Overwrites existing data with new contents.
		void replaceData(T& data) { *m_data = data; };
	
		virtual std::ostream& toStream(std::ostream& out) const { out << *m_data; return out; } ;
		virtual std::istream& fromStream(std::istream& in) { in >> *m_data; return in; } ;
	private:
		T* m_data;
	};

#ifdef OV_SH_SUPPORT_GETTERS
	/*
	 * \class GetterSetterProperty
	 * \author Jussi T. Lindgren (Inria)
	 * \date 2013-11
	 * \brief A property for situations where the data can only be accessed by getters and setters
	 *
	 * \note Type T is the type of the object that has the getters and setters as a member. The
	 * types V and W indicate the actual data type that the getters and setters deal with. We
	 * use two types V and W as in the case of the setter, the type is usually const, but for getter, 
	 * its not. This avoids the compiler getting confused.
	 */
	template<typename T, typename V, typename W> class GetterSetterProperty : public Property
	{
	public:
		GetterSetterProperty(const OpenViBE::CString& name, 
			T& obj, 
			V ( T::*getter)(void) const,
			OpenViBE::boolean ( T::*setter)(W))
			: Property(name), m_obj(obj), m_getterFunction(getter), m_setterFunction(setter) { };
	
		virtual std::ostream& toStream(std::ostream& out) const { 
			// std::cout << "Writing " << (m_obj.*m_getter)() << "\n"; 
			out << (m_obj.*m_getterFunction)() ; return out; } ;
		virtual std::istream& fromStream(std::istream& in) { 
			W tmp; in >> tmp; 
			// std::cout << "Reading " << tmp << "\n";
			(m_obj.*m_setterFunction)(tmp); return in; } ;
	private:

		T& m_obj;
		V ( T::*m_getterFunction )(void) const;
		OpenViBE::boolean ( T::*m_setterFunction )(W);
		
	};
#endif

	/*
	 * \class SettingsHelper
	 * \author Jussi T. Lindgren (Inria)
	 * \date 2013-11
	 * \brief Convenience helper class that eases the saving and loading of variables (properties) to/from the configuration manager
	 *
	 * \note For registering exotic types, the user must provide the << and >> overloads to/from streams
	 */
	class SettingsHelper {
	public:
		SettingsHelper(const char *prefix, OpenViBE::Kernel::IConfigurationManager& rMgr) 
			: m_sPrefix(prefix), m_rConfigurationManager(rMgr) { } ; 
		~SettingsHelper() {
			std::map<OpenViBE::CString, Property*>::const_iterator it = m_vProperties.begin();
			for(;it!=m_vProperties.end();++it) {
				delete(it->second);
			}
			m_vProperties.clear();
		}

		// Register or replace a variable. The variable must remain valid for the lifetime of the SettingsHelper object.
		template<typename T> OpenViBE::boolean add(const OpenViBE::CString& name, T* var) {

			if(!var) 
			{
				// std::cout << "Tried to add a NULL pointer\n";
				return false;
			}

			// If key is in map, replace
			std::map<OpenViBE::CString, Property*>::const_iterator it = m_vProperties.find(name);
			if(it!=m_vProperties.end()) {
				// m_rContext.getLogManager() << LogLevel_Trace << "Replacing key [" << name << "]\n";
				delete it->second;
			}
			
			TypedProperty<T> *myProperty = new TypedProperty<T>(name, var);
			m_vProperties[myProperty->getName()] = myProperty;

			return true;
		}

#ifdef OV_SH_SUPPORT_GETTERS
		// Register or replace a property used via setters and getters. The actual object must be provided as well and
		// must remain valid for the lifetime of the SettingsHelper object.
		template<typename T, typename V, typename W> OpenViBE::boolean add(const OpenViBE::CString& name, 
			T& obj, 
			V ( T::*getter)(void) const,
			OpenViBE::boolean ( T::*setter)(W) ) {

			if(getter == NULL || setter == NULL) 
			{
				// std::cout << "Tried to add a NULL pointer\n";
				return false;
			}

			// If key is in map, replace
			std::map<OpenViBE::CString, Property*>::const_iterator it = m_vProperties.find(name);
			if(it!=m_vProperties.end()) {
				// m_rContext.getLogManager() << LogLevel_Trace << "Replacing key [" << name << "]\n";
				delete it->second;
			}

			GetterSetterProperty<T,V,W> *myProperty = new GetterSetterProperty<T,V,W>(name, obj, getter, setter);
			m_vProperties[myProperty->getName()] = myProperty;

			return true;
		}
#endif

		// Save all registered variables to the configuration manager
		void save(void);

		// Load all registered variables from the configuration manager
		void load(void);

		// Get access to the registered variables
		const std::map<OpenViBE::CString, Property*>& getAllProperties(void) const { return m_vProperties; }
	
	private:
		OpenViBE::CString m_sPrefix;
		OpenViBE::Kernel::IConfigurationManager& m_rConfigurationManager;

		std::map<OpenViBE::CString, Property*> m_vProperties;
	};

};


#endif

