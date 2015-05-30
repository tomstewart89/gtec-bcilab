#ifndef __OpenViBE_Kernel_Scenario_IBoxProto_H__
#define __OpenViBE_Kernel_Scenario_IBoxProto_H__

#include "../ovIKernelObject.h"

namespace OpenViBE
{
	namespace Kernel
	{
		/**
		 * \brief This enum lists all the flags a box can be have
		 * \sa OpenViBE::Kernel::IBoxProto::addFlag
		 */
		enum EBoxFlag
		{
			BoxFlag_CanAddInput,
			BoxFlag_CanModifyInput,
			BoxFlag_CanAddOutput,
			BoxFlag_CanModifyOutput,
			BoxFlag_CanAddSetting,
			BoxFlag_CanModifySetting,
			BoxFlag_IsDeprecated,
			BoxFlag_IsUnstable,
			BoxFlag_CanAddMessageInput,
			BoxFlag_CanModifyMessageInput,
			BoxFlag_CanAddMessageOutput,
			BoxFlag_CanModifyMessageOutput,
		};

		/**
		 * \class IBoxProto
		 * \author Yann Renard (INRIA/IRISA)
		 * \date 2006-07-05
		 * \brief OpenViBE box prototype
		 * \ingroup Group_Scenario
		 * \ingroup Group_Kernel
		 * \ingroup Group_Extend
		 *
		 * This class is used by a plugin algorithm descriptor
		 * to let the OpenViBE platform know what an algorithm
		 * box looks like. It declares several things, like
		 * it input types, output types and settings.
		 *
		 * \sa OpenViBE::Kernel::IBoxAlgorithmDesc
		 */
		class OV_API IBoxProto : public OpenViBE::Kernel::IKernelObject
		{
		public:

			/**
			 * \brief Adds an input to the box
			 * \param sName [in] : the name of the input to add
			 * \param rTypeIdentifier [in] : the type of the input
			 * \return The created input index.
			 */
			virtual OpenViBE::boolean addInput(
				const OpenViBE::CString& sName,
				const OpenViBE::CIdentifier& rTypeIdentifier)=0;
			/**
			* \brief Adds a message input to the box
			* \param sName [in] : the name of the message input to add
			* \return The created message input index.
			*/
			virtual OpenViBE::boolean addMessageInput(
			const OpenViBE::CString& sName)=0;

			/**
			* \brief Adds a message output to the box
			* \param sName [in] : the name of the message output to add
			* \return The created message output index.
			*/
			virtual OpenViBE::boolean addMessageOutput(
			const OpenViBE::CString& sName)=0;

			/**
			 * \brief Adds an output to the box
			 * \param sName [in] : the name of the output to add
			 * \param rTypeIdentifier [in] : the type of the output
			 * \return The created output index.
			 */
			virtual OpenViBE::boolean addOutput(
				const OpenViBE::CString& sName,
				const OpenViBE::CIdentifier& rTypeIdentifier)=0;
			/**
			 * \brief Adds an setting to the box
			 * \param sName [in] : the name of the setting to add
			 * \param rTypeIdentifier [in] : the type of the setting
			 * \param sDefaultValue [in] : the default value of this
			 *        setting (used to initialize the box itself)
			 * \return The created setting index.
			 */

			/*
			virtual OpenViBE::boolean addSetting(
				const OpenViBE::CString& sName,
				const OpenViBE::CIdentifier& rTypeIdentifier,
				const OpenViBE::CString& sDefaultValue)=0;
				//*/

			virtual OpenViBE::boolean addSetting(
				const OpenViBE::CString& sName,
				const OpenViBE::CIdentifier& rTypeIdentifier,
				const OpenViBE::CString& sDefaultValue,
				const OpenViBE::boolean bModifiable = false)=0;
			/**
			 * \brief Adds a flag to the box
			 * \param eBoxFlag [in] : the flag to add to the box
			 * \return \e true in case of success.
			 * \return \e false in case of error.
			 */
			virtual OpenViBE::boolean addFlag(
				const OpenViBE::Kernel::EBoxFlag eBoxFlag)=0;
			/**
			 * \brief Adds a new type supported by inputs of the box
			  * \param rTypeIdentifier [in] : The type identifier
			  * \return \e true in case of success.
			  * \return \e false in case of error.
			  */
			virtual OpenViBE::boolean addInputSupport(
				const OpenViBE::CIdentifier &rTypeIdentifier)=0;
			/**
			 * \brief Adds a new type supported by outputs of the box
			  * \param rTypeIdentifier [in] : The type identifier
			  * \return \e true in case of success.
			  * \return \e false in case of error.
			  */
			virtual OpenViBE::boolean addOutputSupport(
				const OpenViBE::CIdentifier &rTypeIdentifier)=0;

			_IsDerivedFromClass_(OpenViBE::Kernel::IKernelObject, OV_ClassId_Kernel_Scenario_BoxProto)
		};
	};
};

#endif // __OpenViBE_Kernel_Scenario_IBoxProto_H__
