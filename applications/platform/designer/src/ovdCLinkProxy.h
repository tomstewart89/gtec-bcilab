#ifndef __OpenViBEDesigner_CLinkProxy_H__
#define __OpenViBEDesigner_CLinkProxy_H__

#include "ovd_base.h"

namespace OpenViBEDesigner
{
	class CLinkProxy
	{
	public:

		CLinkProxy(const OpenViBE::Kernel::ILink& rLink);
		CLinkProxy(OpenViBE::Kernel::IScenario& rScenario, const OpenViBE::CIdentifier& rLinkIdentifier);
		virtual ~CLinkProxy(void);

		operator OpenViBE::Kernel::ILink* (void);
		operator const OpenViBE::Kernel::ILink* (void);

		OpenViBE::float64 getXSource(void);
		OpenViBE::float64 getYSource(void);
		OpenViBE::float64 getXTarget(void);
		OpenViBE::float64 getYTarget(void);

		void setSource(OpenViBE::float64 f64XSource, OpenViBE::float64 f64YSource);
		void setTarget(OpenViBE::float64 f64XTarget, OpenViBE::float64 f64YTarget);

	protected:

		const OpenViBE::Kernel::ILink* m_pConstLink;
		OpenViBE::Kernel::ILink* m_pLink;
		OpenViBE::float64 m_f64XSource;
		OpenViBE::float64 m_f64YSource;
		OpenViBE::float64 m_f64XTarget;
		OpenViBE::float64 m_f64YTarget;
	};
};

#endif // __OpenViBEDesigner_CLinkProxy_H__
