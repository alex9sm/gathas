#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <mutex>

class ConsoleCapture {
public:
    static ConsoleCapture& getInstance() {
        static ConsoleCapture instance;
        return instance;
    }

    void startCapture() {
        oldCoutBuf = std::cout.rdbuf(coutBuffer.rdbuf());
        oldCerrBuf = std::cerr.rdbuf(cerrBuffer.rdbuf());
    }

    void stopCapture() {
        if (oldCoutBuf) std::cout.rdbuf(oldCoutBuf);
        if (oldCerrBuf) std::cerr.rdbuf(oldCerrBuf);
    }

    void update() {
        std::lock_guard<std::mutex> lock(mutex);

        std::string coutStr = coutBuffer.str();
        if (!coutStr.empty()) {
            lines.push_back(coutStr);
            coutBuffer.str("");
            coutBuffer.clear();
        }

        std::string cerrStr = cerrBuffer.str();
        if (!cerrStr.empty()) {
            lines.push_back("[ERROR] " + cerrStr);
            cerrBuffer.str("");
            cerrBuffer.clear();
        }

        if (lines.size() > maxLines) {
            lines.erase(lines.begin(), lines.begin() + (lines.size() - maxLines));
        }
    }

    const std::vector<std::string>& getLines() const {
        return lines;
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mutex);
        lines.clear();
    }

private:
    ConsoleCapture() = default;
    ~ConsoleCapture() { stopCapture(); }

    std::stringstream coutBuffer;
    std::stringstream cerrBuffer;
    std::streambuf* oldCoutBuf = nullptr;
    std::streambuf* oldCerrBuf = nullptr;
    std::vector<std::string> lines;
    std::mutex mutex;
    const size_t maxLines = 1000;
};