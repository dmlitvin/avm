#include "AVM.hpp"
#include "Lexer.hpp"
#include "Operand.hpp"

bool    AVM::lexerError = false;
bool	AVM::exitFlag = false;
std::vector<IOperand const*(*)(std::string const & value)>	AVM::operandFactory;
AVM		AVM::vm;

void	checkTypes(const IOperand *& left, const IOperand *& right)
{
	int	maxPrecision = std::max(left->getPrecision(), right->getPrecision());

	if (left->getPrecision() < maxPrecision)
	{
		IOperand const	*tmp = AVM::createOperand(static_cast<eOperandType>(maxPrecision), left->toString());

		delete left; left = tmp;
	}
	if (right->getPrecision() < maxPrecision)
	{
		IOperand const	*tmp = AVM::createOperand(static_cast<eOperandType>(maxPrecision), right->toString());

		delete right; right = tmp;
	}
}

AVM::AVM()
{

    using stringFuncArgPair = std::pair<std::string, void (AVM::*)(eOperandType, std::string const &)>;

    funcWithArgs.insert(stringFuncArgPair("push", &AVM::push));
    funcWithArgs.insert(stringFuncArgPair("assert", &AVM::assertVM));

    using stringFuncPair = std::pair<std::string, void (AVM::*)(void)>;

    funcWithoutArgs.insert(stringFuncPair("pop", &AVM::pop));
    funcWithoutArgs.insert(stringFuncPair("dump", &AVM::dump));
    funcWithoutArgs.insert(stringFuncPair("add", &AVM::add));
    funcWithoutArgs.insert(stringFuncPair("sub", &AVM::sub));
    funcWithoutArgs.insert(stringFuncPair("mul", &AVM::mul));
    funcWithoutArgs.insert(stringFuncPair("div", &AVM::div));
    funcWithoutArgs.insert(stringFuncPair("mod", &AVM::mod));
    funcWithoutArgs.insert(stringFuncPair("print", &AVM::print));
    funcWithoutArgs.insert(stringFuncPair("exit", &AVM::exit));

    operandFactory.push_back(&AVM::createInt8);
    operandFactory.push_back(&AVM::createInt16);
    operandFactory.push_back(&AVM::createInt32);
    operandFactory.push_back(&AVM::createInt64);
    operandFactory.push_back(&AVM::createFloat);
    operandFactory.push_back(&AVM::createDouble);

}

void    AVM::runInstruction(instruction_t *instr)
{
	if (instr->arg)
        (this->*funcWithArgs[*instr->name])(static_cast<eOperandType>(instr->arg->type), instr->arg->content);
    else
        (this->*funcWithoutArgs[*instr->name])();
}

IOperand const *AVM::createInt8(std::string const &value) {
    return new Operand<int8_t>(value, 0);
}

IOperand const *AVM::createInt16(std::string const &value) {
    return new Operand<int16_t>(value, 1);
}

IOperand const *AVM::createInt32(std::string const &value) {
    return new Operand<int32_t>(value, 2);
}

IOperand const *AVM::createInt64(std::string const &value) {
    return new Operand<int64_t>(value, 3);
}

IOperand const *AVM::createFloat(std::string const &value) {
    return new Operand<float>(value, 4);
}

IOperand const *AVM::createDouble(std::string const &value) {
    return new Operand<double>(value, 5);
}

IOperand const *AVM::createOperand(eOperandType type, std::string const &value) {
	return (operandFactory[static_cast<int>(type)])(value);
}

void AVM::exit() {
    exitFlag = true;
    std::cout << "machine stopping " << std::endl;
}

void AVM::push(eOperandType type, std::string const &value) {
    vmStack.push_back(AVM::createOperand(type, value));
}

void AVM::assertVM(eOperandType type, std::string const &value) {
    if (!vmStack.empty())
    {
		auto *ptr = AVM::createOperand(type, value);
        if (vmStack.back()->getType() == type
            && *vmStack.back() == *ptr)
            std::cout << "assert success" << std::endl;
        else {
			// std::cout << ptr->toString() << ' ' << vmStack.back()->toString() << std::endl;
            std::cerr << "assert failed !" << std::endl;
            AVM::exit();
        }
		delete ptr;
    }
    else
        std::cerr << "runtime error: stack is empty" << std::endl;
}

