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
#include "List.h"
#include "Vector3.h"
#include "Vector2.h"
#include <type_traits>

namespace m
{
    enum ArgType
    {
        //Shall not be an enum class; must be able to convert it into an int
        kAT_Switch = 0,
        kAT_Single,
        kAT_Dual,
        kAT_Triple
    };

    enum ArgError
    {
        kAE_NoError = 0,
        kAE_MissingValues,
        kAE_NotNumeric,
        kAE_AlreadySet,
        kAE_Required
    };

	enum ArgParseError
	{
		kAPE_NoError = 0,
		kAPE_ArgError,
		kAPE_UnknownArgFound
	};

    class ProgramArgs;
    class ArgDescriptor;
    class ArgValue
    {
        friend class ProgramArgs;
        friend class ArgDescriptor;

    public:
        ~ArgValue();

        ArgDescriptor &descriptor() const
        {
            //Maybe returning a pointer is better?
            return *m_parent;
        }

        bool hasDescriptor() const
        {
            return m_parent != nullptr;
        }

		//TODO: assert idx is in range!
        const String &asString(int idx = 0) const
        {
            return m_values[idx];
        }

        bool asBool(int idx = 0) const
        {
            return m_values[idx].trimmed().toLower() == "true";
        }

        int asInt(int idx = 0, int base = 10) const
        {
            return m_values[idx].toInteger(base);
        }

        double asDouble(int idx = 0) const
        {
            return m_values[idx].toDouble();
        }

        float asFloat(int idx = 0) const
        {
            return static_cast<float>(m_values[idx].toDouble());
        }

        Vector3i asVector3i() const
        {
            return Vector3i(asInt(0), asInt(1), asInt(2));
        }

        Vector3f asVector3f() const
        {
            return Vector3f(asFloat(0), asFloat(1), asFloat(2));
        }

        Vector3d asVector3d() const
        {
            return Vector3d(asDouble(0), asDouble(1), asDouble(2));
        }

        Vector2i asVector2i() const
        {
            return Vector2i(asInt(0), asInt(1));
        }

        Vector2f asVector2f() const
        {
            return Vector2f(asFloat(0), asFloat(1));
        }

        Vector2d asVector2d() const
        {
            return Vector2d(asDouble(0), asDouble(1));
        }

        inline int valueCount() const;

    private:
        ArgValue()
        {
			//Never used
			std::abort();
        }

        ArgValue(const ArgValue &src)
        {
			//Never used
			std::abort();
        }

        ArgValue(ArgValue &&src)
        {
	        //Never used
			std::abort();
        }

        ArgValue(ArgDescriptor *parent)
        {
            m_parent = parent;
            m_values = nullptr;
			m_refs = 0;
        }

		void addRef()
        {
			m_refs++;
        }

		void removeRef()
        {
			if(--m_refs <= 0)
				delete this;
        }

		void allocate(int cnt = 1)
        {
			m_values = reinterpret_cast<String*>(new uint8_t[sizeof(String) * cnt]);
        }

        ArgDescriptor *m_parent;
        String *m_values;
		int m_refs;
    };

    class ArgDescriptor
    {
        friend class ProgramArgs;

    public:
        ArgDescriptor &addAlias(const String &name)
        {
            m_names.add(name);
            return *this;
        }

        ArgDescriptor &setHelpText(const String &txt)
        {
            m_help = txt;
            return *this;
        }

        //A switch is always optional
        ArgDescriptor &setOptional(bool opt)
        {
            m_optional = opt;
            return *this;
        }

        //A switch is never numeric
        ArgDescriptor &setNumeric(bool num = true)
        {
            m_numeric = num;
            return *this;
        }

        //A switch is always unique
        ArgDescriptor &setUnique(bool un)
        {
            m_unique = un;
            return *this;
        }

        String name(int idx = 0) const
        {
            //Make a copy in case list is changed
            return m_names[idx];
        }

		int nameCount() const
        {
			return m_names.size();
        }

        bool isOptional() const
        {
            return m_optional;
        }

        bool isNumeric() const
        {
            return m_numeric;
        }

        bool isUnique() const
        {
            return m_unique;
        }

        bool isSatisfied() const
        {
            return m_type == kAT_Switch || m_optional || !m_values.isEmpty();
        }

        ArgType type() const
        {
            return m_type;
        }

        const String &helpText() const
        {
            return m_help;
        }

        int valueCount() const
        {
			if(m_values.isEmpty())
				return m_default == nullptr ? 0 : 1;
			else
				return m_values.size();
        }

        int subvalueCount() const
        {
            return m_type == kAT_Switch ? 1 : static_cast<int>(m_type);
        }

        const ArgValue &value(int idx = 0) const
        {
			if(m_values.isEmpty() && m_default != nullptr)
				return *m_default;
			else
				return *m_values[idx];
        }

        ArgError lastError() const
        {
            return m_lastErr;
        }

		String errorString() const;

        bool matches(const String &name) const;

		bool isSet() const
		{
			//A switch always have a boolean value
			return m_type == kAT_Switch ? m_values[0]->asBool() : !m_values.isEmpty();
		}

