//////////////////////////////////////////////////////////////////////
//	Predicates.h
//
//	Robust geometric predicates API
//
//	Jonathan Richard Shewchuk (http://www.cs.cmu.edu/~quake/robust.html)
//

#pragma once

#define REAL double

namespace exact
{
	enum position
	{
		on = 0,
		left = 1,
		right = -1,
		inside = 1,
		outside = -1
	};

	void Init();
	
	position orient2d(REAL* pa, REAL* pb, REAL* pc);
	
	position incircle(REAL* pa, REAL* pb, REAL* pc, REAL* pd);
}

