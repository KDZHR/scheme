#include <numeric>

#include <internal_funcs.h>

const std::string kTrueStr = "#t";
const std::string kFalseStr = "#f";

const int64_t kAddInit = 0;
const int64_t kMulInit = 1;
const int64_t kMinInit = INT64_MAX;
const int64_t kMaxInit = INT64_MIN;

AST GetSymBool(bool val) {
    return GetHeap().Make<Symbol>(val ? kTrueStr : kFalseStr);
}

bool ToBool(AST tree) {
    return !(Is<Symbol>(tree) && As<Symbol>(tree)->GetName() == "#f");
}

template <class T>
bool CheckParams(const T& params, const std::vector<std::function<bool(const T&)>>& funcs) {
    bool ok = true;
    for (const auto& func : funcs) {
        ok &= func(params);
    }
    return ok;
}

template <class ErrorType, class T>
void CheckAndThrow(const T& params, const std::vector<std::function<bool(const T&)>>& funcs) {
    if (!CheckParams(params, funcs)) {
        throw ErrorType(kWrongArgs);
    }
}

template <size_t size, class T>
bool HasOnly(const std::vector<T>& vec) {
    return vec.size() == size;
}

template <size_t size, class T>
bool HasAtLeast(const std::vector<T>& vec) {
    return vec.size() >= size;
}

bool AreAllNumbers(const std::vector<AST>& vec) {
    return std::all_of(vec.begin(), vec.end(), [](AST elem) { return Is<Number>(elem); });
}

template <class Cmp>
bool MonoCheck(const std::vector<AST>& vec) {
    Cmp comp;
    bool ok = true;
    for (size_t i = 0; i + 1 < vec.size(); ++i) {
        ok &= comp(As<Number>(vec[i])->GetValue(), As<Number>(vec[i + 1])->GetValue());
    }
    return ok;
}

std::vector<AST> ExtractRawData(Dispatcher& dispatcher, AST tree) {
    std::vector<AST> res;
    auto list = ExtractRawDataWithoutComputing(tree);
    res.reserve(list.size());
    for (auto elem : list) {
        res.push_back(ComputeExpr(dispatcher, elem));
    }
    return res;
}

std::vector<AST> ExtractProperList(Dispatcher& dispatcher, AST tree) {
    auto res = ExtractRawData(dispatcher, tree);
    if (res.empty()) {
        return res;
    }
    if (res.back()) {
        throw RuntimeError(kWrongArgs);
    }
    res.pop_back();
    return res;
}

std::vector<int64_t> ArgsToInt(const std::vector<AST>& list) {
    std::vector<int64_t> res;
    res.reserve(list.size());
    for (const auto& elem : list) {
        if (!Is<Number>(elem)) {
            throw RuntimeError(kWrongArgs);
        }
        res.push_back(As<Number>(elem)->GetValue());
    }
    return res;
}

std::vector<std::string> ArgsToStr(const std::vector<AST>& list) {
    std::vector<std::string> res;
    res.reserve(list.size());
    for (const auto& elem : list) {
        if (!Is<Symbol>(elem)) {
            throw RuntimeError(kWrongArgs);
        }
        res.push_back(As<Symbol>(elem)->GetName());
    }
    return res;
}

AST FuncIsNumber(Dispatcher& dispatcher, const ArgsVec& args) {
    auto cmp_args = ComputeAll(dispatcher, args);
    return GetSymBool(CheckParams(args, {HasOnly<1, AST>, AreAllNumbers}));
}

AST FuncIsPair(Dispatcher& dispatcher, const ArgsVec& args) {
    auto cmp_args = ComputeAll(dispatcher, args);
    CheckAndThrow<RuntimeError>(cmp_args, {HasOnly<1, AST>});
    auto inner_list = ExtractRawData(dispatcher, cmp_args[0]);
    return GetSymBool(!inner_list.empty() && ((inner_list.size() == 2 && inner_list.back()) ||
                                              (inner_list.size() == 3 && !inner_list.back())));
}
AST FuncIsList(Dispatcher& dispatcher, const ArgsVec& args) {
    auto cmp_args = ComputeAll(dispatcher, args);
    CheckAndThrow<RuntimeError>(cmp_args, {HasOnly<1, AST>});
    try {
        ExtractProperList(dispatcher, cmp_args[0]);
    } catch (std::runtime_error) {
        return GetSymBool(false);
    }
    return GetSymBool(true);
}
AST FuncIsNull(Dispatcher& dispatcher, const ArgsVec& args) {
    auto cmp_args = ComputeAll(dispatcher, args);
    CheckAndThrow<RuntimeError>(cmp_args, {HasOnly<1, AST>});
    return GetSymBool(!cmp_args[0]);
}

