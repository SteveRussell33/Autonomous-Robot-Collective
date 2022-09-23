#pragma once

#include <array>

namespace arc {
namespace dsp {

//--------------------------------------------------------------
// https://joelfilho.com/blog/2020/compile_time_lookup_tables_in_cpp/
//
// Implementation of the LUT, using a recursive function
// We use SFINAE to end recursion

// The final iteration: we construct the array
template <std::size_t Length, typename T, typename Generator, typename... Values>
constexpr auto lut_impl(Generator&& f, Values... values) ->
    typename std::enable_if<sizeof...(Values) == (Length - 1), std::array<T, Length>>::type {
    return {values..., std::forward<Generator>(f)(sizeof...(Values))};
}

// All other iterations: we append the values to
template <std::size_t Length, typename T, typename Generator, typename... Values>
constexpr auto lut_impl(Generator&& f, Values... values) ->
    typename std::enable_if<sizeof...(Values) < (Length - 1), std::array<T, Length>>::type {
    return lut_impl<Length, T, Generator, Values..., decltype(f(std::size_t{0}))>(
        std::forward<Generator>(f), values..., f(sizeof...(Values)));
}

// Our lookup table function
//  - Length: the size of our LUT
//  - Generator: the functor that we'll call to generate the LUT on each index
//  - As we're using indexes, we don't need to call std::declval and can just declare a value of
//  size_t
template <std::size_t Length, typename Generator>
constexpr auto LUT(Generator&& f) -> std::array<decltype(f(std::size_t{0})), Length> {
    // We call the implementation
    return lut_impl<
        Length,                     // The size
        decltype(f(std::size_t{0})) // The return type
        >(std::forward<Generator>(f));
}

} // namespace dsp
} // namespace arc
