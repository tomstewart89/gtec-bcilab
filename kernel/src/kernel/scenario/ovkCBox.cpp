#include "ovkCBox.h"
#include "ovkCBoxProto.h"
#include "ovkCBoxListenerContext.h"
#include "ovkCScenario.h"

#include "../ovkCObjectVisitorContext.h"

#include <openvibe/ov_defines.h>



namespace{
	//This class is used to set up the restriction of a stream type for input and output. Each box comes with a
	// decriptor that call functions describe in IBoxProto for intialize the CBox object.
	// This implementation is derived from CBoxProto, to benefit from
	// the implementation of the stream restriction mecanism but neutralizes all other initialization function.
	class CBoxProtoRestriction : public OpenViBE::Kernel::CBoxProto
	{
	public:

		CBoxProtoRestriction(const OpenViBE::Kernel::IKernelContext& rKernelContext, OpenViBE::Kernel::IBox& rBox):
			CBoxProto(rKernelContext, rBox){}

		virtual OpenViBE::boolean addInput(
			const OpenViBE::CString& sName,
			const OpenViBE::CIdentifier& rTypeIdentifier){return true;}

		virtual OpenViBE::boolean addMessageInput(
			const OpenViBE::CString& sName){return true;}
		virtual OpenViBE::boolean addMessageOutput(
			const OpenViBE::CString& sName){return true;}

		virtual OpenViBE::boolean addOutput(
			const OpenViBE::CString& sName,
			const OpenViBE::CIdentifier& rTypeIdentifier){return true;}

		virtual OpenViBE::boolean addSetting(
			const OpenViBE::CString& sName,
			const OpenViBE::CIdentifier& rTypeIdentifier,
			const OpenViBE::CString& sDefaultValue,
			const OpenViBE::boolean bModifiable = false){return true;}

		virtual OpenViBE::boolean addFlag(
			const OpenViBE::Kernel::EBoxFlag eBoxFlag){return true;}
	};
}


using namespace std;
using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;


//___________________________________________________________________//
//                                                                   //

CBox::CBox(const IKernelContext& rKernelContext, CScenario& rOwnerScenario)
	:TAttributable < TKernelObject < IBox > >(rKernelContext)
	,m_rOwnerScenario(rOwnerScenario)
	,m_pBoxAlgorithmDescriptor(NULL)
	,m_pBoxListener(NULL)
	,m_bIsNotifyingDescriptor(false)
	,m_bIsNotificationActive(true)
	,m_bIsObserverNotificationActive(true)
	,m_oIdentifier(OV_UndefinedIdentifier)
	,m_oAlgorithmClassIdentifier(OV_UndefinedIdentifier)
	,m_oProcessingUnitIdentifier(OV_UndefinedIdentifier)
	,m_sName("unnamed")
	,m_pSavedState(NULL)
{
	if(this->hasAttribute(OV_AttributeId_Box_Muted))
	{
		this->getLogManager() << LogLevel_Debug << "Muted attribute is"<< this->getAttributeValue(OV_AttributeId_Box_Muted) <<"\n";
	}
}

CBox::~CBox(void)
{
	if(m_pBoxAlgorithmDescriptor && m_pBoxListener)
	{
		CBoxListenerContext l_oContext(this->getKernelContext(), *this, 0xffffffff);
		m_pBoxListener->uninitialize(l_oContext);
		m_pBoxAlgorithmDescriptor->releaseBoxListener(m_pBoxListener);
	}

	if(m_pSavedState)
	{
		delete m_pSavedState;
	}
}

//___________________________________________________________________//
//                                                                   //

CIdentifier CBox::getIdentifier(void) const
{
	return m_oIdentifier;
}

CString CBox::getName(void) const
{
	return m_sName;
}

CIdentifier CBox::getAlgorithmClassIdentifier(void) const
{
	return m_oAlgorithmClassIdentifier;
}

CIdentifier CBox::getProcessingUnitIdentifier(void) const
{
	return m_oProcessingUnitIdentifier;
}

boolean CBox::setIdentifier(
		const CIdentifier& rIdentifier)
{
	if(m_oIdentifier!=OV_UndefinedIdentifier)
	{
		return false;
	}
	if(rIdentifier==OV_UndefinedIdentifier)
	{
		return false;
	}
	m_oIdentifier=rIdentifier;

	this->notify(BoxModification_IdentifierChanged);

	return true;
}

boolean CBox::setName(
		const CString& sName)
{
	m_sName=sName;

	this->notify(BoxModification_NameChanged);

	return true;
}

boolean CBox::setAlgorithmClassIdentifier(
		const CIdentifier& rAlgorithmClassIdentifier)
{
	m_oAlgorithmClassIdentifier=rAlgorithmClassIdentifier;

	if(!getKernelContext().getPluginManager().canCreatePluginObject(rAlgorithmClassIdentifier))
	{
		this->getLogManager() << LogLevel_Warning << "Box algorithm descriptor not found\n";
		return false;
	}

	if(m_pBoxAlgorithmDescriptor && m_pBoxListener)
	{
		CBoxListenerContext l_oContext(this->getKernelContext(), *this, 0xffffffff);
		m_pBoxListener->uninitialize(l_oContext);
		m_pBoxAlgorithmDescriptor->releaseBoxListener(m_pBoxListener);
	}

	const IPluginObjectDesc* l_pPluginObjectDescriptor=getKernelContext().getPluginManager().getPluginObjectDescCreating(rAlgorithmClassIdentifier);
	m_pBoxAlgorithmDescriptor=dynamic_cast<const IBoxAlgorithmDesc*>(l_pPluginObjectDescriptor);

	if(m_pBoxAlgorithmDescriptor)
	{
		m_pBoxListener=m_pBoxAlgorithmDescriptor->createBoxListener();
		if(m_pBoxListener)
		{
			CBoxListenerContext l_oContext(this->getKernelContext(), *this, 0xffffffff);
			m_pBoxListener->initialize(l_oContext);
		}
	}

	//We use the neutralized version of CBoxProto to just initialize the stream restriction mecanism
	CBoxProtoRestriction oTempProto(this->getKernelContext(), *this);
	m_pBoxAlgorithmDescriptor->getBoxPrototype(oTempProto);

	this->notify(BoxModification_AlgorithmClassIdentifierChanged);

	return true;
}

