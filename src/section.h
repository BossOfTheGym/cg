#pragma once

#include "primitive.h"
#include "trb_tree.h"

#include "core.h"

#include <vector>
#include <cassert>
#include <utility>
#include <algorithm>

//#define DEBUG_SEGMENTS
#ifdef DEBUG_SEGMENTS
#include <iostream>
#endif

namespace sect
{
	using namespace prim;

	namespace
	{
		#ifdef DEBUG_SEGMENTS
		std::ostream& operator << (std::ostream& out, const Vec2& p)
		{
			return out << "(" <<  p.x << " " << p.y << ")";	
		}

		std::ostream& operator << (std::ostream& out, const Line2& l)
		{
			return out << l.v0 << l.v1;
		}
		#endif

		
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
	}

	template<class handle_t>
	struct Intersection
	{
		using Handle = handle_t;

		// intersection point
		Vec2 point;

		// set of intersecting segments
		std::vector<Handle> lines;
	};

	template<class handle_t>
	using Intersections = std::vector<Intersection<handle_t>>;

	// TODO : O(n^2) memory consumption removal, see https://ru.wikipedia.org/wiki/Алгоритм_Бентли_—_Оттманна
	// NOTE : sweeping from up to down (from upper y to lower), from left to right (from left x to right)
	// NOTE : overlapping must processed correctly but rather stangely: after new overlapping segment is found new intersection will be reported
	// NOTE : segments are stored from left to right in what order they intersect the sweep line
	// NOTE : algorithm (in theory) doesn't care in what order we insert horizontal order but there are inserted in special manner
	// NOTE : horizontal segments are processed in the following manner:
	// 1) upper end is its leftmost end, lower end is its rightmost end
	// 2) horizontal segment intersects sweep line in sweep.x so it can be processed by the algorithm correctly
	// 3) horizontal segments are inserted after all notmal segments were inserted
	template<class handle_t, class sampler_t>
	class Sector
	{
	public:
		using Handle  = handle_t;
		using Sampler = sampler_t;

		// sweep line, multiset-like
		struct SweepLineComparator
		{
			SweepLineComparator(Sampler samp, Float eps) : sampler(samp)
			{}

			bool operator () (Handle l0, Handle l1)
			{
				auto line0 = sampler(l0);
				auto line1 = sampler(l1);

				Float x0 = sweep.x;
				if (!horiz(line0))
					intersectsLineY(line0, sweep.y, x0);

				Float x1 = sweep.x;
				if (!horiz(line1))
					intersectsLineY(line1, sweep.y, x1);

				return std::abs(x0 - x1) > eps && x0 < x1;
			}

			bool operator() (Handle l, Float x)
			{
				auto line = sampler(l);

				Float xi = sweep.x;
				if (!horiz(line))
					intersectsLineY(line, sweep.y, xi);

				return std::abs(xi - x) > eps && xi < x;
			}

			bool operator() (Float x, Handle l)
			{
				auto line = sampler(l);

				Float xi = sweep.x;
				if (!horiz(line))
					intersectsLineY(line, sweep.y, xi);

				return std::abs(x - xi) > eps && x < xi;
			}

			Sampler sampler{};
			Float eps = default_eps;

			Vec2 sweep{};
		};

		using sweep_line_traits_t = trb::TreeTraits<Handle, SweepLineComparator, trb::DefaultAllocator, false, true>;

		class SweepLine : public trb::Tree<sweep_line_traits_t>
		{
		public:
			using Tree = trb::Tree<sweep_line_traits_t>;
			using Iterator = typename Tree::Iterator;

			SweepLine(Sampler sampler, Float eps) : Tree(SweepLineComparator(sampler, eps))
			{}

		public:
			const Vec2& sweep() const
			{
				return this->m_compare.sweep;
			}

			void sweep(const Vec2& sw)
			{
				this->m_compare.sweep = sw;
			}

			#ifdef DEBUG_SEGMENTS
			// [DEBUG]
			void debug()
			{
				std::cout << "sweep: ";
				for (auto& l : *this)
					std::cout << this->m_compare.sampler(l) << " ";
				std::cout << std::endl;
			}
			#endif
		};


		// event queue, set-like
		struct PointEvent
		{
			using SweepLineIt = typename SweepLine::Iterator;

			// event point : lower point event, upper point event, intersection point event
			// can be all together
			Vec2 point;

			// segments that have their lower end in the point
			// they will be deleted, added after upper ends are processed, order is not neccessary
			std::vector<SweepLineIt> lowerEnd;

			// segments that intersect in the point will not be stored explitly
			// their order in sweep line will be reversed after event is processed
			// intersection count is not stored explicitly

