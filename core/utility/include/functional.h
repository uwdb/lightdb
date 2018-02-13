#ifndef LIGHTDB_FUNCTIONAL_H
#define LIGHTDB_FUNCTIONAL_H

#include <algorithm>

namespace lightdb::functional {
    template<typename T, typename InputIterator, typename UnaryFunction>
    std::vector<T> transform(InputIterator begin, InputIterator end, UnaryFunction f)
    {
        std::vector<T> values;
        std::transform(begin, end, values.begin(), f);
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
        flatmap(begin, end, values.begin(), f);
        return values;
    }

    template<typename T, typename InputIterator, typename OutputIterator, typename UnaryFunction, typename Predicate>
    OutputIterator transform_if(
            InputIterator begin, InputIterator end, OutputIterator output,
            UnaryFunction f, Predicate p)
    {
        for (; begin != end; ++begin)
            if(p(*begin))
                *++output = f(*begin);
        return output;

/*
        std::vector<T> intermediate;
        std::transform(begin, end, intermediate, f);
        return std::copy_if(intermediate.begin(), intermediate.end(), output, p);*/
    };

    template<typename T, typename InputIterator, typename UnaryFunction, typename Predicate>
    std::vector<T> transform_if(InputIterator begin, InputIterator end, UnaryFunction f, Predicate p)
    {
        std::vector<T> values;
        transform_if<T>(begin, end, values.begin(), f, p);
        return values;
    };
} // namespace lightdb::functional

#endif //LIGHTDB_FUNCTIONAL_H
