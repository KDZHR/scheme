#pragma once

#include <functional>
#include <string>
#include <unordered_map>

#include <object.h>
#include <parser.h>

class Dispatcher;
class Interpreter;

class Interpreter {
public:
    Interpreter();
    std::string Run(const std::string&);
    ~Interpreter();

private:
    Dispatcher* dispatcher_;
};
