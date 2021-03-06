#include "TestAPI.h"
#include <string>
#include <mgpcl/Process.h>
#include <mgpcl/StringIOStream.h>

#define TEST_USER_NAME "montoyo"
#define TEST_GROUP_NAME "sudo"

Declare Test("processes"), Priority(11.0);

TEST
{
    volatile StackIntegrityChecker sic;
    m::String cmdLine;
    m::List<m::ProcessInfo> infos;

    testAssert(m::Process::enumerateProcesses(infos), "could not enumerate processes");

    for(m::ProcessInfo &pi : infos) {
        if(!pi.commandName().startsWith("chrom"_m)) { //Chrome, your dick is too big!
            if(pi.commandLine(cmdLine) && cmdLine.length() > 0)
                std::cout << "[i]\t>> Command line of \"" << pi.commandName().raw() << "\" is \"" << cmdLine.raw() << '\"' << std::endl;
        }
    }

    return true;
}

DISABLED_TEST
{
    volatile StackIntegrityChecker sic;
    
    while(true) {
        std::cout << std::endl << "[>]\tPrompt: ";
        std::string str;
        std::getline(std::cin, str);

        if(str == "STOP")
            break;

        m::String mstr(str.c_str(), static_cast<int>(str.length()));
        m::List<m::String> args;
        m::parseArgs(mstr, args, false);

        for(int i = 0; i < args.size(); i++)
            std::cout << "[>]\tArg " << i + 1 << " = \"" << args[i].raw() << '\"' << std::endl;
    }

    return true;
}

TEST
{
    volatile StackIntegrityChecker sic;

    m::String test("blahblah"_m);
    m::Process proc;
    testAssert(proc.setExecutable(m::String(exeLoc)).pushArg("--print-env"_m).setEnv("MGPCL_TEST"_m, test).redirectSTDIO().start().hasStarted(), "process didn't start");

    m::StringOStream sos; // ...---...
    m::SSharedPtr<m::PipeInputStream> pis(proc.stdOut<m::RefCounter>());
    testAssert(m::IO::transfer(&sos, pis.ptr(), 64), "couldn't read stdout!");
    pis->close();

    testAssert(sos.data().trimmed() == test, "result didn't match input");
    return true;
}

TEST
{
    volatile StackIntegrityChecker sic;

    m::String test("some text"_m);
    m::Process proc;
    testAssert(proc.setExecutable(m::String(exeLoc)).pushArg("--print-hash"_m).redirectSTDIO().start().hasStarted(), "process didn't start");

    m::StringOStream sos;
    m::SSharedPtr<m::PipeOutputStream> pos(proc.stdIn<m::RefCounter>());
    m::SSharedPtr<m::PipeInputStream> pis(proc.stdOut<m::RefCounter>());
    testAssert(m::IO::writeLine(test, pos.ptr()), "couldn't write to stdin!");
    testAssert(m::IO::transfer(&sos, pis.ptr(), 64), "couldn't read stdout!");
    pos->close();
    pis->close();

    testAssert(sos.data().trimmed().toInteger() == test.hash(), "result is wrong");
    return true;
}

#ifdef MGPCL_LINUX
TEST
{
    volatile StackIntegrityChecker sic;
    uid_t someUID;
    gid_t someGID;

    testAssert(!m::linux::getUserUID("jaaj"_m, someUID), "user jaaj shouldn't exist");
    testAssert(!m::linux::getGroupGID("jaaj"_m, someGID), "group jaaj shouldn't exist");
    testAssert(m::linux::getUserUID(TEST_USER_NAME, someUID), "user " TEST_USER_NAME " should exist");
    testAssert(m::linux::getGroupGID(TEST_GROUP_NAME, someGID), "group " TEST_GROUP_NAME " should exist");

    std::cout << "[i]\tCurrent UID is " << m::linux::getCurrentUID() << std::endl;
    std::cout << "[i]\tCurrent GID is " << m::linux::getCurrentGID() << std::endl;
    std::cout << "[i]\tUser ID of montoyo is " << someUID << std::endl;
    std::cout << "[i]\tGroup ID of sudo is " << someGID << std::endl;

    m::Process proc;
    proc.setExecutable("whoami"_m).redirectSTDIO();
    m::linux::setProcessUID(proc, someUID);

    testAssert(proc.start().hasStarted(), "process didn't start");

    m::StringOStream sos;
    m::SSharedPtr<m::PipeInputStream> pis(proc.stdOut<m::RefCounter>());
    testAssert(m::IO::transfer(&sos, pis.ptr(), 64), "couldn't read stdout!");
    pis->close();

    testAssert(sos.data().trimmed() == "montoyo"_m, "whoami should have printed montoyo");
    return true;
}
#endif
