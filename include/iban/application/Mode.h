/// @file Mode.h
/// @brief The operating mode u from the assignment: count, list or search.
// LEARN: One type per file: the file name tells where to find it, and code that
// LEARN: only needs the mode does not have to include Query or Limits.

#pragma once

namespace iban::application {

/// Operating mode u. The values match the input, so the parser can convert
/// a validated number directly.
// LEARN: enum class instead of plain constants: Mode::List cannot be mixed up
// LEARN: with an int, and the compiler can warn when a switch misses a mode.
enum class Mode {
    Count = 0,   ///< u = 0: count the valid numbers
    List = 1,    ///< u = 1: print every valid number with a sequence number
    Search = 2,  ///< u = 2: search for s and stop the other threads early
};

}  // namespace iban::application
