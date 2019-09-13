//
// Created by Dmitro LITVINENKO on 2019-05-15.
//

#include "Lexer.hpp"
#include <sstream>
#include <limits>

const char* const       Lexer::integralArgTypes[]   = { "int8", "int16",
                                                        "int32", "int64"        };

const char* const       Lexer::floatingArgTypes[]   = { "float", "double"       };

const char* const       Lexer::instrWithArg[]       = { "push", "assert"        };

const char* const       Lexer::instrWithoutArg[]    = { "pop", "dump", "add",
                                                        "sub", "mul", "div",
                                                        "mod", "print", "exit"  };

static void checkIntegral(  std::string::iterator begin, const char *underLimit, const char *upperLimit)
{
    bool    isNegative = false;

    if (*begin == '-') isNegative = true;
    if (*begin == '-' || *begin == '+') begin++;

    const char *argContent = begin.base();

    auto    limitLen = std::strlen(isNegative ? underLimit : upperLimit),
            contentLen = std::strlen(argContent);

    if (contentLen < limitLen)
        return ;

    else if (contentLen == limitLen && strcmp(argContent, isNegative ? underLimit : upperLimit) <= 0)
        return ;

    else
    {
        if (isNegative)
            throw Lexer::UnderflowErrorException();
        else
            throw Lexer::OverflowErrorException();
    }
}

static void checkInt8(std::string argContent) { checkIntegral(argContent.begin(), "128", "127"); }

static void checkInt16(std::string argContent) { checkIntegral(argContent.begin(), "32768", "32767"); }

static void checkInt32(std::string argContent) { checkIntegral(argContent.begin(), "2147483648", "2147483647"); }

static void checkInt64(std::string argContent) { checkIntegral(argContent.begin(), "9223372036854775808", "9223372036854775807"); }

static void checkFloat(std::string argContent)
{
    std::stringstream   stream(argContent);

    float fValue;

    stream >> fValue;

    if (fValue == std::numeric_limits<float>::infinity())
        throw Lexer::OverflowErrorException();
    if (fValue == -(std::numeric_limits<float>::infinity()))
        throw Lexer::UnderflowErrorException();

    if (stream.fail())
        throw Lexer::BadArgumentException();
}

static void checkDouble(std::string argContent)
{
    std::stringstream   stream(argContent);

    double dValue;

    stream >> dValue;

    if (dValue == std::numeric_limits<double>::infinity())
        throw Lexer::OverflowErrorException();
    if (dValue == -(std::numeric_limits<double>::infinity()))
        throw Lexer::UnderflowErrorException();

    if (stream.fail())
        throw Lexer::BadArgumentException();
}

void    (*Lexer::checkLimit[6])(std::string) = {    checkInt8,
                                                    checkInt16,
                                                    checkInt32,
                                                    checkInt64,
                                                    checkFloat,
                                                    checkDouble  };

void Lexer::setVmStream(std::istream *vmStream)
{
    vmStream_ = vmStream;
}

Lexer::Lexer(std::list<std::unique_ptr<instruction_t> > &instrList, std::istream *stream)
    : instrList(instrList), vmStream_(stream) {}

std::string Lexer::getIntegralContent(std::string::iterator it, std::string::iterator end)
{
    std::string content;

    while (std::iswspace(*it)) it++;

    if (*it == '-' || *it == '+') content += *it++;

    while (!std::iswspace(*it) && *it != ')' && it < end)
    {
        if (!std::isdigit(*it))
            throw BadArgumentException();
        content += *it++;
    }

    while (std::iswspace(*it)) it++;

    if (*it != ')')
        throw BadArgumentException();
    it++;

    while (std::iswspace(*it)) it++;

    if (it != end)
        throw ExtraSymbolException();

    return content;
}

