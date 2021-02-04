//
// Created by jw910731 on 2/4/21.
//

#include "sandbox.h"

#include <string>
#include <cstring>
#include <utility>

#include <sched.h>

Sandbox::Sandbox(std::string filePath):
    filePath(std::move(filePath)),
   timeLimit(std::nullopt),
   memoryLimit(std::nullopt),
   in(std::nullopt),
   out(std::nullopt),
   err(std::nullopt)
{
    stack = new Byte[STACK_SIZE];
}

void Sandbox::setStdin(std::string val) {
    *in = std::move(val);
}

void Sandbox::setStdout(std::string val) {
    *out = std::move(val);;
}

void Sandbox::setStderr(std::string val) {
    *err = std::move(val);;
}
void Sandbox::setTime(int second) {
    *timeLimit = second;
}
void Sandbox::setMemory(int byteLimit) {
    *memoryLimit = byteLimit;
}