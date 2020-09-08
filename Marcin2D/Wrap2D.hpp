#pragma once
#include "Index2D.hpp"

namespace Marcin2D
{

enum class WrapMode : std::uint8_t {
    Wrap,
    Clamp,
    Mirror
};

template<class Indexer, WrapMode WrapS, WrapMode WrapT = WrapS>
class Wrap2D {
private:
    using value_type = decltype(std::declval<Indexer>().operator()(0, 0));
    using const_value_type = decltype(std::declval<const Indexer>().operator()(0, 0));
    Indexer _indexer;

public:
    constexpr Wrap2D(Indexer&& indexer) :
        _indexer(std::forward<Indexer>(indexer))
    {}

    constexpr value_type operator()(const int x, const int y) {
        return _indexer(
            wrapCoordinate<WrapS>(x, this->width()),
            wrapCoordinate<WrapT>(y, this->height())
        );
    }

    constexpr const_value_type operator()(const int x, const int y) const {
        return _indexer(
            wrapCoordinate<WrapS>(x, this->width()),
            wrapCoordinate<WrapT>(y, this->height())
        );
    }

    constexpr int width() const { return _indexer.width(); }
    constexpr int height() const { return _indexer.height(); }

private:
    template<WrapMode Mode>
    constexpr int wrapCoordinate(int coordinate, const int limit) const {
        switch (Mode) {
            case WrapMode::Wrap: 
                if (coordinate < 0) coordinate = coordinate % limit + limit;
                if (coordinate >= limit) coordinate %= limit;
                return coordinate;
                break;
            case WrapMode::Clamp:
                if (coordinate < 0) return 0;
                if (coordinate >= limit) return limit - 1;
                return coordinate;
                break;
            case WrapMode::Mirror:
                if (coordinate < 0) coordinate = std::abs(coordinate);
                if (coordinate >= limit) coordinate = coordinate - limit * (coordinate / limit);
                return coordinate;
                break;
        }
    }
};

template<WrapMode WrapS, WrapMode WrapT = WrapS, class Indexer>
Wrap2D<Indexer, WrapS, WrapT> make_wrap2d(Indexer&& indexer) {
    return Wrap2D<Indexer, WrapS, WrapT>(std::forward<Indexer>(indexer));
}

}