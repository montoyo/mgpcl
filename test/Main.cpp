//Declare
#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include "TestAPI.h"
#include <mgpcl/ConsoleUtils.h>
#include <mgpcl/INet.h>

#ifdef MGPCL_WIN
#include <mgpcl/WinWMI.h>
#include <Shobjidl.h>
#endif

//Stolen from http://archive.jamisonjudd.com/aperture-science-logo/
//which was copied from a screen capture of the original aperture logo
//from the video game.
static const char g_aperture[] = 
"              .,-:;//;:=,\n"
"          . :H@@@MM@M#H/.,+%;,\n"
"       ,/X+ +M@@M@MM%=,-%HMMM@X/,\n"
"     -+@MM; $M@@MH+-,;XMMMM@MMMM@+-\n"
"    ;@M@@M- XM@X;. -+XXXXXHHH@M@M#@/.\n"
"  ,%MM@@MH ,@%=             .---=-=:=,.\n"
"  =@#@@@MX.,                -%HX$$%%%:;\n"
" =-./@M@M$                   .;@MMMM@MM:\n"
" X@/ -$MM/                    . +MM@@@M$\n"
",@M@H: :@:                    . =X#@@@@-\n"
",@@@MMX, .                    /H- ;@M@M=\n"
".H@@@@M@+,                    %MM+..%#$.\n"
" /MMMM@MMH/.                  XM@MH; =;\n"
"  /%+%$XHH@$=              , .H@@@@MX,\n"
"   .=--------.           -%H.,@@@@@MX,\n"
"   .%MM@@@HHHXX$$$%+- .:$MMX =M@@MM%.\n"
"     =XMMM@MM@MM#H;,-+HMM@M+ /MMMX=\n"
"       =%@M@M#@$-.=$@MM@@@M; %M%=\n"
"         ,:+$+-,/H#MMMMMMM@= =,\n"
"               =++%%%%+/:-.\n";

static TestObject randomTestObj()
{
	int sum = 0;
	for(int i = 0; i < 64; i++) {
		if(rand() % 2 == 0)
			sum += rand();
		else
			sum -= rand();
	}

	return TestObject(sum);
}

#include <mgpcl/Process.h>

static bool g_noSound = false;

static void playSound(const m::String &snd)
{
	if(g_noSound)
		return;

	m::Process proc;

#ifdef MGPCL_WIN
	m::String cmd("(New-Object Media.SoundPlayer \"");
	cmd += snd;
	cmd += "\").PlaySync();";

	proc.setExecutable("C:\\Windows\\System32\\WindowsPowerShell\\v1.0\\powershell.exe").pushArg("-c").pushArg(cmd);
#else
	proc.setExecutable("vlc").pushArg("--intf").pushArg("dummy").pushArg("--vout").pushArg("none").pushArg("--play-and-exit").pushArg(snd);
#endif

	proc.start();
}

static char rectChar(int x, int y, int w, int h)
{
#ifdef MGPCL_WIN
	if(x < 0 || y < 0)
		return ' ';
	else if(x == 0 && y == 0)
		return '\xC9';
	else if(x == w && y == 0)
		return '\xBB';
	else if(x == w && y == h)
		return '\xBC';
	else if(x == 0 && y == h)
		return '\xC8';
	else if(x == 0 || x == w)
		return '\xBA';
	else if(y == 0 || y == h)
		return '\xCD';
	else
		return ' ';
#else
	if(x < 0 || y < 0)
		return ' ';
	else if(x == 0 && y == 0)
		return '\x6C';
	else if(x == w && y == 0)
		return '\x6B';
	else if(x == w && y == h)
		return '\x6A';
	else if(x == 0 && y == h)
		return '\x6D';
	else if(x == 0 || x == w)
		return '\x78';
	else if(y == 0 || y == h)
		return '\x71';
	else
		return ' ';
#endif
}

