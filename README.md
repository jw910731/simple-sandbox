# simple-sandbox [![CodeFactor](https://www.codefactor.io/repository/github/jw910731/simple-sandbox/badge/develop)](https://www.codefactor.io/repository/github/jw910731/simple-sandbox/overview/develop)
A simple sandbox that constraint the resource usage of wrapped program
# Build
simply run following command in project root directory
```bash
mkdir build
cd build
cmake ..
make
```
and executable will be located in `build/src/simple-sandbox`
# tl;dr
```
Command Usage :
        simple-sandbox [options] <executable> [args for executable]
Option List:
        -t, --time : Constraint Time Limit (in millisecond)
        --wall-time : Constraint Wall Time Limit (in millisecond), kill when exceeded
        -m, --memory : Constraint Memory Limit (in KByte = 1024Byte)
        -f, --fsize : Constraint Created / Write File Size (in Byte)
        --in, --out, --err : Redirect corresponding I/O stream from or to designated file
        -h, --help : print this help message
```

# Example Usage
## Time & Wall time
Set CPU time constraint and Real time constraint.

Example:
```shell
simple-sandbox -t <CPU Time Limit> --wall-time <Real Time Limit> <executable>
```
## File size constraint & redirection
File size constraint only work when redirection stdout / stderr to file or the program directly writes file.

Example:
```shell
simple-sandbox -f 1024 --out <Stdout redirection file> --err <Stderr redirection file> <executable>
```
## Memory constrain
Example:
```shell
simple-sandbox -m 1024 <executable>
```
# Todo
- implement file system access control
- be able to forward and set environment variable
- reimplement by using rust and cgroup (but it is impossible for me now >.<)
