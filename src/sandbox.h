//
// Created by jw910731 on 2/4/21.
//

#ifndef SIMPLE_SANDBOX_ROOT_SANDBOX_H
#define SIMPLE_SANDBOX_ROOT_SANDBOX_H

#include <cstdint>
#include <cstddef>
#include <string>
#include <optional>

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
private:
    Byte *stack; //
    std::string filePath;
    std::optional<int> timeLimit, memoryLimit;
    std::optional<std::string> in, out, err;
};

#endif //SIMPLE_SANDBOX_ROOT_SANDBOX_H