boolean CBox::setProcessingUnitIdentifier(
		const CIdentifier& rProcessingUnitIdentifier)
{
	m_oProcessingUnitIdentifier=rProcessingUnitIdentifier;

	this->notify(BoxModification_ProcessingUnitChanged);

	return true;
}

//___________________________________________________________________//
//                                                                   //

boolean CBox::initializeFromAlgorithmClassIdentifier(
		const CIdentifier& rAlgorithmClassIdentifier)
{
	this->disableNotification();

	const IBoxAlgorithmDesc* l_pBoxAlgorithmDesc=dynamic_cast<const IBoxAlgorithmDesc*>(getKernelContext().getPluginManager().getPluginObjectDescCreating(rAlgorithmClassIdentifier));
	if(!l_pBoxAlgorithmDesc)
	{
		this->getLogManager() << LogLevel_Warning << "Algorithm descriptor not found\n";

		this->enableNotification();

		return false;
	}

	clear();
	setName(l_pBoxAlgorithmDesc->getName());
	setAlgorithmClassIdentifier(rAlgorithmClassIdentifier);

	CBoxProto l_oBoxProto(getKernelContext(), *this);
	l_pBoxAlgorithmDesc->getBoxPrototype(l_oBoxProto);

	if(this->hasAttribute(OV_AttributeId_Box_InitialPrototypeHashValue))
	{
		this->setAttributeValue(OV_AttributeId_Box_InitialPrototypeHashValue, this->getPluginManager().getPluginObjectHashValue(rAlgorithmClassIdentifier).toString());
	}
	else
	{
		this->addAttribute(OV_AttributeId_Box_InitialPrototypeHashValue, this->getPluginManager().getPluginObjectHashValue(rAlgorithmClassIdentifier).toString());
	}

	if(this->hasAttribute(OV_AttributeId_Box_Muted))
	{
		this->getLogManager() << LogLevel_Trace << "Muted attribute is"<< this->getAttributeValue(OV_AttributeId_Box_Muted) <<"\n";
	}
	else
	{
		this->addAttribute(OV_AttributeId_Box_Muted, "false");
	}

	this->enableNotification();

	this->notify(BoxModification_Initialized);

	return true;
}

boolean CBox::initializeFromExistingBox(
		const IBox& rExistingBox)
{
	uint32 i;

	this->disableNotification();
	m_bIsObserverNotificationActive = false;

	clear();
	setName(rExistingBox.getName());
	setAlgorithmClassIdentifier(rExistingBox.getAlgorithmClassIdentifier());

	for(i=0; i<rExistingBox.getInputCount(); i++)
	{
		CIdentifier l_oType;
		CString l_sName;
		rExistingBox.getInputType(i, l_oType);
		rExistingBox.getInputName(i, l_sName);
		addInput(l_sName, l_oType);
	}

	for(i=0; i<rExistingBox.getMessageInputCount(); i++)
	{
		CString l_sName;
		rExistingBox.getMessageInputName(i, l_sName);
		addMessageInput(l_sName);
	}

	for(i=0; i<rExistingBox.getOutputCount(); i++)
	{
		CIdentifier l_oType;
		CString l_sName;
		rExistingBox.getOutputType(i, l_oType);
		rExistingBox.getOutputName(i, l_sName);
		addOutput(l_sName, l_oType);
	}

	for(i=0; i<rExistingBox.getMessageOutputCount(); i++)
	{
		CString l_sName;
		rExistingBox.getMessageOutputName(i, l_sName);
		addMessageOutput(l_sName);
	}

	for(i=0; i<rExistingBox.getSettingCount(); i++)
	{
		CIdentifier l_oType;
		CString l_sName;
		CString l_sValue;
		CString l_sDefaultValue;
		boolean l_bModifiability;

		rExistingBox.getSettingType(i, l_oType);
		rExistingBox.getSettingName(i, l_sName);
		rExistingBox.getSettingValue(i, l_sValue);
		rExistingBox.getSettingDefaultValue(i, l_sDefaultValue);
		rExistingBox.getSettingMod(i, l_bModifiability);
		addSetting(l_sName, l_oType, l_sDefaultValue, -1 ,l_bModifiability);
		setSettingValue(i, l_sValue);
	}

	CIdentifier l_oIdentifier=rExistingBox.getNextAttributeIdentifier(OV_UndefinedIdentifier);
	while(l_oIdentifier!=OV_UndefinedIdentifier)
	{
		addAttribute(l_oIdentifier, rExistingBox.getAttributeValue(l_oIdentifier));
		l_oIdentifier=rExistingBox.getNextAttributeIdentifier(l_oIdentifier);
	}

	CIdentifier l_oStreamTypeIdentifier = OV_UndefinedIdentifier;
	while((l_oStreamTypeIdentifier=this->getKernelContext().getTypeManager().getNextTypeIdentifier(l_oStreamTypeIdentifier))!=OV_UndefinedIdentifier)
	{
		if(this->getKernelContext().getTypeManager().isStream(l_oStreamTypeIdentifier))
		{
			//First check if it is a stream
			if(rExistingBox.hasInputSupport(l_oStreamTypeIdentifier))
			{
				this->addInputSupport(l_oStreamTypeIdentifier);
			}
			if(rExistingBox.hasOutputSupport(l_oStreamTypeIdentifier))
			{
				this->addOutputSupport(l_oStreamTypeIdentifier);
			}
		}
	}

	this->enableNotification();

	this->notify(BoxModification_Initialized);

	m_bIsObserverNotificationActive = true;
	this->notifySettingChange(SettingsAllChange);

	return true;
}

