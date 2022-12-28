//#define FMT_HEADER_ONLY  //for printing out
#include "fmt/core.h" // print with fmt
#include <algorithm>
#include <array>
#include <random>
#include <ranges>
#include <tuple>

template <typename Tn, class...Types>
std::pair<Tn, Tn> compareTupleNums(const std::tuple<Types...> &&tuple_of_n)
{
    auto low = std::get<0>(tuple_of_n);
    auto high = low;
    #if DEBUG_BUILD
    size_t comma = 0;
    const size_t size_of = sizeof...(Types);
    int step_ = 10;
    #endif
    std::apply(
        [&](const auto &...Args) {
            auto compare = [&](const auto &n) {
                #if DEBUG_BUILD
                fmt::print("{}", n);
                #endif
                low = (low > n) ? n : low;
                high = (high < n) ? n : high;
                #if DEBUG_BUILD
                if (comma != size_of - 1) fmt::print(", \t");
                if ((comma + 1) % step_ == 0 && comma != 0) fmt::print("\n");
                comma++;
                #endif
            };
            (compare(Args), ...);
            #if DEBUG_BUILD
            fmt::print("\n");
            #endif
        },
        tuple_of_n
    );
    return std::make_pair(low, high);
}

template<typename Tn, class...Types>
std::pair<Tn, Tn> compareTupleNums(const Types&&...numbers)
{
    const std::tuple tuple_of_n = std::forward_as_tuple(numbers...); // tuple of references for forwarding args to a function
    return compareTupleNums<Tn>(tuple_of_n);
}
 <std::size_t N, typename T, std::size_t... Indices>
auto arrayToTupleHelper(const std::array<T,N>& v, std::index_sequence<Indices...>) {
  return std::forward_as_tuple(v[Indices]...);
}

template <std::size_t N, typename T>
auto arrayToTuple(const std::array<T,N>&& v) {
  return arrayToTupleHelper(v, std::make_index_sequence<N>());
}

int main()
{
    static const int dist_range = 1000; //integers between 1 - 1000
    static const int n_count = 100;     //randomly generate this many numbers
    fmt::print("Generated {} random integers: \n", n_count);
    static const auto dice = [](){
        static std::uniform_int_distribution<int> distr{1, dist_range};
        static std::random_device device;
        static std::mt19937 engine{device()};
        return distr(engine);
    };
    std::array<int,n_count> rng_pool;//initialize array to hold %n_count% numbers
    std::ranges::generate(rng_pool.begin(), rng_pool.end(), dice);//roll dice
    const std::tuple tuple_from_array = arrayToTuple<n_count>(std::move(rng_pool));

    const auto[min, max] = compareTupleNums<int>(std::move(tuple_from_array));
    fmt::print("Min: {} Max: {} \n", min, max);

    return 0;
}
