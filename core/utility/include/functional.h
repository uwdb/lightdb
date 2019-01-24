#ifndef LIGHTDB_FUNCTIONAL_H
#define LIGHTDB_FUNCTIONAL_H

#include <iterator>
#include <algorithm>

namespace lightdb::functional {
    template<typename T, typename Container, typename InputIterator, typename UnaryFunction>
    Container transform(InputIterator begin, InputIterator end, UnaryFunction f)
    {
        Container values;
        std::transform(begin, end, std::back_inserter(values), f);
        return values;
    }

    template<typename T, typename InputIterator, typename UnaryFunction>
    std::vector<T> transform(InputIterator begin, InputIterator end, UnaryFunction f) {
        return transform<T, std::vector<T>, InputIterator, UnaryFunction>(begin, end, f);
    }

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

    template<typename T, typename InputIterator, typename Predicate>
    std::vector<T> filter(InputIterator begin, const InputIterator end, const Predicate p)
    {
        std::vector<T> values;
        std::copy_if(begin, end, std::back_inserter(values), p);
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
    }

    template<typename T, typename InputIterator, typename UnaryFunction, typename Predicate>
    std::vector<T> transform_if(const InputIterator begin, const InputIterator end,
                                const UnaryFunction f, const Predicate p)
    {
        std::vector<T> values;
        values.reserve(std::distance(begin, end));
        transform_if<T>(begin, end, std::back_inserter(values), f, p);
        return values;
    }

    template<typename T, typename Input, typename TContainer=std::vector<T>>
    class flatmap_iterator {
    public:
        explicit flatmap_iterator(Input iterator)
                : iterator_(std::move(iterator)), container_{}, current_(std::end(container_))
        { }

        flatmap_iterator(const flatmap_iterator& other) = default;
        flatmap_iterator(flatmap_iterator&&) noexcept = default;

        void operator++() {
            if(current_ != std::end(container_))
                ++current_;
            else {
                container_.clear();
                container_ = static_cast<TContainer&>(iterator_++);
                current_ = std::begin(container_);
            }
        }
        T operator++(int) {
            auto value = **this;
            ++*this;
            return std::move(value);
        }
        T operator*() {
            if(current_ == std::end(container_))
                ++*this;
            return *current_;
        }

        size_t available() const { return std::size(container_); }

    private:
        Input iterator_;
        TContainer container_;
        typename TContainer::iterator current_;
    };

    template<typename T, typename Input, typename TContainer=std::vector<T>>
    static flatmap_iterator<T, Input, TContainer> make_flatmap_iterator(const Input &iterator) {
        return flatmap_iterator<T, Input, TContainer>(iterator);
    }

    template<typename T, typename Inputs, typename TContainer=std::vector<T>>
    class union_iterator {
    public:
        explicit union_iterator(Inputs iterators)
                : iterators_(std::move(iterators))
        { }

        union_iterator(const union_iterator&) = default;
        union_iterator(union_iterator&&) noexcept = default;

        void operator++() {
            for(auto it = std::begin(iterators_); it != std::end(iterators_); it++)
                (*it)++;
        }
        TContainer operator++(int) {
            auto values = **this;
            ++*this;
            return std::move(values);
        }
        TContainer operator*() {
            TContainer values;
            std::transform(std::begin(iterators_), std::end(iterators_),
                           std::back_inserter(values),
                           [](auto &it) { return *it; });
            return values;
        }

        size_t available() const {
            auto available = functional::transform<size_t>(std::begin(iterators_), std::end(iterators_),
                                                           [](auto &it) { return it.available(); });
            return *std::min_element(std::begin(available), std::end(available)); }

    private:
        Inputs iterators_;
    };

    template<typename T, typename Inputs, typename TContainer=std::vector<T>>
    static union_iterator<T, Inputs, TContainer> make_union_iterator(const Inputs &iterators) {
        return union_iterator<T, Inputs, TContainer>(iterators);
    }
} // namespace lightdb::functional

#endif //LIGHTDB_FUNCTIONAL_H
