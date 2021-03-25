#pragma once

#include "trb.h"


namespace sect
{
	class EventQueue
	{
	public:
		void addEvent();

		void findEvent();

		void nextEvent();

		void removeEvent();
	};
}