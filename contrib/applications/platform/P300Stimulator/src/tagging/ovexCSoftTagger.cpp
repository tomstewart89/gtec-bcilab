#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#include "ovexCSoftTagger.h"


using namespace OpenViBEApplications;
using namespace OpenViBE;

CSoftTagger::CSoftTagger(std::string queueName) : ITagger()
{
	m_messageQueueName = queueName;
	m_chunkLength = 3;
	m_maxMessages = 5000;
	open();
}

CSoftTagger::~CSoftTagger()
{
	close();
}

int CSoftTagger::open()
{
		boost::interprocess::message_queue::remove(m_messageQueueName.c_str());
		try
		{
			m_messageQueue = new boost::interprocess::message_queue(boost::interprocess::create_only,
					m_messageQueueName.c_str(),m_maxMessages,m_chunkLength * sizeof(uint64));
		}
		catch(boost::interprocess::interprocess_exception &exception)
		{
			std::cout << "CSoftTagger error while opening the message queue: " << exception.what() << "\n";
			throw;
		}
		return 1;
}

int CSoftTagger::close()
{
	boost::interprocess::message_queue::remove(m_messageQueueName.c_str());
	return 1;
}

int CSoftTagger::write(OpenViBE::uint32 value)
{
	struct timeb currentTime;

	ftime(&currentTime);

	uint64 stimulationTime = currentTime.time * 1000 + currentTime.millitm;

	uint64 message[3];

	message[0] = 0; // unused at the moment
	message[1] = static_cast<uint64>(value);
	message[2] = stimulationTime;
	//std::cout << "Writing stimulus tagger " << value << "\n";

	try
	{
		m_messageQueue->send(&message, sizeof(message), 0);
	}
	catch(boost::interprocess::interprocess_exception &exception)
	{
		std::cout << exception.what() << std::endl;
		throw;
	}	

	return 1;
}

#endif