//___________________________________________________________________//
//                                                                   //

boolean CBox::addInput(
		const CString& sName,
		const CIdentifier& rTypeIdentifier)
{
	if(!this->getTypeManager().isStream(rTypeIdentifier))
	{
		if(rTypeIdentifier==OV_UndefinedIdentifier)
		{
			this->getLogManager() << LogLevel_Warning << "Box '" << getName() << "' input '" << sName << "' maps to OV_UndefinedIdentifier. Please configure the box.\n";
		}
		else
		{
			this->getLogManager() << LogLevel_Warning << "While adding input '" << sName << "' to box '" << getName() << "', unknown stream type identifier " << rTypeIdentifier << "\n";
		}
	}

	CInput i;
	i.m_sName=sName;
	i.m_oTypeIdentifier=rTypeIdentifier;
	m_vInput.push_back(i);

	this->notify(BoxModification_InputAdded, m_vInput.size()-1);

	return true;
}

boolean CBox::removeInput(
		const uint32 ui32InputIndex)
{
	CIdentifier l_oIdentifier;
	size_t i;

	if(ui32InputIndex >= m_vInput.size())
	{
		return false;
	}

	while((l_oIdentifier=m_rOwnerScenario.getNextLinkIdentifierToBoxInput(l_oIdentifier, m_oIdentifier, ui32InputIndex))!=OV_UndefinedIdentifier)
	{
		m_rOwnerScenario.disconnect(l_oIdentifier);
	}

	// $$$
	// The way the links are removed here
	// is not correct because they are all
	// collected and then all removed. In case
	// the box listener callback on box removal,
	// the nextcoming links would potentially be
	// invalid
	vector < CIdentifier > l_vLinksToRemove;
	vector < pair < pair < uint64, uint32 >, pair < uint64, uint32 > > > l_vLink;
	while((l_oIdentifier=m_rOwnerScenario.getNextLinkIdentifierToBox(l_oIdentifier, m_oIdentifier))!=OV_UndefinedIdentifier)
	{
		ILink* l_pLink=m_rOwnerScenario.getLinkDetails(l_oIdentifier);
		if(l_pLink->getTargetBoxInputIndex()>ui32InputIndex)
		{
			pair < pair < uint64, uint32 >, pair < uint64, uint32 > > l;
			l.first.first=l_pLink->getSourceBoxIdentifier().toUInteger();
			l.first.second=l_pLink->getSourceBoxOutputIndex();
			l.second.first=l_pLink->getTargetBoxIdentifier().toUInteger();
			l.second.second=l_pLink->getTargetBoxInputIndex();
			l_vLink.push_back(l);
			l_vLinksToRemove.push_back(l_oIdentifier);
		}
	}

	for(i=0; i<l_vLinksToRemove.size(); i++)
	{
		m_rOwnerScenario.disconnect(l_vLinksToRemove[i]);
	}

	m_vInput.erase(m_vInput.begin()+ui32InputIndex);

	for(i=0; i<l_vLink.size(); i++)
	{
		m_rOwnerScenario.connect(
			l_oIdentifier,
			l_vLink[i].first.first,
			l_vLink[i].first.second,
			l_vLink[i].second.first,
			l_vLink[i].second.second-1,
			OV_UndefinedIdentifier);
	}

	this->notify(BoxModification_InputRemoved, ui32InputIndex);

	return true;
}

uint32 CBox::getInputCount(void) const
{
	return m_vInput.size();
}

boolean CBox::getInputType(
		const uint32 ui32InputIndex,
		CIdentifier& rTypeIdentifier) const
{
	if(ui32InputIndex>=m_vInput.size())
	{
		return false;
	}
	rTypeIdentifier=m_vInput[ui32InputIndex].m_oTypeIdentifier;
	return true;
}

boolean CBox::getInputName(
		const uint32 ui32InputIndex,
		CString& rName) const
{
	if(ui32InputIndex>=m_vInput.size())
	{
		return false;
	}
	rName=m_vInput[ui32InputIndex].m_sName;
	return true;
}

boolean CBox::setInputType(
		const uint32 ui32InputIndex,
		const CIdentifier& rTypeIdentifier)
{
	if(!this->getTypeManager().isStream(rTypeIdentifier))
	{
		this->getLogManager() << LogLevel_Warning << "While changing box '" << getName() << "' input type, unknown stream type identifier " << rTypeIdentifier << "\n";
	}

	if(ui32InputIndex>=m_vInput.size())
	{
		return false;
	}
	m_vInput[ui32InputIndex].m_oTypeIdentifier=rTypeIdentifier;

	this->notify(BoxModification_InputTypeChanged, ui32InputIndex);

	return true;
}

