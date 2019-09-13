#ifndef AVM_HPP
# define AVM_HPP

#include "IOperand.hpp"
#include <stack>
#include <map>
#include <vector>
#include <memory>

struct instruction_t;

class AVM
{

    AVM();
    AVM(AVM const &) = default;
    AVM & operator = (AVM const &) = delete;

    static IOperand const * createInt8     ( std::string const & value );
    static IOperand const * createInt16    ( std::string const & value );
    static IOperand const * createInt32    ( std::string const & value );
    static IOperand const * createInt64    ( std::string const & value );
    static IOperand const * createFloat    ( std::string const & value );
    static IOperand const * createDouble   ( std::string const & value );

    std::vector<IOperand const *>      vmStack;

    static std::vector<IOperand const*(*)(std::string const & value)>       operandFactory;

    std::map<std::string, void (AVM::*)(void)>                              funcWithoutArgs;
    std::map<std::string, void (AVM::*)(eOperandType, std::string const &)> funcWithArgs;

public:

    static IOperand const* createOperand  ( eOperandType type, std::string const & value );

    void    push    ( eOperandType type, std::string const & value );
    void    assertVM( eOperandType type, std::string const & value );

    void    pop     ( void );
    void    dump    ( void );
    void    add     ( void );
    void    sub     ( void );
    void    mul     ( void );
    void    div     ( void );
    void    mod     ( void );
    void    print   ( void );
    void    exit    ( void );

    void    runInstruction(instruction_t *);

    static AVM  vm;
    static bool lexerError;
    static bool exitFlag;

};

std::ostream&operator<<(std::ostream &, IOperand const *);

#endif
