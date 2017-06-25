/* Copyright (C) 2017 BARBOTIN Nicolas
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies
 * or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
 * OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once
#include "String.h"
#include "Config.h"

#ifndef MGPCL_NO_GUI

namespace m
{
	enum MsgBoxButtons
	{
		kMBB_Ok = 0,
		kMBB_OkCancel,
		kMBB_AbortRetryIgnore,
		kMBB_YesNoCancel,
		kMBB_YesNo,
		kMBB_RetryCancel,
		kMBB_CancelTryAgainContinue
	};

	enum MsgBoxResult
	{
		kMBR_MsgBoxError = 0,
		kMBR_Abort,
		kMBR_Cancel,
		kMBR_Continue,
		kMBR_Ignore,
		kMBR_Yes,
		kMBR_No,
		kMBR_Retry,
		kMBR_TryAgain,
		kMBR_Ok
	};

	namespace msgBox
	{
		MsgBoxResult info(const String &text, const String &title, MsgBoxButtons buttons = kMBB_Ok);
		MsgBoxResult warning(const String &text, const String &title, MsgBoxButtons buttons = kMBB_Ok);
		MsgBoxResult error(const String &text, const String &title, MsgBoxButtons buttons = kMBB_Ok);
		MsgBoxResult question(const String &text, const String &title, MsgBoxButtons buttons = kMBB_Ok);
	}
}

#endif
