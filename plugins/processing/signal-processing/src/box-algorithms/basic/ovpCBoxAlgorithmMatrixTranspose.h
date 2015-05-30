#ifndef __OpenViBEPlugins_BoxAlgorithm_MatrixTranspose_H__
#define __OpenViBEPlugins_BoxAlgorithm_MatrixTranspose_H__

#include "../../ovp_defines.h"
#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>


namespace OpenViBEPlugins
{
	namespace SignalProcessing
	{
		class CBoxAlgorithmMatrixTranspose : virtual public OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >
		{
		public:

			virtual void release(void) { delete this; }

			virtual OpenViBE::boolean initialize(void);
			virtual OpenViBE::boolean uninitialize(void);
			virtual OpenViBE::boolean processInput(OpenViBE::uint32 ui32InputIndex);
			virtual OpenViBE::boolean process(void);

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >, OVP_ClassId_BoxAlgorithm_MatrixTranspose);

		protected:

			OpenViBEToolkit::TStreamedMatrixDecoder < CBoxAlgorithmMatrixTranspose > m_oDecoder;
			OpenViBEToolkit::TStreamedMatrixEncoder < CBoxAlgorithmMatrixTranspose > m_oEncoder;

		};


		class CBoxAlgorithmMatrixTransposeDesc : virtual public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			virtual void release(void) { }

			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("Matrix Transpose"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Jussi T. Lindgren"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("Inria"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("Transposes each matrix of the input stream"); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString("Only works for 1 and 2 dimensional matrices. One-dimensional matrixes will be upgraded to two dimensions: [N x 1]"); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("Signal processing/Basic"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("1.0"); }
			virtual OpenViBE::CString getStockItemName(void) const       { return OpenViBE::CString("gtk-sort-ascending"); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_BoxAlgorithm_MatrixTranspose; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::SignalProcessing::CBoxAlgorithmMatrixTranspose; }

			virtual OpenViBE::boolean getBoxPrototype(
				OpenViBE::Kernel::IBoxProto& rBoxAlgorithmPrototype) const
			{
				rBoxAlgorithmPrototype.addInput  ("Input matrix",  OV_TypeId_StreamedMatrix);
				rBoxAlgorithmPrototype.addOutput ("Output matrix", OV_TypeId_StreamedMatrix);

				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_BoxAlgorithm_MatrixTransposeDesc);
		};
	};
};

#endif // __OpenViBEPlugins_BoxAlgorithm_MatrixTranspose_H__