boolean CBox::setInputName(
		const uint32 ui32InputIndex,
		const CString& rName)
{
	if(ui32InputIndex>=m_vInput.size())
	{
		return false;
	}
	m_vInput[ui32InputIndex].m_sName=rName;

	this->notify(BoxModification_InputNameChanged, ui32InputIndex);

	return true;
}

//___________________________________________________________________//
//                                                                   //

boolean CBox::addOutput(
		const CString& sName,
		const CIdentifier& rTypeIdentifier)
{
	if(!this->getTypeManager().isStream(rTypeIdentifier))
	{
		if(rTypeIdentifier==OV_UndefinedIdentifier)
		{
			this->getLogManager() << LogLevel_Warning << "Box '" << getName() << "' output '" << sName << "' maps to OV_UndefinedIdentifier. Please configure the box.\n";
		}
		else
		{
			this->getLogManager() << LogLevel_Warning << "While adding output '" << sName << "' to box '" << getName() << "', unknown stream type identifier " << rTypeIdentifier << "\n";
		}
	}

	COutput o;
	o.m_sName=sName;
	o.m_oTypeIdentifier=rTypeIdentifier;
	m_vOutput.push_back(o);

	this->notify(BoxModification_OutputAdded, m_vOutput.size()-1);

	return true;
}

boolean CBox::removeOutput(
		const uint32 ui32OutputIndex)
{
	CIdentifier l_oIdentifier;
	size_t i;

	if(ui32OutputIndex >= m_vOutput.size())
	{
		return false;
	}

	while((l_oIdentifier=m_rOwnerScenario.getNextLinkIdentifierFromBoxOutput(l_oIdentifier, m_oIdentifier, ui32OutputIndex))!=OV_UndefinedIdentifier)
	{
		m_rOwnerScenario.disconnect(l_oIdentifier);
	}

	// $$$
	// The way the links are removed here
	// is not correct because they are all
	// collected and then all removed. In case
	// the box listener callback on box removal,
	// the nextcoming links would potentially be
	// invalid
	vector < CIdentifier > l_vLinksToRemove;
	vector < pair < pair < uint64, uint32 >, pair < uint64, uint32 > > > l_vLink;
	while((l_oIdentifier=m_rOwnerScenario.getNextLinkIdentifierFromBox(l_oIdentifier, m_oIdentifier))!=OV_UndefinedIdentifier)
	{
		ILink* l_pLink=m_rOwnerScenario.getLinkDetails(l_oIdentifier);
		if(l_pLink->getSourceBoxOutputIndex()>ui32OutputIndex)
		{
			pair < pair < uint64, uint32 >, pair < uint64, uint32 > > l;
			l.first.first=l_pLink->getSourceBoxIdentifier().toUInteger();
			l.first.second=l_pLink->getSourceBoxOutputIndex();
			l.second.first=l_pLink->getTargetBoxIdentifier().toUInteger();
			l.second.second=l_pLink->getTargetBoxInputIndex();
			l_vLink.push_back(l);
			l_vLinksToRemove.push_back(l_oIdentifier);
		}
	}

	for(i=0; i<l_vLinksToRemove.size(); i++)
	{
		m_rOwnerScenario.disconnect(l_vLinksToRemove[i]);
	}

	m_vOutput.erase(m_vOutput.begin()+ui32OutputIndex);

	for(i=0; i<l_vLink.size(); i++)
	{
		m_rOwnerScenario.connect(
			l_oIdentifier,
			l_vLink[i].first.first,
			l_vLink[i].first.second-1,
			l_vLink[i].second.first,
			l_vLink[i].second.second,
			OV_UndefinedIdentifier);
	}

	this->notify(BoxModification_OutputRemoved, ui32OutputIndex);

	return true;
}

uint32 CBox::getOutputCount(void) const
{
	return m_vOutput.size();
}

boolean CBox::getOutputType(
		const uint32 ui32OutputIndex,
		CIdentifier& rTypeIdentifier) const
{
	if(ui32OutputIndex>=m_vOutput.size())
	{
		return false;
	}
	rTypeIdentifier=m_vOutput[ui32OutputIndex].m_oTypeIdentifier;
	return true;
}

boolean CBox::getOutputName(
		const uint32 ui32OutputIndex,
		CString& rName) const
{
	if(ui32OutputIndex>=m_vOutput.size())
	{
		return false;
	}
	rName=m_vOutput[ui32OutputIndex].m_sName;
	return true;
}

boolean CBox::setOutputType(
		const uint32 ui32OutputIndex,
		const CIdentifier& rTypeIdentifier)
{
	if(!this->getTypeManager().isStream(rTypeIdentifier))
	{
		this->getLogManager() << LogLevel_Warning << "While changing box '" << getName() << "' output type, unknown stream type identifier " << rTypeIdentifier << "\n";
	}

	if(ui32OutputIndex>=m_vOutput.size())
	{
		return false;
	}
	m_vOutput[ui32OutputIndex].m_oTypeIdentifier=rTypeIdentifier;

	this->notify(BoxModification_OutputTypeChanged, ui32OutputIndex);

	return true;
}

boolean CBox::setOutputName(
		const uint32 ui32OutputIndex,
		const CString& rName)
{
	if(ui32OutputIndex>=m_vOutput.size())
	{
		return false;
	}
	m_vOutput[ui32OutputIndex].m_sName=rName;

	this->notify(BoxModification_OutputNameChanged, ui32OutputIndex);

	return true;
}

