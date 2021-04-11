#include "section.h"
#include "trb_tree.h"

#include "core.h"

#include <vector>
#include <cassert>
#include <iostream>
#include <algorithm>

namespace sect
{
	namespace
	{
		using namespace prim;

		// NOTE : simplification, no horizontal segments allowed for now
		// NOTE : no overlapping lines allowed
		// NOTE : sweeping from up to down (from upper y to lower), from left to right (from left x to right)
		// NOTE : segments are stored from left to right in what order they intersect the sweep line

		// NOTE : returns line in which v0 is upper vertex and v1 is lower vertex
		Line2 reorder_line(const Line2& l)
		{
			if (l.v0.y < l.v1.y)
				return {l.v1, l.v0};
			return l;
		}

		Float polar_y(const vec2& point)
		{
			return point.x / point.y;
		}

		Float polar_y(const Line2& line)
		{
			auto& [v0, v1] = line;

			return polar_y(v1 - v0);
		}

		struct SweepLineComparator
		{
			// l0 < l1
			bool operator () (const Line2& l0, const Line2& l1)
			{
				// simplification for now
				assert(!horiz(l0));
				assert(!horiz(l1));

				Float x0;
				intersectsLineY(l0, sweepY, x0);

				Float x1;
				intersectsLineY(l1, sweepY, x1);

				return x0 < x1;
			}

			bool operator() (const Line2& l, Float x)
			{
				assert(!horiz(l));

				Float xi;
				intersectsLineY(l, sweepY, xi);

				return xi < x;
			}
				
			bool operator() (Float x, const Line2& l)
			{
				assert(!horiz(l));

				Float xi;
				intersectsLineY(l, sweepY, xi);

				return x < xi;
			}

			Float sweepY{};
		};

		// multiset-like
		using sweep_order_traits_t = trb::TreeTraits<Line2, SweepLineComparator, trb::DefaultAllocator, true, true>;

		class SweepOrder : public trb::Tree<sweep_order_traits_t>
		{
		public:
			// insert, erase and empty already in tree
			bool preceds(const Line2& l0, const Line2& l1)
			{
				return m_compare(l0, l1);
			}

			Float sweepY() const
			{
				return m_compare.sweepY;
			}

			void sweepY(Float y)
			{
				m_compare.sweepY = y;
			}
		};


		struct PointEvent
		{
			using SweepOrderIt = typename SweepOrder::Iterator;

			// event point : lower point event, upper point event, intersection point event
			// can be all together
			vec2 point;

			// segments that have their lower end in the point
			// they will be deleted, added after upper ends are processed, order is not neccessary
			std::vector<SweepOrderIt> lowerEnd;

			// segments that intersect in the point will not be stored expliitly
			// their order in sweep line will be reversed after event is processed
			u32 intersections{};

			// segments that have their upper end in the point
			// they will be inserted, I will search next lower y (if there is no so it can be chosen arbitrarly)
			std::vector<Line2> upperEnd;
		};

		struct PointEventComparator
		{
			// for searching & insertion
			bool operator () (const PointEvent& e0, const PointEvent& e1)
			{
				if (std::abs(e0.point.y - e1.point.y) > eps)
					return e0.point.y > e1.point.y;
				return e0.point.x < e1.point.x;
			}

			// for searching & insertion
			bool operator () (const PointEvent& e0, const vec2& v0)
			{
				if (std::abs(e0.point.y - v0.y) > eps)
					return e0.point.y > v0.y;
				return e0.point.x < v0.x;
			}

			bool operator () (const vec2& v0, const PointEvent& e0)
			{
				if (std::abs(v0.y - e0.point.y) > eps)
					return v0.y > e0.point.y;
				return v0.x < e0.point.x;
			}

			// for finding safe y
			bool operator () (const PointEvent& e0, Float y)
			{
				return e0.point.y > y;
			}

