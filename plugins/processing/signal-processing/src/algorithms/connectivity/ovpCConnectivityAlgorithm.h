#ifndef __OpenViBEToolkit_Connectivity_Algorithm_H__
#define __OpenViBEToolkit_Connectivity_Algorithm_H__

#include <toolkit/algorithms/ovtkTAlgorithm.h>

#define OVP_ClassId_ConnectivityAlgorithm									OpenViBE::CIdentifier(0xDC90C94B, 0xF82AD423)
#define OVP_ClassId_ConnectivityAlgorithmDesc								OpenViBE::CIdentifier(0x04BCB5D2, 0xC22DBE1B)

#define OVP_Algorithm_Connectivity_InputParameterId_InputMatrix1			OpenViBE::CIdentifier(0xD11D5B9C, 0x006D9855)
#define OVP_Algorithm_Connectivity_InputParameterId_InputMatrix2			OpenViBE::CIdentifier(0x2D2EBE5A, 0xF01E7751)
#define OVP_Algorithm_Connectivity_InputParameterId_LookupMatrix			OpenViBE::CIdentifier(0x004F48CB, 0x6949C4E5)
#define OVP_Algorithm_Connectivity_InputParameterId_ui64SamplingRate1		OpenViBE::CIdentifier(0xFC882648, 0x37AF00C3)
#define OVP_Algorithm_Connectivity_InputParameterId_ui64SamplingRate2		OpenViBE::CIdentifier(0x4822A00F, 0x83CE1ACF)

#define OVP_Algorithm_Connectivity_OutputParameterId_OutputMatrix			OpenViBE::CIdentifier(0x486FE9F4, 0xE3FD2A80)

#define OVP_Algorithm_Connectivity_InputTriggerId_Initialize				OpenViBE::CIdentifier(0xC2C64C31, 0x65FB8B2D)
#define OVP_Algorithm_Connectivity_InputTriggerId_Process					OpenViBE::CIdentifier(0x573C4A07, 0x99475AFC)

#define OVP_Algorithm_Connectivity_OutputTriggerId_ProcessDone				OpenViBE::CIdentifier(0x19287D21, 0xB5604D73)

namespace OpenViBEPlugins
{
		class CConnectivityAlgorithm : public OpenViBEToolkit::TAlgorithm < OpenViBE::Plugins::IAlgorithm >
		{
		public:

				virtual void release(void) {delete this;}

				virtual OpenViBE::boolean initialize(void);
				virtual OpenViBE::boolean uninitialize(void);
				virtual OpenViBE::boolean process(void);

				_IsDerivedFromClass_(OpenViBEToolkit::TAlgorithm < OpenViBE::Plugins::IAlgorithm >, OVP_ClassId_ConnectivityAlgorithm);

		protected:

				OpenViBE::Kernel::TParameterHandler <OpenViBE::IMatrix*> ip_pSignal1;
				OpenViBE::Kernel::TParameterHandler <OpenViBE::IMatrix*> ip_pSignal2;
				OpenViBE::Kernel::TParameterHandler <OpenViBE::uint64> ip_ui64SamplingRate1;
				OpenViBE::Kernel::TParameterHandler <OpenViBE::uint64> ip_ui64SamplingRate2;
				OpenViBE::Kernel::TParameterHandler <OpenViBE::IMatrix*> ip_pChannelPairs;
				OpenViBE::Kernel::TParameterHandler <OpenViBE::IMatrix*> op_pMatrix;
		};

		class CConnectivityAlgorithmDesc : public OpenViBE::Plugins::IAlgorithmDesc
		{
		public:

				virtual OpenViBE::boolean getAlgorithmPrototype(
						OpenViBE::Kernel::IAlgorithmProto& rAlgorithmPrototype) const
					{
					rAlgorithmPrototype.addInputParameter (OVP_Algorithm_Connectivity_InputParameterId_InputMatrix1,     "Signal 1", OpenViBE::Kernel::ParameterType_Matrix);
					rAlgorithmPrototype.addInputParameter (OVP_Algorithm_Connectivity_InputParameterId_InputMatrix2,     "Signal 2", OpenViBE::Kernel::ParameterType_Matrix);
					rAlgorithmPrototype.addInputParameter(OVP_Algorithm_Connectivity_InputParameterId_LookupMatrix, "Pairs of channel", OpenViBE::Kernel::ParameterType_Matrix);
					rAlgorithmPrototype.addInputParameter(OVP_Algorithm_Connectivity_InputParameterId_ui64SamplingRate1, "Sampling Rate of signal 1", OpenViBE::Kernel::ParameterType_UInteger);
					rAlgorithmPrototype.addInputParameter(OVP_Algorithm_Connectivity_InputParameterId_ui64SamplingRate2, "Sampling Rate of signal 2", OpenViBE::Kernel::ParameterType_UInteger);

//					rAlgorithmPrototype.addOutputParameter(OVP_Algorithm_Connectivity_OutputParameterId_OutputMatrix,    "Matrix", OpenViBE::Kernel::ParameterType_Matrix);

					rAlgorithmPrototype.addInputTrigger   (OVP_Algorithm_Connectivity_InputTriggerId_Initialize,   "Initialize");
					rAlgorithmPrototype.addInputTrigger   (OVP_Algorithm_Connectivity_InputTriggerId_Process,      "Process");

					rAlgorithmPrototype.addOutputTrigger  (OVP_Algorithm_Connectivity_OutputTriggerId_ProcessDone, "Process done");

						return true;
					}
				_IsDerivedFromClass_(OpenViBE::Plugins::IAlgorithmDesc, OVP_ClassId_ConnectivityAlgorithmDesc);
		};

};  // namespace OpenViBEPlugins
#endif // __OpenViBEToolkit_Connectivity_Algorithm_H__