boolean CBox::addInputSupport(const OpenViBE::CIdentifier& rTypeIdentifier)
{
	m_vSupportInputType.push_back(rTypeIdentifier);
	return true;
}

boolean CBox::hasInputSupport(const OpenViBE::CIdentifier& rTypeIdentifier) const
{
	//If there is no type specify, we allow all
	if(m_vSupportInputType.empty())
		return true;

	for(size_t i =0; i < m_vSupportInputType.size(); ++i)
	{
		if(m_vSupportInputType[i] == rTypeIdentifier)
			return true;
	}
	return false;
}

boolean CBox::addOutputSupport(const OpenViBE::CIdentifier& rTypeIdentifier)
{
	m_vSupportOutputType.push_back(rTypeIdentifier);
	return true;
}

boolean CBox::hasOutputSupport(const OpenViBE::CIdentifier& rTypeIdentifier) const
{
	//If there is no type specify, we allow all
	if(m_vSupportOutputType.empty())
		return true;

	for(size_t i =0; i < m_vSupportOutputType.size(); ++i)
	{
		if(m_vSupportOutputType[i] == rTypeIdentifier)
			return true;
	}
	return false;
}

boolean CBox::setSupportTypeFromAlgorithmIdentifier(const CIdentifier &rTypeIdentifier)
{

	const IPluginObjectDesc* l_pPluginObjectDescriptor=getKernelContext().getPluginManager().getPluginObjectDescCreating(rTypeIdentifier);
	const IBoxAlgorithmDesc *l_pBoxAlgorithmDescriptor=dynamic_cast<const IBoxAlgorithmDesc*>(l_pPluginObjectDescriptor);
	if(l_pBoxAlgorithmDescriptor == NULL)
	{
		this->getLogManager() << LogLevel_Error << "Tried to initialize with an unregistered algorithm\n";
		return false;
	}

	//We use the neutralized version of CBoxProto to just initialize the stream restriction mecanism
	CBoxProtoRestriction oTempProto(this->getKernelContext(), *this);
	l_pBoxAlgorithmDescriptor->getBoxPrototype(oTempProto);
	return true;
}

//___________________________________________________________________//
//                                                                   //

boolean CBox::addSetting(const CString& sName,
			 const CIdentifier& rTypeIdentifier,
			 const CString& sDefaultValue, const int32 i32Index,
			 const boolean bModifiability)
{
	CString l_sValue(sDefaultValue);
	if(this->getTypeManager().isEnumeration(rTypeIdentifier))
	{
		if(this->getTypeManager().getEnumerationEntryValueFromName(rTypeIdentifier, sDefaultValue)==OV_UndefinedIdentifier)
		{
			if(this-getTypeManager().getEnumerationEntryCount(rTypeIdentifier)!=0)
			{
				// get value to the first enum entry
				// and eventually correct this after
				uint64 l_ui64Value=0;
				this->getTypeManager().getEnumerationEntry(rTypeIdentifier, 0, l_sValue, l_ui64Value);

				// Find if the default value string actually is an identifier, otherwise just keep the zero index name as default.
				CIdentifier l_oIdentifier;
				l_oIdentifier.fromString(sDefaultValue);

				// Finally, if it is an identifier, then a name should be found
				// from the type manager ! Otherwise l_sValue is left to the default.
				CString l_sCandidateValue=this->getTypeManager().getEnumerationEntryNameFromValue(rTypeIdentifier, l_oIdentifier.toUInteger());
				if(l_sCandidateValue!=CString(""))
				{
					l_sValue=l_sCandidateValue;
				}
			}
		}
	}

	CSetting s;
	s.m_sName=sName;
	s.m_oTypeIdentifier=rTypeIdentifier;
	s.m_sDefaultValue=l_sValue;
	s.m_sValue=l_sValue;
	s.m_bMod=bModifiability;

	int32 l_i32Index = i32Index;

	if(i32Index>static_cast<int32>(m_vSetting.size())) {
		// Don't accept pushes that are not either inside the existing array or an append right at the end
		this->getLogManager() << LogLevel_Error << "Tried to push '" << sName << "' to slot " << i32Index << " with the array size being " << static_cast<int32>(m_vSetting.size()) << "\n";
		return false;
	}

	int32 l_i32InsertLocation;

	if(i32Index < 0 || i32Index == static_cast<int32>(m_vSetting.size()))
	{
		m_vSetting.push_back(s);
		l_i32InsertLocation = (static_cast<int32>(m_vSetting.size()))-1;
	}
	else
	{
		vector<CSetting>::iterator l_it = m_vSetting.begin();
		l_it += l_i32Index;
		m_vSetting.insert(l_it, s);
		l_i32InsertLocation = i32Index;
		
	}

	//if this setting is modifiable, keep its index
	if(bModifiability)
	{
		m_vModifiableSettingIndexes.push_back(l_i32Index);
	}

	this->getLogManager() << LogLevel_Debug << "Pushed '" << m_vSetting[l_i32InsertLocation].m_sName << "' : '" << m_vSetting[l_i32InsertLocation].m_sValue << "' to slot " << l_i32InsertLocation << " with the array size now " << static_cast<int32>(m_vSetting.size()) << "\n";

	this->notify(BoxModification_SettingAdded, l_i32InsertLocation);
	this->notifySettingChange(SettingAdd, l_i32InsertLocation);

	return true;
}