			// segments that have their upper end in the point
			// they will be inserted, I will search next lower y (if there is no so it can be chosen arbitrarly)
			std::vector<Handle> upperEnd;
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

		using event_queue_traits_t = trb::TreeTraits<PointEvent, PointEventComparator, trb::DefaultAllocator, false, false>;

		class EventQueue : public trb::Tree<event_queue_traits_t>
		{
		public:
			using Tree     = trb::Tree<event_queue_traits_t>;
			using Node     = typename Tree::Node;
			using Iterator = typename Tree::Iterator;


			EventQueue(Float eps) : Tree(PointEventComparator(eps))
			{}

		public: // utility functions
			void push(const Vec2& point)
			{
				this->insert(point);
			}

			void pop()
			{
				this->erase(this->begin());
			}

			Iterator front()
			{
				return this->begin();
			}

			bool preceds(const PointEvent& event, const Vec2& point)
			{
				return this->m_compare(event, point);
			}
		};


		using SweepLineEx  = typename SweepLine::Extract;
		using SweepLineIt  = typename SweepLine::Iterator;
		using PointEventIt = typename EventQueue::Iterator;

		using ExtractBuffer = std::vector<SweepLineEx>;

	public:
		Sector(const std::vector<Handle>& lines, Sampler sampler, Float eps) 
			: m_sweepLine(sampler, eps)
			, m_eventQueue(eps)
			, m_sampler(sampler)
			, m_eps(eps)
		{
			initialize(lines);
		}

	public:
		Intersections<Handle> sect()
		{
			while (!m_eventQueue.empty())
			{
				auto it = m_eventQueue.front();
				m_event = std::move(*it);
				m_eventQueue.erase(it);

				handlePointEvent();
			}

			assert(m_sweepLine.empty());

			return std::move(m_intersections);
		}

	private:
		// NOTE : initializing event queue. Here only upper-end segments are inserted, 
		// lower-end segments will be inserted after upper-end is processed
		void initialize(const std::vector<Handle>& lines)
		{
			#ifdef DEBUG_SEGMENTS
			std::cout << "*** init ***" << std::endl;
			#endif

			for (auto& line : lines)
				insertUpperEndEvent(line);

			#ifdef DEBUG_SEGMENTS
			std::cout << std::endl;
			#endif
		}


		// NOTE : called only from initialization step, line hasn't been inserted yet, so pure line is passed
		void insertUpperEndEvent(Handle l)
		{
			auto [v0, v1] = reorder_line(m_sampler(l), m_eps);

			auto [it, _] = m_eventQueue.insert(v0);
			it->upperEnd.push_back(l);

			#ifdef DEBUG_SEGMENTS
			std::cout << "upper: " << v0 << std::endl;
			#endif
		}

		// NOTE : called after upper-end of a segment was processed so existing position is passed
		void insertLowerEndEvent(SweepLineIt line)
		{
			auto [v0, v1] = reorder_line(m_sampler(*line), m_eps);

			auto [it, _] = m_eventQueue.insert(v1);
			it->lowerEnd.push_back(line);

			#ifdef DEBUG_SEGMENTS
			std::cout << "lower: " << v1 << std::endl;
			#endif
		}

		// NOTE : called if intersection was found
		void insertIntersectionEvent(const Vec2& point)
		{
			auto [it, _] = m_eventQueue.insert(point);

			#ifdef DEBUG_SEGMENTS
			std::cout << "inter: " << point << std::endl;
			#endif
		}


		void handlePointEvent()
		{
			#ifdef DEBUG_SEGMENTS
			std::cout << "event: " << m_event.point << std::endl;
			#endif

			m_sweepLine.sweep(m_event.point);

			#ifdef DEBUG_SEGMENTS
			m_sweepLine.debug();
			#endif

			u32 intersections = countIntersections();

			if (intersections + m_event.lowerEnd.size() + m_event.upperEnd.size() > 1)
				reportIntersection();

			if (!m_event.lowerEnd.empty())
			{
				removeLowerEndSegments();

				#ifdef DEBUG_SEGMENTS
				std::cout << "le rem ";
				m_sweepLine.debug();
				#endif
			}

			if (intersections + m_event.upperEnd.size() != 0)
			{
				insertInterUpper(intersections);

				#ifdef DEBUG_SEGMENTS
				std::cout << "iu rem ";
				m_sweepLine.debug();
				#endif
			}

			findNewEventPoints(intersections);

			#ifdef DEBUG_SEGMENTS
			std::cout << std::endl;
			#endif
		}

