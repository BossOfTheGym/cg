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

		Float polar_y(const Vec2& point)
		{
			return point.x / point.y;
		}

		Float polar_y(const Line2& line)
		{
			auto& [v0, v1] = line;

			return polar_y(v1 - v0);
		}

		std::ostream& operator << (std::ostream& out, const prim::Vec2& p)
		{
			return out << "(" <<  p.x << " " << p.y << ")";	
		}

		std::ostream& operator << (std::ostream& out, const prim::Line2& l)
		{
			return out << l.v0 << l.v1;
		}


		struct SweepLineComparator
		{
			bool operator () (const Line2& l0, const Line2& l1)
			{
				// simplification
				assert(!horiz(l0));
				assert(!horiz(l1));

				Float x0;
				intersectsLineY(l0, sweep.y, x0);

				Float x1;
				intersectsLineY(l1, sweep.y, x1);
				
				return std::abs(x0 - x1) > eps && x0 < x1;
			}

			bool operator() (const Line2& l, Float x)
			{
				// simplification
				assert(!horiz(l));

				Float xi;
				intersectsLineY(l, sweep.y, xi);
				
				return std::abs(xi - x) > eps && xi < x;
			}
				
			bool operator() (Float x, const Line2& l)
			{
				// simplification
				assert(!horiz(l));

				Float xi;
				intersectsLineY(l, sweep.y, xi);
				
				return std::abs(x - xi) > eps && x < xi;
			}

			Vec2 sweep{};
			Float eps = default_eps;
		};

		// multiset-like
		using sweep_order_traits_t = trb::TreeTraits<Line2, SweepLineComparator, trb::DefaultAllocator, false, true>;

		class SweepOrder : public trb::Tree<sweep_order_traits_t>
		{
		public:
			// insert, erase and empty already in tree
			bool preceds(const Line2& l0, const Line2& l1)
			{
				return m_compare(l0, l1);
			}

			Vec2 sweep() const
			{
				return m_compare.sweep;
			}

			void sweep(Vec2 sw)
			{
				m_compare.sweep = sw;
			}

			void debug()
			{
				std::cout << "sweep: ";
				for (auto& l : *this)
					std::cout << l << " ";
				std::cout << std::endl;
			}
		};


		struct PointEvent
		{
			using SweepOrderIt = typename SweepOrder::Iterator;

			// event point : lower point event, upper point event, intersection point event
			// can be all together
			Vec2 point;

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
				return std::abs(e0.point.x - e1.point.x) > eps && e0.point.x < e1.point.x;
			}

			// for searching & insertion
			bool operator () (const PointEvent& e0, const Vec2& v0)
			{
				if (std::abs(e0.point.y - v0.y) > eps)
					return e0.point.y > v0.y;
				return std::abs(e0.point.x - v0.x) > eps && e0.point.x < v0.x;
			}

			bool operator () (const Vec2& v0, const PointEvent& e0)
			{
				if (std::abs(v0.y - e0.point.y) > eps)
					return v0.y > e0.point.y;
				return std::abs(v0.x - e0.point.x) > eps && v0.x < e0.point.x;
			}

			// for finding safe y
			bool operator () (const PointEvent& e0, Float y)
			{
				return std::abs(e0.point.y - y) > eps && e0.point.y > y;
			}

			bool operator () (Float y, const PointEvent& e0)
			{
				return std::abs(y - e0.point.x) > eps && y > e0.point.y;
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
			void push(const Vec2& point)
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

			bool preceds(Iterator it, const Vec2& point)
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
				initialize(lines);

				while (!m_queue.empty())
				{
					auto it = m_queue.front();

					handlePointEvent(it);

					m_queue.pop();
				}

				assert(m_sweepLine.empty());

				return std::move(m_intersections);
			}

		private:
			// NOTE : initializing event queue. Here only upper-end segments are inserted, 
			// lower-end segments will be inserted after upper-end is processed
			void initialize(const std::vector<Line2>& lines)
			{
				std::cout << "*** init ***" << std::endl;

				for (auto& line : lines)
					insertUpperEndEvent(line);

				std::cout << std::endl;
			}


			// NOTE : called only from initialization step, line hasn't been inserted yet, so pure line is passed
			void insertUpperEndEvent(const Line2& line)
			{
				auto [v0, v1] = reorder_line(line);

				auto [it, _] = m_queue.insert(v0);
				it->upperEnd.push_back(line);

				std::cout << "upper: " << v0 << std::endl;
			}

			// NOTE : called after upper-end of a segment was processed so existing position is passed
			void insertLowerEndEvent(SweepOrderIt line)
			{
				auto [v0, v1] = reorder_line(*line);

				auto [it, _] = m_queue.insert(v1);
				it->lowerEnd.push_back(line);

				std::cout << "lower: " << v1 << std::endl;
			}

			// NOTE : called if intersection was found
			void insertIntersectionEvent(const Vec2& point)
			{
				auto [it, _] = m_queue.insert(point);
				it->intersections = 1; // just to note that intersection occured

				std::cout << "inter: " << point << std::endl;
			}


			void handlePointEvent(PointEventIt event)
			{
				std::cout << "event: " << event->point << std::endl;

				m_sweepLine.sweep(event->point);

				m_sweepLine.debug();

				if (event->intersections != 0 || event->lowerEnd.size() + event->upperEnd.size() > 1)	
					reportIntersection(event);

				if (!event->lowerEnd.empty())
				{
					removeLowerEndSegments(event);

					std::cout << "le rem ";
					m_sweepLine.debug();
				}

				if (event->intersections != 0)
				{
					reverseIntersectionOrder(event);
				
					std::cout << "int rev ";
					m_sweepLine.debug();
				}
				
				if (!event->upperEnd.empty())
				{
					insertUpperEndSegments(event);
				
					std::cout << "ue add ";
					m_sweepLine.debug();
				}
				
				//reorderInterInsertUpper(event);

				findNewEventPoints(event);

				std::cout << std::endl;
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
				// TODO : check possible impossible

				// NOTE : we extract and then we reinsert all segments in order not to spoil 
				// lower-end iterators that had already been inserted previously
				auto newOrder = [&] () 
				{
					auto l0 = m_sweepLine.lowerBound(event->point.x);
					auto l1 = m_sweepLine.upperBound(event->point.x);

					// extract and reverse
					std::vector<SweepOrder::Extract> extracted;
					while (l0 != l1)
						extracted.push_back(m_sweepLine.extract(l0++));
					std::reverse(extracted.begin(), extracted.end());

					return extracted;
				} ();

				// reinsert back
				auto n0 = newOrder.begin();
				auto n1 = newOrder.end();
				auto insertPos = m_sweepLine.lowerBound(event->point.x);
				if (m_sweepLine.empty())
				{
					auto [inserted, _] = m_sweepLine.insert(std::move(*n0++));
					while (n0 != n1)
						inserted = m_sweepLine.insertAfter(inserted, std::move(*n0++));
				}
				else
				{
					if (insertPos != m_sweepLine.begin())
					{
						--insertPos;
						while (n0 != n1)
							insertPos = m_sweepLine.insertAfter(insertPos, std::move(*n0++));
					}
					else
					{
						while (n0 != n1)
							m_sweepLine.insertBefore(insertPos, std::move(*n0++));
					}
				}
			}

			void insertUpperEndSegments(PointEventIt event)
			{
				// TODO : check possible impossible

				// sort with priority to polar angle
				auto pred = [=] (const auto& l0, const auto& l1)
				{
					auto p0 = polar_y(l0);
					auto p1 = polar_y(l1);

					return std::abs(p0 - p1) > m_eps && p0 < p1;
				};

				std::sort(event->upperEnd.begin(), event->upperEnd.end(), pred);

				// iterators
				auto l0 = m_sweepLine.lowerBound(event->point.x);
				auto l1 = m_sweepLine.upperBound(event->point.x);
				auto u0 = event->upperEnd.begin();
				auto u1 = event->upperEnd.end();
				while (l0 != l1 && u0 != u1) // l0 == l1 => no intersecting segments were in sweep line or we can't insert anymore
				{
					auto pl = polar_y(*l0);
					auto pu = polar_y(*u0);

					if (pl < pu)
					{
						++l0;
					}
					else // pl > pu
					{
						auto inserted = m_sweepLine.insertBefore(l0, *u0++);
						insertLowerEndEvent(inserted);
					}
				}
				
				// inserted all, nothing to do
				if (u0 == u1)
					return;

				// l0 == l1, check carefully whether we can insert after them
				// all remaining segments have bigger polar_y so we insert before l1 that is another point
				if (m_sweepLine.empty())
				{
					auto [inserted, _] = m_sweepLine.insert(*u0++);
					insertLowerEndEvent(inserted);
					while (u0 != u1)
					{
						inserted = m_sweepLine.insertAfter(inserted, *u0++);
						insertLowerEndEvent(inserted);
					}
				}
				else
				{
					if (l1 == m_sweepLine.end())
					{
						auto inserted = --l1;
						while (u0 != u1)
						{
							inserted = m_sweepLine.insertAfter(inserted, *u0++);
							insertLowerEndEvent(inserted);
						}
					}
					else
					{
						while (u0 != u1)
						{
							auto inserted = m_sweepLine.insertBefore(l1, *u0++);
							insertLowerEndEvent(inserted);
						}
					}
				}
			}

			void reorderInterInsertUpper(PointEventIt event)
			{
				// TODO : insert only version of reverseUpperEndSegments + insertUpperEndSegments

			}

			void findNewEventPoints(PointEventIt event)
			{
				// TODO : check possible impossible

				auto beg = m_sweepLine.begin();
				auto end = m_sweepLine.end();

				auto l0 = m_sweepLine.lowerBound(event->point.x);
				auto l1 = m_sweepLine.upperBound(event->point.x);
				if (event->intersections == 0 && event->upperEnd.empty()) // only lower-end lines were in event
				{
					if (l0 == beg || l0 == end)
						return;

					findNewEvent(l0 - 1, l0, event);
				}
				else
				{
					if (l0 != beg)
						findNewEvent(l0 - 1, l0, event);

					if (l1 != end)
						findNewEvent(l1 - 1, l1, event);
				}
			}

			// NOTE : l0 preceds l1, l0 != end(), l1 != end()
			void findNewEvent(SweepOrderIt l0, SweepOrderIt l1, PointEventIt event)
			{			
				// TODO : check possible impossible

				Vec2 i0;
				Vec2 i1;
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

			Float m_eps = default_eps;
		};
	}

	std::vector<Intersection> section_n_lines(const std::vector<prim::Line2>& lines)
	{
		return Sector().sect(lines);
	}
}
