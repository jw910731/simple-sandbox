//
// Created by jw910731 on 2/4/21.
//
#include <iostream>
#include <argparse/argparse.hpp>
#include "sandbox.h"

using namespace std;
using namespace argparse;

void setup_argparse(ArgumentParser &program){
    program.add_argument("executable")
        .help("Executable file to wrap in sandbox");
    program.add_argument("-t", "--time")
        .help("Constraint Time Limit in millisecond.")
        .action([](const string &val){ return stoi(val); });
    program.add_argument("-m", "--memory")
        .help("Constraint Memory Limit in byte")
        .action([](const string &val){ return stoi(val); });
    program.add_argument("--in")
        .help("Redirect stdin Stream from Designated File");
    program.add_argument("--out")
        .help("Redirect stdout Stream to Designated File");
    program.add_argument("--err")
        .help("Redirect stderr Stream to Designated File");
}

int main(int argc, const char **argv){
    ArgumentParser program("simple-sandbox");
    setup_argparse(program);

    // parse argument
    try {
        program.parse_args(argc, argv);
    }
    catch (const std::runtime_error& e) {
        cerr << e.what() << endl;
        cerr << program;
        exit(1);
    }
    catch(const std::logic_error &e){
        cerr << "Argument Parsing Error" << endl;
        cerr << program;
        exit(1);
    }
    // Sandbox Setup
    Sandbox sandbox(program.get("executable"));
    // get optional arguments
    map<const string, void(Sandbox::*)(string)> stringOpt = {
            {"--in", &Sandbox::setStdin},
            {"--out", &Sandbox::setStdout},
            {"--err", &Sandbox::setStderr}
    };
    map<string, void (Sandbox::*)(int)> intOpt = {
            {"--time", &Sandbox::setTime},
            {"--memory", &Sandbox::setMemory}
    };
    for(auto [k, v] : stringOpt){
        if(auto val = program.present(k)){
            (sandbox.*v)(*val);
        }
    }
    for(auto [k, v] : intOpt){
        if(auto val = program.present<int>(k)){
            (sandbox.*v)(*val);
        }
    }
    // run sandbox
    sandbox.run({});
    return 0;
}
