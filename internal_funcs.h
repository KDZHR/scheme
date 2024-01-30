#pragma once
#include <object.h>

AST FuncIsNumber(Dispatcher&, const ArgsVec&);
AST FuncIsPair(Dispatcher&, const ArgsVec&);
AST FuncIsList(Dispatcher&, const ArgsVec&);
AST FuncIsNull(Dispatcher&, const ArgsVec&);
AST FuncIsBoolean(Dispatcher&, const ArgsVec&);
AST FuncIsSymbol(Dispatcher&, const ArgsVec&);

AST FuncEqual(Dispatcher&, const ArgsVec&);
AST FuncLess(Dispatcher&, const ArgsVec&);
AST FuncGreater(Dispatcher&, const ArgsVec&);
AST FuncLessEqual(Dispatcher&, const ArgsVec&);
AST FuncGreaterEqual(Dispatcher&, const ArgsVec&);

AST FuncAdd(Dispatcher&, const ArgsVec&);
AST FuncSub(Dispatcher&, const ArgsVec&);
AST FuncMul(Dispatcher&, const ArgsVec&);
AST FuncDiv(Dispatcher&, const ArgsVec&);

AST FuncMin(Dispatcher&, const ArgsVec&);
AST FuncMax(Dispatcher&, const ArgsVec&);

AST FuncAbs(Dispatcher&, const ArgsVec&);

AST FuncQuote(Dispatcher&, const ArgsVec&);

AST FuncNot(Dispatcher&, const ArgsVec&);
AST FuncAnd(Dispatcher&, const ArgsVec&);
AST FuncOr(Dispatcher&, const ArgsVec&);

AST FuncCons(Dispatcher&, const ArgsVec&);
AST FuncCar(Dispatcher&, const ArgsVec&);
AST FuncCdr(Dispatcher&, const ArgsVec&);

AST FuncList(Dispatcher&, const ArgsVec&);
AST FuncListRef(Dispatcher&, const ArgsVec&);
AST FuncListTail(Dispatcher&, const ArgsVec&);

AST FuncIf(Dispatcher&, const ArgsVec&);
AST FuncDefine(Dispatcher&, const ArgsVec&);
AST FuncSet(Dispatcher&, const ArgsVec&);
AST FuncSetCar(Dispatcher&, const ArgsVec&);
AST FuncSetCdr(Dispatcher&, const ArgsVec&);
AST FuncLambda(Dispatcher&, const ArgsVec&);
