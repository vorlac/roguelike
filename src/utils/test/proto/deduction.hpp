#pragma once

namespace rl::test::prototype_a {

    template <typename T, std::size_t size>
    struct optimize_ref
    {
        using t1 = typename std::remove_cvref_t<T>;
        using t2 = typename std::add_lvalue_reference_t<typename t1>;
        constexpr static bool test = sizeof(T) > size * 2;

        using type = std::conditional_t<test,
                                        std::add_lvalue_reference_t<t2>,
                                        std::remove_cvref_t<t1>>;
    };

    int test()
    {
        const optimize_ref<uint64_t, 1U>::type i1{ 'c' };
        const optimize_ref<char, 12U>::type i2{ 'c' };
    }
}

namespace rl::test::prototype_b {
    template <typename T>
    struct List
    {
        template <typename TData>
        struct Data
        {
            TData data{};
        };

        auto get_last(this auto&& self)
        {
            return self.size == 0
                     ? nullptr
                     : &self.pool[self.tail_idx].data;
        }

        size_t size{ 0 };
        size_t tail_idx{ 0 };
        std::vector<Data<T>> pool{};
    };

    int main()
    {
        const List<int> a{ 1234, 3, { { 1 }, { 2 }, { 3 }, { 4 } } };
        a.get_last();  // returns: const int*

        List<int> b{ 1234, 3, { { 1 }, { 2 }, { 3 }, { 4 } } };
        b.get_last();  // returns: int*
    }
}
