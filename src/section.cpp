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

		// NOTE : sweeping from up to down (from upper y to lower), from left to right (from left x to right)
		// NOTE : segments are stored from left to right in what order they intersect the sweep line
		// NOTE : algorithm (in theory) doesn't care in what order we insert horizontal order
		// NOTE : horizontal segments are processed in the following manner:
		// 1) upper end is its leftmost end, lower end is its rightmost end
		// 2) horizontal segment intersects sweep line in sweep.x so it can be processed by the algorithm correctly
		// 3) horizontal segments are inserted after all notmal segments were inserted
		
		// NOTE : returns line in which v0 is upper vertex and v1 is lower vertex
		Line2 reorder_line(const Line2& l, Float eps = default_eps)
		{
			if (!horiz(l))
			{
				if (l.v0.y > l.v1.y)
					return l;	
				return {l.v1, l.v0};
			}

			if (l.v0.x < l.v1.x)
				return l;
			return {l.v1, l.v0};
		}

		// NOTE : ctan, minus required because we sweep from top to bottom
		// NOTE : no horizontal segments here!
		Float sweep_polar_y(const Vec2& point, Float eps = default_eps)
		{
			assert(std::abs(point.y) > eps);

			return -point.x / point.y;
		}

		// NOTE : no horizontal segments here!
		Float sweep_polar_y(const Line2& line, Float eps = default_eps)
		{
			assert(!horiz(line, eps));

			auto [v0, v1] = reorder_line(line, eps);

			return sweep_polar_y(v1 - v0);
		}


		// [DEBUG]
		std::ostream& operator << (std::ostream& out, const prim::Vec2& p)
		{
			return out << "(" <<  p.x << " " << p.y << ")";	
		}

		// [DEBUG]
		std::ostream& operator << (std::ostream& out, const prim::Line2& l)
		{
			return out << l.v0 << l.v1;
		}


		struct SweepLineComparator
		{
			bool operator () (const Line2& l0, const Line2& l1)
			{
				Float x0 = sweep.x;
				if (!horiz(l0))
					intersectsLineY(l0, sweep.y, x0);

				Float x1 = sweep.x;
				if (!horiz(l1))
					intersectsLineY(l1, sweep.y, x1);

				return std::abs(x0 - x1) > eps && x0 < x1;
			}

			bool operator() (const Line2& l, Float x)
			{
				Float xi = sweep.x;
				if (!horiz(l))
					intersectsLineY(l, sweep.y, xi);

				return std::abs(xi - x) > eps && xi < x;
			}
				
			bool operator() (Float x, const Line2& l)
			{
				Float xi = sweep.x;
				if (!horiz(l))
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
			const Vec2& sweep() const
			{
				return m_compare.sweep;
			}

			void sweep(Vec2 sw)
			{
				m_compare.sweep = sw;
			}

			// [DEBUG]
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

			// segments that intersect in the point will not be stored explitly
			// their order in sweep line will be reversed after event is processed
			// intersection count is not stored explicitly

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


		// TODO : check possible impossible
		// TODO : maybe passing iterator by reference will be more efficient (micro optimization)
		class Sector
		{
		public:
			using SweepOrderIt = SweepOrder::Iterator;
			using SweepOrderEx = SweepOrder::Extract;

			using PointEventIt = EventQueue::Iterator;

		public:
			Sector(Float eps) : m_eps(eps)
			{}

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
				// [DEBUG]
				std::cout << "*** init ***" << std::endl;

				for (auto& line : lines)
					insertUpperEndEvent(line);

				// [DEBUG]
				std::cout << std::endl;
			}


			// NOTE : called only from initialization step, line hasn't been inserted yet, so pure line is passed
			void insertUpperEndEvent(const Line2& line)
			{
				auto [v0, v1] = reorder_line(line, m_eps);

				auto [it, _] = m_queue.insert(v0);
				it->upperEnd.push_back(line);

				// [DEBUG]
				std::cout << "upper: " << v0 << std::endl;
			}

			// NOTE : called after upper-end of a segment was processed so existing position is passed
			void insertLowerEndEvent(SweepOrderIt line)
			{
				auto [v0, v1] = reorder_line(*line, m_eps);

				auto [it, _] = m_queue.insert(v1);
				it->lowerEnd.push_back(line);

				// [DEBUG]
				std::cout << "lower: " << v1 << std::endl;
			}

			// NOTE : called if intersection was found
			void insertIntersectionEvent(const Vec2& point)
			{
				auto [it, _] = m_queue.insert(point);

				// [DEBUG]
				std::cout << "inter: " << point << std::endl;
			}


			void handlePointEvent(PointEventIt event)
			{
				// [DEBUG]
				std::cout << "event: " << event->point << std::endl;

				m_sweepLine.sweep(event->point);

				// [DEBUG]
				m_sweepLine.debug();

				u32 lowerInter = countLowerInter(event);

				if (lowerInter + event->upperEnd.size() > 1)
					reportIntersection(event);

				if (!event->lowerEnd.empty())
				{
					removeLowerEndSegments(event);

					// [DEBUG]
					std::cout << "le rem ";
					m_sweepLine.debug();
				}

				if (lowerInter != event->lowerEnd.size()) // not only lower-ends were in intersection
				{
					reverseIntersectionOrder(event);
				
					// [DEBUG]
					std::cout << "int rev ";
					m_sweepLine.debug();
				}
				
				if (!event->upperEnd.empty())
				{
					insertUpperEndSegments(event);
				
					// [DEBUG]
					std::cout << "ue add ";
					m_sweepLine.debug();
				}

				findNewEventPoints(event, lowerInter);

				// [DEBUG]
				std::cout << std::endl;
			}

			u32 countLowerInter(PointEventIt event)
			{
				auto l0 = m_sweepLine.lowerBound(event->point.x);
				auto l1 = m_sweepLine.upperBound(event->point.x);

				u32 inters = 0u;
				if (l0 != m_sweepLine.end())
					inters += std::distance(l0, l1);
				return inters;
			}

			// NOTE : checked in handleEventPoint
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

			// NOTE : checked in handleEventPoint
			void removeLowerEndSegments(PointEventIt event)
			{
				for (auto& it : event->lowerEnd)
					m_sweepLine.erase(it);
			}

			// NOTE : checked in handleEventPoint
			void reverseIntersectionOrder(PointEventIt event)
			{
				// TODO : check possible impossible

				// NOTE : we extract and then we reinsert all segments in order not to spoil 
				// lower-end iterators that had already been inserted previously
				extractIntersecting(event);

				// reinsert back
				auto e0 = m_extractBuffer.begin();
				auto e1 = m_extractBuffer.end();
				auto insertPos = m_sweepLine.lowerBound(event->point.x);
				if (m_sweepLine.empty())
				{
					auto [inserted, _] = m_sweepLine.insert(std::move(*e0++));
					while (e0 != e1)
						inserted = m_sweepLine.insertAfter(inserted, std::move(*e0++));
				}
				else
				{
					if (insertPos != m_sweepLine.begin())
					{
						--insertPos;
						while (e0 != e1)
							insertPos = m_sweepLine.insertAfter(insertPos, std::move(*e0++));
					}
					else
					{
						while (e0 != e1)
							m_sweepLine.insertBefore(insertPos, std::move(*e0++));
					}
				}
			}

			// NOTE : checked in handleEventPoint
			void insertUpperEndSegments(PointEventIt event)
			{
				// TODO : check possible impossible
				// TODO : horizontal segments

				// sort with priority to polar angle
				auto pred = [=] (const auto& l0, const auto& l1)
				{
					auto p0 = sweep_polar_y(l0, m_eps);
					auto p1 = sweep_polar_y(l1, m_eps);

					return std::abs(p0 - p1) > m_eps && p0 < p1;
				};

				auto h0 = moveBackHorizUpper(event); // first horizontal
				auto h1 = event->upperEnd.end();     // last horizontal
				auto u0 = event->upperEnd.begin();   // first normal
				std::sort(u0, h0, pred);

				// iterators
				auto l0 = m_sweepLine.lowerBound(event->point.x);
				auto l1 = m_sweepLine.upperBound(event->point.x);
				while (l0 != l1 && !horiz(*l0) && u0 != h0) // insert until can or horizontal segment is met
				{
					auto pl = sweep_polar_y(*l0, m_eps);
					auto pu = sweep_polar_y(*u0, m_eps);

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
				
				// check carefully whether we can insert after them
				// all remaining segments have bigger sweep_polar_y so we insert before l1 that is another point
				if (l0 == l1)
				{
					if (m_sweepLine.empty())
					{
						auto [inserted, _] = m_sweepLine.insert(*u0++);
						insertLowerEndEvent(inserted);
						while (u0 != h1) // rest + horizontal
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
							while (u0 != h1) // rest + horizontal
							{
								inserted = m_sweepLine.insertAfter(inserted, *u0++);
								insertLowerEndEvent(inserted);
							}
						}
						else
						{
							while (u0 != h1) // rest + horizontal
							{
								auto inserted = m_sweepLine.insertBefore(l1, *u0++);
								insertLowerEndEvent(inserted);
							}
						}
					}

					return;
				}

				// there were interescting lines and cycle broke due to horizontal segment in sweep line
				if (horiz(*l0))
				{
					while (u0 != h1) // rest + horizontal
					{
						auto inserted = m_sweepLine.insertBefore(l0, *u0++);
						insertLowerEndEvent(inserted);
					}

					return;
				}

				// u0 == h0, all nonhorizontal segments were inserted
				// see NOTE, we can simply insert after all segments because order of horizontal segments arbitrarly 
				// (but they all inserted after normal)
				auto inserted = --l1;
				while (u0 != h1)
				{
					inserted = m_sweepLine.insertAfter(inserted, *u0++);
					insertLowerEndEvent(inserted);
				}
			}

			// NOTE : simpler version of reverse + insertUpper (just to be sure that you can use std::multiset)
			void reorderInsert_test(PointEventIt event)
			{
				// TODO : check possible impossible

				extractIntersecting(event);

				// simple merge
				auto n0 = m_extractBuffer.begin();
				auto n1 = m_extractBuffer.end();
				auto u0 = event->upperEnd.begin();
				auto u1 = event->upperEnd.end();
				while (n0 != n1 && u0 != u1)
				{
					auto pn = sweep_polar_y(**n0); // double pointer
					auto pu = sweep_polar_y(*u0);

					if (pn < pu)
					{
						m_sweepLine.insert(std::move(*n0++));
					}
					else
					{
						auto [inserted, _] = m_sweepLine.insert(*u0++);
						insertLowerEndEvent(inserted);
					}
				}
				while (n0 != n1)
				{
					m_sweepLine.insert(std::move(*n0++));
				}
				while (u0 != u1)
				{
					auto [inserted, _] = m_sweepLine.insert(*u0++);
					insertLowerEndEvent(inserted);
				}
			}

			void findNewEventPoints(PointEventIt event, u32 lowerInter)
			{
				// TODO : check possible impossible

				auto beg = m_sweepLine.begin();
				auto end = m_sweepLine.end();

				auto l0 = m_sweepLine.lowerBound(event->point.x);
				auto l1 = m_sweepLine.upperBound(event->point.x);
				if (lowerInter == event->lowerEnd.size()) // only lower-end lines were in event
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
			}


			void extractIntersecting(PointEventIt event)
			{
				auto l0 = m_sweepLine.lowerBound(event->point.x);
				auto l1 = m_sweepLine.upperBound(event->point.x);

				// extract and reverse
				m_extractBuffer.clear();
				while (l0 != l1)
					m_extractBuffer.push_back(m_sweepLine.extract(l0++));
				std::reverse(m_extractBuffer.begin(), m_extractBuffer.end());
			}

			auto moveBackHorizUpper(PointEventIt event) -> decltype(event->upperEnd.begin())
			{
				auto u0 = event->upperEnd.begin();
				auto u1 = event->upperEnd.end();
				while (u0 != u1)
				{
					if (horiz(*u0))
						std::iter_swap(u0, --u1);
					else
						++u0;
				}
				return u1;
			}


		private:
			std::vector<Intersection> m_intersections;
			std::vector<SweepOrderEx> m_extractBuffer;

			SweepOrder m_sweepLine;
			EventQueue m_queue;

			Float m_eps = default_eps;
		};
	}

	std::vector<Intersection> section_n_lines(const std::vector<prim::Line2>& lines, Float eps)
	{
		return Sector(eps).sect(lines);
	}
}