boolean CBox::removeSetting(
		const uint32 ui32SettingIndex)
{
	uint32 i=0;
	vector<CSetting>::iterator it=m_vSetting.begin();
	for(i=0; i<ui32SettingIndex && it!=m_vSetting.end(); i++)
	{
		it++;
	}
	if(it==m_vSetting.end())
	{
		return false;
	}

	it=m_vSetting.erase(it);

	//update the modifiable setting indexes
	vector<uint32>::iterator it2=m_vModifiableSettingIndexes.begin();
	for (i=0; i<m_vModifiableSettingIndexes.size(); i++)
	{
		if(m_vModifiableSettingIndexes[i]==ui32SettingIndex)
		{
			m_vModifiableSettingIndexes.erase(it2);
		}
		else if(m_vModifiableSettingIndexes[i]>ui32SettingIndex)
		{
			m_vModifiableSettingIndexes[i]-=1;
		}
		it2++;
	}

	this->notify(BoxModification_SettingRemoved, ui32SettingIndex);
	this->notifySettingChange(SettingDelete, ui32SettingIndex);

	return true;
}

uint32 CBox::getSettingCount(void) const
{
	return m_vSetting.size();
}

boolean CBox::getSettingType(
		const uint32 ui32SettingIndex,
		CIdentifier& rTypeIdentifier) const
{
	if(ui32SettingIndex>=m_vSetting.size())
	{
		return false;
	}
	rTypeIdentifier=m_vSetting[ui32SettingIndex].m_oTypeIdentifier;
	return true;
}

boolean CBox::getSettingName(
		const uint32 ui32SettingIndex,
		CString& rName) const
{
	if(ui32SettingIndex>=m_vSetting.size())
	{
		return false;
	}
	rName=m_vSetting[ui32SettingIndex].m_sName;
	return true;
}

boolean CBox::getSettingDefaultValue(
		const uint32 ui32SettingIndex,
		CString& rDefaultValue) const
{
	if(ui32SettingIndex>=m_vSetting.size())
	{
		return false;
	}
	rDefaultValue=m_vSetting[ui32SettingIndex].m_sDefaultValue;
	return true;
}

boolean CBox::getSettingValue(
		const uint32 ui32SettingIndex,
		CString& rValue) const
{
	if(ui32SettingIndex>=m_vSetting.size())
	{
		return false;
	}
	rValue=m_vSetting[ui32SettingIndex].m_sValue;
	return true;
}

boolean CBox::setSettingType(
		const uint32 ui32SettingIndex,
		const CIdentifier& rTypeIdentifier)
{
	if(ui32SettingIndex>=m_vSetting.size())
	{
		return false;
	}
	if(m_vSetting[ui32SettingIndex].m_oTypeIdentifier == rTypeIdentifier)
	{
		// no change, don't bother notifying
		return true;
	}

	m_vSetting[ui32SettingIndex].m_oTypeIdentifier=rTypeIdentifier;

	this->notify(BoxModification_SettingTypeChanged, ui32SettingIndex);
	this->notifySettingChange(SettingChange, ui32SettingIndex);

	return true;
}

boolean CBox::setSettingName(
		const uint32 ui32SettingIndex,
		const CString& rName)
{
	if(ui32SettingIndex>=m_vSetting.size())
	{
		return false;
	}
	m_vSetting[ui32SettingIndex].m_sName=rName;

	this->notify(BoxModification_SettingNameChanged, ui32SettingIndex);
	this->notifySettingChange(SettingChange, ui32SettingIndex);

	return true;
}

boolean CBox::setSettingDefaultValue(
		const uint32 ui32SettingIndex,
		const CString& rDefaultValue)
{
	if(ui32SettingIndex>=m_vSetting.size())
	{
		return false;
	}
	m_vSetting[ui32SettingIndex].m_sDefaultValue=rDefaultValue;

	this->notify(BoxModification_SettingDefaultValueChanged, ui32SettingIndex);

	return true;
}

boolean CBox::setSettingValue(
		const uint32 ui32SettingIndex,
		const CString& rValue)
{
	if(ui32SettingIndex>=m_vSetting.size())
	{
		return false;
	}
	m_vSetting[ui32SettingIndex].m_sValue=rValue;

	this->notify(BoxModification_SettingValueChanged, ui32SettingIndex);
	this->notifySettingChange(SettingValueUpdate, ui32SettingIndex);

	return true;
}

void CBox::notifySettingChange(BoxEventMessageType eType, int32 i32FirstIndex, int32 i32SecondIndex)
{
	if( m_bIsObserverNotificationActive)
	{
		BoxEventMessage l_oEvent;
		l_oEvent.m_eType = eType;
		l_oEvent.m_i32FirstIndex = i32FirstIndex;
		l_oEvent.m_i32SecondIndex = i32SecondIndex;

		this->setChanged();
		this->notifyObservers(&l_oEvent);
	}
}

//*
boolean CBox::getSettingMod(
		const OpenViBE::uint32 ui32SettingIndex,
		OpenViBE::boolean& rValue) const
{
	if(ui32SettingIndex>=m_vSetting.size())
	{
		return false;
	}
	rValue=m_vSetting[ui32SettingIndex].m_bMod;
	return true;
}


boolean CBox::setSettingMod(
		const OpenViBE::uint32 ui32SettingIndex,
		const OpenViBE::boolean rValue)
{
	if(ui32SettingIndex>=m_vSetting.size())
	{
		return false;
	}
	m_vSetting[ui32SettingIndex].m_bMod=rValue;

	//this->notify(BoxModification_SettingNameChanged, ui32SettingIndex);
	return true;
}

