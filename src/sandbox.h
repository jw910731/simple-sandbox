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
#include <chrono>

#include <sys/resource.h>

class Sandbox {
public:
    std::optional<unsigned long> timeLimit, memoryLimit, fileSizeLimit, walltimeLimit;
    std::optional<std::string> in, out, err;

    class Report {
    public:
        std::string fExitStat;
        int exitCode;
        struct rusage rus;
    };

    Sandbox(std::string filePath);

    void run(const std::vector<std::string> &args);

    [[nodiscard]] std::optional<Report> getReport() const;

private:
    // in microsecond
    // now it's 1/100 second
    const static unsigned WALLTIME_INTERVAL = 10 * 1000;
    // child program executable file path
    std::string filePath;
    // final report object (only available after child process exited)
    Report report;
    // is the child program is being captured by the parent
    bool returned = false;
    // start time
    std::chrono::high_resolution_clock::time_point startTime, endTime;
    // time limit duration
    std::chrono::milliseconds timeLimitDur{}, walltimeDur{};
    // child pid
    pid_t childpid{};

    void child(const std::vector<std::string> &args);

    void parent();

    // child only method
    void setupFd();

    void setupLimit();

    // parent only method
    void childKiller() const; // force stop child
    void waitChild(); // wait for child and handle wall timer and call resource reporter when child process normally end
    void genReport(struct rusage *rus, int wstatus);

    void timeChecker();
};

#endif //SIMPLE_SANDBOX_ROOT_SANDBOX_H
