#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#ifndef __CSoftTagger_H__
#define __CSoftTagger_H__

#include <sys/timeb.h>
#include <boost/interprocess/ipc/message_queue.hpp>

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <cmath>

#include "ovexITagger.h"

namespace OpenViBEApplications
{
	class CSoftTagger : public ITagger
	{
	public :
		CSoftTagger(std::string queueName = "openvibeExternalStimulations");
		virtual ~CSoftTagger();
		virtual int open();
		virtual int write(OpenViBE::uint32 value);
	protected:
		virtual int close();
	private:
		std::string m_messageQueueName;
		int m_chunkLength;
		int m_maxMessages;

		// openvibe currently uses messages of length of 3
		boost::interprocess::message_queue* m_messageQueue;
};
}
#endif

#endif
