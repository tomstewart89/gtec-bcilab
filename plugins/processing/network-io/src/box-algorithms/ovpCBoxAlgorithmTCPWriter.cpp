
#ifdef TARGET_HAS_Boost

#include "ovpCBoxAlgorithmTCPWriter.h"

#include <ctime>
#include <iostream>
#include <string>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/detail/endian.hpp>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::NetworkIO;

using boost::asio::ip::tcp;

void CBoxAlgorithmTCPWriter::startAccept() 
{ 
	boost::asio::ip::tcp::socket* l_pSocket = new boost::asio::ip::tcp::socket(m_pAcceptor->get_io_service());
	
	// Since startAccept will only be called inside ioService.poll(), there is no need to access control m_vSockets
	m_vSockets.push_back(l_pSocket);

	this->getLogManager() << LogLevel_Debug << "We are now using " << (uint32)m_vSockets.size() << " socket(s)\n";

	m_pAcceptor->async_accept(*l_pSocket, 
		boost::bind(&CBoxAlgorithmTCPWriter::handleAccept, this, boost::asio::placeholders::error, l_pSocket));
}

void CBoxAlgorithmTCPWriter::handleAccept(const boost::system::error_code& ec, boost::asio::ip::tcp::socket* pSocket)
{ 
	if(!m_pAcceptor->is_open())
	{
		this->getLogManager() << LogLevel_Debug << "handleAccept() was called with acceptor already closed\n";
		return;
	}

	if(!ec) 
	{
		this->getLogManager() << LogLevel_Debug << "Handling a new incoming connection\n";

		// Send the known configuration to the client
		if( m_pActiveDecoder != &m_StimulationDecoder || m_ui64OutputStyle==TCPWRITER_RAW ) 
		{
			try 
			{
				boost::asio::write(*pSocket, boost::asio::buffer((void *)&m_ui32RawVersion, sizeof(uint32)));
				boost::asio::write(*pSocket, boost::asio::buffer((void *)&m_ui32Endianness, sizeof(uint32)));
				boost::asio::write(*pSocket, boost::asio::buffer((void *)&m_ui32Frequency, sizeof(uint32)));
				boost::asio::write(*pSocket, boost::asio::buffer((void *)&m_ui32NumberOfChannels, sizeof(uint32)));
				boost::asio::write(*pSocket, boost::asio::buffer((void *)&m_ui32NumberOfSamplesPerChunk, sizeof(uint32)));
				boost::asio::write(*pSocket, boost::asio::buffer((void *)&m_ui32Reserved0, sizeof(uint32)));
				boost::asio::write(*pSocket, boost::asio::buffer((void *)&m_ui32Reserved1, sizeof(uint32)));
				boost::asio::write(*pSocket, boost::asio::buffer((void *)&m_ui32Reserved2, sizeof(uint32)));
			} 
			catch (boost::system::system_error l_oError) 
			{
				this->getLogManager() << LogLevel_Warning << "Issue '" << l_oError.code().message().c_str() << "' with writing header to client\n";		
			}
		}
	} 
	else 
	{
		// @fixme should the socket be closed in this case?
		this->getLogManager() << LogLevel_Warning << "Issue '" << ec.message().c_str() << "' with accepting a connection.\n";
	}
	// Already schedule the accepting of the next connection
	startAccept();
} 

boolean CBoxAlgorithmTCPWriter::initialize(void)
{
	IBox& l_rStaticBoxContext=this->getStaticBoxContext();

	l_rStaticBoxContext.getInputType(0, m_oInputType);
	if(m_oInputType == OV_TypeId_StreamedMatrix) 
	{
		m_pActiveDecoder = &m_MatrixDecoder;
	} 
	else if(m_oInputType == OV_TypeId_Signal)
	{
		m_pActiveDecoder = &m_SignalDecoder;
	}
	else
	{
		m_pActiveDecoder = &m_StimulationDecoder;
	}
	m_pActiveDecoder->initialize(*this,0);

	uint64 l_ui64Port = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 0);
	m_ui64OutputStyle = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 1);

	m_ui32RawVersion = htonl(1);
#if defined(BOOST_LITTLE_ENDIAN)
	m_ui32Endianness = htonl(1);
#elif defined(BOOST_BIG_ENDIAN)
	m_ui32Endianness = htonl(2);
#elif defined(BOOST_PDP_ENDIAN)
	m_ui32Endianness = htonl(3);
#else
	m_ui32Endianness = htonl(0);
	this->getLogManager() << LogLevel_Warning << "Platform endianness was not recognized\n";