AST FuncIsBoolean(Dispatcher& dispatcher, const ArgsVec& args) {
    auto cmp_args = ComputeAll(dispatcher, args);
    CheckAndThrow<RuntimeError>(cmp_args, {HasOnly<1, AST>});
    bool ans = false;
    if (Is<Symbol>(cmp_args[0])) {
        auto str = As<Symbol>(cmp_args[0])->GetName();
        if (str == kFalseStr || str == kTrueStr) {
            ans = true;
        }
    }
    return GetSymBool(ans);
}

AST FuncIsSymbol(Dispatcher& dispatcher, const ArgsVec& args) {
    auto cmp_args = ComputeAll(dispatcher, args);
    CheckAndThrow<RuntimeError>(cmp_args, {HasOnly<1, AST>});
    return GetSymBool(Is<Symbol>(cmp_args[0]));
}

AST FuncEqual(Dispatcher& dispatcher, const ArgsVec& args) {
    auto cmp_args = ComputeAll(dispatcher, args);
    CheckAndThrow<RuntimeError>(cmp_args, {AreAllNumbers});
    return GetSymBool(CheckParams(cmp_args, {MonoCheck<std::equal_to<int64_t>>}));
}

AST FuncLess(Dispatcher& dispatcher, const ArgsVec& args) {
    auto cmp_args = ComputeAll(dispatcher, args);
    CheckAndThrow<RuntimeError>(cmp_args, {AreAllNumbers});
    return GetSymBool(CheckParams(cmp_args, {MonoCheck<std::less<int64_t>>}));
}

AST FuncGreater(Dispatcher& dispatcher, const ArgsVec& args) {
    auto cmp_args = ComputeAll(dispatcher, args);
    CheckAndThrow<RuntimeError>(cmp_args, {AreAllNumbers});
    return GetSymBool(CheckParams(cmp_args, {MonoCheck<std::greater<int64_t>>}));
}

AST FuncLessEqual(Dispatcher& dispatcher, const ArgsVec& args) {
    auto cmp_args = ComputeAll(dispatcher, args);
    CheckAndThrow<RuntimeError>(cmp_args, {AreAllNumbers});
    return GetSymBool(CheckParams(cmp_args, {MonoCheck<std::less_equal<int64_t>>}));
}

AST FuncGreaterEqual(Dispatcher& dispatcher, const ArgsVec& args) {
    auto cmp_args = ComputeAll(dispatcher, args);
    CheckAndThrow<RuntimeError>(cmp_args, {AreAllNumbers});
    return GetSymBool(CheckParams(cmp_args, {MonoCheck<std::greater_equal<int64_t>>}));
}

AST FuncAdd(Dispatcher& dispatcher, const ArgsVec& args) {
    auto int_list = ArgsToInt(ComputeAll(dispatcher, args));
    auto res = std::accumulate(int_list.begin(), int_list.end(), kAddInit, std::plus());
    return GetHeap().Make<Number>(res);
}
AST FuncSub(Dispatcher& dispatcher, const ArgsVec& args) {
    auto int_list = ArgsToInt(ComputeAll(dispatcher, args));
    CheckAndThrow<RuntimeError>(int_list, {HasAtLeast<2, int64_t>});
    auto res = std::accumulate(std::next(int_list.begin()), int_list.end(), *int_list.begin(),
                               std::minus());
    return GetHeap().Make<Number>(res);
}
AST FuncMul(Dispatcher& dispatcher, const ArgsVec& args) {
    auto int_list = ArgsToInt(ComputeAll(dispatcher, args));
    auto res = std::accumulate(int_list.begin(), int_list.end(), kMulInit, std::multiplies());
    return GetHeap().Make<Number>(res);
}
AST FuncDiv(Dispatcher& dispatcher, const ArgsVec& args) {
    auto int_list = ArgsToInt(ComputeAll(dispatcher, args));
    CheckAndThrow<RuntimeError>(int_list, {HasAtLeast<2, int64_t>});
    auto res = std::accumulate(std::next(int_list.begin()), int_list.end(), *int_list.begin(),
                               std::divides());
    return GetHeap().Make<Number>(res);
}

int64_t GetMin(int64_t lhs, const int64_t& rhs) {
    return std::min(lhs, rhs);
}

