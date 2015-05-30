#include "ovdCSettingViewFactory.h"

#include "ovdCBooleanSettingView.h"
#include "ovdCIntegerSettingView.h"
#include "ovdCFloatSettingView.h"
#include "ovdCStringSettingView.h"
#include "ovdCFilenameSettingView.h"
#include "ovdCScriptSettingView.h"
#include "ovdCColorSettingView.h"
#include "ovdCColorGradientSettingView.h"
#include "ovdCEnumerationSettingView.h"
#include "ovdCBitMaskSettingView.h"

using namespace OpenViBEDesigner;
using namespace OpenViBE;
using namespace OpenViBEDesigner::Setting;

CSettingViewFactory::CSettingViewFactory(const CString &rBuilderName, const OpenViBE::Kernel::IKernelContext& rKernelContext):
	m_sBuilderName(rBuilderName), m_rKernelContext(rKernelContext)
{

}

CSettingViewFactory::~CSettingViewFactory()
{

}

CAbstractSettingView *CSettingViewFactory::getSettingView(Kernel::IBox &rBox,
														  uint32 ui32Index)
{
	CIdentifier l_oSettingType;
	rBox.getSettingType(ui32Index, l_oSettingType);

	if(l_oSettingType==OV_TypeId_Boolean)
		return new CBooleanSettingView(rBox, ui32Index, m_sBuilderName);
	if(l_oSettingType==OV_TypeId_Integer)
		return new CIntegerSettingView(rBox, ui32Index, m_sBuilderName, m_rKernelContext);
	if(l_oSettingType==OV_TypeId_Float)
		return new CFloatSettingView(rBox, ui32Index, m_sBuilderName, m_rKernelContext);
	if(l_oSettingType==OV_TypeId_String)
		return new CStringSettingView(rBox, ui32Index, m_sBuilderName);
	if(l_oSettingType==OV_TypeId_Filename)
		return new CFilenameSettingView(rBox, ui32Index, m_sBuilderName, m_rKernelContext);
	if(l_oSettingType==OV_TypeId_Script)
		return new CScriptSettingView(rBox, ui32Index, m_sBuilderName, m_rKernelContext);
	if(l_oSettingType==OV_TypeId_Color)
		return new CColorSettingView(rBox, ui32Index, m_sBuilderName, m_rKernelContext);
	if(l_oSettingType==OV_TypeId_ColorGradient)
		return new CColorGradientSettingView(rBox, ui32Index, m_sBuilderName, m_rKernelContext);
	if(m_rKernelContext.getTypeManager().isEnumeration(l_oSettingType))
		return new CEnumerationSettingView(rBox, ui32Index, m_sBuilderName, m_rKernelContext, l_oSettingType);
	if(m_rKernelContext.getTypeManager().isBitMask(l_oSettingType))
		return new CBitMaskSettingView(rBox, ui32Index, m_sBuilderName, m_rKernelContext, l_oSettingType);

	//By default we consider every settings as a string
	return new CStringSettingView(rBox, ui32Index, m_sBuilderName);
}
