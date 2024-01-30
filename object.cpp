#include <object.h>
#include <internal_funcs.h>
#include <memory>
#include <iostream>

void Object::AddDependency(AST obj) {
    ++dependencies_[obj];
}

void Object::RemoveDependency(AST obj) {
    auto it = dependencies_.find(obj);
    if (it == dependencies_.end()) {
        throw RuntimeError("There is no such a dependency");
    }
    --it->second;
    if (it->second == 0) {
        dependencies_.erase(it);
    }
}

void Object::Mark() {
    if (marked_) {
        return;
    }
    marked_ = true;
    for (auto [ptr, cnt] : dependencies_) {
        if (ptr) {
            ptr->Mark();
        }
    }
}

void Object::Unmark() {
    marked_ = false;
}

bool Object::GetMarker() {
    return marked_;
}

Dispatcher::Dispatcher(bool add_internal_funcs) : prev_layer_(nullptr) {
    if (add_internal_funcs) {
        AddInternalFunctions();
    }
}

Dispatcher::Dispatcher(Dispatcher* prev_layer) : prev_layer_(prev_layer) {
    AddDependency(prev_layer);
}

AST Dispatcher::Resolve(const std::string& name) {
    Dispatcher* cur_disp = this;
    while (true) {
        auto iter = cur_disp->scope_.find(name);
        if (iter != cur_disp->scope_.end()) {
            return iter->second;
        }
        if (!cur_disp->prev_layer_) {
            throw NameError("Can't resolve this name");
        }
        cur_disp = cur_disp->prev_layer_;
    }
}

void Dispatcher::Define(const std::string& name, AST obj) {
    auto it = scope_.find(name);
    if (it != scope_.end()) {
        RemoveDependency(it->second);
    }
    scope_[name] = obj;
    AddDependency(obj);
}
void Dispatcher::Set(const std::string& name, AST obj) {
    Dispatcher* cur_disp = this;
    while (true) {
        auto iter = cur_disp->scope_.find(name);
        if (iter != cur_disp->scope_.end()) {
            cur_disp->RemoveDependency(iter->second);
            iter->second = obj;
            cur_disp->AddDependency(iter->second);
            return;
        }
        if (!cur_disp->prev_layer_) {
            throw NameError("Can't resolve this name");
        }
        cur_disp = cur_disp->prev_layer_;
    }
}
void Dispatcher::AddInternalFunctions() {
    // I'm really sorry for this one
    std::unordered_map<std::string, Func> internal_funcs = {
        {"number?", &FuncIsNumber},
        {"pair?", &FuncIsPair},
        {"list?", &FuncIsList},
        {"null?", &FuncIsNull},
        {"boolean?", &FuncIsBoolean},
        {"symbol?", &FuncIsSymbol},

        {"=", &FuncEqual},
        {"<", &FuncLess},
        {">", &FuncGreater},
        {"<=", &FuncLessEqual},
        {">=", &FuncGreaterEqual},

        {"+", &FuncAdd},
        {"-", &FuncSub},
        {"*", &FuncMul},
        {"/", &FuncDiv},

        {"min", &FuncMin},
        {"max", &FuncMax},

        {"abs", &FuncAbs},

        {"quote", &FuncQuote},

        {"not", &FuncNot},
        {"and", &FuncAnd},
        {"or", &FuncOr},

        {"cons", &FuncCons},
        {"car", &FuncCar},
        {"cdr", &FuncCdr},

        {"list", &FuncList},
        {"list-ref", &FuncListRef},
        {"list-tail", &FuncListTail},

        {"if", &FuncIf},
        {"define", &FuncDefine},
        {"set!", &FuncSet},
        {"set-car!", &FuncSetCar},
        {"set-cdr!", &FuncSetCdr},
        {"lambda", &FuncLambda},
    };
    for (const auto& [name, func] : internal_funcs) {
        scope_[name] = GetHeap().Make<InternalFunction>(func);
        AddDependency(scope_[name]);
    }
}

AST Dispatcher::Compute(Dispatcher& dispatcher) {
    throw RuntimeError("Can't compute internal structure");
}
std::string Dispatcher::Serialize() {
    throw RuntimeError("Can't serialize internal structure");
}

Dispatcher* Dispatcher::GetPrevDispatcher() {
    return prev_layer_;
}

AST Dispatcher::Clone() {
    throw RuntimeError("Can't clone dispatcher");
}

std::string SerializeExpr(AST tree) {
    if (tree) {
        return tree->Serialize();
    }
    return "()";
}

Number::Number(int64_t value) : value_(value) {
}

int64_t Number::GetValue() const {
    return value_;
}

AST Number::Compute(Dispatcher& dispatcher) {
    return this;
}

std::string Number::Serialize() {
    return std::to_string(value_);
}

AST Number::Clone() {
    return GetHeap().Make<Number>(value_);
}

Symbol::Symbol(std::string name) : name_(std::move(name)) {
}

const std::string& Symbol::GetName() const {
    return name_;
}

AST Symbol::Compute(Dispatcher& dispatcher) {
    if (name_ == "#t" || name_ == "#f") {
        return GetHeap().Make<Symbol>(name_);
    }
    return dispatcher.Resolve(name_);
}

std::string Symbol::Serialize() {
    return name_;
}

AST Symbol::Clone() {
    return GetHeap().Make<Symbol>(name_);
}

Cell::Cell() : first_(nullptr), second_(nullptr) {
}

AST Cell::GetFirst() const {
    return first_;
}

