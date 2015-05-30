#ifndef __OpenViBE_Designer_Setting_CAbstractSettingView_H__
#define __OpenViBE_Designer_Setting_CAbstractSettingView_H__

#include "../ovd_base.h"

namespace OpenViBEDesigner
{
	namespace Setting
	{
		class CAbstractSettingView{

		public:
			virtual ~CAbstractSettingView(void);

			//Store the value of the setting in rValue
			virtual void getValue(OpenViBE::CString &rValue) const = 0;
			//Set the view with the value contains in rValue
			virtual void setValue(const OpenViBE::CString &rValue) = 0;

			//Get the label which contains the name of the setting
			virtual ::GtkWidget* getNameWidget(void);
			//Get the table of widget which display the value of the setting (the entry and all interaction buttons)
			virtual ::GtkWidget* getEntryWidget(void);

			//This function is use to update the setting index when a setting is suppressed or inserted
			virtual void setSettingIndex(OpenViBE::uint32 m_ui32NewIndex);
			//Get the index of the setting
			virtual OpenViBE::uint32 getSettingIndex(void);

		protected:
			//Initialize the common part of all view. If sBuilderName and sWidgetName are not NULL, the entryTable and the label
			//will be set according to these informations.
			//If there are NULL, name and entry widget will have to be set after with corresponding setter.
			CAbstractSettingView(OpenViBE::Kernel::IBox& rBox, OpenViBE::uint32 ui32Index,
								 const char* sBuilderName, const char* sWidgetName);

			//Return the box which contains the setting
			virtual OpenViBE::Kernel::IBox& getBox(void);

			//Set the pWidget as the new widget name
			virtual void setNameWidget(::GtkWidget* pWidget);
			//Set the pWidget as the new widget name
			virtual void setEntryWidget(::GtkWidget* pWidget);

			//Set the setting view with the current value of the setting
			virtual void initializeValue();

			//Return a vector which contains the list of the widget contains int the widget pWidget
			virtual void extractWidget(::GtkWidget* pWidget, std::vector< ::GtkWidget * > &rVector);

			//Return the part of the entry table which is not revert or default button
			virtual ::GtkWidget* getEntryFieldWidget(void);

		private:
			//Generate the label of the setting
			virtual void generateNameWidget(void);
			//Generate the table of widget which display the value of the setting (the entry and all interaction buttons)
			virtual ::GtkWidget* generateEntryWidget(void);


			OpenViBE::Kernel::IBox& m_rBox;
			OpenViBE::uint32 m_ui32Index;
			OpenViBE::CString m_sSettingWidgetName;
			::GtkWidget* m_pNameWidget;
			::GtkWidget* m_pEntryNameWidget;
			::GtkWidget* m_pEnrtyFieldWidget;

			//If we don't store the builder, the setting name will be free when we'll unref the builder
			::GtkBuilder *m_pBuilder;
			OpenViBE::boolean m_bOnValueSetting;
		};
	}

}

#endif // __OpenViBE_Designer_Setting_CAbstractSettingView_H__
