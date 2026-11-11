/// @file main.cpp
/// @brief Program entry point. For now it only verifies that the toolchain
///        works; later it wires all layers together (composition root).

#include <iostream>
#include <thread>

int main() {
    // LEARN: std::cout writes text to standard output.
    // LEARN: '\n' ends the line without explicitly flushing the output stream.
    std::cout << "IBAN Calculator 0.1.0: toolchain OK\n";

    // Display the number of hardware threads reported by the system.
    // This is a hint, not a guaranteed number of available CPU cores.
    const unsigned int hardwareThreads = std::thread::hardware_concurrency();
    std::cout << "Hardware threads: " << hardwareThreads << '\n';

    // LEARN: Returning 0 indicates successful program termination.
    return 0;
}
