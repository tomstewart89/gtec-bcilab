#include "ovkCPlayerManager.h"
#include "ovkCPlayer.h"

#include "../../ovk_tools.h"

#include <system/ovCMath.h>

using namespace OpenViBE;
using namespace Kernel;
using namespace std;

CPlayerManager::CPlayerManager(const IKernelContext& rKernelContext)
	:TKernelObject<IPlayerManager>(rKernelContext)
{
}

boolean CPlayerManager::createPlayer(
	CIdentifier& rPlayerIdentifier)
{
	rPlayerIdentifier=getUnusedIdentifier();
	m_vPlayer[rPlayerIdentifier]=new CPlayer(getKernelContext());
	return true;
}

boolean CPlayerManager::releasePlayer(
	const CIdentifier& rPlayerIdentifier)
{
	map<CIdentifier, CPlayer*>::iterator itPlayer;
	itPlayer=m_vPlayer.find(rPlayerIdentifier);
	if(itPlayer==m_vPlayer.end())
	{
		return false;
	}
	delete itPlayer->second;
	m_vPlayer.erase(itPlayer);
	return true;
}

IPlayer& CPlayerManager::getPlayer(
	const CIdentifier& rPlayerIdentifier)
{
	map<CIdentifier, CPlayer*>::const_iterator itPlayer;
	itPlayer=m_vPlayer.find(rPlayerIdentifier);
	if(itPlayer==m_vPlayer.end())
	{
		this->getLogManager() << LogLevel_Fatal << "Player " << rPlayerIdentifier << " does not exist !\n";
	}
	if(!itPlayer->second)
	{
		this->getLogManager() << LogLevel_Fatal << "NULL Player (this should never happen) !\n";
	}
	return *itPlayer->second;
}

CIdentifier CPlayerManager::getNextPlayerIdentifier(
	const CIdentifier& rPreviousIdentifier) const
{
	return getNextIdentifier < CPlayer* >(m_vPlayer, rPreviousIdentifier);
}

CIdentifier CPlayerManager::getUnusedIdentifier(void) const
{
	uint64 l_ui64Identifier=System::Math::randomUInteger64();
	CIdentifier l_oResult;
	map<CIdentifier, CPlayer*>::const_iterator i;
	do
	{
		l_ui64Identifier++;
		l_oResult=CIdentifier(l_ui64Identifier);
		i=m_vPlayer.find(l_oResult);
	}
	while(i!=m_vPlayer.end() || l_oResult==OV_UndefinedIdentifier);
	return l_oResult;
}
