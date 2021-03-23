//#include <iostream>
//#include <vector>
//#include <utility>
//#include <algorithm>
//#include <cstdint>
//#include <cstdlib>
//#include <memory>
//#include <type_traits>
//#include <optional>
//#include <chrono>
//#include <random>
//
//#include <glm/common.hpp>
//#include <glm/glm.hpp>
//#include <glm/ext.hpp>
//
//#include "PoolAllocator.h"

//struct alignas(128) Test
//{
//	int a;
//	double b;
//	char c[10];
//};
//
//void testAllocator()
//{
//	PoolAllocator<Test> pool(1 << 13);
//	std::vector<Test*> allocated(1'000'000);
//	for(int i = 0; i < 1'000'000; i++)
//	{
//		auto ptr = pool.allocate();
//		allocated[i] = ptr;
//	}
//	for(auto ptr : allocated)
//	{
//		pool.deallocate(ptr);
//	}
//}
//
//
//// list
//template<class ... T> 
//struct type_list
//{};
//
//// pair
//template<class T, class U>
//struct pair
//{};
//
//
//// pop back
//template<class ... T>
//struct pop_back;
//
//template<class ... T, class U>
//struct pop_back<type_list<T..., U>>
//{
//	using result = type_list<T...>;
//};
//
//template<class ... T>
//using pop_back_t = typename pop_back<T...>::result;
//
//
//// pop front
//template<class ... T>
//struct pop_front;
//
//template<class T, class ... U>
//struct pop_front<type_list<T, U...>>
//{
//	using result = type_list<U...>;
//};
//
//template<class ... T>
//using pop_front_t = typename pop_front<T...>::result;
//
//
//// push back
//template<class ... T>
//struct push_back;
//
//template<class ... T, class U>
//struct push_back<type_list<T...>, U>
//{
//	using result = type_list<T..., U>;
//};
//
//template<class ... T>
//using push_back_t = typename push_back<T...>::result;
//
//
//// concat
//template<class ... T>
//struct concat;
//
//template<class ... T, class ... U>
//struct concat<type_list<T...>, type_list<U...>>
//{
//	using result = type_list<T..., U...>;
//};
//
//template<class ... T, class U>
//struct concat<type_list<T...>, U>
//{
//	using result = type_list<T..., U>;
//};
//
//template<class T, class ... U>
//struct concat<T, type_list<U...>>
//{
//	using result = type_list<T, U...>;
//};
//
//template<class ... T>
//using concat_t = typename concat<T...>::result;
//
//
//// expand type n times
//template<class T, size_t N>
//struct n_times
//{
//	using prev_t = typename n_times<T, N - 1>::result;
//	using result = concat_t<prev_t, T>;
//};
//
//template<class T>
//struct n_times<T, 0>
//{
//	using result = type_list<>;
//};
//
//template<class T, size_t N>
//using n_times_t = typename n_times<T, N>::result;
//
//
//// zip
//template<class ... T>
//struct zip;
//
//template<class HT, class ... T, class HU, class ... U>
//struct zip<type_list<HT, T...>, type_list<HU, U...>>
//{
//	using prev_t = typename zip<type_list<T...>, type_list<U...>>::result;
//	using result = concat<pair<HT, HU>, prev_t>;
//};
//
//template<class ... T>
//struct zip<type_list<T...>, type_list<>>
//{
//	using result = type_list<>;
//};
//
//template<class ... T>
//struct zip<type_list<>, type_list<T...>>
//{
//	using result = type_list<>;
//};
//
//template<class ... T>
//using zip_t = typename zip<T...>::result;
//
//
//// I should really stop, I dont't need it yet
//// slice [b : e : s] 
//template<class T, size_t b, size_t e, size_t s>
//struct slice;
//
//template<class ... T, size_t b, size_t e, size_t s>
//struct slice<type_list<T...>, b, e, s>
//{
//
//};
//
//
//// sequence
//template<size_t ... Indices>
//struct sequence
//{
//	static constexpr size_t size = sizeof(Indices);
//};
//
//
//
//// sequence : t0, t1, t2, ... , tn
//// dti != dtj
//// goal : ti -> t'i so dt'i = dt'j
//template<class Float>
//class CubicSpline
//{
//public:
//    class SplineNode
//    {
//    public:
//        SplineNode(Float a = {}, Float b = {}, Float c = {}, Float d = {}, Float left = {}, Float right = {})
//            : m_a{a}, m_b{b}, m_c{c}, m_d{d}, m_left{left}, m_right{right}
//        {}
//
//        SplineNode(const SplineNode&) = default;
//        SplineNode(SplineNode&&)      = default;
//
//        ~SplineNode() = default;
//
//        SplineNode& operator = (const SplineNode&) = default;
//        SplineNode& operator = (SplineNode&&)      = default;
//
//
//    public:
//        Float computeValue(Float arg)
//        {
//            Float dlv = m_left - arg;
//            Float drv = m_right - arg;
//
//            return m_a * drv * drv * drv - m_b * dlv * dlv * dlv - m_c * dlv + m_d * drv;
//        }
//
//        Float computeDerivative(Float arg)
//        {
//            Float dlv = m_left - arg;
//            Float drv = m_right - arg;
//
//            return  -3 * m_a * drv * drv + 3 * m_b * dlv * dlv + m_c - m_d;
//        }
//
//
//    public:
//        Float getA() const
//        {
//            return m_a;
//        }
//
//        Float getB() const
//        {
//            return m_b;
//        }
//
//        Float getC() const
//        {
//            return m_c;
//        }
//
//        Float getD() const
//        {
//            return m_d;
//        }
//
//        Float getLeft() const
//        {
//            return m_left;
//        }
//
//        Float getRight() const
//        {
//            return m_right;
//        }
//
//
//        void setA(Float a)
//        {
//            m_a = a;
//        }
//
//        void setB(Float b)
//        {
//            m_b = b;
//        }
//
//        void setC(Float c)
//        {
//            m_c = c;
//        }
//
//        void setD(Float d)
//        {
//            m_d = d;
//        }
//
//        void setLeft(Float left)
//        {
//            m_left = left;
//        }
//
//        void setRight(Float right)
//        {
//            m_right = right;
//        }
//
//
//    private:
//        Float m_a{};
//        Float m_b{};
//        Float m_c{};
//        Float m_d{};
//
//        Float m_left{};
//        Float m_right{};
//    };
//
//
//private:
//    struct SystemLine
//    {
//        Float a{};
//        Float b{};
//        Float c{};
//        Float d{};
//    };
//
//
//public:
//    CubicSpline(const std::vector<Float>& args, const std::vector<Float>& values, Float firstMoment = static_cast<Float>(0), Float lastMoment = static_cast<Float>(0))
//    {
//        const int count   = std::min(args.size(), values.size());
//        const int last    = (int)count - 1;
//        const int prelast = (int)count - 2;
//
//        // initialize systme
//        std::vector<SystemLine> moments(count);
//
//        // boundary condition
//        moments[0].b = static_cast<Float>(1);    
//        moments[0].d = firstMoment; 
//
//        moments.back().b = static_cast<Float>(1);
//        moments.back().d = lastMoment; 
//
//        // initializing system
//        for (int i = 1; i < last; i++)
//        {
//            auto hCurr = args[  i  ] - args[i - 1];
//            auto hNext = args[i + 1] - args[  i  ];
//
//            moments[i].a = hCurr / 6;
//            moments[i].b = (hNext + hCurr) / 3;
//            moments[i].c = hNext / 6;
//            moments[i].d = (values[i + 1] - values[i]) / hNext - (values[i] - values[i - 1]) / hCurr;
//        }
//
//        // finding solution of a linear system
//        // forward pass
//        for (int i = 0; i < last; i++)
//        {
//            auto b = moments[i].b;
//            moments[i].b = static_cast<Float>(1);
//            moments[i].c /= b;
//            moments[i].d /= b;
//
//            auto a = moments[i + 1].a;
//            moments[i + 1].a = static_cast<Float>(0);
//            moments[i + 1].b -= moments[i].c * a;
//            moments[i + 1].d -= moments[i].d * a;
//        }
//        { //last
//            auto b = moments.back().b;
//            moments.back().b = static_cast<Float>(1);
//            moments.back().d /= b;
//        }
//
//        // backward pass
//        for (int i = last; i > 1; i--)
//        {
//            auto c = moments[i - 1].c;
//            moments[i - 1].c = static_cast<Float>(0);
//            moments[i - 1].d -= moments[i].d * c;
//        }
//
//        //alloc
//        m_splineNodes.resize(count - 1);
//
//        //computing spline polynomial coefs
//        for (int i = 0; i < count - 1; i++)
//        {
//            auto mCurr = moments[  i  ].d;
//            auto mNext = moments[i + 1].d;
//            auto h = args[i + 1] - args[  i  ];
//
//            auto a = mCurr / 6 / h;
//            auto b = mNext / 6 / h;
//            auto c = (values[i + 1] - mNext * h * h / 6) / h;
//            auto d = (values[  i  ] - mCurr * h * h / 6) / h;
//
//            auto left  = args[  i  ];
//            auto right = args[i + 1];
//
//            m_splineNodes[i] = SplineNode(a, b, c, d, left, right);
//        }
//
//        m_left  = args.front();
//        m_right = args.back();
//        m_valueLeft  = values.front();
//        m_valueRight = values.back();
//    }
//
//
//public:
//    Float operator() (Float arg)
//    {
//        if (arg <= m_left)
//        {
//            return m_valueLeft;
//        }
//        if (arg >= m_right)
//        {
//            return m_valueRight;
//        }
//
//        auto it = std::lower_bound(m_splineNodes.begin(), m_splineNodes.end(), arg,
//            [] (auto& splineNode, auto& arg)
//            {
//                return splineNode.getRight() < arg;
//            }
//        );
//
//        return it->computeValue(arg);
//    }
//
//
//private:
//    std::vector<SplineNode> m_splineNodes{};
//
//    Float m_left{};
//    Float m_right{};
//    Float m_valueLeft{};
//    Float m_valueRight{};
//};
//
//
//template<class T>
//void dummy(T a)
//{
//    const T& b = a;
//}