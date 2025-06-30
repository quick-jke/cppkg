#ifndef QUICK_CPPKG_COMAND_HPP
#define QUICK_CPPKG_COMAND_HPP

class CommandHandler {
public:
    virtual void execute() = 0;
    virtual ~CommandHandler() = default;
};

#endif
