#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#ifndef __ovCoAdaptP300PropertyReader__
#define __ovCoAdaptP300PropertyReader__


#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>
#include <xml/IReader.h>

#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stack>

#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
#include "../visualisation/glGObject.h"
#endif

namespace
{
	/**
	 * Helper class to do some conversions
	 */
	class _AutoCast_
	{
	public:
		_AutoCast_(const char * settingValue) : m_sSettingValue(settingValue) {  }

		#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
		/**
		 * Convert the string to a GColor
		 */
		operator OpenViBEApplications::GColor (void)
		{
			OpenViBEApplications::GColor l_oColor;
			int r=0, g=0, b=0;
			sscanf(m_sSettingValue, "%i,%i,%i", &r, &g, &b);
			l_oColor.red=(GLfloat)r/100.0f;
			l_oColor.green=(GLfloat)g/100.0f;
			l_oColor.blue=(GLfloat)b/100.0f;
			return l_oColor;
		}
		#endif
		
		 /**
		 * Convert time in milliseconds to OpenViBE time, encoded as uint64
		 */
		operator OpenViBE::uint64 (void)
		{
			OpenViBE::float64 l_f64FlashDuration = strtod(m_sSettingValue,NULL);
			OpenViBE::uint64 l_ui64IntegerPart = (OpenViBE::uint64)l_f64FlashDuration;
			OpenViBE::float64 l_f64FractionalPart = (OpenViBE::float64)(l_f64FlashDuration-l_ui64IntegerPart);
			l_ui64IntegerPart = l_ui64IntegerPart << 32;
			OpenViBE::uint64 l_ui64FlashDuration = l_ui64IntegerPart+(OpenViBE::uint64)(l_f64FractionalPart*(1LL<<32));
			return 	l_ui64FlashDuration;		
		}		

		const char * m_sSettingValue;
	};
};

namespace OpenViBEApplications
{
	enum SpellingMode {
		ODDBALL,
		CALIBRATION_MODE,
		COPY_MODE,
		FREE_MODE,
		REPLAY_MODE
	};	
		
	/**
	 * This is the base class from which all other property readers will derive. \n This class is responsible for
	 * reading the file and creating an IReader. \n The derived class has to implement
	 * openChild, processChildData and closeChild which will process the xml files
	 */
	class CoAdaptP300PropertyReader : public XML::IReaderCallback
	{
				
	public:	
		/**
		 * @param kernelContext we need the kernel context to expand the variables in the xml files
		 */
		CoAdaptP300PropertyReader(OpenViBE::Kernel::IKernelContext* kernelContext) : m_pKernelContext(kernelContext) {}

		/**
		 * This methods opens and reads the file into a buffer. This buffer pointer is then used to create
		 * a XML reader object. The process function is called on this object which will consequently call
		 * the functions openChild, processChildData and closeChild implemented in the derived classes  
		 * @param propertyFile the xml file from which we wish to read the properties
		 */
		virtual void readPropertiesFromFile(OpenViBE::CString propertyFile);
		
		virtual OpenViBE::Kernel::IKernelContext* getKernelContext()
		{
			return m_pKernelContext;
		}

	protected:
		/**
		 * To be implemented in the derived reader classes
		 * @param sName name of the element tag
		 * @param sAttributeName name of an attribute in the element tag
		 * @param sAttributeValue value of that attribute
		 * @param ui64AttributeCount number of attributes in the element
		 */		
		virtual void openChild(const char* sName, const char** sAttributeName, const char** sAttributeValue, XML::uint64 ui64AttributeCount){} 
		/**
		 * To be implemented in the derived reader classes
		 * @param sData the data between opening and closing of element
		 */		
		virtual void processChildData(const char* sData){} 
		/**
		 * To be implemented in the derived reader classes
		 */		
		virtual void closeChild(void){}
		
		/**
		 * for debuggin purposes
		 */
		void writeAttribute(const char* sName, const char** sAttributeName, const char** sAttributeValue, XML::uint64 ui64AttributeCount);
		/**
		 * for debuggin purposes
		 */		
		void writeElement(const char* sName, const char* sData);
		
	protected:	

		OpenViBE::Kernel::IKernelContext* m_pKernelContext;
		std::stack<OpenViBE::CString> m_vNode;
	};
};
#endif


#endif
