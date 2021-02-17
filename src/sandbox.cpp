//
// Created by jw910731 on 2/4/21.
//

#include "sandbox.h"
#include "debug.h"

#include <iostream>
#include <string>
#include <utility>
#include <cstring>
#include <sstream>
#include <chrono>
#include <csignal>
#include <iomanip>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sched.h>
#include <unistd.h>
#include <fcntl.h>

// declare helper functions
static inline std::chrono::microseconds usecondTrans(struct timeval &time);

void alarmCallback(int signum);

void quitCallback(int signum);

static void setRlimitHelper(int res, rlim_t limit);

static char *const *prepare_helper(const std::vector<std::string> &vec);

static inline std::chrono::microseconds rlimCpuTimeHelper(struct rusage &rus);

std::ostream &operator<<(std::ostream &os, std::chrono::nanoseconds ns) {
    using namespace std;
    using namespace std::chrono;
    char fill = os.fill();
    os.fill('0');
    auto s = duration_cast<seconds>(ns);
    ns -= s;
    auto ms = duration_cast<milliseconds>(ns);
    ns -= ms;
    auto us = duration_cast<microseconds>(ns);
    os << setw(2) << s.count() << "s:"
       << setw(2) << ms.count() << "ms:"
       << setw(2) << us.count() << "us";
    os.fill(fill);
    return os;
};

Sandbox::Sandbox(std::string filePath) :
        filePath(std::move(filePath)) {}

void Sandbox::run(const std::vector<std::string> &args) {
    // setup time duration of internal field
    if (auto time = timeLimit) {
        timeLimitDur = std::chrono::milliseconds(*time);
    }
    if (auto time = walltimeLimit) {
        walltimeDur = std::chrono::milliseconds(*time);
    }
    // perform child process spawning
    pid_t pid = fork();
    if (pid < 0) {
        // fork failed
        perror("fork()");
        exit(-1);
    }
    if (pid == 0) {
        // child process job
        child(args);
    } else {
        childpid = pid;
        // parent process job
        parent();
    }
}

// I'm so fucking afraid any of these dangerous cast got anything wrong...
static char *const *prepare_helper(const std::vector<std::string> &vec) {
    char **ret = new char *[vec.size() + 1];
    for (size_t i = 0; i < vec.size(); ++i) {
        ret[i] = const_cast<char *>(vec[i].c_str());
    }
    // add NULL terminator
    ret[vec.size()] = NULL;
    return const_cast<char *const *>(ret);
}

void Sandbox::setupFd() {
    if (in) {
        close(0);
        open(in->c_str(), O_RDONLY);
    }
    if (out) {
        close(1);
        open(out->c_str(), O_RDWR | O_CREAT | O_TRUNC, 0664);
    }
    if (err) {
        close(2);
        open(err->c_str(), O_RDWR | O_CREAT | O_TRUNC, 0664);
    }
}

static void setRlimitHelper(int res, rlim_t limit) {
    struct rlimit rlim = {.rlim_cur=limit, .rlim_max=limit};
    if (setrlimit(res, &rlim) < 0) {
        perror("rlimit()");
        exit(-1);
    }
}

void Sandbox::setupLimit() {
#define RLIM(res, val) setRlimitHelper(RLIMIT_##res, val)
    if (timeLimit) {
        RLIM(CPU, *timeLimit / 1000);
    }
    if (memoryLimit) {
        RLIM(AS, *memoryLimit);
    }
    if (fileSizeLimit) {
        RLIM(FSIZE, *fileSizeLimit);
    }
    // some default limit
    RLIM(CORE, 0);
    RLIM(NOFILE, 64);
    RLIM(MEMLOCK, 0);
}

void Sandbox::child(const std::vector<std::string> &args) {
    // prepare args and env
    char *const *prepared_args = prepare_helper(args);
    // preserve for env passing
    char *const *prepared_envs = prepare_helper({});
    // setup fd for redirection
    setupFd();
    // setup rlimit
    setupLimit();
    // execute real program
    int ret = execve(filePath.c_str(), prepared_args, prepared_envs);
    if(ret < 0){
        perror("execve()");
        exit(-1);
    }
}

static volatile sig_atomic_t alarmFlag = 0, quitFlag = 0;

void alarmCallback(int signum) {
    alarmFlag = 1;
}

void quitCallback(int signum) {
    quitFlag = 1;
    // couldn't output debug message for not synchronous signals
}

void signalHelper() {
    struct sigaction alarm, quit;
    bzero(&quit, sizeof(struct sigaction));
    bzero(&alarm, sizeof(struct sigaction));
    alarm.sa_handler = alarmCallback;
    quit.sa_handler = quitCallback;
    // setup SIGALRM
    sigaction(SIGALRM, &alarm, NULL);

    // default signal handling stolen from ioi/isolate
    struct SigRule {
        int signum;
        enum {
            IGN, INT, FATAL
        } Action;
    };
    static const struct SigRule signalRules[] = {
            {SIGHUP,  SigRule::INT},
            {SIGINT,  SigRule::INT},
            {SIGQUIT, SigRule::INT},
            {SIGILL,  SigRule::FATAL},
            {SIGABRT, SigRule::FATAL},
            {SIGFPE,  SigRule::FATAL},
            {SIGSEGV, SigRule::FATAL},
            {SIGPIPE, SigRule::IGN},
            {SIGTERM, SigRule::INT},
            {SIGUSR1, SigRule::IGN},
            {SIGUSR2, SigRule::IGN},
            {SIGBUS,  SigRule::FATAL},
            {SIGTTOU, SigRule::IGN},
    };
    // setup default rules
    constexpr int ruleSize = (sizeof(signalRules) / sizeof(*signalRules));
    for (auto val : signalRules) {
        switch (val.Action) {
            case SigRule::INT:
            case SigRule::FATAL:
                sigaction(val.signum, &quit, NULL);
                break;
            case SigRule::IGN:
                signal(val.signum, SIG_IGN);
                break;
        }
    }
}

