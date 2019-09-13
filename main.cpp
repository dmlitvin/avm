#include "Lexer.hpp"
#include "AVM.hpp"

int     main(int ac, char **av)
{
    std::ifstream                               file;
    std::list<std::unique_ptr<instruction_t> >  instructions;
    Lexer                                       lexer(instructions);

    if (ac == 2)
    {
        file.open(av[1]);
        lexer.setVmStream(&file);
        if (!file.is_open()) {
            std::cerr << "Error opening file!" << std::endl;
            return 0;
        }
    }
    else
        lexer.setVmStream(&std::cin);

    lexer.readBuf();

    auto revIt = instructions.rbegin();
    if (revIt == instructions.rend())
    {
        std::cerr << "missing exit" << std::endl;
        return 1;
    }
    if (*(revIt->get()->name) != "exit")
    {
        std::cerr << "Missing exit instruction !" << std::endl;
        AVM::exitFlag = true;
    }

    if (!AVM::lexerError)
    {
        for (std::unique_ptr<instruction_t> &value : instructions)
            if (!AVM::exitFlag)
                AVM::vm.runInstruction(value.get());
            else
                break;
    }
    // system("leaks avm");
    return 0;
}
