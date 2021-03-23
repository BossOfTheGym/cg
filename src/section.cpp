#include "section.h"

namespace sect
{
	using vec2 = prim::vec2;

	using Line2 = prim::Line2;

	// for now only two segments can intersect in the same point
	struct Event
	{
		enum class Type : i32
		{
			Lower,
			Intersection,
			Upper
		};
		
		std::vector<Line2*> segments;
		vec2* pt{};
		Type type{};
	};

	struct Intersection
	{
		std::vector<Line2*> intersection;
	};

	class SweepLine
	{
	private:
	};

	// stores events
	class EventQueue
	{
	public:
		void extractMin()
		{
			// TODO
		}

	private:
		// 
	};


	void section_n_lines(std::vector<prim::Line2>& lines)
	{
	
	}
}