AST FuncMin(Dispatcher& dispatcher, const ArgsVec& args) {
    auto int_list = ArgsToInt(ComputeAll(dispatcher, args));
    CheckAndThrow<RuntimeError>(int_list, {HasAtLeast<1, int64_t>});
    auto res = std::accumulate(int_list.begin(), int_list.end(), kMinInit, GetMin);
    return GetHeap().Make<Number>(res);
}

int64_t GetMax(int64_t lhs, const int64_t& rhs) {
    return std::max(lhs, rhs);
}

AST FuncMax(Dispatcher& dispatcher, const ArgsVec& args) {
    auto int_list = ArgsToInt(ComputeAll(dispatcher, args));
    CheckAndThrow<RuntimeError>(int_list, {HasAtLeast<1, int64_t>});
    auto res = std::accumulate(int_list.begin(), int_list.end(), kMaxInit, GetMax);
    return GetHeap().Make<Number>(res);
}

AST FuncAbs(Dispatcher& dispatcher, const ArgsVec& args) {
    auto int_list = ArgsToInt(ComputeAll(dispatcher, args));
    CheckAndThrow<RuntimeError>(int_list, {HasOnly<1, int64_t>});
    return GetHeap().Make<Number>(std::abs(int_list[0]));
}

AST FuncQuote(Dispatcher& dispatcher, const ArgsVec& args) {
    CheckAndThrow<RuntimeError>(args, {HasOnly<1, AST>});
    return args[0];
}

AST FuncNot(Dispatcher& dispatcher, const ArgsVec& args) {
    auto cmp_args = ComputeAll(dispatcher, args);
    CheckAndThrow<RuntimeError>(cmp_args, {HasOnly<1, AST>});
    return GetSymBool(!ToBool(cmp_args[0]));
}

AST FuncAnd(Dispatcher& dispatcher, const ArgsVec& args) {
    AST last_comp;
    for (auto elem : args) {
        last_comp = ComputeExpr(dispatcher, elem);
        if (!ToBool(last_comp)) {
            return last_comp;
        }
    }
    if (args.empty()) {
        return GetSymBool(true);
    }
    return last_comp;
}

AST FuncOr(Dispatcher& dispatcher, const ArgsVec& args) {
    AST last_comp;
    for (auto elem : args) {
        last_comp = ComputeExpr(dispatcher, elem);
        if (ToBool(last_comp)) {
            return last_comp;
        }
    }
    if (args.empty()) {
        return GetSymBool(false);
    }
    return last_comp;
}

AST FuncCons(Dispatcher& dispatcher, const ArgsVec& args) {
    auto cmp_args = ComputeAll(dispatcher, args);
    CheckAndThrow<RuntimeError>(cmp_args, {HasOnly<2, AST>});
    auto cur_cell = As<Cell>(GetHeap().Make<Cell>());
    cur_cell->SetFirst(cmp_args[0]);
    cur_cell->SetSecond(cmp_args[1]);
    return cur_cell;
}
AST FuncCar(Dispatcher& dispatcher, const ArgsVec& args) {
    auto cmp_args = ComputeAll(dispatcher, args);
    CheckAndThrow<RuntimeError>(cmp_args, {HasOnly<1, AST>});
    if (!Is<Cell>(cmp_args[0])) {
        throw RuntimeError(kWrongArgs);
    }
    return As<Cell>(cmp_args[0])->GetFirst();
}
AST FuncCdr(Dispatcher& dispatcher, const ArgsVec& args) {
    auto cmp_args = ComputeAll(dispatcher, args);
    CheckAndThrow<RuntimeError>(cmp_args, {HasOnly<1, AST>});
    if (!Is<Cell>(cmp_args[0])) {
        throw RuntimeError(kWrongArgs);
    }
    return As<Cell>(cmp_args[0])->GetSecond();
}

AST CreateList(const std::vector<AST>& list) {
    if (list.empty()) {
        return nullptr;
    }
    Cell* root_cell = nullptr;
    Cell* cur_cell = nullptr;
    for (auto elem : list) {
        if (!root_cell) {
            cur_cell = root_cell = As<Cell>(GetHeap().Make<Cell>());
        } else {
            cur_cell->SetSecond(GetHeap().Make<Cell>());
            cur_cell = As<Cell>(cur_cell->GetSecond());
        }
        cur_cell->SetFirst(elem);
    }
    return root_cell;
}

