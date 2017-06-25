#include "TestAPI.h"
#include <mgpcl/ProgramArgs.h>

Declare Test("pargs"), Priority(6.0);

TEST
{
	volatile StackIntegrityChecker sic;
	const char *argv[] = { "ignore_me.exe", "-s", "--arg-1", "42", "--arg-1", "666", "--arg-2", "1", "2", "3", "--arg-3", "test", "rem1", "rem2" };

	m::ProgramArgs pa(14, argv);
	m::ArgDescriptor &a1 = pa.add("--arg-1", m::kAT_Single).setNumeric().setOptional(false).setUnique(false).setHelpText("Changes arg1");
	m::ArgDescriptor &a2 = pa.add("--arg-2", m::kAT_Triple).setNumeric().setHelpText("Changes arg2");
	m::ArgDescriptor &a3 = pa.add("--arg-3", m::kAT_Single).setDefault("gayben").setHelpText("Changes arg3");
	m::ArgDescriptor &a4 = pa.add("--arg-4", m::kAT_Single).setDefault("woops!").setHelpText("Changes arg4");
	m::ArgDescriptor &p1 = pa.add("--sw-1", m::kAT_Switch).addAlias("-s").setHelpText("Toggles switch 1");
	m::ArgDescriptor &p2 = pa.add("--sw-2", m::kAT_Switch).setHelpText("Toggles switch 2");
	pa.addHelpSwitch("--help").addAlias("-h").setHelpText("Displays this text");

	pa.setHelpHeader("Hello this is gaben!");
	//pa.printHelp();

	if(pa.parse() != m::kAPE_NoError) {
		if(pa.erroringDescriptor() == nullptr)
			std::cout << "[!]\tParse error on unknown descriptor, probably unknown arg was found!" << std::endl;
		else
			std::cout << "[!]\tParse error on descriptor \"" << pa.erroringDescriptor()->name().raw() << "\", error is: " << pa.erroringDescriptor()->lastError() << std::endl;

		return false;
	}

	testAssert(a1.valueCount() == 2 && a1.value(0).asInt() == 42 && a1.value(1).asInt() == 666, "Invalid param 1");
	testAssert(a2.valueCount() == 1 && a2.value().asVector3i() == m::Vector3i(1, 2, 3), "Invalid param 2");
	testAssert(a3.valueCount() == 1 && a3.value().asString() == "test", "Invalid param 3");
	testAssert(!a4.isSet() && a4.value().asString() == "woops!", "Invalid param 4");
	testAssert(p1.isSet() && p1.value().asBool(), "Invalid switch 1");
	testAssert(!p2.isSet() && !p2.value().asBool(), "Invalid switch 2");

	testAssert(pa.valueCount() == 7, "Invalid value count");
	testAssert(pa.remainingArgsBegin() == 5, "Invalid remaining args begin pos");
	testAssert(pa.value(5).asString() == "rem1", "Invalid remaining arg 1");
	testAssert(pa.value(6).asString() == "rem2", "Invalid remaining arg 1");
	testAssert(pa.executablePath() == "ignore_me.exe", "Invalid executable path");

	//Let's try again
	pa.setAcceptsRemainingArgs(false);
	testAssert(pa.parse() == m::kAPE_UnknownArgFound && pa.unrecongnizedArgument() == "rem1", "Invalid argument NOT found");
	return true;
}