		ArgDescriptor &setDefault(const String &a);
		ArgDescriptor &setDefault(const String &a, const String &b);
		ArgDescriptor &setDefault(const String &a, const String &b, const String &c);

		ArgDescriptor &setDefault(int val)
		{
			return setDefault(String::fromInteger(val));
		}

		ArgDescriptor &setDefault(float val)
		{
			return setDefault(String::fromDouble(static_cast<double>(val)));
		}

		ArgDescriptor &setDefault(double val)
		{
			return setDefault(String::fromDouble(val));
		}

		template<typename T> ArgDescriptor &setDefault(const Vector2<T> &val)
		{
			static_assert(std::is_arithmetic<T>::value, "not a valid type");

			if(std::is_integral<T>::value)
				return setDefault(String::fromInteger(static_cast<int>(val.x())), String::fromInteger(static_cast<int>(val.y())));
			else
				return setDefault(String::fromDouble(static_cast<double>(val.x())), String::fromDouble(static_cast<double>(val.y())));
		}

		template<typename T> ArgDescriptor &setDefault(const Vector3<T> &val)
		{
			static_assert(std::is_arithmetic<T>::value, "not a valid type");

			if(std::is_integral<T>::value)
				return setDefault(String::fromInteger(static_cast<int>(val.x())), String::fromInteger(static_cast<int>(val.y())), String::fromInteger(static_cast<int>(val.z())));
			else
				return setDefault(String::fromDouble(static_cast<double>(val.x())), String::fromDouble(static_cast<double>(val.y())), String::fromDouble(static_cast<double>(val.z())));
		}

    private:
		ArgDescriptor()
		{
			//Never used
			std::abort();
		}

		ArgDescriptor(const ArgDescriptor &src)
		{
			//Never used
			std::abort();
		}

		ArgDescriptor(ArgDescriptor &&src)
		{
			//Never used
			std::abort();
		}

        ArgDescriptor(ArgType t, const String &name);
		~ArgDescriptor();
        ArgValue *parse(int *argc, const char ***argv);
		void clearValues();

        ArgError m_lastErr;
        ArgType m_type;
        bool m_optional;
        bool m_numeric;
        bool m_unique;

        List<String> m_names;
        List<ArgValue*> m_values; //Ref counted
		ArgValue *m_default;
        String m_help;
    };

    inline int ArgValue::valueCount() const
    {
        return m_parent == nullptr ? 1 : m_parent->subvalueCount();
    }

    class ProgramArgs
    {
    public:
        ProgramArgs()
        {
            m_argc = 0;
            m_argv = nullptr;
            m_err = nullptr;
			m_remainingIdx = -1;
			m_acceptsRem = true;
			m_helpSwitch = nullptr;
			m_lastErr = kAPE_NoError;
        }

        ProgramArgs(int argc, const char **argv)
        {
            m_argc = argc;
            m_argv = argv;
            m_err = nullptr;
			m_remainingIdx = -1;
			m_acceptsRem = true;
			m_helpSwitch = nullptr;
			m_lastErr = kAPE_NoError;
        }

        ~ProgramArgs();

        void set(int argc, const char **argv)
        {
            m_argc = argc;
            m_argv = argv;
        }

        ArgDescriptor *argByName(const String &name);
        ArgDescriptor &add(const String &name, ArgType t);
		ArgDescriptor &addHelpSwitch(const String &name); //Cannot have multiple help switches, previous will be overidden
		ArgParseError parse();

        int valueCount() const
        {
            return m_args.size();
        }

        const ArgValue &value(int idx) const
        {
            return *m_args[idx];
        }

        ArgDescriptor *erroringDescriptor() const
        {
            return m_err;
        }

		String executablePath() const
        {
			return m_argc > 0 ? String(m_argv[0]) : String();
        }

		bool hasRemainingArgs() const
        {
			return m_remainingIdx >= 0;
        }

		int remainingArgsBegin() const
        {
			return m_remainingIdx;
        }

		const String &helpHeader() const
        {
			return m_helpHdr;
        }

		void setHelpHeader(const String &str)
        {
			m_helpHdr = str;
        }

		void setAcceptsRemainingArgs(bool a)
        {
			m_acceptsRem = a;
        }

		bool acceptsRemainingArgs() const
        {
			return m_acceptsRem;
        }

		const String &unrecongnizedArgument() const
        {
			return m_unrecognized;
        }

		ArgParseError lastError() const
        {
			return m_lastErr;
        }

		String errorString() const;
		void printHelp() const;

    private:
        int m_argc;
        const char **m_argv;
		int m_remainingIdx;
		bool m_acceptsRem;
        ArgDescriptor *m_err; //Just a ref
		ArgDescriptor *m_helpSwitch; //Just a ref
        List<ArgDescriptor*> m_descr; //ProgramArgs cares about ArgDescriptor allocation/deallocation
        List<ArgValue*> m_args; //Ref counted
		String m_helpHdr;
		String m_unrecognized;
		ArgParseError m_lastErr;
    };

}

