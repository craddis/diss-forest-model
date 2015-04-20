#ifndef KEITTLAB_RANDOM_HPP
#define KEITTLAB_RANDOM_HPP

#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <random>

namespace keittlab {

inline std::default_random_engine& urng()
{
  static std::default_random_engine engine;
  return engine;
}

inline std::random_device::result_type randomize()
{
  static std::random_device device;
  auto seed = device();
  urng().seed(seed);
  return seed;
}

inline int pick_a_number(int from, int thru)
{
  assert(from <= thru);
  static std::uniform_int_distribution<> z;
  return z(urng(), decltype(z)::param_type{from, thru});
}

inline double pick_a_number(double from, double upto)
{
  assert(from <= upto && upto - from < std::numeric_limits<double>::max());
  static std::uniform_real_distribution<> z;
  return z(urng(), decltype(z)::param_type{from, upto});
}

template<typename T>
inline T& one_of(T& x, T& y, double px = 0.5)
{
  assert(0. <= px && px <= 1.);
  static std::bernoulli_distribution z;
  return z(urng(), typename decltype(z)::param_type{px}) ? x : y;
}

template<typename T>
inline T const& one_of(T const& x, T const& y, double px = 0.5)
{
  assert(0. <= px && px <= 1.);
  static std::bernoulli_distribution z;
  return z(urng(), typename decltype(z)::param_type{px}) ? x : y;
}

inline bool maybe(double p = 0.5)
{ return one_of(true, false, p); }

template<typename Forward>
inline Forward one_of_range(Forward begin, Forward end)
{
  if (begin == end) return end;
  return std::next(begin, pick_a_number(0, std::distance(begin, end) - 1));
}

template<typename Container>
inline auto one_of(Container& c) -> decltype(begin(c))
{ return one_of_range(begin(c), end(c)); }

template<typename Container>
inline auto one_of(Container const& c) -> decltype(begin(c))
{ return one_of_range(begin(c), end(c)); }

template<typename T, std::size_t N>
inline T& one_of(T (&v)[N])
{ return *one_of_range(std::begin(v), std::end(v)); }

template<typename T>
inline T one_of(std::initializer_list<T> z)
{ return *one_of_range(begin(z), end(z)); }

} // namespace keittlab

#endif // KEITTLAB_RANDOM_HPP

