/// @file ListVariants.cpp
/// @brief The list variants that are not templates. See ListVariants.h.

#include "iban/application/ListVariants.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <ostream>
#include <thread>
#include <vector>

#include "iban/domain/ForEachValid.h"
#include "iban/domain/Range.h"
#include "iban/domain/RangeSplit.h"
#include "iban/infrastructure/BoundedBuffer.h"
#include "iban/infrastructure/ForkJoin.h"

namespace iban::application {

std::int64_t listWithWriterThread(const Query& query, std::ostream& out, std::size_t capacity) {
    const std::vector<domain::Range> parts =
        domain::splitRange(domain::Range{query.begin, query.end}, query.threadCount);
    infrastructure::BoundedBuffer<std::int64_t> buffer(capacity);

    // The consumer: the only thread that numbers and writes, so the sequence
    // number needs no lock and always ascends in the order of the lines.
    std::int64_t written = 0;
    std::thread writer([&buffer, &out, &written] {
        // LEARN: pop() waits while the buffer is empty and returns std::nullopt
        // LEARN: only after close(), when every number has been written.
        while (const std::optional<std::int64_t> number = buffer.pop()) {
            ++written;
            out << written << ' ' << *number << '\n';
        }
    });

    try {
        // The producers: the p counting threads of the assignment.
        infrastructure::runInParallel(
            parts.size(), [&parts, &buffer, modulus = query.modulus](std::size_t index) {
                domain::forEachValid(parts[index], modulus, [&buffer](std::int64_t number) {
                    // push only fails after close(), which happens after the
                    // producers have finished, so the result can be ignored.
                    static_cast<void>(buffer.push(number));
                });
            });
    } catch (...) {
        // A producer thread could not be started: let the writer finish what is
        // in the buffer and join it before passing the error on.
        buffer.close();
        writer.join();
        throw;
    }
    // Every producer has been joined, so nothing more will be pushed.
    buffer.close();
    writer.join();
    // After join, the writer's last write to written is visible here.
    return written;
}

}  // namespace iban::application