boolean CBox::hasModifiableSettings(void)const
{
	uint32 i=0;
	boolean rValue = false;
	while((i<m_vSetting.size())&&(!rValue))
	{
		rValue = m_vSetting[i].m_bMod;
		i++;
	}
	return rValue;
}

uint32* CBox::getModifiableSettings(uint32& rCount)const
{
	uint32* l_pReturn = NULL;
	rCount = m_vModifiableSettingIndexes.size();

	return l_pReturn;

}


//*/

//___________________________________________________________________//
//                                                                   //

void CBox::clear(void)
{
	if(m_pBoxAlgorithmDescriptor && m_pBoxListener)
	{
		CBoxListenerContext l_oContext(this->getKernelContext(), *this, 0xffffffff);
		m_pBoxListener->uninitialize(l_oContext);
		m_pBoxAlgorithmDescriptor->releaseBoxListener(m_pBoxListener);
	}

	m_pBoxAlgorithmDescriptor=NULL;
	m_oAlgorithmClassIdentifier=OV_UndefinedIdentifier;
	m_sName="";
	m_vInput.clear();
	m_vOutput.clear();
	m_vSetting.clear();
	m_vMessageInput.clear();
	m_vMessageOutput.clear();
	this->removeAllAttributes();
}

void CBox::enableNotification(void)
{
	m_bIsNotificationActive=true;
}

void CBox::disableNotification(void)
{
	m_bIsNotificationActive=false;
}

void CBox::notify(
		const EBoxModification eBoxModificationType,
		const uint32 ui32Index)
{
	if(m_pBoxListener && !m_bIsNotifyingDescriptor && m_bIsNotificationActive)
	{
		CBoxListenerContext l_oContext(this->getKernelContext(), *this, ui32Index);
		m_bIsNotifyingDescriptor=true;
		m_pBoxListener->process(l_oContext, eBoxModificationType);
		m_bIsNotifyingDescriptor=false;
	}
}

//___________________________________________________________________//
//                                                                   //

boolean CBox::acceptVisitor(
		IObjectVisitor& rObjectVisitor)
{
	CObjectVisitorContext l_oObjectVisitorContext(getKernelContext());
	return rObjectVisitor.processBegin(l_oObjectVisitorContext, *this) && rObjectVisitor.processEnd(l_oObjectVisitorContext, *this);
}

boolean CBox::addMessageInput(
		const CString& sName)
{
	//this->getLogManager() << LogLevel_Fatal << "adding message input named "<< sName << "for box "<< m_sName << "\n";
	CMessageInput l_oMessageInput;
	l_oMessageInput.m_sName = sName;
	m_vMessageInput.push_back(l_oMessageInput);

	this->notify(BoxModification_MessageInputAdded, m_vMessageInput.size()-1);

	return true;
}

boolean CBox::removeMessageInput(
		const uint32 ui32InputIndex)
{
	CIdentifier l_oIdentifier;
	size_t i;

	if(ui32InputIndex >= m_vMessageInput.size())
	{
		return false;
	}

	while((l_oIdentifier=m_rOwnerScenario.getNextMessageLinkIdentifierToBoxInput(l_oIdentifier, m_oIdentifier, ui32InputIndex))!=OV_UndefinedIdentifier)
	{
		m_rOwnerScenario.disconnectMessage(l_oIdentifier);
	}

	// $$$
	// The way the links are removed here
	// is not correct because they are all
	// collected and then all removed. In case
	// the box listener callback on box removal,
	// the nextcoming links would potentially be
	// invalid
	vector < CIdentifier > l_vMessageLinksToRemove;
	vector < pair < pair < uint64, uint32 >, pair < uint64, uint32 > > > l_vMessageLink;
	while((l_oIdentifier=m_rOwnerScenario.getNextMessageLinkIdentifierToBox(l_oIdentifier, m_oIdentifier))!=OV_UndefinedIdentifier)
	{
		ILink* l_pLink=m_rOwnerScenario.getMessageLinkDetails(l_oIdentifier);
		if(l_pLink->getTargetBoxInputIndex()>ui32InputIndex)
		{
			pair < pair < uint64, uint32 >, pair < uint64, uint32 > > l;
			l.first.first=l_pLink->getSourceBoxIdentifier().toUInteger();
			l.first.second=l_pLink->getSourceBoxOutputIndex();
			l.second.first=l_pLink->getTargetBoxIdentifier().toUInteger();
			l.second.second=l_pLink->getTargetBoxInputIndex();
			l_vMessageLink.push_back(l);
			l_vMessageLinksToRemove.push_back(l_oIdentifier);
		}
	}

	for(i=0; i<l_vMessageLinksToRemove.size(); i++)
	{
		m_rOwnerScenario.disconnectMessage(l_vMessageLinksToRemove[i]);
	}

	m_vMessageInput.erase(m_vMessageInput.begin()+ui32InputIndex);

	for(i=0; i<l_vMessageLink.size(); i++)
	{
		m_rOwnerScenario.connectMessage(
			l_oIdentifier,
			l_vMessageLink[i].first.first,
			l_vMessageLink[i].first.second,
			l_vMessageLink[i].second.first,
			l_vMessageLink[i].second.second-1,
			OV_UndefinedIdentifier);
	}

	this->notify(BoxModification_MessageInputRemoved, ui32InputIndex);

	return true;
}

