/*MIT License

Copyright (c) 2022 Jason Turner

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <cstddef>
#include <functional>
#include <optional>
#include <utility>
#include <array>
#include <limits>
#include <map>
// because these are macros we cannot meaningfully use namespaces

// https://stackoverflow.com/questions/57137351/line-is-not-constexpr-in-msvc
// https://www.chiark.greenend.org.uk/~sgtatham/coroutines.html
#define LAMBDA_CO_CONCATENATE_(a, b) a##b
#define LAMBDA_CO_CONCATENATE(a, b) LAMBDA_CO_CONCATENATE_(a, b)
#define LAMBDA_CO_CONSTEXPR_LINE long(LAMBDA_CO_CONCATENATE(__LINE__, U))


namespace lambda_coroutines {
// Internal tool for checking lambda coroutine requirements
template<typename ParamType> constexpr void lambda_co_status_check(ParamType &value)
{
  static_assert(std::is_integral_v<ParamType>, "integral state value required");
  static_assert(!std::is_const_v<ParamType>, "mutable lambda required");
  static_assert(sizeof(value) >= 4, "state value must be at least 32 bits");
}

#define lambda_co_begin(state_variable)                                                                      \
  lambda_coroutines::lambda_co_status_check(state_variable);                              \
  switch (auto &coroutine_lambda_internal_state_ref = state_variable; coroutine_lambda_internal_state_ref) { \
  default:                                                                                                   \
  case 0:

#define lambda_co_return(return_value) \
  }                                    \
  return return_value

#define lambda_co_yield(...)                                      \
  coroutine_lambda_internal_state_ref = LAMBDA_CO_CONSTEXPR_LINE; \
  return __VA_ARGS__;                                             \
  case LAMBDA_CO_CONSTEXPR_LINE:


#define lambda_co_end() }

template<typename Lambda>
[[nodiscard]] constexpr auto
  range(Lambda lambda, std::size_t skip_ = 0, std::optional<std::size_t> length_ = {}, std::size_t stride_ = 1)
{
  struct Range
  {
    Lambda generator;
    std::optional<std::size_t> length;
    std::size_t stride;
    std::size_t position = 0;

    struct Iterator
    {
      std::reference_wrapper<Lambda> lambda;
      std::reference_wrapper<std::size_t> position;
      std::optional<std::size_t> length;
      std::size_t stride = 1;
      bool is_end = false;

      [[nodiscard]] constexpr bool has_more() const noexcept
      {
        if (length && (position.get() >= length.value())) { return false; }

        return true;
      }

      constexpr bool increment_position()
      {
        if (has_more()) {
          ++position.get();
          return true;
        } else {
          return false;
        }
      }

      constexpr auto &operator++()
      {
        for (std::size_t i = 1; i < stride; ++i) {
          if (increment_position()) { lambda(); }
        }
        return *this;
      }

      constexpr bool operator==(const Iterator &rhs) const
      {
        // return false;
        if (is_end && !rhs.has_more()) { return true; }
        if (rhs.is_end && !has_more()) { return true; }
        if (rhs.is_end && is_end) { return true; }

        return false;
      }

      constexpr bool operator!=(const Iterator &rhs) const { return !(*this == rhs); }

      [[nodiscard]] constexpr decltype(auto) operator*()
      {
        increment_position();
        return lambda();
      }
    };

    // cppcheck-suppress functionConst
    [[nodiscard]] constexpr auto begin() noexcept { return Iterator{ generator, position, length, stride, false }; }

    // cppcheck-suppress functionConst
    [[nodiscard]] constexpr auto end() noexcept { return Iterator{ generator, position, length, stride, true }; }
  };

  for (std::size_t i = 0; i < skip_; ++i) { lambda(); }
  return Range{ std::move(lambda), length_, stride_ };
}

template<typename Lambda> [[nodiscard]] constexpr auto while_has_value(Lambda lambda)
{
  struct Range
  {
    Lambda generator;

    struct Iterator
    {
      std::reference_wrapper<Lambda> lambda;
      std::invoke_result_t<Lambda> current_value{};

      Iterator(Lambda &lambda_, bool is_end_)
        : lambda(lambda_), current_value([&]() { return is_end_ ? decltype(current_value){} : lambda(); }())
      {}

      constexpr auto &operator++()
      {
        current_value = lambda();
        return *this;
      }

      constexpr bool operator==(const Iterator &rhs) const
      {
        if (!current_value && !rhs.current_value) { return true; }

        return false;
      }

      constexpr bool operator!=(const Iterator &rhs) const { return !(*this == rhs); }

      [[nodiscard]] constexpr decltype(auto) operator*() { return current_value.value(); }
    };

    // cppcheck-suppress functionConst
    [[nodiscard]] constexpr auto begin() noexcept { return Iterator{ generator, false }; }

    // cppcheck-suppress functionConst
    [[nodiscard]] constexpr auto end() noexcept { return Iterator{ generator, true }; }
  };

  return Range{ std::move(lambda) };
}

}// namespace lambda_coroutines
