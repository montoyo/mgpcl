/* Copyright (C) 2020 BARBOTIN Nicolas
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

#include "mgpcl/ProgramArgs.h"
#include <iostream>

/*************************************** ArgValue ***************************************/

m::ArgValue::~ArgValue()
{
    if(m_values != nullptr) {
        const int cnt = valueCount();
        for(int i = 0; i < cnt; i++)
            m_values[i].~String();

        delete[] reinterpret_cast<uint8_t*>(m_values);
    }
}

/*************************************** ArgDescriptor ***************************************/

m::ArgDescriptor::ArgDescriptor(ArgType t, const String &n)
{
    m_type = t;
    m_optional = true;
    m_numeric = false;
    m_unique = true;
    m_names.add(n);
    m_lastErr = kAE_NoError;
    m_default = nullptr;
}

m::ArgDescriptor::~ArgDescriptor()
{
    for(ArgValue *v : m_values)
        v->removeRef();

    if(m_default != nullptr)
        m_default->removeRef();
}

bool m::ArgDescriptor::matches(const String &name) const
{
    for(const String &s: m_names) {
        if(name == s)
            return true;
    }

    return false;
}

m::ArgValue *m::ArgDescriptor::parse(int *argc, const char ***argv)
{
    if((m_unique || m_type == kAT_Switch) && !m_values.isEmpty()) {
        m_lastErr = kAE_AlreadySet;
        return nullptr;
    }

    int needed = static_cast<int>(m_type) + 1; //Don't forget (*argv)[0] == argName, and then comes values
    if(*argc < needed) {
        m_lastErr = kAE_MissingValues;
        return nullptr;
    }

    ArgValue *ret = new ArgValue(this);
    ret->allocate(needed);

    if(m_type == kAT_Switch)
        new(ret->m_values) String("true"_m);
    else {
        bool wrongFormat = false;

        for(int i = 0; i < needed - 1; i++) {
            new(ret->m_values + i) String((*argv)[i + 1]);

            if(m_numeric && !wrongFormat && !ret->m_values[i].isNumber())
                wrongFormat = true;
        }

        if(wrongFormat) {
            m_lastErr = kAE_NotNumeric;
            delete ret; //Destructor will care about allocated strings. Also, no need to use removeRef()
            return nullptr;
        }
    }

    //Everything went fine
    m_values.add(ret);
    ret->addRef();

    *argc -= needed;
    *argv += needed;
    return ret;
}

m::ArgDescriptor &m::ArgDescriptor::setDefault(const String &a)
{
    if(m_default != nullptr)
        m_default->removeRef();

    mAssert(m_type == kAT_Single, "invalid default value");

    m_default = new ArgValue(this);
    m_default->allocate();
    new(m_default->m_values) String(a);

    m_default->addRef();
    return *this;
}

m::ArgDescriptor &m::ArgDescriptor::setDefault(const String &a, const String &b)
{
    if(m_default != nullptr)
        m_default->removeRef();

    mAssert(m_type == kAT_Dual, "invalid default value");

    m_default = new ArgValue(this);
    m_default->allocate(2);
    new(m_default->m_values + 0) String(a);
    new(m_default->m_values + 1) String(b);

    m_default->addRef();
    return *this;
}

m::ArgDescriptor &m::ArgDescriptor::setDefault(const String &a, const String &b, const String &c)
{
    if(m_default != nullptr)
        m_default->removeRef();

    mAssert(m_type == kAT_Triple, "invalid default value");

    m_default = new ArgValue(this);
    m_default->allocate(3);
    new(m_default->m_values + 0) String(a);
    new(m_default->m_values + 1) String(b);
    new(m_default->m_values + 2) String(c);

    m_default->addRef();
    return *this;
}

void m::ArgDescriptor::clearValues()
{
    for(ArgValue *v : m_values)
        v->removeRef();

    m_values.cleanup();
}

m::String m::ArgDescriptor::errorString() const
{
    switch(m_lastErr) {
    case kAE_NoError:
        return "No error"_m;

    case kAE_MissingValues:
        return m_type == kAT_Single ? "Missing value"_m : "Missing values"_m;

    case kAE_NotNumeric:
        return "One or more value has an invalid numeric format"_m;

    case kAE_AlreadySet:
        return "Multiple instances of this argument found"_m;

    case kAE_Required:
        return "This argument is required and wasn't set"_m;

    default:
        return "Unknown error"_m;
    }
}

/*************************************** ProgramArgs ***************************************/

m::ProgramArgs::~ProgramArgs()
{
    for(ArgValue *v : m_args)
        v->removeRef();

    for(ArgDescriptor *d: m_descr)
        delete d;
}

