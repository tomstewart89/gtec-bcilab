#ifndef __XML_IXMLHANDLER_H_
#define __XML_IXMLHANDLER_H_

#include "defines.h"
#include "IXMLNode.h"

namespace XML
{
	/**
	 * @class IXMLHandler
	 * @author Serrière Guillaume (INRIA/Loria)
	 * @brief This class is design to help about XML manipulation.
	 * @sa XML
	 */
	class OV_API IXMLHandler
	{
	public:
		/**
		 * @brief Release the handler.
		 */
		virtual void release(void)=0;

		//Parsing
		/**
		 * @brief Parse file points by sPath and return the root name of the document.
		 * @param sPath [in] : Path to the File
		 * @return The root node of the document, or NULL if there is an error.
		 */
		virtual XML::IXMLNode* parseFile(const char* sPath)=0;

		/**
		 * @brief Parse the string sString on uiSize caracters and return the root name of the document.
		 * @param sString [in] : String which contains the XML
		 * @param uiSize [in] : Size of the part to analyze
		 * @return The root node of the parse part, or NULL if there is an error.
		 */
		virtual XML::IXMLNode* parseString(const char* sString, const uint32& uiSize)=0;

		//XML extraction
		/**
		 * @brief Write the XML corresponding to the node rNode in the file points by sPath. If the file exists
		 * it will be erase.
		 * @param rNode [in] : The node to write.
		 * @param sPath [in] : The path to the file.
		 * @return True on success, false otherwise.
		 */
		virtual XML::boolean writeXMLInFile(const IXMLNode &rNode, const char* sPath) const =0;

	protected:
		virtual ~IXMLHandler() { }
	};

	extern OV_API XML::IXMLHandler* createXMLHandler(void);
}

#endif // __XML_IXMLHANDLER_H_
