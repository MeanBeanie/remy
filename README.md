# Remy

Literally what it says on the tin, simply a quick and easy way to make/manage/build code projects

Build with `g++ -std=c++20 *.cpp -o remy` and put the binary wherever

- Todo
    - [x] Generate Basic Build Command
    - [x] Manage Files and Create Needed
    - [x] Conditional Build Options
        - Should have easy to use flags

## Example

```
[remy]
$BUILD: g++
$FLAGS: -std=c++20
$SRCS: *.cpp

[remy-debug]
$BUILD: g++
$FLAGS: -std=c++20 -g
$SRCS: *.cpp

[Files]
main.cpp
README.md
LICENSE
```
