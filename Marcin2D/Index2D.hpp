#pragma once
#include <iterator>
#include <type_traits>
#include <utility>
#include <cassert>

namespace Marcin2D
{

template<class Iterator>
class Index2D {
    static_assert(std::is_same<typename std::iterator_traits<Iterator>::iterator_category, std::random_access_iterator_tag>::value,
                  "Iterator has to be random access iterator.");
    Iterator _begin, _end;
    const int _width, _height;

public:
    using reference = typename std::iterator_traits<Iterator>::reference;
    using const_reference = const typename std::remove_reference<reference>::type &;

    constexpr Index2D(Iterator begin, Iterator end, const int width, const int height) :
        _begin(begin),
        _end(end),
        _width(width),
        _height(height)
    {
        assert((_end - _begin >= _width * _height && "Passed range is smaller than passed width and height."));
    }

    constexpr reference operator()(const int x, const int y) {
        assert((x >= 0 && x < _width && y >= 0 && y < _height && "Passed coordinates are outside of valid range."));
        return *(_begin + y * _width + x);
    }

    constexpr const_reference operator()(const int x, const int y) const {
        assert((x >= 0 && x < _width && y >= 0 && y < _height && "Passed coordinates are outside of valid range."));
        return *(_begin + y * _width + x);
    }

    constexpr int width() const { return _width; }
    constexpr int height() const { return _height; }
};

template<class Iterator>
Index2D<Iterator> make_index2d(Iterator begin, Iterator end, int width, int height) {
    return Index2D<Iterator>(begin, end, width, height);
}


}