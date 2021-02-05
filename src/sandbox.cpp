//
// Created by jw910731 on 2/4/21.
//

#include "sandbox.h"

#include <iostream>
#include <string>
#include <utility>
#include <cstring>

#include <sys/types.h>
#include <sys/wait.h>
#include <sched.h>
#include <unistd.h>

Sandbox::Sandbox(std::string filePath):
    filePath(std::move(filePath)){}

void Sandbox::setStdin(std::string val) {
    *in = std::move(val);
}

void Sandbox::setStdout(std::string val) {
    *out = std::move(val);
}

void Sandbox::setStderr(std::string val) {
    *err = std::move(val);
}
void Sandbox::setTime(int second) {
    *timeLimit = second;
}
void Sandbox::setMemory(int byteLimit) {
    *memoryLimit = byteLimit;
}

void Sandbox::run(std::vector<std::string> args) {
    args.emplace(args.begin(), filePath);
    // perform child process spawning
    pid_t pid = fork();
    if(pid < 0) {
        // fork failed
        perror("fork()");
        exit(-1);
    }
    if(pid == 0) {
        // child process job
        child(args);
    }
    else{
        // parent process job
        parent(pid);
    }
}

// I'm so fucking afraid any of these dangerous cast got anything wrong...
char *const* prepare_helper(const std::vector<std::string> &vec){
    char **ret = new char *[vec.size()+1];
    for(size_t i = 0 ; i < vec.size() ; ++i){
        ret[i] = const_cast<char*>(vec[i].c_str());
    }
    ret[vec.size()] = NULL;
    return const_cast<char*const*>(ret);
}

void Sandbox::child(const std::vector<std::string> &args) {
    // TODO: set resource limit
    // prepare args and env
    // 0 => program name, 1 ~ size-1 => arg, size => NULL terminator
    char *const* prepared_args = prepare_helper(args);
    // TODO: prepare proper env
    char *const* prepared_envs = prepare_helper({});
    // execute real program
    execve(filePath.c_str(), prepared_args, prepared_envs);
}

void Sandbox::parent(pid_t pid) {
    // TODO: setitimer -> SIGALRM, wall time timer
    // TODO: collect exited child proc and collect child proc resource usage
    pid_t p = waitpid(pid, NULL, 0);
    if(p < 0){
        perror("waitpid()");
        exit(-1);
    }
    std::cout << "Child Exited." << std::endl;

}