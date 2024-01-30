#include <iostream>
#include <scheme.h>

Interpreter interpreter;
void Eval(const std::string &s) {
    std::cout << interpreter.Run(s) << '\n';
}

int main() {
    //    std::cout << interpreter.Run("(define slow-add (lambda (x y) (if (= x 0) y (slow-add (- x 1) (+ y 1)))))");
    //    std::cout << interpreter.Run("(slow-add 3 3)");
//    Eval("(+ 1 2)");
    Eval(R"EOF(
        (define (foo x)
            (define (bar) (set! x (+ (* x 2) 2)) x)
            bar
        )
    )EOF");
    Eval("(define my-foo (foo 20))");
    Eval("(define foo 1543)");
    Eval("(my-foo)");
    return 0;
}