std::string Lexer::getFloatingContent(std::string::iterator it, std::string::iterator end)
{
    size_t      dotCount = 0;
    std::string content;

    while (std::iswspace(*it)) it++;

    if (*it == '-' || *it == '+') content += *it++;

    while (!std::iswspace(*it) && *it != ')' && it < end)
    {
        if (*it == '.')
        {
            if (dotCount)
                throw BadArgumentException();
            else
                dotCount++;
        }
        else if (!std::isdigit(*it) && dotCount)
            throw BadArgumentException();
        content += *it++;
    }

    while (std::iswspace(*it)) it++;

    if (*it != ')')
        throw BadArgumentException();
    it++;

    while (std::iswspace(*it)) it++;

    if (it != end)
        throw ExtraSymbolException();

    return content;
}

arg_t*   Lexer::getArg(std::string::iterator it, std::string::iterator end)
{
    if (it == end) throw MissingArgumentException();

    std::string argTypeStr;
    int         argTypeNb = 0;

    while (it < end && !std::iswspace(*it) && *it != '(') argTypeStr += *it++;

    while (std::iswspace(*it)) it++;

    if (it == end || *it != '(') throw BadArgumentException();

    for (const char* tmp : integralArgTypes)
    {
        if (argTypeStr == tmp) {
            std::string argString = getIntegralContent(it + 1, end);
            checkLimit[argTypeNb](argString);
            return new arg_t(argTypeNb, argString);
        }
        argTypeNb++;
    }

    for (const char* tmp : floatingArgTypes)
    {
        if (argTypeStr == tmp) {
            std::string argString = getFloatingContent(it + 1, end);
            Lexer::checkLimit[argTypeNb](argString);
            return new arg_t(argTypeNb, argString);
        }
        argTypeNb++;
    }
    throw UnknownArgumentTypeException();
}

void Lexer::collectInstr(std::string &line)
{
    std::string             instrName;
    std::string::iterator   it = line.begin();
    std::string::iterator   end = line.end();

    while (std::iswspace(*it)) it++;
    while (!std::iswspace(*it) && it < end) instrName += *it++;
    while (std::iswspace(*it)) it++;

    for (const char* tmp : instrWithArg)
        if (instrName == tmp)
        {
            arg_t   *argument = getArg(it, end);
            instrList.push_back(std::unique_ptr<instruction_t>(new instruction_t(tmp, argument)));
            return ;
        }
    for (const char* tmp : instrWithoutArg)
        if (instrName == tmp)
        {
            while (std::iswspace(*it)) it++;
            if (it != end)
                throw Lexer::ExtraSymbolException();
            instrList.push_back(std::unique_ptr<instruction_t>(new instruction_t(tmp)));
            return ;
        }
    throw Lexer::UnknownInstructionException();
}

void Lexer::readBuf( void )
{
    bool        endRead = false;
    size_t      commentPosition = 0;
    size_t      lineNb = 0;
    std::string line;

    while (!endRead)
    {
        endRead = (std::getline(*vmStream_, line).eof());
        lineNb++;

        try
        {
            if ((commentPosition = line.find(';')) != std::string::npos
                && line[commentPosition + 1] != ';')
                line.resize(commentPosition);
            else if (commentPosition != std::string::npos)
            {
                line.resize(commentPosition);

                if (!std::all_of(line.begin(), line.end(), isspace))
                    collectInstr(line);
                break ;
            }
            if (!std::all_of(line.begin(), line.end(), isspace))
                collectInstr(line);
        }
        catch (std::exception & error)
        {
            std::cerr << "Error on line " << lineNb << " " << error.what() << std::endl;
            AVM::lexerError = true;
        }
    }
}

const char *Lexer::OverflowErrorException::what() const throw() {
    return "overflow on argument!";
}

const char *Lexer::MissingArgumentException::what() const throw() {
    return "missing argument!";
}

const char *Lexer::ExtraSymbolException::what() const throw() {
    return "extra symbols after argument or instruction!";
}

const char *Lexer::UnderflowErrorException::what() const throw() {
    return "underflow on argument!";
}

const char *Lexer::UnknownArgumentTypeException::what() const throw() {
    return "unknown argument type!";
}

const char *Lexer::BadArgumentException::what() const throw() {
    return "bad argument!";
}

const char *Lexer::UnknownInstructionException::what() const throw() {
    return "unknown instruction!";
}

const char *Lexer::DivisionByZeroException::what() const throw() {
    return "division by zero !";
}
