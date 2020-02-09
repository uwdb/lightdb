#ifndef LIGHTDB_FUNCTIONAL_H
#define LIGHTDB_FUNCTIONAL_H

#include <glog/logging.h>
#include <iterator>
#include <algorithm>

namespace lightdb::functional {
    template<typename T, template<typename> typename TContainer>
    inline T& single(TContainer<T> &container) {
        const auto begin = std::begin(container);
        assert(begin + 1 == std::end(container));
        return *begin;
    }

    template<typename T, template<typename> typename TContainer>
    inline const T& single(const TContainer<T> &container) {
        const auto begin = std::cbegin(container);
        assert(begin + 1 == std::cend(container));
        return *begin;
    }

    template<typename T, template<typename> typename TContainer>
    inline T single(TContainer<T> &&container) {
        const auto begin = std::begin(container);
        assert(begin + 1 == std::end(container));
        return *begin;
    }

    template<typename T, typename Container, typename InputIterator, typename UnaryFunction>
    Container transform(const InputIterator begin, const InputIterator end, UnaryFunction f) {
        Container values;
        std::transform(begin, end, std::back_inserter(values), f);
        return values;
    }

    template<typename T, typename InputIterator, typename UnaryFunction,
            template<typename> typename OutContainer=std::vector>
    OutContainer<T> transform(const InputIterator &begin, const InputIterator &end, UnaryFunction f) {
        return transform<T, OutContainer<T>, InputIterator, UnaryFunction>(begin, end, f);
    }

    template<typename T, typename Container, typename UnaryFunction,
             template<typename> typename OutContainer=std::vector>
    OutContainer<T> transform(const Container &container, UnaryFunction f) {
        return transform<T>(std::cbegin(container), std::cend(container), f);
    }

    template<typename InIterator, typename OutIterator>
    void flatten(InIterator begin, InIterator end, OutIterator output) {
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
    OutIterator flatmap(InIterator begin, InIterator end, Function f) {
        OutIterator values;
        flatmap(begin, end, std::back_inserter(values), f);
        return values;
    }

    template<typename T, typename InputIterator, typename Predicate,
             template<typename> typename OutContainer=std::vector>
    OutContainer<T> filter(InputIterator begin, const InputIterator end, const Predicate p) {
        OutContainer<T> values;
        std::copy_if(begin, end, std::back_inserter(values), p);
        return values;
    }

    template<typename T, typename InputIterator, typename OutputIterator, typename UnaryFunction, typename Predicate>
    OutputIterator transform_if(
            InputIterator begin, const InputIterator end, OutputIterator output,
            const UnaryFunction f, const Predicate p) {
        for (; begin != end; ++begin)
            if(p(*begin))
                *output++ = f(*begin);
        return output;
    }

    template<typename T, typename InputIterator, typename UnaryFunction, typename Predicate,
             template<typename> typename OutContainer=std::vector>
    OutContainer<T> transform_if(const InputIterator begin, const InputIterator end,
                                const UnaryFunction f, const Predicate p) {
        OutContainer<T> values;
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
        const T operator++(int) {
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
        Input &iterator() { return iterator_; }

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
        const TContainer operator++(int) {
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

        const Inputs &iterators() const { return iterators_; }

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

    template<typename Element, typename Iterator>
    class downcast_iterator {
    public:
        explicit downcast_iterator(Iterator &iterator)
                : iterator_(iterator)
        { }

        constexpr explicit downcast_iterator()
                : iterator_(Iterator::eos())
        { }

        bool operator==(const downcast_iterator<Element, Iterator> &other) const { return iterator_ == other.iterator_; }
        bool operator!=(const downcast_iterator<Element, Iterator> &other) const { return !(*this == other); }

        void operator++() {
            assert(iterator_ != Iterator::eos());
            ++iterator_;
        }

        // We can't return a T for an abstract class, and ++this invalidates the reference, so
        // disable this function for abstract classes
        template <typename T=Element, typename = typename std::enable_if<!std::is_abstract<T>::value>>
        T operator++(int) {
            auto value = **this;
            ++*this;
            return std::move(value);
        }

        // This is the fallback postincrement for abstract classes -- best we can do is return the underlying iterator
        template <typename T=Element, typename = typename std::enable_if<std::is_abstract<T>::value>>
        auto operator++(typename std::enable_if<std::is_abstract<T>::value, int>::type) {
            assert(iterator_ != Iterator::eos());
            return iterator_++;
        }

        Element& operator*() { return (*iterator_).template expect_downcast<Element>(); }

        static const downcast_iterator<Element, Iterator> eos() {
            return downcast_iterator<Element, Iterator>();
        }

    private:
        Iterator &iterator_;
    };
} // namespace lightdb::functional

#endif //LIGHTDB_FUNCTIONAL_H
