#ifndef OPERAND_HPP
# define OPERAND_HPP

#include "AVM.hpp"
#include "Lexer.hpp"
#include <sstream>
#include <cmath>

template <typename T>
struct Operand : IOperand
{

    Operand(std::string const & stringValue, int precision) : strValue_(stringValue), precision_(precision)
    {
        std::stringstream stream(stringValue);

        long double value;  stream >> value;

        argValue_ = value;
    }

    int                 getPrecision()  const override { return precision_;                             }
    eOperandType        getType()       const override { return static_cast<eOperandType>(precision_);  }
    std::string const&  toString()      const override { return strValue_;                              }

    bool        operator==(IOperand const & other) const override
    {
        std::stringstream   stream(other.toString());
        T           tmp;    stream >> tmp;

        return tmp == argValue_;
    }

    IOperand const *    operator+   ( IOperand const & ) const override;
    IOperand const *    operator-   ( IOperand const & ) const override;
    IOperand const *    operator*   ( IOperand const & ) const override;
    IOperand const *    operator/   ( IOperand const & ) const override;
    IOperand const *    operator%   ( IOperand const & ) const override;

    ~Operand()  = default;

private:

    Operand()   = default;

    T           argValue_;
    std::string strValue_;
    int         precision_;

};

template<>
bool        Operand<int8_t>::operator==(IOperand const & other) const
{
    return toString() == other.toString();
}

template<typename T>
IOperand const *    Operand<T>::operator+   ( IOperand const & other) const
{
    long double         rightArgument;
    T                   tmp;
    std::stringstream   stream(other.toString());
                        stream >> rightArgument;

    tmp = rightArgument; tmp += argValue_;

    if (rightArgument > 0 && argValue_ > 0 && tmp < 0)
        throw Lexer::OverflowErrorException();
    if (rightArgument < 0 && argValue_ < 0 && tmp < 0)
        throw Lexer::UnderflowErrorException();
    
    return AVM::createOperand(static_cast<eOperandType>(precision_), std::to_string(tmp));
}

template<typename T>
IOperand const *    Operand<T>::operator-   ( IOperand const & other) const
{
    long double         rightArgument;
    T                   tmp;
    std::stringstream   stream(other.toString());
                        stream >> rightArgument;

    tmp = argValue_; tmp -= rightArgument;
    
    if (argValue_ > 0 && rightArgument < 0 && tmp < 0)
        throw Lexer::OverflowErrorException();
    if (argValue_ < 0 && rightArgument > 0 && tmp > 0)
        throw Lexer::UnderflowErrorException();

    return AVM::createOperand(static_cast<eOperandType>(precision_), std::to_string(tmp));
}

template<typename T>
IOperand const *    Operand<T>::operator*   ( IOperand const & other ) const
{
    long double rightArgument;
    T tmp;
    std::stringstream   stream(other.toString());
    stream >> rightArgument;

    tmp = rightArgument; tmp *= argValue_;

    if ((rightArgument < 0 && argValue_ < 0 && tmp / rightArgument != argValue_)
    ||  (rightArgument > 0 && argValue_ > 0 && tmp / rightArgument != argValue_ ))
        throw Lexer::OverflowErrorException();
    if ((argValue_ < 0 || rightArgument < 0) && (tmp / rightArgument != argValue_))
        throw Lexer::UnderflowErrorException();

    return AVM::createOperand(static_cast<eOperandType>(precision_), std::to_string(tmp));
}

template<typename T>
IOperand const *    Operand<T>::operator/   ( IOperand const & other ) const
{
    int64_t rightArgument;  std::stringstream   stream(other.toString());
                                                stream >> rightArgument;

    if (rightArgument == 0)
        throw Lexer::DivisionByZeroException();

    return AVM::createOperand(static_cast<eOperandType>(precision_), std::to_string((int64_t)(argValue_ / rightArgument)));
}

template<typename T>
IOperand const *    Operand<T>::operator%   ( IOperand const & other ) const
{
    int64_t rightArgument;  std::stringstream   stream(other.toString());
                                                stream >> rightArgument;

    if (rightArgument == 0)
        throw Lexer::DivisionByZeroException();

    return AVM::createOperand(static_cast<eOperandType>(precision_), std::to_string(argValue_ % rightArgument));
}

template<>
IOperand const *    Operand<float>::operator%   ( IOperand const & other ) const
{
    float   rightArgument;  std::stringstream   stream(other.toString());
                                                stream >> rightArgument;

    if (rightArgument == 0.0f)
        throw Lexer::DivisionByZeroException();

    return AVM::createOperand(static_cast<eOperandType>(precision_), std::to_string(fmod(argValue_, rightArgument)));
}

template<>
IOperand const *    Operand<double>::operator%   ( IOperand const & other ) const
{
    double   rightArgument;  std::stringstream  stream(other.toString());
                                                stream >> rightArgument;

    if (rightArgument == 0.0)
        throw Lexer::DivisionByZeroException();

    return AVM::createOperand(static_cast<eOperandType>(precision_), std::to_string(fmod(argValue_, rightArgument)));
}

template<>
IOperand const *    Operand<float>::operator/   ( IOperand const & other ) const
{
    float   rightArgument, tmp; std::stringstream   stream(other.toString());
                                                    stream >> rightArgument;

    if (rightArgument == 0.0f)
        throw Lexer::DivisionByZeroException();

    tmp = argValue_ / rightArgument;

    if (tmp == std::numeric_limits<float>::infinity())
        throw Lexer::OverflowErrorException();
    if (tmp == -std::numeric_limits<float>::infinity())
        throw Lexer::UnderflowErrorException();

    return AVM::createOperand(eOperandType::Float, std::to_string(tmp));
}

template<>
IOperand const *    Operand<double>::operator/   ( IOperand const & other ) const
{
    double   rightArgument, tmp; std::stringstream  stream(other.toString());
                                                    stream >> rightArgument;

    if (rightArgument == 0.0)
        throw Lexer::DivisionByZeroException();

    tmp = argValue_ / rightArgument; stream << tmp;

    if (tmp == std::numeric_limits<double>::infinity())
        throw Lexer::OverflowErrorException();
    if (tmp == -std::numeric_limits<double>::infinity())
        throw Lexer::UnderflowErrorException();

    return AVM::createOperand(eOperandType::Double, std::to_string(tmp));
}

template<>
IOperand const *    Operand<float>::operator*   ( IOperand const & other ) const
{
    float   rightArgument, tmp; std::stringstream   stream(other.toString());
                                                    stream >> rightArgument;

    tmp = argValue_ * rightArgument;

    if (tmp == std::numeric_limits<float>::infinity())
        throw Lexer::OverflowErrorException();
    if (tmp == -std::numeric_limits<float>::infinity())
        throw Lexer::UnderflowErrorException();

    return AVM::createOperand(eOperandType::Float, std::to_string(tmp));
}

template<>
IOperand const *    Operand<double>::operator*   ( IOperand const & other ) const
{
    double   rightArgument, tmp; std::stringstream  stream(other.toString());
                                                    stream >> rightArgument;

    tmp = argValue_ * rightArgument;

    if (tmp == std::numeric_limits<double>::infinity())
        throw Lexer::OverflowErrorException();
    if (tmp == -std::numeric_limits<double>::infinity())
        throw Lexer::UnderflowErrorException();

    return AVM::createOperand(eOperandType::Double, std::to_string(tmp));
}

#endif
