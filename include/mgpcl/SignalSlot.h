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
#include "List.h"
#include <type_traits>

namespace m
{
    class SignalBase;
    class Signal0;
    template<typename A1> class Signal1;
    template<typename A1, typename A2> class Signal2;
    template<typename A1, typename A2, typename A3> class Signal3;
    template<typename A1, typename A2, typename A3, typename A4> class Signal4;

    class SlotCapable
    {
        friend class SignalBase;
        friend class Signal0;
        template<typename A1> friend class Signal1;
        template<typename A1, typename A2> friend class Signal2;
        template<typename A1, typename A2, typename A3> friend class Signal3;
        template<typename A1, typename A2, typename A3, typename A4> friend class Signal4;

    public:
        SlotCapable()
        {
            m_callDelete = true;
        }

        inline SlotCapable(SlotCapable &src);
        inline SlotCapable(SlotCapable &&src);
        inline ~SlotCapable();

    private:
        void onSignalRemoval(SignalBase *s)
        {
            for(int i = 0; i < m_signals.size(); i++) {
                if(m_signals[i] == s) {
                    m_signals.remove(i);
                    return;
                }
            }
        }

        void onSignalMoved(SignalBase *src, SignalBase *dst)
        {
            for(int i = 0; i < m_signals.size(); i++) {
                if(m_signals[i] == src) {
                    m_signals[i] = dst;
                    return;
                }
            }
        }

        void putSignal(SignalBase *s)
        {
            if(m_signals.indexOf(s) < 0)
                m_signals.add(s);
        }

        bool m_callDelete;
        List<SignalBase*> m_signals;
    };

    class SignalBase
    {
        friend class SlotCapable;

    public:
        class Slot
        {
        public:
            typedef bool(SlotCapable::*Func)();

            SlotCapable *object;
            int offset;
            uint8_t function[sizeof(Func)];

            void setFunc(const void *data)
            {
                mem::copy(function, data, sizeof(Func));
            }

            void getFunc(void *dst) const
            {
                mem::copy(dst, function, sizeof(Func));
            }

            void setFunc(const Slot &src)
            {
                mem::copy(function, src.function, sizeof(Func));
            }
        };

        SignalBase()
        {
            m_callDelete = true;
        }

        SignalBase(SignalBase &src) : m_slots(src.m_slots)
        {
            m_callDelete = true;

            for(Slot &s : m_slots)
                s.object->putSignal(this);
        }

        SignalBase(SignalBase &&src) : m_slots(std::move(src.m_slots))
        {
            m_callDelete = true;
            src.m_callDelete = false;

            for(Slot &s : m_slots)
                s.object->onSignalMoved(&src, this);
        }

        virtual ~SignalBase()
        {
            if(m_callDelete) {
                for(Slot &s : m_slots)
                    s.object->onSignalRemoval(this);
            }
        }

    protected:
        void onObjectCopy(SlotCapable *src, SlotCapable *dst)
        {
            const int sz = m_slots.size();
            for(int i = 0; i < sz; i++) {
                if(m_slots[i].object == src) {
                    Slot s;
                    s.object = dst;
                    s.setFunc(m_slots[i]);

                    m_slots.add(s);
                }
            }
        }

        void onObjectRemoval(SlotCapable *slot)
        {
            for(int i = m_slots.size() - 1; i >= 0; i--) {
                if(m_slots[i].object == slot)
                    m_slots.remove(i);
            }
        }

        void onObjectMove(SlotCapable *oldPtr, SlotCapable *newPtr)
        {
            for(Slot &s : m_slots) {
                if(s.object == oldPtr)
                    s.object = newPtr;
            }
        }

        List<Slot> m_slots;
        bool m_callDelete;
    };

    /************************************************* ENTERING COPY PASTA ZONE *************************************************/

    class Signal0 : public SignalBase
    {
    public:
        template<class T> void connect(T *obj, bool(T::*fc)())
        {
            static_assert(std::is_base_of<SlotCapable, T>::value, "class must be SlotCapable");

            Slot s;
            s.object = obj;
            s.offset = static_cast<int>(reinterpret_cast<char*>(obj) - reinterpret_cast<char*>(s.object));
            s.setFunc(&fc);

            m_slots.add(s);
            obj->putSignal(this);
        }

        void operator() () const
        {
            bool(SlotCapable::*fc)();
            char *obj;

            for(const Slot &s : m_slots) {
                obj = reinterpret_cast<char*>(s.object) + s.offset;
                s.getFunc(&fc);

                if((reinterpret_cast<SlotCapable*>(obj)->*fc)())
                    return;
            }
        }
    };

