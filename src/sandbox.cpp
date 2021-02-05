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
#include <sys/stat.h>
#include <sys/resource.h>
#include <sched.h>
#include <unistd.h>
#include <fcntl.h>

Sandbox::Sandbox(std::string filePath):
    filePath(std::move(filePath)){}

void Sandbox::setStdin(std::string val) {
    in = std::move(val);
}

void Sandbox::setStdout(std::string val) {
    out = std::move(val);
}

void Sandbox::setStderr(std::string val) {
    err = std::move(val);
}
void Sandbox::setTime(unsigned long second) {
    timeLimit = second;
}
void Sandbox::setMemory(unsigned long byteLimit) {
    memoryLimit = byteLimit;
}
void Sandbox::setFileSize(unsigned long byteLimit) {
    fileSizeLimit = byteLimit;
}

void Sandbox::run(const std::vector<std::string> &args) {
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
static char *const* prepare_helper(const std::vector<std::string> &vec) {
    char **ret = new char *[vec.size()+1];
    for(size_t i = 0 ; i < vec.size() ; ++i){
        ret[i] = const_cast<char*>(vec[i].c_str());
    }
    // add NULL terminator
    ret[vec.size()] = NULL;
    return const_cast<char*const*>(ret);
}

void Sandbox::setupFd() {
    if(in){
        close(0);
        open(in->c_str(), O_RDONLY);
    }
    if(out){
        close(1);
        open(out->c_str(), O_RDWR | O_CREAT | O_TRUNC, 0664);
    }
    if(err){
        close(2);
        open(err->c_str(), O_RDWR | O_CREAT | O_TRUNC, 0664);
    }
}

static void setRlimitHelper(int res, rlim_t limit) {
    struct rlimit rlim = {.rlim_cur=limit, .rlim_max=limit};
    if(setrlimit(res, &rlim) < 0){
        perror("rlimit()");
        exit(-1);
    }
}

void Sandbox::setupLimit() {
#define RLIM(res, val) setRlimitHelper(RLIMIT_##res, val)
    if(timeLimit){
        // double soft limit for hard limit
        struct rlimit rlim = {.rlim_cur=*timeLimit, .rlim_max=(*timeLimit)*2};
        if(setrlimit(RLIMIT_CPU, &rlim) < 0){
            perror("rlimit()");
            exit(-1);
        }
    }
    if(memoryLimit){
        RLIM(AS, *memoryLimit);
    }
    if(fileSizeLimit){
        RLIM(FSIZE, *fileSizeLimit);
    }
    // some default limit
    RLIM(CORE, 0);
    RLIM(NOFILE, 64);
    RLIM(MEMLOCK, 0);
}

void Sandbox::child(const std::vector<std::string> &args) {
    // prepare args and env
    char *const* prepared_args = prepare_helper(args);
    // preserve for env passing
    char *const* prepared_envs = prepare_helper({});
    // setup fd for redirection
    setupFd();
    // setup rlimit
    setupLimit();
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