#ifndef __XML_IXMLNODE_H_
#define __XML_IXMLNODE_H_

#include "defines.h"
#include <string>

namespace XML
{
	/**
	 * @class IXMLNode
	 * @author Serrière Guillaume (INRIA/Loria)
	 * @brief Symbolize a node in a XML tree structure.
	 * @sa XML
	 */
	class OV_API IXMLNode
	{
	public:
		virtual void release(void)=0;

		virtual const char* getName() const =0;

		//Attribute
		/**
		 * @brief Add the attribute sAttributeName with value
		 * sAttributeValue to the node.
		 * @param sAttributeName [in] : Name of the attribute
		 * @param sAttributeValue [in] : Value of the attribute
		 * @return true in success, false otherwise
		 */
		virtual XML::boolean addAttribute(const char* sAttributeName, const char* sAttributeValue)=0;

		/**
		 * @brief Indicate if an attribute exists or not.
		 * @param sAttributeName [in] : Name of the attribute
		 * @return true if attribute exists, false otherwise
		 */
		virtual XML::boolean hasAttribute(const char* sAttributeName) const =0;

		/**
		 * @brief Return the value of an attribute.
		 * @param sAttributeName [in] : Name of the attribute
		 * @return Value of the attribute
		 */
		virtual const char* getAttribute(const char* sAttributeName) const =0;

		//PCDATA
		/**
		 * @brief Set the PCDATA of the node.
		 * @param childData [in] : Value of the PCDATA
		 */
		virtual void setPCData(const char* childData)=0;

		/**
		 * @brief Return the PCDATA of the node.
		 * @return Value of PCDATA
		 */
		virtual const char* getPCData(void) const =0;

		//Child
		/**
		 * @brief Add a node child of the
		 * @param ChildNode [in] : The Node that will became the new child
		 */
		virtual void addChild(XML::IXMLNode* ChildNode)=0;

		/**
		 * @brief Return the ith child of the node.
		 * @param iChildIndex [in] : index of the child.
		 * @return The ith child of the node.
		 */
		virtual XML::IXMLNode* getChild(const XML::uint32 iChildIndex) const =0;

		/**
		 * @brief Return the first child with the name sName.
		 * @param sName [in]] : Name of th child
		 * @return The first child of the node which name is sName.
		 */
		virtual XML::IXMLNode* getChildByName(const char* sName) const =0;

		/**
		 * @brief Return the amount of child the node has.
		 * @return Amount of child.
		 */
		virtual XML::uint32 getChildCount(void) const =0;

		//XML generation
		/**
		 * @brief Return a string which contains the XML of the node. The string is dynamically instantiate so
		 * it requires to be free.
		 * @param depth [in] : Amount of indentation
		 * @return XML string describing the node and its childs.
		 */
		virtual char* getXML(const XML::uint32 depth=0) const =0;

	protected:
		virtual ~IXMLNode(void) {}
	};

	/**
	 * @brief Create a new node with the name sName. The node is created dynamically and requires to be free.
	 * @param sName [in] : Name of the node
	 * @return New node
	 */
	extern OV_API XML::IXMLNode* createNode(const char* sName);

}

#endif // IXMLNODE_H