m::ArgDescriptor *m::ProgramArgs::argByName(const String &name)
{
    for(ArgDescriptor *d: m_descr) {
        if(d->matches(name))
            return d;
    }

    return nullptr;
}

m::ArgDescriptor &m::ProgramArgs::add(const String &n, ArgType t)
{
    ArgDescriptor *ret = new ArgDescriptor(t, n);
    m_descr.add(ret);

    return *ret;
}

m::ArgDescriptor &m::ProgramArgs::addHelpSwitch(const String &name)
{
    ArgDescriptor *ret = new ArgDescriptor(kAT_Switch, name);
    m_descr.add(ret);

    m_helpSwitch = ret;
    return *ret;
}

m::ArgParseError m::ProgramArgs::parse()
{
    int argc          = m_argc - 1; //First argument is program binary location, so ignore it...
    const char **argv = m_argv + 1;

    //Cleanup existing args & error
    for(ArgDescriptor *ad : m_descr)
        ad->clearValues();

    for(ArgValue *v : m_args)
        v->removeRef();

    m_args.cleanup();
    m_err = nullptr;
    m_remainingIdx = 0;
    m_unrecognized.clear();

    //Parse from descriptors
    bool argFound = true;
    while(argFound && argc > 0) {
        ArgDescriptor *d = argByName(*argv);

        if(d == nullptr)
            argFound = false;
        else {
            ArgValue *v = d->parse(&argc, &argv);
            if(v == nullptr) {
                m_err = d;
                m_lastErr = kAPE_ArgError;

                return kAPE_ArgError; //Something wrong happened.
            }

            m_args.add(v);
            v->addRef();
            m_remainingIdx++;
        }
    }

    if(argc <= 0) //<=> argFound == true
        m_remainingIdx = -1; //No remaining args

    const bool ignoreReqs = m_helpIgnoreReqs && m_helpSwitch != nullptr && !m_helpSwitch->m_values.isEmpty();

    //Make sure everyone has its value
    for(ArgDescriptor *d: m_descr) {
        if(d->m_type == kAT_Switch && d->m_values.isEmpty()) {
            ArgValue *av = new ArgValue(d);
            av->allocate();
            new(av->m_values) String("false"_m);

            av->addRef(); //Ref will be destroyed by ArgDescriptor
            d->m_values.add(av);
        } if(!d->isSatisfied() && !ignoreReqs) {
            d->m_lastErr = kAE_Required;

            m_err = d;
            m_lastErr = kAPE_ArgError;
            return kAPE_ArgError;
        }
    }

    //Append remaining arguments
    if(argc > 0) {
        if(!m_acceptsRem) {
            m_unrecognized = *argv;
            m_lastErr = kAPE_UnknownArgFound;
            return kAPE_UnknownArgFound;
        }
    }

    while(argc > 0) {
        ArgValue *v = new ArgValue(nullptr); //No parent
        v->allocate();
        new(v->m_values) String(*argv);

        //Append value & next arg
        m_args.add(v);
        v->addRef();

        argc--;
        argv++;
    }

    //Print help if needed
    if(m_helpSwitch != nullptr && m_helpSwitch->isSet())
        printHelp();

    m_lastErr = kAPE_NoError;
    return kAPE_NoError;
}

void m::ProgramArgs::printHelp() const
{
    int maxlen = 0;
    String *descr = new String[m_descr.size()];

    for(int i = 0; i < m_descr.size(); i++) {
        descr[i] = m_descr[i]->name();
        for(int j = 1; j < m_descr[i]->nameCount(); j++) {
            descr[i] += ", ";
            descr[i] += m_descr[i]->name(j);
        }

        descr[i] += ": ";
        if(descr[i].length() > maxlen)
            maxlen = descr[i].length();
    }

    if(!m_helpHdr.isEmpty())
        std::cout << m_helpHdr.raw() << std::endl << std::endl;

    for(int i = 0; i < m_descr.size(); i++) {
        int delta = maxlen - descr[i].length();
        std::cout << descr[i].raw();

        for(int j = 0; j < delta; j++)
            std::cout << ' ';

        std::cout << m_descr[i]->helpText().raw() << std::endl;
    }

    delete[] descr;
}

m::String m::ProgramArgs::errorString() const
{
    if(m_lastErr == kAPE_NoError)
        return "No error"_m;
    else if(m_lastErr == kAPE_UnknownArgFound)
        return "Found unrecognized argument "_m + m_unrecognized;
    else if(m_lastErr == kAPE_ArgError) {
        String ret("Argument \""_m);
        ret += m_err->name();
        ret += "\" errored: "_m;
        ret += m_err->errorString();

        return ret;
    }

    return "Unknown error"_m;
}
