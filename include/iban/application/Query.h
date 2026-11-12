/// @file Query.h
/// @brief What the program must compute: a validated request built from the
///        input b e m p u s. Every Query that leaves the parser is valid, so
///        the rest of the program does not repeat the checks.
// LEARN: The application layer describes the use cases (count, list, search).
// LEARN: It may use the domain layer, but the domain never uses this layer.

#pragma once

#include <cstdint>
#include <optional>

#include "iban/application/Limits.h"
#include "iban/application/Mode.h"

namespace iban::application {

/// A validated request. Field names are written out; the comments give the
/// letter from the assignment.
// LEARN: A struct with public fields: it only carries data and has no rules of
// LEARN: its own, because the parser guarantees validity when it creates one.
struct Query {
    std::int64_t begin = kMinBound;  ///< b: inclusive lower bound of the range
    std::int64_t end = kMinBound;    ///< e: exclusive upper bound of the range
    int modulus = kMinModulus;       ///< m: the modulus of the m-check
    int threadCount = kMinThreads;   ///< p: number of threads to use
    Mode mode = Mode::Count;         ///< u: operating mode
    /// s: search parameter. Present when it was given in the input; always
    /// present in search mode.
    // LEARN: std::optional says in the type that s may be missing, instead of
    // LEARN: a magic value such as -1 or 0 that could also be real input.
    std::optional<std::int64_t> target;
};
// LEARN: Defaults come from Limits.h, so a Query is never uninitialised and
// LEARN: m and p already hold allowed values. An empty range (b == e) is not a
// LEARN: valid request; only the parser creates valid ones.

}  // namespace iban::application
