//
// Created by Dmitro LITVINENKO on 2019-05-15.
//

#ifndef AVM_LEXER_HPP
#define AVM_LEXER_HPP

#include <fstream>
#include <iostream>
#include <list>
#include <string>
#include <cstring>
#include <algorithm>
#include "AVM.hpp"

struct  arg_t
{
    int             type;
    std::string     content;

    arg_t(int type, std::string & content)
        : type(type), content(std::move(content)) {}
};

struct  instruction_t
{
    std::string *name;
    arg_t       *arg;

    explicit instruction_t(const char *name, arg_t *arg = 0)
        : name(new std::string(name)), arg(arg) {}
    ~instruction_t() { delete name; delete arg; }
};

struct Lexer
{

    struct DivisionByZeroException : std::exception
    {
        DivisionByZeroException() = default;
        ~DivisionByZeroException() throw() = default;
        DivisionByZeroException&operator=(DivisionByZeroException&) = delete;
        const char * what() const throw();
    };

    struct UnknownInstructionException : std::exception
    {
        UnknownInstructionException() = default;
        ~UnknownInstructionException() throw() = default;
        UnknownInstructionException&operator=(UnknownInstructionException&) = delete;
        const char * what() const throw();
    };

    struct OverflowErrorException : std::exception
    {
        OverflowErrorException() = default;
        ~OverflowErrorException() throw() = default;
        OverflowErrorException&operator=(OverflowErrorException&) = delete;
        const char * what() const throw();
    };

    struct UnderflowErrorException : std::exception
    {
        UnderflowErrorException() = default;
        ~UnderflowErrorException() throw() = default;
        UnderflowErrorException&operator=(UnderflowErrorException&) = delete;
        const char * what() const throw();
    };

    struct BadArgumentException : std::exception
    {
        BadArgumentException() = default;
        ~BadArgumentException() throw() = default;
        BadArgumentException&operator=(BadArgumentException&) = delete;
        const char * what() const throw();
    };

    struct MissingArgumentException : std::exception
    {
        MissingArgumentException() = default;
        ~MissingArgumentException() throw() = default;
        MissingArgumentException&operator=(MissingArgumentException&) = delete;
        const char * what() const throw();
    };

    struct UnknownArgumentTypeException : std::exception
    {
        UnknownArgumentTypeException() = default;
        ~UnknownArgumentTypeException() throw() = default;
        UnknownArgumentTypeException&operator=(UnknownArgumentTypeException&) = delete;
        const char * what() const throw();
    };

    struct ExtraSymbolException : std::exception
    {
        ExtraSymbolException() = default;
        ~ExtraSymbolException() throw() = default;
        ExtraSymbolException&operator=(ExtraSymbolException&) = delete;
        const char * what() const throw();
    };

    Lexer(std::list<std::unique_ptr<instruction_t> >  &instrList, std::istream *stream = 0);

    void                                        setVmStream(std::istream *vmStream);
    void                                        readBuf();

    ~Lexer()                                    = default;

    Lexer &operator = (const Lexer &object)     = delete;

    static const char* const                    instrWithArg[];
    static const char* const                    instrWithoutArg[];
    static const char* const                    floatingArgTypes[];
    static const char* const                    integralArgTypes[];

private:

    Lexer()                                     = default;

    void                                        collectInstr(std::string & line);
    arg_t                                       *getArg(std::string::iterator it, std::string::iterator end);
    std::string                                 getIntegralContent(std::string::iterator it, std::string::iterator end);
    std::string                                 getFloatingContent(std::string::iterator it, std::string::iterator end);

    std::list<std::unique_ptr<instruction_t> >  &instrList;
    std::istream                                *vmStream_;

    static void                                 (*checkLimit[6])(std::string);

};

#endif //AVM_LEXER_HPP
