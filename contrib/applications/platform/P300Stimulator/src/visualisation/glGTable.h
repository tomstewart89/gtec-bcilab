#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator

#ifndef __GTable_H__
#define __GTable_H__
#if defined TARGET_HAS_ThirdPartyModulesForCoAdaptStimulator
#include <cmath>

#include "glGContainer.h"

namespace OpenViBEApplications
{
	/**
	 * This class is a specialisation of the GContainer and mainly differs from it in that you don't have
	 * to specify a position for the child when you add it. It will arrange the objects itself in the table
	 * as you add them
	 */
	class GTable : public GContainer
	{
	public:
		GTable();
		/*GTable(OpenViBE::float32 w, OpenViBE::float32 h) : GContainer(w,h) {}			
		GTable(OpenViBE::float32 x, OpenViBE::float32 y, OpenViBE::float32 w, OpenViBE::float32 h) : GContainer(x,y,w,h) {}	*/		
		
		/**
		 * @param nElements the number of elements that the GTable has to contain, it will compute the number of rows
		 * and columns automatically
		 */
		GTable(OpenViBE::uint32 nElements);
		
		/**
		 * @param nRows number of rows
		 * @param nCols number of columns
		 */
		GTable(OpenViBE::uint32 nRows, OpenViBE::uint32 nCols);
		
		GTable(const GTable& gtable);
		
		virtual GTable& operator= (GTable const& mainTable);		
		
		virtual ~GTable()
		{
			//std::cout << "GTable deconstructor called\n";
		}		
		
		virtual void addChild(GObject* child, OpenViBE::float32 depth);
		virtual GObject*& getChild(OpenViBE::uint32 rowIndex, OpenViBE::uint32 colIndex) const;
		virtual OpenViBE::uint32 getRowDimension() { return m_ui32RowDimension; }
		virtual OpenViBE::uint32 getColumnDimension() { return m_ui32ColumnDimension; }
		
		//inherited functions
		virtual GTable* clone() const
		{
			return new GTable(*this);
		}		
		virtual void draw();
		virtual std::string toString() const;
		
		//inherited setters
		virtual void setDimParameters(BoxDimensions dim) { GContainer::setDimParameters(dim); }		
		virtual void setBackgroundColor(GColor color) { GContainer::setBackgroundColor(color); }	
		
		//inherited getters
		virtual OpenViBE::float32 getX() const { return GContainer::getX(); }
		virtual OpenViBE::float32 getY() const { return GContainer::getY(); }
		virtual OpenViBE::float32 getWidth() const { return GContainer::getWidth(); }
		virtual OpenViBE::float32 getHeight() const { return GContainer::getHeight(); }
		virtual OpenViBE::float32 getDepth() const { return GContainer::getDepth(); }
		virtual GColor getBackgroundColor() const { return GContainer::getBackgroundColor(); }			
		virtual GObject*& getChild(OpenViBE::uint32 childIndex) const { return GContainer::getChild(childIndex); }
		virtual OpenViBE::uint32 getNumberOfChildren() const { return GContainer::getNumberOfChildren(); }
		
	protected:
		OpenViBE::uint32 m_ui32RowDimension;
		OpenViBE::uint32 m_ui32ColumnDimension;
	};
};

#endif
#endif

#endif
