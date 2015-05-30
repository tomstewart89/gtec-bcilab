#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#include "glGTable.h"
#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEApplications;

GTable::GTable() {}	

GTable::GTable(OpenViBE::uint32 nElements)
{
	m_ui32RowDimension = (OpenViBE::uint32)std::floor(0.5+std::sqrt((double)nElements));
	m_ui32ColumnDimension = (OpenViBE::uint32)std::ceil((double)nElements/(double)m_ui32RowDimension);	
}

GTable::GTable(OpenViBE::uint32 nRows, OpenViBE::uint32 nCols) : GContainer()
{
	m_ui32RowDimension = nRows;
	m_ui32ColumnDimension = nCols;	
}

GTable::GTable(const GTable& gtable) : GContainer(gtable)
{		
	m_ui32RowDimension = gtable.m_ui32RowDimension;
	m_ui32ColumnDimension = gtable.m_ui32ColumnDimension;
}

GTable& GTable::operator= (GTable const& mainTable)
{
	if(this!=&mainTable)
	{
		this->GContainer::operator=(mainTable);
		this->m_ui32RowDimension = mainTable.m_ui32RowDimension;
		this->m_ui32ColumnDimension = mainTable.m_ui32ColumnDimension;
	}
	return *this;
}	

GObject*& GTable::getChild(OpenViBE::uint32 rowIndex, OpenViBE::uint32 colIndex) const 
{ 
	OpenViBE::uint32 childIndex = rowIndex*m_ui32ColumnDimension+colIndex;
	return this->getChild(childIndex); 
}

void GTable::draw()
{
	GContainer::draw();
}

void GTable::addChild(GObject* child, float32 depth)
{
	float32 l_f32CellWidth = (float32)1.0f/m_ui32ColumnDimension;
	float32 l_f32CellHeight = (float32)1.0f/m_ui32RowDimension;
	float32 l_f32UpperLeftStartPointY = 1.0f-l_f32CellHeight;
	
	uint32 l_ui32ChildIndex = m_lChildren->size();
	uint32 i = l_ui32ChildIndex/m_ui32ColumnDimension;
	uint32 j = l_ui32ChildIndex%m_ui32ColumnDimension;
	
	GContainer::addChild(child, j*l_f32CellWidth, l_f32UpperLeftStartPointY-i*l_f32CellHeight, l_f32CellWidth, l_f32CellHeight, depth);
}

std::string GTable::toString() const
{
	return std::string("GTable");
}
#endif

#endif
