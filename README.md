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
use `simple-sandbox -h` to get further help. I'm too lazy to write this section.
# Todo
- implement file system access control
- be able to forward and set environment variable
- reimplement by using rust and cgroup (but it is impossible for me now >.<)