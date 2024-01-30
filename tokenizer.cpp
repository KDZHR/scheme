#include <tokenizer.h>
#include <error.h>

const std::string kGoodChars = "<=>*/#!+-()";
const std::string kSymBody = "<=>*/#?!-";
const std::string kSymStart = "<=>*/#+-";

bool IsGoodChar(int sym) {
    if (sym == EOF) {
        return false;
    }
    return isalnum(sym) || kGoodChars.find(sym) != std::string::npos;
}

bool IsSymBody(int sym) {
    if (sym == EOF) {
        return false;
    }
    return isalnum(sym) || kSymBody.find(sym) != std::string::npos;
}

bool IsSymStart(int sym) {
    if (sym == EOF) {
        return false;
    }
    return isalpha(sym) || kSymStart.find(sym) != std::string::npos;
}

bool SymbolToken::operator==(const SymbolToken& other) const {
    return name == other.name;
}

bool QuoteToken::operator==(const QuoteToken&) const {
    return true;
}

bool DotToken::operator==(const DotToken&) const {
    return true;
}

bool ConstantToken::operator==(const ConstantToken& other) const {
    return value == other.value;
}

void Tokenizer::SkipEmptyChars() {
    while (!in_->eof() && (in_->peek() == ' ' || in_->peek() == '\n')) {
        in_->get();
    }
}

Tokenizer::Tokenizer(std::istream* in) : in_(in), is_end_(false), without_token_(true) {
}

bool Tokenizer::IsEnd() {
    if (without_token_) {
        NextOnce();
    }
    return is_end_;
}

void Tokenizer::NextOnce() {
    SkipEmptyChars();
    without_token_ = false;
    if (in_->eof()) {
        is_end_ = true;
        return;
    }
    auto x = in_->get();
    if (isdigit(x) || ((x == '+' || x == '-') && isdigit(in_->peek()))) {
        bool is_neg = (x == '-');
        int64_t val = (isdigit(x) ? x - '0' : 0);
        while (isdigit(in_->peek())) {
            val = 10 * val + (in_->get() - '0') * (is_neg ? -1 : 1);
        }
        cur_token_ = Token{ConstantToken{val}};
    } else if (x == '(' || x == ')') {
        cur_token_ = Token{x == '(' ? BracketToken::OPEN : BracketToken::CLOSE};
    } else if (x == '\'') {
        cur_token_ = Token{QuoteToken{}};
    } else if (x == '.') {
        cur_token_ = Token{DotToken{}};
    } else {
        if (!IsSymStart(x)) {
            throw SyntaxError("Bad symbolic start");
        }
        std::string cur_str(1, x);
        while (IsSymBody(in_->peek())) {
            cur_str.push_back(in_->get());
        }
        cur_token_ = Token{SymbolToken{cur_str}};
    }
}

void Tokenizer::Next() {
    if (without_token_) {
        NextOnce();
    }
    NextOnce();
}
Token Tokenizer::GetToken() {
    if (without_token_) {
        NextOnce();
    }
    return cur_token_;
}
