#ifndef __OpenViBEKernel_Kernel_Player_CMessageWithData_H__
#define __OpenViBEKernel_Kernel_Player_CMessageWithData_H__

#include "../ovkTKernelObject.h"
#include "ovkTMessage.h"

#include <map>
#include <openvibe/ov_all.h>

namespace OpenViBE
{
	namespace Kernel
	{
		class CMessageWithData : public OpenViBE::Kernel::TMessage<OpenViBE::Kernel::TKernelObject<OpenViBE::Kernel::IMessageWithData> >
		{
		public:

			CMessageWithData(const OpenViBE::Kernel::IKernelContext& rKernelContext);
			~CMessageWithData();

			// Getters
			// Note that returned pointers are invalid after processMessage() scope has passed.
			bool getValueUint64(const CString &key, OpenViBE::uint64& rValueOut) const;
			bool getValueFloat64(const CString &key, OpenViBE::float64& rValueOut) const;
			bool getValueCString(const CString &key, const OpenViBE::CString** pValueOut) const;
			bool getValueIMatrix(const CString &key, const OpenViBE::IMatrix** pValueOut) const;

			// Setters
			bool setValueUint64(const CString &key, uint64 valueIn);
			bool setValueFloat64(const CString &key, float64 valueIn);
			bool setValueCString(const CString &key, const CString &valueIn);
			bool setValueIMatrix(const CString &key, const IMatrix &valueIn);
			
			// Getters for key tokens
			virtual const OpenViBE::CString* getFirstCStringToken() const;
			virtual const OpenViBE::CString* getFirstUInt64Token() const;
			virtual const OpenViBE::CString* getFirstFloat64Token() const;
			virtual const OpenViBE::CString* getFirstIMatrixToken() const;

			// Functions to iterate over key tokens, start with NULL.
			virtual const OpenViBE::CString* getNextCStringToken(const OpenViBE::CString &previousToken) const;
			virtual const OpenViBE::CString* getNextUInt64Token(const OpenViBE::CString &previousToken) const;
			virtual const OpenViBE::CString* getNextFloat64Token(const OpenViBE::CString &previousToken) const;
			virtual const OpenViBE::CString* getNextIMatrixToken(const OpenViBE::CString &previousToken) const;

		private:
			// Data definitions
			std::map<CString, uint64> m_oUint64s;
			std::map<CString, CString > m_oStrings;
			std::map<CString, float64> m_oFloat64s;
			std::map<CString, IMatrix* > m_oMatrices;

			_IsDerivedFromClass_Final_(OpenViBE::Kernel::TMessage<OpenViBE::Kernel::TKernelObject<OpenViBE::Kernel::IMessageWithData> >, OVK_ClassId_Kernel_Player_MessageWithData);
		};
	};
};

#endif // __OpenViBEKernel_Kernel_Player_CMessageWithData_H__
