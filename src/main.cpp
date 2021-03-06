//
// Created by jw910731 on 2/4/21.
//
#include <iostream>
#include <vector>
#include <map>
#include "sandbox.h"
#include "debug.h"
using namespace std;

const static char *helpMsg = "Command Usage :\n\
        %s [options] <executable> [args for executable]\n\
Option List:\n\
        -t, --time : Constraint Time Limit (in millisecond)\n\
        --wall-time : Constraint Wall Time Limit (in millisecond), kill when exceeded\n\
        -m, --memory : Constraint Memory Limit (in Byte)\n\
        -f, --fsize : Constraint Created / Write File Size (in Byte)\n\
        --in, --out, --err : Redirect corresponding I/O stream from or to designated file\n\
        -h, --help : print this help message\n";

inline size_t isOptArg(const string &_s){
    const char *s = _s.c_str(), *tmp = s;
    // not terminated
    while(*tmp != '\0' && (tmp - s) < 2){
        if(*tmp != '-'){
            break;
        }
        ++tmp;
    }
    return tmp - s;
}

vector<string> arg_parser(const vector<string> &args, Sandbox &sandbox){
    // args lookup table
    const static map<const string, optional<string> Sandbox::*> stringOpt = {
            {"--in",  &Sandbox::in},
            {"--out", &Sandbox::out},
            {"--err", &Sandbox::err},
    };
    // map {string : pair<Sandbox field, divide ratio>}
    const static map<string, pair<optional<unsigned long> Sandbox::*, unsigned long>> intOpt = {
            {"--time", {&Sandbox::timeLimit, 1}},
            {"-t",      {&Sandbox::timeLimit, 1}},
            {"--wall-time", {&Sandbox::walltimeLimit, 1}},
            {"--memory", {&Sandbox::memoryLimit, 1}},
            {"-m", {&Sandbox::memoryLimit, 1}},
            {"--fsize", {&Sandbox::fileSizeLimit, 1}},
            {"-f", {&Sandbox::fileSizeLimit, 1}},
    };

    // iterator for args
    auto it = args.cbegin();
    ++it; // to skip sandbox executable path
    // sandbox argument
    while(it != args.cend()){
        size_t opt = isOptArg(*it);
        if(opt == 0) break;
        // if sandbox argument not starts from "-" / "--"
        if(opt > 2) throw logic_error("Argument parse error");
        auto strMapIt = stringOpt.find(*it);
        auto intMapIt = intOpt.find(*it);
        // parse sandbox args
        if(strMapIt != stringOpt.end()){
            (sandbox.*(strMapIt->second)) = *(++it);
        }
        else if(intMapIt != intOpt.end()){
            auto &tmp = intMapIt->second; // mapped pair
            sandbox.*(tmp.first) = stoul(*(++it)) / tmp.second;
        }
        else{
            if(*it == "-h" || *it == "--help"){
                printf(helpMsg, args[0].c_str());
                exit(0);
            }
            throw logic_error("Argument parse error");
        }
        ++it;
    }
    // process executable
    if(it == args.cend()) throw logic_error("Argument not enough");
    sandbox.setExecPath(*it); // keep the program name as the first argument
    // prepare executable args
    vector<string> exeArgs;
    while(it != args.cend()){
        exeArgs.emplace_back(*(it++));
    }
    return exeArgs;
}

int main(int argc, const char **argv) {
    vector<string> args;
    for(int i = 0 ; i < argc ; ++i){
        args.emplace_back(string(argv[i]));
    }
    Sandbox sandbox;
    try {
        auto parsed = arg_parser(args, sandbox);
        // run sandbox and stuck until child exited
        sandbox.run(parsed);
        cout << sandbox.getReport()->fExitStat;
    }
    catch (std::logic_error &e) {
        cout << "Argument Parse Error" << endl;
        printf(helpMsg, argv[0]);
    }
    return 0;
}
