#include <sstream>

#include "scheme.h"

Interpreter::Interpreter() : dispatcher_(As<Dispatcher>(GetHeap().Make<Dispatcher>(true))) {
}

std::string Interpreter::Run(const std::string& line) {
    std::stringstream ss{line};
    Tokenizer tokenizer(&ss);
    auto ast = Read(&tokenizer);
    if (!ast) {
        throw RuntimeError("Unable to evaluate");
    }
    auto str = SerializeExpr(ast->Compute(*dispatcher_));
    GetHeap().Clean(dispatcher_);
    return str;
}

Interpreter::~Interpreter() {
    GetHeap().CleanAll();
}