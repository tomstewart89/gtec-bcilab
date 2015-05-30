#ifndef __OpenViBE_Kernel_Player_IMessageWithData_H__
#define __OpenViBE_Kernel_Player_IMessageWithData_H__

#include "../ovIKernelObject.h"

namespace OpenViBE
{
	namespace Kernel
	{
		/**
		 * \class IMessageWithData
		 * \author Loic Mahe (Inria)
		 * \date 2013-10-02
		 * \brief A type of message that can contain different kinds of arbitrary data
		 * \ingroup Group_Player
		 * \ingroup Group_Kernel
		 *
		 * A message that can contain different kinds of data. The intended usage is for the message to be exchanged between boxes. 
		 * A message can hold four types of data: uint64, float64, CString and IMatrix.
		 * Each data item is accessed by its string identifier key using a getter/setter corresponding to the data type. 
		 * The key is unique within the data type.
		 *
		 * Although the IMessageWithData inherits IMessage, the parent class' identifier and timestamp fields are not automatically filled.
		 * For example, we are not timestamping the message on its creation. This is because the IMessageWithData is a type that 
		 * was designed to be immediately processed anyway during the same kernel scheduler tick. So for most use-cases, 
		 * the time stamp does not make sense. If needed (for example for debug purposes), the caller who fills the message
		 * can also stamp it or set it an identifier using the setters inherited from the parent class.
		 *
		 */
		class OV_API IMessageWithData : public OpenViBE::Kernel::IMessage
		{
		public:
			//@}
			/** \name Getters */
			//@{

			// Note that any returned pointers from the getters are invalid after processMessage() scope has passed.

			/**
			 * \brief Gets the integer value stored under this key
			 * \param key : a reference to the name of the key
			 * \param rValueOut : the associated data. Unmodified in case of error.
			 * \return \e true in case of success
			 * \return \e false in case of error
			 */
				virtual bool getValueUint64(const CString &key, OpenViBE::uint64& rValueOut) const=0;
			/**
			 * \brief Gets the float value stored under this key
			 * \param key : a reference to the name of the key
			 * \param rValueOut : the associated data. Unmodified in case of error.
			 * \return \e true in case of success
			 * \return \e false in case of error
			 */
				virtual bool getValueFloat64(const CString &key, OpenViBE::float64& rValueOut) const=0;
			/**
			 * \brief Gets a pointer to the CString value stored under this key
			 * \note User should copy the content, the returned pointer will be invalid later.
			 * \param key : a reference to the name of the key
			 * \param pValueOut : pointer to the associated data. NULL in case of error. Do not free.
			 * \return \e true if fetched ok, false otherwise
			 */
				virtual bool getValueCString(const CString &key, const OpenViBE::CString** pValueOut) const=0;
			/**
			 * \brief Gets a pointer to the CMatrix value stored under this key
			 * \note User should copy the content, the returned pointer will be invalid later.
			 * \param key : a reference to the name of the key
			 * \param pValueOut : pointer to the associated data. NULL in case of error. Do not free.
			 * \return \e true in case of success
			 * \return \e false in case of error
			 */
				virtual bool getValueIMatrix(const CString &key, const OpenViBE::IMatrix** pOutMatrix) const=0;
			//@}
			/** \name Setters */
			//@{
			/**
			 * \brief Sets the message internal UInt64 value stored under this key
			 * \param key : the name of the key
			 * \param valueIn : the value to put into the message
			 * \return \e true in case of success
			 * \return \e false in case of error
			 */
				virtual bool setValueUint64(const CString &key, uint64 valueIn)=0;
			/**
			 * \brief Sets the message internal Float64 value stored under this key
			 * \param key : the name of the key
			 * \param valueIn : the value to put in the message
			 * \return \e true in case of success
			 * \return \e false in case of error
			 */
				virtual bool setValueFloat64(const CString &key, float64 valueIn)=0;
			/**
			 * \brief Sets the message internal CString value stored under this key
			 * \note The message will make an internal full copy of valueIn.
			 * \param key : the name of the key
			 * \param valueIn : the value to put in the message
			 * \return \e true in case of success
			 * \return \e false in case of error
			 */
				virtual bool setValueCString(const CString &key, const CString &valueIn)=0;
			/**
			 * \brief Sets the message internal IMatrix value stored under this key
			 * \note The message will make an internal full copy of valueIn.
			 * \param key : the name of the key
			 * \param valueIn : the data to put in the message
			 * \return \e true in case of success
			 * \return \e false in case of error
			 */
				virtual bool setValueIMatrix(const CString &key, const IMatrix &valueIn)=0;
			//@}
			/** \name Getters and iterators for keys */
			//@{
			/**
			 * \brief Get the first key of the CString container
			 * \return \e a pointer to the key in case of success
			 * \return \e NULL in case of error or container is empty
			 */
				virtual const OpenViBE::CString* getFirstCStringToken() const=0;
			/**
			 * \brief Get the first key of the UInt64 container
			 * \return \e a pointer to the key in case of success
			 * \return \e NULL in case of error or container is empty
			 */
				virtual const OpenViBE::CString* getFirstUInt64Token() const=0;
			/**
			 * \brief Get the first key of the Float64 container
			 * \return \e a pointer to the key in case of success
			 * \return \e NULL in case of error or container is empty
			 */
				virtual const OpenViBE::CString* getFirstFloat64Token() const=0;
			/**
			 * \brief Get the first key of the CMatrix container
			 * \return \e a pointer to the key in case of success
			 * \return \e NULL in case of error or container is empty
			 */
				virtual const OpenViBE::CString* getFirstIMatrixToken() const=0;
			/**
			 * \brief Get the next key of the CString container
			 * \param previousToken : a reference to the previous key
			 * \return \e a pointer to the key in case of success
			 * \return \e NULL in case of error or if no more tokens are available
			 */
				virtual const OpenViBE::CString* getNextCStringToken(const OpenViBE::CString &previousToken) const=0;
			/**
			 * \brief Get the next key of the UInt64 container
			 * \param previousToken : a reference to the previous key
			 * \return \e a pointer to the key in case of success
			 * \return \e NULL in case of error or if no more tokens are available
			 */
				virtual const OpenViBE::CString* getNextUInt64Token(const OpenViBE::CString &previousToken) const=0;
			/**
			 * \brief Get the next key of the Float64 container
			 * \param previousToken : a reference to the previous key
			 * \return \e a pointer to the key in case of success
			 * \return \e NULL in case of error or if no more tokens are available
			 */
				virtual const OpenViBE::CString* getNextFloat64Token(const OpenViBE::CString &previousToken) const=0;
			/**
			 * \brief Get the next key of the CMatrix container
			 * \param previousToken : a reference to the previous key
			 * \return \e a pointer to the key in case of success
			 * \return \e NULL in case of error or if no more tokens are available
			 */
				virtual const OpenViBE::CString* getNextIMatrixToken(const OpenViBE::CString &previousToken) const=0;
			//@}

			_IsDerivedFromClass_(OpenViBE::Kernel::IMessage, OV_ClassId_Kernel_Player_MessageWithData)

		};
	};
};

#endif // __OpenViBE_Kernel_Player_IMessageWithData_H__