uint32 CBox::getMessageInputCount(void) const
{
	//this->getLogManager() << LogLevel_Fatal << "box "<< m_sName << " has " << (uint64)m_vMessageInput.size() << " message input\n";
	return m_vMessageInput.size();
}



boolean CBox::getMessageInputName(
		const uint32 ui32InputIndex,
		CString& rName) const
{
	if(ui32InputIndex>=m_vMessageInput.size())
	{
		return false;
	}
	rName=m_vMessageInput[ui32InputIndex].m_sName;
	return true;
}


boolean CBox::setMessageInputName(
		const uint32 ui32InputIndex,
		const CString& rName)
{
	if(ui32InputIndex>=m_vMessageInput.size())
	{
		return false;
	}
	m_vMessageInput[ui32InputIndex].m_sName=rName;

	this->notify(BoxModification_MessageInputNameChanged, ui32InputIndex);

	return true;
}

//
boolean CBox::addMessageOutput(
		const CString& sName)
{
	//this->getLogManager() << LogLevel_Fatal << "adding message Output named "<< sName << "for box "<< m_sName << "\n";
	CMessageOutput l_oMessageOutput;
	l_oMessageOutput.m_sName = sName;
	m_vMessageOutput.push_back(l_oMessageOutput);

	this->notify(BoxModification_MessageOutputAdded, m_vMessageOutput.size()-1);

	return true;
}

boolean CBox::removeMessageOutput(
		const uint32 ui32OutputIndex)
{
	CIdentifier l_oIdentifier;
	size_t i;

	if(ui32OutputIndex >= m_vMessageOutput.size())
	{
		return false;
	}

	while((l_oIdentifier=m_rOwnerScenario.getNextMessageLinkIdentifierFromBoxOutput(l_oIdentifier, m_oIdentifier, ui32OutputIndex))!=OV_UndefinedIdentifier)
	{
		m_rOwnerScenario.disconnectMessage(l_oIdentifier);
	}

	// $$$
	// The way the links are removed here
	// is not correct because they are all
	// collected and then all removed. In case
	// the box listener callback on box removal,
	// the nextcoming links would potentially be
	// invalid
	vector < CIdentifier > l_vMessageLinksToRemove;
	vector < pair < pair < uint64, uint32 >, pair < uint64, uint32 > > > l_vMessageLink;
	while((l_oIdentifier=m_rOwnerScenario.getNextMessageLinkIdentifierFromBox(l_oIdentifier, m_oIdentifier))!=OV_UndefinedIdentifier)
	{
		ILink* l_pLink=m_rOwnerScenario.getMessageLinkDetails(l_oIdentifier);
		if(l_pLink->getSourceBoxOutputIndex()>ui32OutputIndex)
		{
			pair < pair < uint64, uint32 >, pair < uint64, uint32 > > l;
			l.first.first=l_pLink->getSourceBoxIdentifier().toUInteger();
			l.first.second=l_pLink->getSourceBoxOutputIndex();
			l.second.first=l_pLink->getTargetBoxIdentifier().toUInteger();
			l.second.second=l_pLink->getTargetBoxInputIndex();
			l_vMessageLink.push_back(l);
			l_vMessageLinksToRemove.push_back(l_oIdentifier);
		}
	}

	for(i=0; i<l_vMessageLinksToRemove.size(); i++)
	{
		m_rOwnerScenario.disconnectMessage(l_vMessageLinksToRemove[i]);
	}

	m_vMessageOutput.erase(m_vMessageOutput.begin()+ui32OutputIndex);

	for(i=0; i<l_vMessageLink.size(); i++)
	{
		m_rOwnerScenario.connectMessage(
			l_oIdentifier,
			l_vMessageLink[i].first.first,
			l_vMessageLink[i].first.second-1,
			l_vMessageLink[i].second.first,
			l_vMessageLink[i].second.second,
			OV_UndefinedIdentifier);
	}

	this->notify(BoxModification_MessageOutputRemoved, ui32OutputIndex);

	return true;
}

uint32 CBox::getMessageOutputCount(void) const
{
	//this->getLogManager() << LogLevel_Fatal << "box "<< m_sName << " has " << (uint64)m_vMessageOutput.size() << " message Output\n";
	return m_vMessageOutput.size();
}



boolean CBox::getMessageOutputName(
		const uint32 ui32InputIndex,
		CString& rName) const
{
	if(ui32InputIndex>=m_vMessageOutput.size())
	{
		return false;
	}
	rName=m_vMessageOutput[ui32InputIndex].m_sName;
	return true;
}


boolean CBox::setMessageOutputName(
		const uint32 ui32InputIndex,
		const CString& rName)
{
	if(ui32InputIndex>=m_vMessageOutput.size())
	{
		return false;
	}
	m_vMessageOutput[ui32InputIndex].m_sName=rName;

	this->notify(BoxModification_MessageOutputNameChanged, ui32InputIndex);

	return true;
}

void CBox::storeState(void)
{
	if(m_pSavedState != NULL)
	{
		delete m_pSavedState;
	}
	m_pSavedState = new CBox(getKernelContext(), m_rOwnerScenario);
	m_pSavedState->initializeFromExistingBox(*this);
}

void CBox::restoreState(void)
{
	if(!m_pSavedState) 
	{
		this->getLogManager() << LogLevel_Warning << "Tried to restore state with no state saved\n";
		return;
	}
	this->initializeFromExistingBox(*m_pSavedState);
}

//

