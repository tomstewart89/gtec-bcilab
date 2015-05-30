#if defined(TARGET_HAS_ThirdPartyEIGEN)

#ifndef __WindowFunctions_H__
#define __WindowFunctions_H__

#include "../../ovp_defines.h"
#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>
#include <Eigen/Dense>
#include <cmath>

class WindowFunctions {

public:

	OpenViBE::boolean bartlett(Eigen::VectorXd& vecXdWindow, OpenViBE::uint32 ui32WinSize);
	OpenViBE::boolean hamming(Eigen::VectorXd& vecXdWindow, OpenViBE::uint32 ui32WinSize);
	OpenViBE::boolean hann(Eigen::VectorXd& vecXdWindow, OpenViBE::uint32 ui32WinSize);
	OpenViBE::boolean parzen(Eigen::VectorXd& vecXdWindow, OpenViBE::uint32 ui32WinSize);
	OpenViBE::boolean welch(Eigen::VectorXd& vecXdWindow, OpenViBE::uint32 ui32WinSize);

};


#endif //__WindowFunctions_H__
#endif //TARGET_HAS_ThirdPartyEIGEN
