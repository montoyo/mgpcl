#include <mgpcl/Config.h>

#ifndef MGPCL_NO_GUI
#include "TestAPI.h"
#include <mgpcl/MsgBox.h>
#include <mgpcl/Time.h>
#include <mgpcl/Atomic.h>
#include <mgpcl/Thread.h>

Declare Test("gui"), Priority(13.0);

TEST
{
    volatile StackIntegrityChecker sic;

#ifdef MGPCL_WIN
    m::Atomic running(1);
    int dlgCount = 0;

    m::FunctionalThread thread([&running, &dlgCount] ()
    {
        while(running.get()) {
            m::time::sleepMs(100);
            HWND wnd = FindWindow(nullptr, "This is the title");

            if(wnd != nullptr) {
                if(EndDialog(wnd, 0) != FALSE)
                    dlgCount++;
            }
        }
    }, "DialogKiller2000");

    thread.start();
#endif

    m::msgBox::info("This is the text", "This is the title", m::kMBB_OkCancel);
    m::msgBox::warning("This is the text", "This is the title", m::kMBB_YesNo);
    m::msgBox::error("This is the text", "This is the title", m::kMBB_CancelTryAgainContinue);
    m::msgBox::question("This is the text", "This is the title", m::kMBB_AbortRetryIgnore);
    
#ifdef MGPCL_WIN
    running.set(0);
    thread.join();
    testAssert(dlgCount == 4, "closed an invalid amount of dialogs!");
#endif

    return true;
}

#endif
