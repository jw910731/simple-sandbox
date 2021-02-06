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
    program.add_argument("args")
        .remaining();
    program.add_argument("-t", "--time")
        .help("Constraint Time Limit (in millisecond)")
        .action([](const string &val){ return stoul(val); });
    program.add_argument("--wall-time")
            .help("Constraint Wall Time Limit (in millisecond), kill when exceeded.")
            .action([](const string &val){ return stoul(val); });
    program.add_argument("-m", "--memory")
        .help("Constraint Memory Limit (in KByte=1024Byte)")
        .action([](const string &val){ return stoul(val)/1024; });
    program.add_argument("-f", "--fsize")
        .help("Constraint Created / Write File Size (in Byte)")
        .action([](const string &val){ return stoul(val); });
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
    vector<string> args = {program.get("executable")};
    // Gather argument for the internal program
    try {
        vector<string> tmp = move(program.get<std::vector<std::string>>("args"));
        args.insert(args.end(), tmp.begin(), tmp.end());
    } catch (std::logic_error& e) {
        // empty catch block for no arg
    }

    // Sandbox Setup
    Sandbox sandbox(program.get("executable"));
    // get optional arguments
    const static map<const string, optional<string> Sandbox::*> stringOpt = {
            {"--in", &Sandbox::in},
            {"--out", &Sandbox::out},
            {"--err", &Sandbox::err},
    };
    const static map<string, optional<unsigned long> Sandbox::*> intOpt = {
            {"--time", &Sandbox::timeLimit},
            {"--wall-time", &Sandbox::walltimeLimit},
            {"--memory", &Sandbox::memoryLimit},
            {"--fsize", &Sandbox::fileSizeLimit},
    };
    // pass optional flag to sandbox
    for(auto [k, v] : stringOpt){
        if(auto val = program.present(k)){
            (sandbox.*v) = *val;
        }
    }
    for(auto [k, v] : intOpt){
        if(auto val = program.present<unsigned long>(k)){
            (sandbox.*v) = *val;
        }
    }
    // run sandbox and stuck until child exited
    sandbox.run(args);
    cout << sandbox.getReport()->fExitStat;
    return 0;
}