			bool operator () (Float y, const PointEvent& e0)
			{
				return y > e0.point.y;
			}

			Float eps = default_eps;
		};

		// set-like
		using event_queue_traits_t = trb::TreeTraits<PointEvent, PointEventComparator, trb::DefaultAllocator, false, false>;

		class EventQueue : public trb::Tree<event_queue_traits_t>
		{
		public:
			using Tree     = trb::Tree<event_queue_traits_t>;
			using Node     = typename Tree::Node;
			using Iterator = typename Tree::Iterator;


		public: // utility functions
			void push(const vec2& point)
			{
				insert(point);
			}

			void pop()
			{
				erase(begin());
			}

			Iterator front()
			{
				return begin();
			}

			bool preceds(Iterator it, const vec2& point)
			{
				assert(it != end());

				return m_compare(*it, point);
			}
		};


		class Sector
		{
		public:
			using SweepOrderIt = SweepOrder::Iterator;
			using PointEventIt = EventQueue::Iterator;

		public:
			std::vector<Intersection> sect(const std::vector<Line2>& lines)
			{
				std::cout << "*** init ***" << std::endl;
				initialize(lines);
				while (!m_queue.empty())
				{
					auto it = m_queue.front();

					std::cout << "*** event ***" << std::endl;
					handlePointEvent(it);

					m_queue.pop();
				}

				return std::move(m_intersections);
			}

		private:
			// NOTE : initializing event queue. Here only upper-end segments are inserted, 
			// lower-end segments will be inserted after upper-end is processed
			void initialize(const std::vector<Line2>& lines)
			{
				for (auto& line : lines)
					insertUpperEndEvent(line);
			}


			// NOTE : called only from initialization step, line hasn't been inserted yet, so pure line is passed
			void insertUpperEndEvent(const Line2& line)
			{
				auto [v0, v1] = reorder_line(line);

				auto [it, _] = m_queue.insert(v0);
				it->upperEnd.push_back(line);

				std::cout << "upper: " << v0.x << " " << v0.y << std::endl;
			}

			// NOTE : called after upper-end of a segment was processed so existing position is passed
			void insertLowerEndEvent(SweepOrderIt line)
			{
				auto [v0, v1] = reorder_line(*line);

				auto [it, _] = m_queue.insert(v1);
				it->lowerEnd.push_back(line);

				std::cout << "lower: " << v1.x << " " << v1.y << std::endl;
			}

			// NOTE : called if intersection was found
			void insertIntersectionEvent(const vec2& point)
			{
				auto [it, inserted] = m_queue.insert(point);
				if (!inserted)
					it->intersections += 1; // new intersecting line
				else
					it->intersections = 2; // newly created

				std::cout << "inter: " << point.x << " " << point.y << std::endl;
			}


			void handlePointEvent(PointEventIt event)
			{
				std::cout << "event: " << event->point.x << " " << event->point.y << std::endl;

				m_sweepLine.sweepY(event->point.y);

				if (event->intersections + event->lowerEnd.size() + event->upperEnd.size() > 1)				
					reportIntersection(event);

				if (!event->lowerEnd.empty())
					removeLowerEndSegments(event);

				if (event->intersections > 0)
					reverseIntersectionOrder(event);

				if (!event->upperEnd.empty())
					insertUpperEndSegments(event);

				findNewEventPoints(event);
			}

			void reportIntersection(PointEventIt event)
			{
				// lowed-end segments are already inserted
				// intersecting too
				// so we add all segments from lower bound to upper bound of an intersection
				Intersection inter{event->point};
				
				auto l0 = m_sweepLine.lowerBound(event->point.x);
				auto l1 = m_sweepLine.upperBound(event->point.x);
				while (l0 != l1)
					inter.lines.push_back(*l0++);

				for (auto& line : event->upperEnd)
					inter.lines.push_back(line);

				m_intersections.push_back(std::move(inter));
			}

