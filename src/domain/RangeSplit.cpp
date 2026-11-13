/// @file RangeSplit.cpp
/// @brief Implementation of the fair range split. See RangeSplit.h.

#include "iban/domain/RangeSplit.h"

#include <cstddef>
#include <cstdint>
#include <stdexcept>

namespace iban::domain {

std::vector<Range> splitRange(Range range, int parts) {
    if (parts < 1) {
        throw std::invalid_argument("parts must be at least 1");
    }
    if (range.end < range.begin) {
        throw std::invalid_argument("range end must not be before its begin");
    }

    // n = q * p + r with 0 <= r < p: every part gets q values, the first r get
    // one extra. The running begin gives the same result as the closed form
    // begin(i) = b + i * q + min(i, r).
    // LEARN: Example: n = 10, p = 4 gives q = 2, r = 2, so sizes 3, 3, 2, 2.
    const std::int64_t count = parts;
    const std::int64_t base = range.size() / count;       // q
    const std::int64_t remainder = range.size() % count;  // r

    std::vector<Range> result;
    // LEARN: reserve allocates memory for all parts at once, instead of growing
    // LEARN: the vector step by step while push_back adds parts.
    result.reserve(static_cast<std::size_t>(parts));
    std::int64_t begin = range.begin;
    for (std::int64_t index = 0; index < count; ++index) {
        const std::int64_t size = base + (index < remainder ? 1 : 0);
        result.push_back(Range{begin, begin + size});
        begin += size;  // the next part starts where this one ends: no gap, no overlap
    }
    return result;
}

}  // namespace iban::domain
