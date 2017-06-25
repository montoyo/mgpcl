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

#define M_DECLARE_NIFTY_COUNTER(name) class _ncc_##name \
	{ \
	public: \
		_ncc_##name(); \
		~_ncc_##name(); \
	};

#define M_SPAWN_NIFTY_COUNTER(name) static _ncc_##name _ncv_##name;
#define M_DEFINE_NIFTY_COUNTER(name) static int _nci_##name; \
	static void _ncf_ctor_##name(); \
	static void _ncf_dtor_##name(); \
	\
	_ncc_##name::_ncc_##name() \
	{ \
		if(_nci_##name++ == 0) \
			_ncf_ctor_##name(); \
	} \
	\
	_ncc_##name::~_ncc_##name() \
	{ \
		if(--_nci_##name == 0) \
			_ncf_dtor_##name(); \
	}

#define M_NIFTY_COUNTER_CTOR(name) static void _ncf_ctor_##name()
#define M_NIFTY_COUNTER_DTOR(name) static void _ncf_dtor_##name()

//========================== HOW TO USE: ==========================
//
//1. At the beginning of your header, include me:
//		#include <mgpcl/NiftyCounter.h>
//
//2. At the end of your header, add:
//		M_DECLARE_NIFTY_COUNTER(MyThing)
//		#ifndef MY_THING_SOURCE
//		M_SPAWN_NIFTY_COUNTER(MyThing)
//		#endif
//
//3. At the beginning of your source, add:
//		#define MY_THING_SOURCE
//		#include "your_header.h"
//
//		... additional includes ...
//		
//		M_DEFINE_NIFTY_COUNTER(MyThing)
//		M_NIFTY_COUNTER_CTOR(MyThing)
//		{
//			//Do whatever initialization you need to do here
//		}
//
//		M_NIFTY_COUNTER_DTOR(MyThing)
//		{
//			//Do whatever destruction you need to do here
//		}