		u32 countIntersections()
		{
			auto l0 = m_sweepLine.lowerBound(m_event.point.x);
			auto l1 = m_sweepLine.upperBound(m_event.point.x);

			u32 intersections = 0u;
			if (l0 != m_sweepLine.end())
			{
				intersections += std::distance(l0, l1);
				intersections -= m_event.lowerEnd.size();
			}
			return intersections;
		}

		// NOTE : checked in handleEventPoint
		void reportIntersection()
		{
			// lowed-end segments are already inserted
			// intersecting too
			// so we add all segments from lower bound to upper bound of an intersection
			Intersection<Handle> inter{m_event.point};

			auto l0 = m_sweepLine.lowerBound(m_event.point.x);
			auto l1 = m_sweepLine.upperBound(m_event.point.x);
			while (l0 != l1)
				inter.lines.push_back(*l0++);

			for (auto& line : m_event.upperEnd)
				inter.lines.push_back(line);

			m_intersections.push_back(std::move(inter));
		}

		// NOTE : checked in handleEventPoint
		void removeLowerEndSegments()
		{
			for (auto& it : m_event.lowerEnd)
				m_sweepLine.erase(it);
		}

		void extractIntersecting()
		{
			auto l0 = m_sweepLine.lowerBound(m_event.point.x);
			auto l1 = m_sweepLine.upperBound(m_event.point.x);

			// extract and reverse
			m_extractBuffer.clear();
			while (l0 != l1)
				m_extractBuffer.push_back(m_sweepLine.extract(l0++));
			std::reverse(m_extractBuffer.begin(), m_extractBuffer.end());
		}

		void insertInterUpper(u32 intersections)
		{
			auto pred = [&] (const auto& l0, const auto& l1)
			{
				auto [l00, l01] = reorder_line(m_sampler(l0));
				auto [l10, l11] = reorder_line(m_sampler(l1));

				return turn(m_event.point, l01, l11) == Turn::Left;
			};

			std::sort(m_event.upperEnd.begin(), m_event.upperEnd.end(), pred);

			extractIntersecting();

			auto e0 = m_extractBuffer.begin();
			auto e1 = m_extractBuffer.end();
			auto u0 = m_event.upperEnd.begin();
			auto u1 = m_event.upperEnd.end();
			while(e0 != e1 && u0 != u1)
			{
				if (pred(**e0, *u0))
				{
					auto [inserted, _] = m_sweepLine.insert(std::move(*e0++));
				}
				else
				{
					auto [inserted, _] = m_sweepLine.insert(*u0++);
					insertLowerEndEvent(inserted);
				}
			}
			while (e0 != e1)
			{
				auto [inserted, _] = m_sweepLine.insert(std::move(*e0++));
			}
			while (u0 != u1)
			{
				auto [inserted, _] = m_sweepLine.insert(*u0++);
				insertLowerEndEvent(inserted);
			}
		}

		void findNewEventPoints(u32 intersections)
		{
			// TODO : check possible impossible
			// TODO : O(n^2) memory consumption removal, see https://ru.wikipedia.org/wiki/Алгоритм_Бентли_—_Оттманна

			auto beg = m_sweepLine.begin();
			auto end = m_sweepLine.end();

			auto l0 = m_sweepLine.lowerBound(m_event.point.x);
			auto l1 = m_sweepLine.upperBound(m_event.point.x);
			if (intersections + m_event.upperEnd.size() == 0) // only lower-end lines were in event
			{
				if (l0 != beg && l0 != end)
					findNewEvent(l0 - 1, l0);
			}
			else
			{
				if (l0 != beg)
				{
					if (l0 != end) // should never happen but happens
						findNewEvent(l0 - 1, l0);
				}

				if (l1 != end)
				{
					if (l1 - 1 != end) // should never happen but happens
						findNewEvent(l1 - 1, l1);
				}
			}
		}

		// NOTE : l0 preceds l1, l0 != end(), l1 != end()
		void findNewEvent(SweepLineIt l0, SweepLineIt l1)
		{			
			// TODO : check possible impossible

			Vec2 i0, i1;
			auto status = intersectSegSeg(m_sampler(*l0), m_sampler(*l1), i0, i1); 
			if (status == Status::Intersection && m_eventQueue.preceds(m_event, i0))
				insertIntersectionEvent(i0);
		}


	private:
		ExtractBuffer         m_extractBuffer;
		Intersections<Handle> m_intersections;

		PointEvent m_event;

		SweepLine  m_sweepLine;
		EventQueue m_eventQueue;
		Sampler    m_sampler;
		Float	   m_eps;
	};

	template<class handle_t, class sampler_t>
	Intersections<handle_t> section_n_lines(const std::vector<handle_t>& lines, sampler_t sampler, Float eps = default_eps)
	{
		return Sector(lines, sampler, eps).sect();
	}
}