#endif

	m_ui32Frequency = 0;
	m_ui32NumberOfChannels	= 0;
	m_ui32NumberOfSamplesPerChunk = 0;
	m_ui32Reserved0 = 0;
	m_ui32Reserved1 = 0;
	m_ui32Reserved2 = 0;

	this->getLogManager() << LogLevel_Trace << "Setting up an acceptor at port " << l_ui64Port << "\n";

	try 
	{
#ifdef TARGET_OS_Windows
		// On Windows, unless we deny reuse_addr, it seems several different servers can bind to the same socket. This is not what we want.
		m_pAcceptor = new tcp::acceptor(m_oIOService, tcp::endpoint(tcp::v4(), (uint32)l_ui64Port), false);
#else
		// On Linux, unless we allow reuse_addr, disconnection may set the socket to TIME_WAIT state and prevent opening it again until that state expires
		m_pAcceptor = new tcp::acceptor(m_oIOService, tcp::endpoint(tcp::v4(), (uint32)l_ui64Port), true);
#endif
	}
	catch (boost::system::system_error l_oError) 
	{
		this->getLogManager() << LogLevel_Warning << "Got error '" << l_oError.code().message().c_str() << "' allocating acceptor to port " << (uint32)l_ui64Port << "\n";
		m_pActiveDecoder->uninitialize();
		m_pActiveDecoder=NULL;
		m_pAcceptor=NULL; // if new throws, deleting the returned m_pAcceptor causes problems on Linux. So we NULL it.
		return false;
	}

	boost::asio::socket_base::linger l_oOption(true, 0);
	m_pAcceptor->set_option(l_oOption);

	startAccept();

	m_oIOService.poll();

	return true;
}
/*******************************************************************************/

boolean CBoxAlgorithmTCPWriter::uninitialize(void)
{
	if(m_pActiveDecoder) 
	{
		m_pActiveDecoder->uninitialize();
		m_pActiveDecoder=NULL;
	}

	for(uint32 i=0;i<m_vSockets.size();i++) 
	{
		boost::asio::ip::tcp::socket* l_oTmpSock = m_vSockets[i];
		if(l_oTmpSock->is_open()) 
		{
			try
			{
				l_oTmpSock->shutdown(boost::asio::socket_base::shutdown_both);
				l_oTmpSock->close();
			}
			catch (boost::system::system_error l_oError) 
			{
				// Just report...
				this->getLogManager() << LogLevel_Warning << "Error in uninitialize() socket shutdown/close: '" << l_oError.code().message().c_str() << "'\n";
			}
		}
	}
	m_oIOService.poll();

	m_oIOService.stop();

	for(uint32 i=0;i<m_vSockets.size();i++) 
	{
		delete m_vSockets[i];
	}
	m_vSockets.clear();

	delete m_pAcceptor;
	m_pAcceptor = NULL;
	
	return true;
}
/*******************************************************************************/

boolean CBoxAlgorithmTCPWriter::processInput(uint32 ui32InputIndex)
{
	// ready to process !
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();

	return true;
}
/*******************************************************************************/

boolean CBoxAlgorithmTCPWriter::sendToClients(const void* pBuffer, uint32 ui32BufferLength)
{
	std::vector<boost::asio::ip::tcp::socket*>::iterator it = m_vSockets.begin();
	while(it!=m_vSockets.end()) 
	{
		boost::asio::ip::tcp::socket* tmpSock = (*it);
		bool hadError = false;
		if(tmpSock->is_open()) 
		{
			try 
			{
				boost::asio::write(*tmpSock, boost::asio::buffer(pBuffer,ui32BufferLength));
			} 
			catch (boost::system::system_error l_oError) 
			{
				this->getLogManager() << LogLevel_Warning << "Got error '" << l_oError.code().message().c_str() << "' while trying to write to socket\n";
				hadError = true;
			};
		}
		if(hadError) 
		{
			// Close the socket
			this->getLogManager() << LogLevel_Debug << "Closing the socket\n";
			try
			{
				tmpSock->shutdown(boost::asio::socket_base::shutdown_both);
				tmpSock->close();
			} 
			catch (boost::system::system_error l_oError) 
			{
				// Just report...
				this->getLogManager() << LogLevel_Warning << "Error while socket shutdown/close: '" << l_oError.code().message().c_str() << "'\n";
			}
			m_oIOService.poll();
			delete tmpSock;
			it = m_vSockets.erase(it);
		} else {
			it++;
		}
	}
	return true;
}

