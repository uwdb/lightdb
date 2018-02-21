#ifndef LIGHTDB_FUNCTIONAL_H
#define LIGHTDB_FUNCTIONAL_H

#include <algorithm>

namespace lightdb::functional {
    template<typename T, typename InputIterator, typename UnaryFunction>
    std::vector<T> transform(InputIterator begin, InputIterator end, UnaryFunction f)
    {
        std::vector<T> values;
        values.reserve(end - begin);
        std::transform(begin, end, std::back_inserter(values), f);
        return values;
    };

    template<typename InIterator, typename OutIterator>
    void flatten(InIterator begin, InIterator end, OutIterator output)
    {
        std::for_each(begin, end, [output](auto &current) {
            std::copy(std::begin(current), std::end(current), output);
        });
    }

    template<typename InIterator, typename OutIterator, typename Function>
    OutIterator flatmap(InIterator begin, InIterator end, OutIterator output, Function f) {
        std::for_each(begin, end, [f, output](auto &current) {
            auto value = f(current);
            std::copy(std::begin(value), std::end(value), output);
        });
        return output;
    }

    template<typename OutIterator, typename InIterator, typename Function>
    OutIterator flatmap(InIterator begin, InIterator end, Function f)
    {
        OutIterator values;
        flatmap(begin, end, std::back_inserter(values), f);
        return values;
    }

    template<typename T, typename InputIterator, typename OutputIterator, typename UnaryFunction, typename Predicate>
    OutputIterator transform_if(
            InputIterator begin, const InputIterator end, OutputIterator output,
            const UnaryFunction f, const Predicate p)
    {
        for (; begin != end; ++begin)
            if(p(*begin))
                *output++ = f(*begin);
        return output;
    };

    template<typename T, typename InputIterator, typename UnaryFunction, typename Predicate>
    std::vector<T> transform_if(const InputIterator begin, const InputIterator end,
                                const UnaryFunction f, const Predicate p)
    {
        std::vector<T> values;
        values.reserve(end - begin);
        transform_if<T>(begin, end, std::back_inserter(values), f, p);
        return values;
    };
} // namespace lightdb::functional

#endif //LIGHTDB_FUNCTIONAL_H