			void removeLowerEndSegments(PointEventIt event)
			{
				for (auto& it : event->lowerEnd)
					m_sweepLine.erase(it);
			}

			void reverseIntersectionOrder(PointEventIt event)
			{
				auto l0 = m_sweepLine.lowerBound(event->point.x);
				auto l1 = m_sweepLine.upperBound(event->point.x);

				--l1;
				for (u32 i = 0 ; i < event->intersections / 2; i++)
					std::swap(*l0++, *l1--);
			}

			void insertUpperEndSegments(PointEventIt event)
			{
				auto pred = [] (const auto& l0, const auto& l1)
				{
					return polar_y(l0) < polar_y(l1);
				};

				// sort with prioruty to polar angle
				std::sort(event->upperEnd.begin(), event->upperEnd.end(), pred);

				// iterators
				auto u0 = event->upperEnd.begin();
				auto u1 = event->upperEnd.end();
				auto i0 = m_sweepLine.lowerBound(event->point.x);
				auto i1 = m_sweepLine.upperBound(event->point.x);
				
				// CRASH CRASH CRASH CRASH CRASH CRASH CRASH CRASH CRASH CRASH CRASH CRASH CRASH CRASH
				// TODO : crashes because l0 can be equal l1 so segments are inserted in qrong order
				// CRASH CRASH CRASH CRASH CRASH CRASH CRASH CRASH CRASH CRASH CRASH CRASH CRASH CRASH

				auto last = i0; // position after that we will insert the rest
				while (i0 != i1 && u0 != u1) // mix-in upper-ends into sweep line
				{
					auto pi = polar_y(*i0);
					auto pu = polar_y(*u0);

					if (pi > pu) // if polar angle of an upper-end is less then insert before current position
					{
						auto inserted = m_sweepLine.insertBefore(i0, *u0++);
						insertLowerEndEvent(inserted);
					}
					else if (pi < pu) // else increase current position
					{
						last = i0++;
					}
					else
					{
						std::cout << "[WARNING] : overlapping segments deetected." << std::endl;
					}
				}

				if (last == m_sweepLine.end() && !m_sweepLine.empty())
					last = --m_sweepLine.end();

				while (u0 != u1) // insert the remaining after last
				{
					last = m_sweepLine.insertAfter(last, *u0++);
					insertLowerEndEvent(last);
				}
			}

			void findNewEventPoints(PointEventIt event)
			{
				auto beg = m_sweepLine.begin();
				auto end = m_sweepLine.end();

				auto l0 = m_sweepLine.lowerBound(event->point.x);
				auto l1 = m_sweepLine.upperBound(event->point.x);
				if (event->intersections + event->upperEnd.size() == 0) // only lower-end lines were in event
				{
					if (l0 == beg || l0 == end) // seems legit
						return;

					findNewEvent(l0 - 1, l0, event);
				}
				else // intersecting segments are still in event
				{
					if (l0 != beg) // seems legit
						findNewEvent(l0 - 1, l0, event);

					if (l1 != end)
						findNewEvent(l1 - 1, l1, event);
				}
			}

			// NOTE : l0 preceds l1, l0 != end(), l1 != end()
			void findNewEvent(SweepOrderIt l0, SweepOrderIt l1, PointEventIt event)
			{				
				vec2 i0;
				vec2 i1;
				auto status = intersectSegSeg(*l0, *l1, i0, i1); 
				if (status == Status::Intersection)
				{
					if (m_queue.preceds(event, i0))
						insertIntersectionEvent(i0);
				}
				else if (status == Status::Overlap)
				{
					std::cout << "[WARNING]: segments overlap" << std::endl;
				}
			}


		private:
			std::vector<Intersection> m_intersections;
			SweepOrder m_sweepLine;
			EventQueue m_queue;
		};
	}

	std::vector<Intersection> section_n_lines(const std::vector<prim::Line2>& lines)
	{
		return Sector().sect(lines);
	}
}
