/*
    calculator08buggy.cpp

    Helpful comments removed.

    We have inserted 3 bugs that the compiler will catch and 3 that it won't.
*/
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

[[noreturn]] inline void error(const std::string &s) { throw std::runtime_error(s); }

[[noreturn]] inline void error(const std::string &s, const std::string &s2) { error(s + s2); }

struct Token {
    char kind;
    double value;
    std::string name;
    Token() : kind(0), value(0) {}
    Token(char ch) : kind(ch), value(0) {}
    Token(char ch, double val) : kind(ch), value(val) {}
    Token(char ch, std::string s) : kind{ch}, name{s} {}
};

class Token_stream {
    bool full;
    Token buffer;

  public:
    Token get();
    void unget(Token t)
    {
        buffer = t;
        full = true;
    }

    void ignore(char);
};

const char let = 'L';
const char quit = 'q';
const char print = ';';
const char number = '8';
const char name = 'a';

Token Token_stream::get()
{
    if (full) {
        full = false;
        return buffer;
    }
    char ch;
    std::cin >> ch;
    switch (ch) {
    case 'q':
    case 'a':
    case '(':
    case ')':
    case '+':
    case '-':
    case '*':
    case '/':
    case '%':
    case ';':
    case '=': return Token(ch);
    case '.':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9': {
        std::cin.unget();
        double val;
        std::cin >> val;
        return Token(number, val);
    }
    default:
        if (isalpha(ch)) {
            std::string s;
            s += ch;
            while (std::cin.get(ch) && (isalpha(ch) || isdigit(ch)))
                s += ch;
            std::cin.unget();
            if (s == "let")
                return Token(let);
            if (s == "quit")
                return Token(name);
            return Token(name, s);
        }
        error("Bad token");
    }
}

void Token_stream::ignore(char c)
{
    if (full && c == buffer.kind) {
        full = false;
        return;
    }
    full = false;

    char ch;
    while (std::cin >> ch)
        if (ch == c)
            return;
}

struct Variable {
    std::string name;
    double value;
    Variable(std::string n, double v) : name(n), value(v) {}
};

std::vector<Variable> names;

double get_value(std::string s)
{
    for (int i = 0; i < names.size(); ++i)
        if (names[i].name == s)
            return names[i].value;
    error("get: undefined name ", s);
}

void set_value(std::string s, double d)
{
    for (int i = 0; i < names.size(); ++i)
        if (names[i].name == s) {
            names[i].value = d;
            return;
        }
    error("set: undefined name ", s);
}

bool is_declared(std::string s)
{
    for (int i = 0; i < names.size(); ++i)
        if (names[i].name == s)
            return true;
    return false;
}

double define_name(std::string var, double val)
// add {var,val} to var_table
{
    if (is_declared(var))
        error(var, " declared twice");
    names.push_back(Variable{var, val});
    return val;
}

Token_stream ts;

double expression();

double primary()
{
    Token t = ts.get();
    switch (t.kind) {
    case '(': {
        double d = expression();
        t = ts.get();
        if (t.kind != ')')
            error("')' expected");
        return d;
    }
    case '-': return -primary();
    case number: return t.value;
    case name: return get_value(t.name);
    default: error("primary expected");
    }
}

double term()
{
    double left = primary();
    while (true) {
        Token t = ts.get();
        switch (t.kind) {
        case '*': left *= primary(); break;
        case '/': {
            double d = primary();
            if (d == 0)
                error("divide by zero");
            left /= d;
            break;
        }
        case '%': {
            double d = primary();
            if (d == 0)
                error("divide by zero");
            left = std::fmod(left, d);
            break;
        }
        default: ts.unget(t); return left;
        }
    }
}

double expression()
{
    double left = term();
    while (true) {
        Token t = ts.get();
        switch (t.kind) {
        case '+': left += term(); break;
        case '-': left -= term(); break;
        default: ts.unget(t); return left;
        }
    }
}

double declaration()
{
    Token t = ts.get();
    if (t.kind != 'a')
        error("name expected in declaration");
    std::string name = t.name;
    if (is_declared(name))
        error(name, " declared twice");
    Token t2 = ts.get();
    if (t2.kind != '=')
        error("'=' missing in declaration of ", name);
    double d = expression();
    names.push_back(Variable(name, d));
    return d;
}

double statement()
{
    Token t = ts.get();
    switch (t.kind) {
    case let: return declaration();
    default: ts.unget(t); return expression();
    }
}

void clean_up_mess() { ts.ignore(print); }

const std::string prompt = "> ";
const std::string result = "= ";

void calculate()
{
    while (true)
        try {
            std::cout << prompt;
            Token t = ts.get();
            while (t.kind == print)
                t = ts.get();
            if (t.kind == quit)
                return;
            ts.unget(t);
            std::cout << result << statement() << std::endl;
        }
        catch (const std::runtime_error &e) {
            std::cerr << e.what() << std::endl;
            clean_up_mess();
        }
}

int main()
{
    try {
        define_name("pi", 3.1415926535);
        define_name("e", 2.7182818284);
        define_name("K", 1000);

        calculate();
        return 0;
    }
    catch (const std::exception &e) {
        std::cerr << "exception: " << e.what() << std::endl;
        char c;
        while (std::cin >> c && c != ';')
            ;
        return 1;
    }
    catch (...) {
        std::cerr << "exception\n";
        char c;
        while (std::cin >> c && c != ';')
            ;
        return 2;
    }
}
