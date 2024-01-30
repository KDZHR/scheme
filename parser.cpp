#include <error.h>
#include <parser.h>

const std::string kQuoteStr = "quote";

AST RecursiveRead(Tokenizer* tokenizer);

AST ReadList(Tokenizer* tokenizer) {
    auto cur_token = tokenizer->GetToken();
    Cell* root_cell = nullptr;
    Cell* right_cell = nullptr;
    if (std::get<BracketToken>(cur_token) == BracketToken::OPEN) {
        tokenizer->Next();
        while (!tokenizer->IsEnd() && tokenizer->GetToken() != Token{BracketToken::CLOSE}) {
            cur_token = tokenizer->GetToken();
            if (cur_token == Token{DotToken()}) {
                if (!root_cell) {
                    throw SyntaxError("Improper list initialization without first element");
                }
                tokenizer->Next();
                if (tokenizer->IsEnd()) {
                    throw SyntaxError("Improper list initialization without second element");
                }
                auto v2 = RecursiveRead(tokenizer);
                if (tokenizer->IsEnd()) {
                    throw SyntaxError("No matching close bracket");
                }
                auto bracket_close = tokenizer->GetToken();
                tokenizer->Next();
                if (bracket_close != Token{BracketToken::CLOSE}) {
                    throw SyntaxError("New character instead of close bracket of improper list");
                }
                right_cell->SetSecond(v2);
                return root_cell;
            } else {
                auto res = RecursiveRead(tokenizer);
                if (!right_cell) {
                    right_cell = As<Cell>(GetHeap().Make<Cell>());
                    root_cell = right_cell;
                } else {
                    right_cell->SetSecond(GetHeap().Make<Cell>());
                    right_cell = As<Cell>(right_cell->GetSecond());
                }
                right_cell->SetFirst(res);
            }
        }
        if (tokenizer->IsEnd()) {
            throw SyntaxError("No matching close bracket");
        } else {
            tokenizer->Next();
        }
        return root_cell;
    } else {
        throw SyntaxError("Wrong bracket placement");
    }
}

AST RecursiveRead(Tokenizer* tokenizer) {
    if (tokenizer->IsEnd()) {
        throw SyntaxError("Empty stream");
    }
    auto cur_token = tokenizer->GetToken();
    if (std::holds_alternative<BracketToken>(cur_token)) {
        if (std::get<BracketToken>(cur_token) == BracketToken::CLOSE) {
            throw SyntaxError("No matching open bracket");
        }
        return ReadList(tokenizer);
    }
    tokenizer->Next();
    if (std::holds_alternative<SymbolToken>(cur_token)) {
        return GetHeap().Make<Symbol>(std::get<SymbolToken>(cur_token).name);
    } else if (std::holds_alternative<ConstantToken>(cur_token)) {
        return GetHeap().Make<Number>(std::get<ConstantToken>(cur_token).value);
    } else if (std::holds_alternative<QuoteToken>(cur_token)) {
        if (tokenizer->IsEnd()) {
            throw SyntaxError("Expected expression after quote");
        }
        auto inner_elem = RecursiveRead(tokenizer);
        auto root = GetHeap().Make<Cell>();
        As<Cell>(root)->SetFirst(GetHeap().Make<Symbol>(kQuoteStr));
        As<Cell>(root)->SetSecond(GetHeap().Make<Cell>());
        As<Cell>(As<Cell>(root)->GetSecond())->SetFirst(inner_elem);
        return root;
    } else {
        throw SyntaxError("Wrong syntax");
    }
}

AST Read(Tokenizer* tokenizer) {
    auto res = RecursiveRead(tokenizer);
    if (!tokenizer->IsEnd()) {
        throw SyntaxError("Extra symbols in input stream");
    }
    return res;
}