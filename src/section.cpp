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
		// NOTE : sweeping from up to down (from upper y to lower), from left to right (from left x to right)
		// NOTE : segments are stored from left to right in what order they intersect the sweep line

		// NOTE : returns line in which v0 is upper vertex and v1 is lower vertex
		Line2 reorder_line(const Line2& l)
		{
			if (l.v0.y < l.v1.y)
				return {l.v1, l.v0};
			return l;
		}


		struct SweepLineComparator
		{
			// l0 < l1
			bool operator () (const Line2& l0, const Line2& l1)
			{
				// simplification for now
				assert(!horiz(l0));
				assert(!horiz(l1));

				Float i0;
				intersectsLineY(l0, sweepY, i0);

				Float i1;
				intersectsLineY(l1, sweepY, i1);

				return i0 < i1;
			}

			bool operator() (const Line2& l, Float x)
			{
				assert(!horiz(l));

				Float i;
				intersectsLineY(l, sweepY, i);

				return i < x;
			}
				
			bool operator() (Float x, const Line2& l)
			{
				assert(!horiz(l));

				Float i;
				intersectsLineY(l, sweepY, i);

				return x < i;
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

			bool preceds(Iterator l0, Iterator l1)
			{
				if (l0 == end())
					return false;
				if (l1 == end())
					return true;
				return preceds(*l0, *l1);
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
			std::vector<Line2> lowerEnd;

			// segments that purely intersect in the point
			// their order in sweep line will be reversed
			SweepOrderIt l0; // leftmost of the bundle
			SweepOrderIt l1; // rightmost of the bundle

			// segments that have their upper end in the point
			// they will be inserted, I will search next lower y (if there is no so it can be chosen arbitrarly)
			std::vector<Line2> upperEnd;
		};

		struct PointEventComparator
		{
			// for searching & insertion
			bool operator () (const PointEvent& e0, const PointEvent& e1)
			{
				if (std::abs(e0.point.y - e1.point.y) <= eps)
					return e0.point.y > e1.point.y;
				return e0.point.x < e1.point.x;
			}

			// for searching & insertion
			bool operator () (const PointEvent& e0, const vec2& v0)
			{
				if (std::abs(e0.point.y - v0.y) <= eps)
					return e0.point.y > v0.y;
				return e0.point.x < v0.x;
			}

			bool operator () (const vec2& v0, const PointEvent& e0)
			{
				if (std::abs(v0.y - e0.point.y) <= eps)
					return v0.y > e0.point.y;
				return v0.x < e0.point.x;
			}

			// for finding safe y
			bool operator () (const PointEvent& e0, Float y)
			{
				return e0.point.y < y;
			}

			bool operator () (Float y, const PointEvent& e0)
			{
				return y < e0.point.y;
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

			// NOTE : special upper-bound method to find safe y that will be used to insert upperEnd segments into sweep line
			Float lowerY(Float y)
			{
				/*
				Node* ub = m_nil;
				Node* node = m_root;
				while (node != m_nil)
				{
				if (node->key.point.y <= y)
				{
				node = node->right;
				}
				else
				{
				ub = node;
				node = node->left;
				}
				}

				if (ub != m_nil)
				return y + (ub->key.point.y - y) / 2;
				return y + std::abs(y);
				*/
				if (auto ub = upperBound(y); ub != end())
					return y + (ub->point.y - y) / 2;
				return y + std::abs(y);
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


			// NOTE : called only from initialization step,
			void insertUpperEndEvent(const Line2& line)
			{
				auto [v0, v1] = reorder_line(line);

				auto [it, inserted] = m_queue.insert(v0);
				if (inserted)
				{
					it->l0 = m_sweepLine.end();
					it->l1 = m_sweepLine.end();
				}
				it->upperEnd.push_back(line);
			}

			// NOTE : called after upper-end of a segment was processed
			void insertLowerEndEvent(const Line2& line)
			{
				auto [v0, v1] = reorder_line(line);

				auto [it, inserted] = m_queue.insert(v1);
				if (inserted)
				{
					it->l0 = m_sweepLine.end();
					it->l1 = m_sweepLine.end();
				}
				it->lowerEnd.push_back(line);
			}

			// NOTE : called if intersection was found
			// NOTE : l0 preceds l1
			void insertIntersectionEvent(SweepOrderIt l0, SweepOrderIt l1, const vec2& point)
			{
				auto [it, inserted] = m_queue.insert(point);
				if (inserted)
				{
					it->l0 = m_sweepLine.end();
					it->l1 = m_sweepLine.end();
				}
				if (m_sweepLine.preceds(l0, it->l0))
					it->l0 = l0;
				if (m_sweepLine.preceds(it->l1, l1))
					it->l1 = l1;
			}


			void handlePointEvent(PointEventIt event)
			{
				m_sweepLine.sweepY(event->point.y);

				u32 interCount = countIntersections(event);

				if (interCount + event->lowerEnd.size() + event->upperEnd.size() != 0)				
					reportIntersection(event, interCount);

				reverseIntersectionOrder(event, interCount);

				insertUpperEndSegments(event, interCount);

				findNewEventPoints(event, interCount);
			}

			u32 countIntersections(PointEventIt event)
			{
				u32 count = 0;
				auto l0 = event->l0;
				auto l1 = event->l1;
				if (l0 != m_sweepLine.end())
					for (count = 1; l0 != l1; ++count, ++l0);
				return count;
			}

			void reportIntersection(PointEventIt event, u32 interCount)
			{
				Intersection inter{event->point};
				for (auto& line : event->lowerEnd)
					inter.lines.push_back(line);

				for (auto& line : event->upperEnd)
					inter.lines.push_back(line);

				auto l0 = event->l0;
				for (u32 i = 0; i < interCount; i++)
					inter.lines.push_back(*l0);

				m_intersections.push_back(std::move(inter));
			}

			void reverseIntersectionOrder(PointEventIt event, u32 interCount)
			{
				auto l0 = event->l0;
				auto l1 = event->l1;
				for (u32 i = 0; i < interCount / 2; i++)
					std::swap(*l0++, *l1--);
				std::swap(event->l0, event->l1);
			}

			void insertUpperEndSegments(PointEventIt event, u32 interCount)
			{
				if (!event->upperEnd.empty())
				{
					m_sweepLine.sweepY(m_queue.lowerY(event->point.y));
					for (auto& line : event->upperEnd)
					{
						auto [it, _] = m_sweepLine.insert(line);

						insertLowerEndEvent(line);
						if (m_sweepLine.preceds(it, event->l0))
							event->l0 = it;
						if (m_sweepLine.preceds(event->l1, it))
							event->l1 = it;
					}
					m_sweepLine.sweepY(event->point.y);

					event->upperEnd.clear();
				}
			}

			void findNewEventPoints(PointEventIt event, u32 interCount)
			{
				auto end = m_sweepLine.end();
				auto beg = m_sweepLine.begin();
				if (interCount + event->upperEnd.size() == 0)
				{
					auto l0 = m_sweepLine.lowerBound(event->point.x);
					if (l0 != beg && l0 != end) // I check if iterator equals begin() because of standart: begin() must not be decrementable
						--l0;

					auto l1 = m_sweepLine.upperBound(event->point.x);

					findNewEvent(l0, l1, event);
				}
				else
				{
					auto l0 = event->l0;
					if (l0 != beg) // I check if iterator equals begin() because of standart: begin() must not be decrementable
						--l0;

					auto l1 = event->l1;
					if (l1 != end)
						++l0;

					findNewEvent(l0, event->l0, event);
					findNewEvent(event->l1, l1, event);
				}
			}

			// NOTE : l0 preceds l1
			void findNewEvent(SweepOrderIt l0, SweepOrderIt l1, PointEventIt event)
			{
				auto end = m_sweepLine.end();
				if (l0 == end || l1 == end)
					return;

				vec2 i0;
				vec2 i1;
				auto status = intersectSegSeg(*l0, *l1, i0, i1); 
				if (status == Status::Intersection)
				{
					if (m_queue.preceds(event, i0))
						insertIntersectionEvent(l0, l1, i0);
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