AST FuncList(Dispatcher& dispatcher, const ArgsVec& args) {
    return CreateList(ComputeAll(dispatcher, args));
}
AST FuncListRef(Dispatcher& dispatcher, const ArgsVec& args) {
    auto cmp_args = ComputeAll(dispatcher, args);
    CheckAndThrow<RuntimeError>(cmp_args, {HasOnly<2, AST>});
    auto inner_list = ExtractProperList(dispatcher, cmp_args[0]);
    if (!Is<Number>(cmp_args[1])) {
        throw RuntimeError(kWrongArgs);
    }
    auto num = As<Number>(cmp_args[1])->GetValue();
    if (num < 0 || static_cast<size_t>(num) >= inner_list.size()) {
        throw RuntimeError(kWrongArgs);
    }
    return inner_list[num];
}
AST FuncListTail(Dispatcher& dispatcher, const ArgsVec& args) {
    auto cmp_args = ComputeAll(dispatcher, args);
    CheckAndThrow<RuntimeError>(cmp_args, {HasOnly<2, AST>});
    auto inner_list = ExtractProperList(dispatcher, cmp_args[0]);
    if (!Is<Number>(cmp_args[1])) {
        throw RuntimeError(kWrongArgs);
    }
    auto num = As<Number>(cmp_args[1])->GetValue();
    if (num < 0 || static_cast<size_t>(num) > inner_list.size()) {
        throw RuntimeError(kWrongArgs);
    }
    return CreateList({inner_list.begin() + num, inner_list.end()});
}

AST FuncIf(Dispatcher& dispatcher, const ArgsVec& args) {
    if (args.size() != 2 && args.size() != 3) {
        throw SyntaxError(kWrongArgs);
    }
    if (ToBool(ComputeExpr(dispatcher, args[0]))) {
        return ComputeExpr(dispatcher, args[1]);
    } else {
        return (args.size() == 2 ? nullptr : ComputeExpr(dispatcher, args[2]));
    }
}
AST FuncDefine(Dispatcher& dispatcher, const ArgsVec& args) {
    CheckAndThrow<SyntaxError>(args, {HasAtLeast<2, AST>});
    if (Is<Symbol>(args[0])) {
        CheckAndThrow<SyntaxError>(args, {HasOnly<2, AST>});
        dispatcher.Define(As<Symbol>(args[0])->GetName(), ComputeExpr(dispatcher, args[1]));
    } else {
        auto names = ArgsToStr(ExtractProperListWithoutComputing(args[0]));
        auto func = GetHeap().Make<CustomFunction>(
            dispatcher, std::vector<std::string>(std::next(names.begin()), names.end()),
            ArgsVec(std::next(args.begin()), args.end()));
        dispatcher.Define(names[0], func);
    }
    return nullptr;
}
AST FuncSet(Dispatcher& dispatcher, const ArgsVec& args) {
    CheckAndThrow<SyntaxError>(args, {HasOnly<2, AST>});
    if (!Is<Symbol>(args[0])) {
        throw RuntimeError(kWrongArgs);
    }
    dispatcher.Set(As<Symbol>(args[0])->GetName(), ComputeExpr(dispatcher, args[1]));
    return nullptr;
}
AST FuncSetCar(Dispatcher& dispatcher, const ArgsVec& args) {
    CheckAndThrow<SyntaxError>(args, {HasOnly<2, AST>});
    auto cmp_args = ComputeAll(dispatcher, args);
    if (!Is<Cell>(cmp_args[0])) {
        throw RuntimeError(kWrongArgs);
    }
    As<Cell>(cmp_args[0])->SetFirst(cmp_args[1]);
    return nullptr;
}
AST FuncSetCdr(Dispatcher& dispatcher, const ArgsVec& args) {
    CheckAndThrow<SyntaxError>(args, {HasOnly<2, AST>});
    auto cmp_args = ComputeAll(dispatcher, args);
    if (!Is<Cell>(cmp_args[0])) {
        throw RuntimeError(kWrongArgs);
    }
    As<Cell>(cmp_args[0])->SetSecond(cmp_args[1]);
    return nullptr;
}
AST FuncLambda(Dispatcher& dispatcher, const ArgsVec& args) {
    CheckAndThrow<SyntaxError>(args, {HasAtLeast<2, AST>});
    auto func_args = ArgsToStr(ExtractProperListWithoutComputing(args[0]));
    return GetHeap().Make<CustomFunction>(dispatcher, func_args,
                                          ArgsVec(std::next(args.begin()), args.end()));
}