void AVM::pop() {
    if (vmStack.empty())
    {
        std::cerr << "the stack is empty !" << std::endl;
        AVM::exit();
    }
    else
        vmStack.pop_back();
}

std::ostream&operator<<(std::ostream & stream, IOperand const * operand)
{
    static const char* types[] = {"int8", "int16", "int32", "int64", "float", "double"};

    return stream << types[operand->getPrecision()] << '\t' << operand->toString();
}

void AVM::dump() {
    auto it = vmStack.rbegin(), end = vmStack.rend();

    if (vmStack.empty())
        std::cerr << "runtime error: empty stack" << std::endl;
    else
        while (it < end)
            std::cout << *it++ << std::endl;
}

void AVM::print()
{
	if (!vmStack.empty())
	{
		if (!vmStack.back()->getPrecision())
			std::cout << static_cast<char>(std::atoi(vmStack.back()->toString().c_str())) << std::endl;
		else
			std::cerr << "print_assert failed !" << std::endl;
	}
	else
	{
		std::cerr << "runtime error: empty stack" << std::endl;
		AVM::exit();
	}
}

void AVM::add() {
	if (vmStack.size() > 1)
	{
		IOperand const	*right = vmStack.back();
		vmStack.pop_back();
		IOperand const	*left = vmStack.back();
		vmStack.pop_back();
		checkTypes(left, right);
		try
		{
			vmStack.push_back(*left + *right);
		}
		catch (std::exception const & ex)
		{
			std::cout << "runtime error instruction add: " << ex.what() << std::endl;
			AVM::exit();
		}
		delete right;
		delete left;
	}
	else
	{
		std::cerr << "add failed, not enough arguments !" << std::endl;
		AVM::exit();
	}
}

void AVM::sub() {
	if (vmStack.size() > 1)
	{
		IOperand const	*right = vmStack.back();
		vmStack.pop_back();
		IOperand const	*left = vmStack.back();
		vmStack.pop_back();
		checkTypes(left, right);
		try
		{
			vmStack.push_back(*left - *right);
		}
		catch (std::exception const & ex)
		{
			std::cout << "runtime error instruction sub: " << ex.what() << std::endl;
			AVM::exit();
		}
		delete right;
		delete left;
	}
	else
	{
		std::cerr << "sub failed, not enough arguments !" << std::endl;
		AVM::exit();
	}
}

void AVM::mul() {
	if (vmStack.size() > 1)
	{
		IOperand const	*right = vmStack.back();
		vmStack.pop_back();
		IOperand const	*left = vmStack.back();
		vmStack.pop_back();
		checkTypes(left, right);
		try
		{
			vmStack.push_back(*left * *right);
		}
		catch (std::exception const & ex)
		{
			std::cout << "runtime error instruction mul: " << ex.what() << std::endl;
			AVM::exit();
		}
		delete right;
		delete left;
	}
	else
	{
		std::cerr << "mul failed, not enough arguments !" << std::endl;
		AVM::exit();
	}
}

void AVM::div() {
	if (vmStack.size() > 1)
	{
		IOperand const	*right = vmStack.back();
		vmStack.pop_back();
		IOperand const	*left = vmStack.back();
		vmStack.pop_back();
		checkTypes(left, right);
		try
		{
			vmStack.push_back(*left / *right);
		}
		catch (std::exception const & ex)
		{
			std::cout << "runtime error instruction div: " << ex.what() << std::endl;
			AVM::exit();
		}
		delete right;
		delete left;
	}
	else
	{
		std::cerr << "div failed, not enough arguments !" << std::endl;
		AVM::exit();
	}
}

void AVM::mod() {
	if (vmStack.size() > 1)
	{
		IOperand const	*right = vmStack.back();
		vmStack.pop_back();
		IOperand const	*left = vmStack.back();
		vmStack.pop_back();
		checkTypes(left, right);
		try
		{
			vmStack.push_back(*left % *right);
		}
		catch (std::exception const & ex)
		{
			std::cout << "runtime error instruction mod: " << ex.what() << std::endl;
			AVM::exit();
		}
		delete right;
		delete left;
	}
	else
	{
		std::cerr << "mod failed, not enough arguments !" << std::endl;
		AVM::exit();
	}
}