void Sandbox::parent() {
    startTime = std::chrono::high_resolution_clock::now();
    DEBUG(std::cerr << "Child Pid: " << childpid << std::endl);
    struct itimerval timer = {
            .it_interval = {.tv_usec=WALLTIME_INTERVAL},
            .it_value = {.tv_usec=WALLTIME_INTERVAL},
    };
    setitimer(ITIMER_REAL, &timer, NULL);
    signalHelper();
    waitChild();
    DEBUG(std::cerr << "Child Exited." << std::endl);
}

void Sandbox::childKiller() const {
    kill(-childpid, SIGKILL);
    kill(childpid, SIGKILL);
}

void Sandbox::timeChecker() {
    using namespace std::chrono;
    auto now = high_resolution_clock::now();
    if (walltimeLimit) {
        if (now - startTime > walltimeDur) {
            endTime = now;
            // exceed hard time
            childKiller();
        }
    }
    if (timeLimit) {
        struct rusage rus;
        int stat = getrusage(RUSAGE_CHILDREN, &rus);
        if (stat < 0) {
            perror("getrusage()");
        } else if (rlimCpuTimeHelper(rus) > timeLimitDur) {
            endTime = now;
            childKiller();
        }
    }
}

void Sandbox::waitChild() {
    int wstat;
    struct rusage rus;
    while (1) {
        if (quitFlag) {
            childKiller();
        }
        if (alarmFlag) {
            timeChecker();
            alarmFlag = 0;
        }
        int p = wait4(childpid, &wstat, 0, &rus);
        auto now = std::chrono::high_resolution_clock::now();
        // on error
        if (p < 0) {
            // interrupted by signal should be ignored
            if (errno == EINTR) {
                continue;
            }
            std::cerr << "Error occurred while waiting child." << std::endl;
            perror("wait4");
            exit(-1);
        } else {
            endTime = now;
            break;
        }
    }
    // exit normally or by signal termination

    genReport(&rus, wstat);
}

static inline std::chrono::microseconds usecondTrans(struct timeval &time) {
    return std::chrono::microseconds(time.tv_usec) + std::chrono::seconds(time.tv_sec);
}

static inline std::chrono::microseconds rlimCpuTimeHelper(struct rusage &rus) {
    return usecondTrans(rus.ru_utime) + usecondTrans(rus.ru_stime);
}


void Sandbox::genReport(struct rusage *rus, int wstatus) {
    std::stringstream ss;
    returned = true;
    // copy the rusage struct to report object
    memcpy(&report.rus, rus, sizeof(struct rusage));
    report.exitCode = WEXITSTATUS(wstatus);
    bool errFlag = false;
    if (WIFEXITED(wstatus)) {
        errFlag = true;
        if (report.exitCode != 0) {
            ss << "Exit with return code " << report.exitCode << " [RE] ";
        } else {
            ss << "Exit success ";
        }
    }
    auto cpuTime = rlimCpuTimeHelper(*rus);
    DEBUG(std::cerr << rus->ru_utime.tv_sec << ":" << rus->ru_utime.tv_usec << " " << rus->ru_stime.tv_sec << ":"
                    << rus->ru_stime.tv_usec << std::endl);
    DEBUG(std::cerr << cpuTime << std::endl);
    if (walltimeLimit && (endTime - startTime) > walltimeDur) {
        errFlag = true;
        ss << "[TLE (Wall)] ";
    } else if (timeLimit && (endTime - startTime) > timeLimitDur) {
        errFlag = true;
        ss << "[TLE] ";
    }
    if (unsigned long memLim = memoryLimit && rus->ru_maxrss > (long int) (memLim / 1000)) {
        errFlag = true;
        ss << "[MLE] ";
    }
    if (!errFlag) {
        if (WIFSIGNALED(wstatus)) {
            if (WTERMSIG(wstatus) == SIGXFSZ) {
                ss << "[OLE] ";
            } else {
                ss << "Caught fatal signal " << WTERMSIG(wstatus) << " [SE] ";
            }
        } else if (WIFSTOPPED(wstatus)) {
            ss << "Stopped by signal " << WSTOPSIG(wstatus) << " [SE] ";
        } else {
            std::cerr << "Program kill with unknown status()."
                      << std::hex << wstatus << std::dec << std::endl;
        }
    }
    // output all string to report
    report.fExitStat = ss.str();
}

std::optional<Sandbox::Report> Sandbox::getReport() const {
    if (returned) {
        return report;
    }
    return std::nullopt;
}