static void drawCenteredRect(int rw, int rh)
{
	int sx = (m::console::getSize().x() - rw) / 2;
	int ex = sx + rw;

	m::console::setTextColor(m::kCC_White);

#ifdef MGPCL_LINUX
	std::cout << "\033(0";
#endif

	for(int y = 0; y <= rh; y++) {
		for(int x = 0; x <= ex; x++)
			std::cout << rectChar(x - sx, y, rw, rh);

		std::cout << std::endl;
	}

#ifdef MGPCL_LINUX
	std::cout << "\033(B";
#endif

	m::console::resetColor();
}

#define WINDOW_TITLE "Wheatley Laboratories"

int main(int argc, char *argv[])
{
	for(int i = 1; i < argc; i++) {
		if(strcmp(argv[i], "--print-test-objs") == 0)
			TestObject::setPrintDebug();
		else if(strcmp(argv[i], "--print-env") == 0) {
			std::cout << m::Process::env("MGPCL_TEST").raw() << std::endl;
			return 0;
		} else if(strcmp(argv[i], "--print-hash") == 0) {
			std::string line;
			std::getline(std::cin, line);

			m::String str(line.c_str(), static_cast<int>(line.size()));
			std::cout << str.hash() << std::endl;
			return 0;
		} else if(strcmp(argv[i], "--no-sound") == 0)
			g_noSound = true;
		else if(strcmp(argv[i], "--test") == 0) {
			i++;
			if(i < argc) {
				if(testAPI::testOnly(argv[i]))
					return -1;
			} else
				std::cout << "[?] Note: --test argument was specified with no following test name" << std::endl;
		} else if(strcmp(argv[i], "--except") == 0) {
			i++;
			if(i < argc)
				testAPI::testExcept(argv[i]);
			else
				std::cout << "[?] Note: --except argument was specified with no following test name" << std::endl;
		} else
			std::cout << "[?] Note: ignoring unrecognized CLI argument \"" << argv[i] << "\"" << std::endl;
	}

	m::console::setTitle(WINDOW_TITLE);
	m::console::setTextColor(m::kCC_Yellow);
	std::cout << g_aperture << std::endl;
	m::console::resetColor();
	playSound("silo.wav");

#ifdef MGPCL_WIN
	m::wmi::acquire(); //INIT COM
	m::wmi::release();

	HWND consoleWnd = FindWindow(nullptr, WINDOW_TITLE);
	ITaskbarList4 *taskBar = nullptr;

	if(consoleWnd != nullptr) {
		CoCreateInstance(CLSID_TaskbarList, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskbarList4, reinterpret_cast<void**>(&taskBar));

		if(taskBar != nullptr)
			taskBar->SetProgressState(consoleWnd, TBPF_NORMAL);
	}

	std::function<void(int, int, bool)> cbFunc([taskBar, consoleWnd] (int pos, int total, bool success) {
		if(taskBar != nullptr) {
			taskBar->SetProgressValue(consoleWnd, static_cast<ULONGLONG>(pos), static_cast<ULONGLONG>(total));

			if(!success)
				taskBar->SetProgressState(consoleWnd, TBPF_ERROR);
		}
	});
#else
	std::function<void(int, int, bool)> cbFunc([] (int pos, int total, bool success) {});
#endif

	if(m::inet::initialize() != m::inet::kIE_NoError) {
		std::cerr << "Failed to init net lib; tests can't continue" << std::endl;
		return -2;
	}

	m::inet::initSSL();
	bool result = testAPI::runAll(argv[0], cbFunc);

#ifdef MGPCL_WIN
	if(taskBar != nullptr)
		taskBar->Release();
#endif

	drawCenteredRect(50, 4);

	{
		m::String str(result ? " TESTS PASSED " : " TESTS FAILED ");
		m::Vector2i cur(m::console::getCursorPos());
		int sx = (m::console::getSize().x() - str.length()) / 2;

		m::console::setCursorPos(sx, cur.y() - 3);
		m::console::setColor(result ? m::kCC_Black : m::kCC_White, result ? m::kCC_LightGreen : m::kCC_Red);
		std::cout << str.raw();
		m::console::resetColor();
		m::console::setCursorPos(cur);
	}

	if(result)
		playSound("passed.wav");

	m::console::resetColor();
#ifdef _WIN32
	system("pause");
#endif

	return result ? 0 : 255;
}
