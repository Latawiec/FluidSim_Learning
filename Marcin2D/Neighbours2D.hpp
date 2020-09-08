#pragma once

namespace Marcin2D
{

template<class Indexer>
struct Neighbours2D {
private:
    using value_type = decltype(std::declval<Indexer>().operator()(0, 0));
    using const_value_type = decltype(std::declval<const Indexer>().operator()(0, 0));
    Indexer _indexer;

public:
    template<class ValueType>
    struct Neighbourhood {
        ValueType top, left, mid, right, bot;
    };

    Neighbours2D(Indexer&& indexer)
    : _indexer(std::forward<Indexer>(indexer))
    {}

    constexpr Neighbourhood<value_type> operator()(const int x, const int y) {
        return {
                _indexer(x,   y-1),
                _indexer(x-1, y),
                _indexer(x,   y),
                _indexer(x+1, y),
                _indexer(x,   y+1)
        };
    }

    constexpr Neighbourhood<const_value_type> operator()(const int x, const int y) const {
        return {
            _indexer(x,   y-1),
            _indexer(x-1, y),
            _indexer(x,   y),
            _indexer(x+1, y),
            _indexer(x,   y+1)
        };
    }
    
    constexpr int width() const { return _indexer.width(); }
    constexpr int height() const { return _indexer.height(); }
};

template<class Indexer>
Neighbours2D<Indexer> make_neighbours2d(Indexer&& indexer) {
    return Neighbours2D<Indexer>(std::forward<Indexer>(indexer));
}

}