boolean CBoxAlgorithmTCPWriter::process(void)
{
	// the dynamic box context describes the current state of the box inputs and outputs (i.e. the chunks)
	IBoxIO& l_rDynamicBoxContext=this->getDynamicBoxContext();

	// Process the asio loop once (e.g. see if there's new connections)
	m_oIOService.poll();

	for(uint32 j=0; j<l_rDynamicBoxContext.getInputChunkCount(0); j++)
	{
		m_pActiveDecoder->decode(j);
		if(m_pActiveDecoder->isHeaderReceived())
		{
			if(m_pActiveDecoder == &m_MatrixDecoder) 
			{
				m_ui32NumberOfChannels = static_cast<uint32> (m_MatrixDecoder.getOutputMatrix()->getDimensionSize(0) );
				m_ui32NumberOfSamplesPerChunk = static_cast<uint32> (m_MatrixDecoder.getOutputMatrix()->getDimensionSize(1) );

				m_oChunkTranspose.setDimensionCount(2);
				m_oChunkTranspose.setDimensionSize(0,m_ui32NumberOfSamplesPerChunk);
				m_oChunkTranspose.setDimensionSize(1,m_ui32NumberOfChannels);
			} 
			else if(m_pActiveDecoder == &m_SignalDecoder)
			{
				m_ui32Frequency = static_cast<uint32> ( m_SignalDecoder.getOutputSamplingRate() );
				m_ui32NumberOfChannels = static_cast<uint32> ( m_SignalDecoder.getOutputMatrix()->getDimensionSize(0) );
				m_ui32NumberOfSamplesPerChunk = static_cast<uint32> ( m_SignalDecoder.getOutputMatrix()->getDimensionSize(1) );

				m_oChunkTranspose.setDimensionCount(2);
				m_oChunkTranspose.setDimensionSize(0,m_ui32NumberOfSamplesPerChunk);
				m_oChunkTranspose.setDimensionSize(1,m_ui32NumberOfChannels);
			}
			else
			{
				// Stimulus, do nothing
			}
		}
		if(m_pActiveDecoder->isBufferReceived()) 
		{
			if(m_pActiveDecoder == &m_MatrixDecoder) 
			{
				const IMatrix* l_pMatrix = m_MatrixDecoder.getOutputMatrix();

				// Transpose
				const float64 *l_pSource = l_pMatrix->getBuffer();
				float64 *l_pDestination = m_oChunkTranspose.getBuffer();
				for(uint32 c=0;c<m_ui32NumberOfChannels;c++) 
				{
					for(uint32 s=0;s<m_ui32NumberOfSamplesPerChunk;s++) 
					{
						l_pDestination[s*m_ui32NumberOfChannels+c] = l_pSource[c*m_ui32NumberOfSamplesPerChunk+s];
					}
				}

				sendToClients((void *)l_pDestination, l_pMatrix->getBufferElementCount()*sizeof(float64));
			} 
			else if(m_pActiveDecoder == &m_SignalDecoder)
			{
				const IMatrix* l_pMatrix = m_SignalDecoder.getOutputMatrix();

				// Transpose
				const float64 *l_pSource = l_pMatrix->getBuffer();
				float64 *l_pDestination = m_oChunkTranspose.getBuffer();
				for(uint32 c=0;c<m_ui32NumberOfChannels;c++) 
				{
					for(uint32 s=0;s<m_ui32NumberOfSamplesPerChunk;s++) 
					{
						l_pDestination[s*m_ui32NumberOfChannels+c] = l_pSource[c*m_ui32NumberOfSamplesPerChunk+s];
					}
				}

				sendToClients((void *)l_pDestination, l_pMatrix->getBufferElementCount()*sizeof(float64));
			} 
			else // stimulus
			{
				const IStimulationSet* l_pStimulations = m_StimulationDecoder.getOutputStimulationSet();
				for(uint32 j=0; j<l_pStimulations->getStimulationCount(); j++)
				{
					const uint64 l_ui64StimulationCode = l_pStimulations->getStimulationIdentifier(j);
					// uint64 l_ui64StimulationDate = l_pStimulations->getStimulationDate(j);
					this->getLogManager() << LogLevel_Trace << "Sending out " << l_ui64StimulationCode << "\n";

					switch(m_ui64OutputStyle) {
						case TCPWRITER_RAW:
							sendToClients((void*)&l_ui64StimulationCode,sizeof(l_ui64StimulationCode));
							break;
						case TCPWRITER_HEX:
							{
							CString  l_sTmp = CIdentifier(l_ui64StimulationCode).toString() + CString("\n");
							const char *l_sPtr = l_sTmp.toASCIIString();
							sendToClients((void*)l_sPtr,strlen(l_sPtr));
							}
							break;
						case TCPWRITER_STRING: 
							{
							CString l_sTmp = this->getTypeManager().getEnumerationEntryNameFromValue(OV_TypeId_Stimulation, l_ui64StimulationCode);
							if(l_sTmp==CString("")) 
							{
								l_sTmp = CString("Unregistered_stimulus ") + CIdentifier(l_ui64StimulationCode).toString();
							}
							l_sTmp = l_sTmp + CString("\n");

							const char *l_sPtr = l_sTmp.toASCIIString();
							sendToClients((void*)l_sPtr,strlen(l_sPtr));
							}
							break;
						default:
							this->getLogManager() << LogLevel_Error << "Unknown stimulus output style\n";
							return false;
							break;
					}
				}
			}

		}
		if(m_pActiveDecoder->isEndReceived())
		{
		}
	}

	return true;
}

#endif // TARGET_HAS_Boost
