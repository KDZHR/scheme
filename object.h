#pragma once

#include <functional>
#include <set>
#include <map>

#include <error.h>

class Object;
class Dispatcher;

using AST = Object*;
using ArgsVec = std::vector<AST>;
typedef AST (*Func)(Dispatcher&, const ArgsVec&);

const std::string kWrongArgs = "Wrong function arguments";

class Dispatcher;
class Heap;

std::string SerializeExpr(AST);

class Object {
    friend Heap;

public:
    virtual AST Compute(Dispatcher& dispatcher) = 0;
    virtual std::string Serialize() = 0;
    virtual AST Clone() = 0;
    virtual ~Object() = default;

protected:
    void AddDependency(AST);
    void RemoveDependency(AST);
    void Mark();
    void Unmark();
    bool GetMarker();

private:
    std::map<AST, size_t> dependencies_;
    bool marked_;
};

class Number : public Object {
    friend Heap;
    Number(int64_t value);

public:
    Number(const Number&) = delete;
    int64_t GetValue() const;
    AST Compute(Dispatcher& dispatcher);
    std::string Serialize();
    AST Clone();

private:
    int64_t value_;
};

class Dispatcher : public Object {
    friend Heap;
    explicit Dispatcher(bool add_internal_funcs);
    explicit Dispatcher(Dispatcher*);

public:
    Dispatcher(const Dispatcher&) = delete;
    AST Compute(Dispatcher& dispatcher);
    std::string Serialize();
    AST Resolve(const std::string&);
    void Define(const std::string&, AST);
    void Set(const std::string&, AST);
    void AddInternalFunctions();
    Dispatcher* GetPrevDispatcher();
    AST Clone();

private:
    std::unordered_map<std::string, AST> scope_;
    Dispatcher* prev_layer_;
};

class Symbol : public Object {
    friend Heap;
    Symbol(std::string name);

public:
    Symbol(const Symbol&) = delete;
    const std::string& GetName() const;
    AST Compute(Dispatcher& dispatcher);
    std::string Serialize();
    AST Clone();

private:
    std::string name_;
};

class Cell : public Object {
    friend Heap;
    Cell();

public:
    Cell(const Cell&) = delete;
    AST GetFirst() const;
    AST GetSecond() const;
    void SetFirst(AST ptr);
    void SetSecond(AST ptr);
    AST Compute(Dispatcher& dispatcher);
    std::string Serialize();
    AST Clone();

private:
    AST first_;
    AST second_;
};

ArgsVec ComputeAll(Dispatcher&, const ArgsVec&);
ArgsVec ExtractRawDataWithoutComputing(AST);
ArgsVec ExtractProperListWithoutComputing(AST);

class Function : public Object {
    friend Heap;

public:
    virtual AST Apply(Dispatcher&, const ArgsVec&) = 0;
    AST Compute(Dispatcher& dispatcher);
    std::string Serialize();
};

class InternalFunction : public Function {
    friend Heap;

public:
    InternalFunction(const InternalFunction&) = delete;
    InternalFunction(const Func&);
    AST Apply(Dispatcher&, const ArgsVec&);
    AST Clone();

private:
    const Func func_;
};

class CustomFunction : public Function {
    friend Heap;
    CustomFunction(CustomFunction&);
    CustomFunction(Dispatcher&, std::vector<std::string>, std::vector<AST>);

public:
    AST Apply(Dispatcher&, const ArgsVec&);
    AST Clone();

private:
    std::vector<std::string> args_names_;
    std::vector<AST> commands_;
    Dispatcher* dispatcher_;
};

AST ComputeExpr(Dispatcher&, AST);
AST GetCopyOfExpr(AST);

class Heap {
public:
    template <class T, class... Args>
    requires std::is_base_of_v<Object, T> AST Make(Args&&... args) {
        AST ptr = new T(std::forward<Args>(args)...);
        objects_.insert(ptr);
        return ptr;
    }
    void Clean(Dispatcher* root);
    void CleanAll();

private:
    std::set<AST> objects_;
};

Heap& GetHeap();
///////////////////////////////////////////////////////////////////////////////

// Runtime type checking and convertion.
// This can be helpful: https://en.cppreference.com/w/cpp/memory/shared_ptr/pointer_cast

template <class T>
requires std::is_base_of_v<Object, T> T* As(const AST& obj) {
    return dynamic_cast<T*>(obj);
}

template <class T>
bool Is(const AST& obj) {
    if (!obj) {
        return false;
    }
    return typeid(*obj) == typeid(T);
}
