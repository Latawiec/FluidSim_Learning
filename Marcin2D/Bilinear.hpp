#pragma once

namespace Marcin2D
{

template<class Indexer>
struct Bilinear {
private:
    using value_type = typename std::decay<decltype(std::declval<Indexer>().operator()(0, 0))>::type;
    Indexer _indexer;

public:

    Bilinear(Indexer&& indexer)
    : _indexer(std::forward<Indexer>(indexer))
    {}

    constexpr value_type operator()(const float x, const float y) const {
        int fx = static_cast<int>(floor(x));
        int fy = static_cast<int>(floor(y));

        // vertical
        const float dy = y - fy;

        const auto& lt = _indexer(fx, fy);
        const auto& lb = _indexer(fx, fy + 1);
        const auto left_linear = lt * (1.f - dy) + lb * dy;

        const auto& rt = _indexer(fx + 1, fy);
        const auto& rb = _indexer(fx + 1, fy + 1);
        const auto right_linear = rt * (1.f - dy) + rb * dy;

        // horizontal
        const float dx = x - fx;
        const auto bi_linear = left_linear * (1.f - dx) + right_linear * dx;

        return bi_linear;
    }
    
    constexpr int width() const { return _indexer.width(); }
    constexpr int height() const { return _indexer.height(); }
};

template<class Indexer>
Bilinear<Indexer> make_bilinear(Indexer&& indexer) {
    return Bilinear<Indexer>(std::forward<Indexer>(indexer));
}

}