AST Cell::GetSecond() const {
    return second_;
}
void Cell::SetFirst(AST ptr) {
    if (first_) {
        RemoveDependency(first_);
    }
    first_ = ptr;
    AddDependency(ptr);
}
void Cell::SetSecond(AST ptr) {
    if (second_) {
        RemoveDependency(second_);
    }
    second_ = ptr;
    AddDependency(second_);
}
AST Cell::Clone() {
    throw RuntimeError("Can't clone a cell");
}

ArgsVec ComputeAll(Dispatcher& dispatcher, const ArgsVec& args) {
    ArgsVec ans;
    ans.reserve(args.size());
    for (auto elem : args) {
        ans.push_back(ComputeExpr(dispatcher, elem));
    }
    return ans;
}

ArgsVec ExtractRawDataWithoutComputing(AST tree) {
    if (!tree) {
        return {};
    }
    std::vector<AST> res;
    while (Is<Cell>(tree)) {
        res.push_back(As<Cell>(tree)->GetFirst());
        tree = As<Cell>(tree)->GetSecond();
    }
    if (res.empty()) {
        throw RuntimeError(kWrongArgs);
    }
    res.push_back(tree);
    return res;
}

ArgsVec ExtractProperListWithoutComputing(AST tree) {
    auto data = ExtractRawDataWithoutComputing(tree);
    if (data.empty()) {
        return data;
    }
    if (data.back()) {
        throw RuntimeError(kWrongArgs);
    }
    data.pop_back();
    return data;
}

AST Cell::Compute(Dispatcher& dispatcher) {
    if (!first_) {
        throw RuntimeError("Function is missing");
    }
    auto func = As<Function>(first_->Compute(dispatcher));
    if (!func) {
        throw RuntimeError("This expression can't be used as a function");
    }
    func = As<Function>(func->Clone());
    auto args_np = ExtractProperListWithoutComputing(second_);
    return func->Apply(dispatcher, args_np);
}

std::string Cell::Serialize() {
    std::string res = "(";
    AST cur_second = this;
    while (cur_second) {
        if (res != "(") {
            res += ' ';
        }
        res += SerializeExpr(As<Cell>(cur_second)->GetFirst());
        auto new_second = As<Cell>(cur_second)->GetSecond();
        if (new_second && !Is<Cell>(new_second)) {
            res += " . ";
            res += new_second->Serialize();
            break;
        } else {
            cur_second = new_second;
        }
    }
    res += ")";
    return res;
}

InternalFunction::InternalFunction(const Func& func) : func_(func) {
}
AST InternalFunction::Apply(Dispatcher& dispatcher, const ArgsVec& args) {
    return (*func_)(dispatcher, args);
}
AST InternalFunction::Clone() {
    return GetHeap().Make<InternalFunction>(*func_);
}

AST Function::Compute(Dispatcher& dispatcher) {
    throw RuntimeError("Can't compute function");
}

CustomFunction::CustomFunction(Dispatcher& dispatcher, std::vector<std::string> args,
                               std::vector<AST> commands)
    : args_names_(std::move(args)),
      commands_(std::move(commands)),
      dispatcher_(As<Dispatcher>(GetHeap().Make<Dispatcher>(&dispatcher))) {
    AddDependency(dispatcher_);
    for (auto cmd : commands_) {
        AddDependency(cmd);
    }
}

CustomFunction::CustomFunction(CustomFunction& other)
    : args_names_(other.args_names_),
      commands_(other.commands_),
      dispatcher_(
          As<Dispatcher>(GetHeap().Make<Dispatcher>(other.dispatcher_->GetPrevDispatcher()))) {
    AddDependency(dispatcher_);
    for (auto cmd : commands_) {
        AddDependency(cmd);
    }
}

AST CustomFunction::Apply(Dispatcher& dispatcher, const ArgsVec& args_values) {
    auto cmp_values = ComputeAll(dispatcher, args_values);
    if (cmp_values.size() != args_names_.size()) {
        throw RuntimeError(kWrongArgs);
    }
    for (size_t i = 0; i < cmp_values.size(); ++i) {
        dispatcher_->Define(args_names_[i], cmp_values[i]);
    }
    AST result;
    for (const auto& cmd : commands_) {
        result = ComputeExpr(*dispatcher_, cmd);
    }
    return result;
}

AST CustomFunction::Clone() {
    return GetHeap().Make<CustomFunction>(*this);
}

std::string Function::Serialize() {
    return "Just a function";
}
AST ComputeExpr(Dispatcher& dispatcher, AST tree) {
    if (tree) {
        return tree->Compute(dispatcher);
    }
    return tree;
}
AST GetCopyOfExpr(AST tree) {
    if (!tree) {
        return tree;
    }
    return tree->Clone();
}

namespace {
std::unique_ptr<Heap> heap;
}

Heap& GetHeap() {
    if (!heap) {
        heap = std::make_unique<Heap>();
    }
    return *heap;
}

void Heap::Clean(Dispatcher* root) {
    for (auto ptr : objects_) {
        ptr->Unmark();
    }
    root->Mark();
    for (auto it = objects_.begin(); it != objects_.end();) {
        if ((*it)->GetMarker()) {
            ++it;
        } else {
            delete *it;
            it = objects_.erase(it);
        }
    }
}

void Heap::CleanAll() {
    for (auto it = objects_.begin(); it != objects_.end(); ++it) {
        delete *it;
    }
    objects_.clear();
}