//
// Created by jw910731 on 2/4/21.
//

#ifndef SIMPLE_SANDBOX_ROOT_SANDBOX_H
#define SIMPLE_SANDBOX_ROOT_SANDBOX_H

#include <cstdint>
#include <cstddef>
#include <string>
#include <optional>
#include <vector>

class Sandbox{
public:
    using Byte = uint8_t;
    static constexpr size_t STACK_SIZE = 1024*8192;
    Sandbox(std::string filePath);
    void setTime(int second);
    void setMemory(int byteLimit);
    void setStdin(std::string val);
    void setStdout(std::string val);
    void setStderr(std::string val);
    void run(const std::vector<std::string> &args);
private:
    std::string filePath;
    std::optional<int> timeLimit, memoryLimit;
    std::optional<std::string> in, out, err;
    void child(const std::vector<std::string> &args);
    void parent(pid_t);
    // child only method
    void setupFd();
};

#endif //SIMPLE_SANDBOX_ROOT_SANDBOX_H