    template<typename A1> class Signal1 : public SignalBase
    {
    public:
        template<class T> void connect(T *obj, bool(T::*fc)(A1))
        {
            static_assert(std::is_base_of<SlotCapable, T>::value, "class must be SlotCapable");

            Slot s;
            s.object = obj;
            s.offset = static_cast<int>(reinterpret_cast<char*>(obj) - reinterpret_cast<char*>(s.object));
            s.setFunc(&fc);

            m_slots.add(s);
            obj->putSignal(this);
        }

        void operator() (A1 a1) const
        {
            bool(SlotCapable::*fc)(A1);
            char *obj;

            for(const Slot &s : m_slots) {
                obj = reinterpret_cast<char*>(s.object) + s.offset;
                s.getFunc(&fc);

                if((reinterpret_cast<SlotCapable*>(obj)->*fc)(a1))
                    return;
            }
        }
    };

    template<typename A1, typename A2> class Signal2 : public SignalBase
    {
    public:
        template<class T> void connect(T *obj, bool(T::*fc)(A1, A2))
        {
            static_assert(std::is_base_of<SlotCapable, T>::value, "class must be SlotCapable");

            Slot s;
            s.object = obj;
            s.offset = static_cast<int>(reinterpret_cast<char*>(obj) - reinterpret_cast<char*>(s.object));
            s.setFunc(&fc);

            m_slots.add(s);
            obj->putSignal(this);
        }

        void operator() (A1 a1, A2 a2) const
        {
            bool(SlotCapable::*fc)(A1, A2);
            char *obj;

            for(const Slot &s : m_slots) {
                obj = reinterpret_cast<char*>(s.object) + s.offset;
                s.getFunc(&fc);

                if((reinterpret_cast<SlotCapable*>(obj)->*fc)(a1, a2))
                    return;
            }
        }
    };

    template<typename A1, typename A2, typename A3> class Signal3 : public SignalBase
    {
    public:
        template<class T> void connect(T *obj, bool(T::*fc)(A1, A2, A3))
        {
            static_assert(std::is_base_of<SlotCapable, T>::value, "class must be SlotCapable");

            Slot s;
            s.object = obj;
            s.offset = static_cast<int>(reinterpret_cast<char*>(obj) - reinterpret_cast<char*>(s.object));
            s.setFunc(&fc);

            m_slots.add(s);
            obj->putSignal(this);
        }

        void operator() (A1 a1, A2 a2, A3 a3) const
        {
            bool(SlotCapable::*fc)(A1, A2, A3);
            char *obj;

            for(const Slot &s : m_slots) {
                obj = reinterpret_cast<char*>(s.object) + s.offset;
                s.getFunc(&fc);

                if((reinterpret_cast<SlotCapable*>(obj)->*fc)(a1, a2, a3))
                    return;
            }
        }
    };

    template<typename A1, typename A2, typename A3, typename A4> class Signal4 : public SignalBase
    {
    public:
        template<class T> void connect(T *obj, bool(T::*fc)(A1, A2, A3, A4))
        {
            static_assert(std::is_base_of<SlotCapable, T>::value, "class must be SlotCapable");

            Slot s;
            s.object = obj;
            s.offset = static_cast<int>(reinterpret_cast<char*>(obj) - reinterpret_cast<char*>(s.object));
            s.setFunc(&fc);

            m_slots.add(s);
            obj->putSignal(this);
        }

        void operator() (A1 a1, A2 a2, A3 a3, A4 a4) const
        {
            bool(SlotCapable::*fc)(A1, A2, A3, A4);
            char *obj;

            for(const Slot &s : m_slots) {
                obj = reinterpret_cast<char*>(s.object) + s.offset;
                s.getFunc(&fc);

                if((reinterpret_cast<SlotCapable*>(obj)->*fc)(a1, a2, a3, a4))
                    return;
            }
        }
    };

    /************************************************** LEAVING COPY PASTA ZONE *************************************************/

    template<typename A1 = void, typename A2 = void, typename A3 = void, typename A4 = void, typename NoCare = void> class Signal
    {
    };

    template<typename A1, typename A2, typename A3, typename A4> class Signal<A1, A2, A3, A4, void> : public Signal4<A1, A2, A3, A4>
    {
    };

    template<typename A1, typename A2, typename A3> class Signal<A1, A2, A3, void, void> : public Signal3<A1, A2, A3>
    {
    };

    template<typename A1, typename A2> class Signal<A1, A2, void, void, void> : public Signal2<A1, A2>
    {
    };

    template<typename A1> class Signal<A1, void, void, void, void> : public Signal1<A1>
    {
    };

    template<> class Signal<void, void, void, void, void> : public Signal0
    {
    };

    inline SlotCapable::SlotCapable(SlotCapable &src) : m_signals(src.m_signals)
    {
        for(SignalBase *s : m_signals)
            s->onObjectCopy(&src, this);

        m_callDelete = true;
    }

    inline SlotCapable::SlotCapable(SlotCapable &&src) : m_signals(std::move(src.m_signals))
    {
        for(SignalBase *s : m_signals)
            s->onObjectMove(&src, this);

        m_callDelete = true;
        src.m_callDelete = false;
    }

    inline SlotCapable::~SlotCapable()
    {
        if(m_callDelete) {
            for(SignalBase *s : m_signals)
                s->onObjectRemoval(this);
        }
    }

}
