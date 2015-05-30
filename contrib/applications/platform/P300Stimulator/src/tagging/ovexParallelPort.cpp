#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

// author: Dieter Devlaminck
// affiliation: INRIA
// date: 26/01/2013
#if defined TARGET_OS_Linux || (defined TARGET_OS_Windows && defined TARGET_HAS_ThirdPartyInpout)

#include "ovexParallelPort.h"

#if defined TARGET_OS_Windows
	//#include "StdAfx.h"
	#include "windows.h"
	#include <inpout32.h>
#endif

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEApplications;

ParallelPort::ParallelPort(unsigned short portNumber, OpenViBE::uint32 sampleFrequency) : ITagger()
{
	m_usPortNumber = portNumber;
	m_iParallelPortHandle = -1;
	m_bOpen = false;
	m_ui32SampleFrequency = sampleFrequency;
}

ParallelPort::~ParallelPort()
{
	close();
}

int ParallelPort::open()
{
	#if defined TARGET_OS_Linux
		char l_cDeviceName[20];
		snprintf(l_cDeviceName,20,"/dev/parport%d",m_usPortNumber);
		//std::cout << "Trying to open " << l_cDeviceName << "\n";
		m_iParallelPortHandle = ::open(l_cDeviceName, O_RDWR);
		if (m_iParallelPortHandle<0)
		{
			perror ("Opening of the parallel port failed!");
			return 0;
		}

		m_bOpen = true;

		if (ioctl(m_iParallelPortHandle, PPCLAIM))
		{
			perror("PPCLAIM failed!");
			this->close();
			return 0;
		}

		int gmode;
		ioctl(m_iParallelPortHandle, PPGETMODE, &gmode);
		/*if ()
			std::cout << "The mode of pport is " << gmode << std::endl;*/

		int mode = IEEE1284_MODE_EPP;
		if(ioctl(m_iParallelPortHandle,PPNEGOT,&mode)) 
		{
			//std::cout<< "PPNEGOT successful!" << std::endl;
			return 1;
		}
		else if (ioctl(m_iParallelPortHandle, PPSETMODE, &mode))
		{
			std::cout << "PPNEGOT failed, passively setting mode!" << std::endl;
			return 1;
		}
		else
		{
			std::cout << "None of the mode setting methods seems to work" << std::endl;
			return 0;
		}

		//return 1;
	#elif defined TARGET_OS_Windows
		m_bOpen = true;
		if (IsInpOutDriverOpen())
		{
			m_bOpen = true;
			return 1;
		}
		else
		{
			return 0;
		}
	#endif
}

int ParallelPort::close()
{
	if (m_bOpen)
	{
		#if defined TARGET_OS_Linux
			std::cout << "Releasing and closing the parallel port!" << std::endl;
			while(!ioctl(m_iParallelPortHandle,PPRELEASE)) {}
			::close(m_iParallelPortHandle);
			return 1;
		#elif defined TARGET_OS_Windows
			return 1;
		#endif
	}
	else
		return 1;
}

int ParallelPort::write(uint32 value)
{
	unsigned short l_cValue = value;
	if (m_bOpen)
	{
		#if defined TARGET_OS_Linux
			unsigned char status;
			unsigned char mask = (PARPORT_STATUS_ERROR
					     | PARPORT_STATUS_BUSY);
			unsigned char val = (PARPORT_STATUS_ERROR
					     | PARPORT_STATUS_BUSY);
			struct ppdev_frob_struct frob;
			struct timespec ts;

			/* Wait for printer to be ready */
			for (;;) {
				ioctl(m_iParallelPortHandle, PPRSTATUS, &status);

				if ((status & mask) != val) 
					break;
				else
					std::cout << "status of pport, status " << (unsigned int)status << std::endl;

				ioctl(m_iParallelPortHandle, PPRELEASE);
				sleep(1);
				ioctl(m_iParallelPortHandle, PPCLAIM);
			}

			/* Set the data lines */
			ioctl(m_iParallelPortHandle, PPWDATA, &l_cValue);

			/* Delay for a bit */
			ts.tv_sec = 0;
			ts.tv_nsec = 1000;
			nanosleep(&ts, NULL);

			/* Pulse strobe */
			frob.mask = PARPORT_CONTROL_STROBE;
			frob.val = PARPORT_CONTROL_STROBE;
			ioctl(m_iParallelPortHandle, PPFCONTROL, &frob);
			nanosleep (&ts, NULL);

			/* End the pulse */
			frob.val = 0;
			ioctl(m_iParallelPortHandle, PPFCONTROL, &frob);
			ts.tv_nsec = (long)1000000*(std::ceil(1000.0/m_ui32SampleFrequency)+1);
			nanosleep (&ts, NULL); //TODO: we might use the time class of OpenViBE instead

			return 1;
		#elif defined TARGET_OS_Windows
			DlPortWritePortUshort(m_usPortNumber, l_cValue);
			Sleep(static_cast<DWORD>(std::ceil(1000.0/m_ui32SampleFrequency)+1)); //TODO: we might use the time class of OpenViBE instead
			return 1;
		#endif
	}
	else
		return 1;
}


#endif

#endif
