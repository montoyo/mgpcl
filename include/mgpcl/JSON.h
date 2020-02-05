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

#pragma once
#include "String.h"
#include "List.h"
#include "FlatMap.h"
#include "IOStream.h"
#include "SharedPtr.h"

namespace m
{
    enum JSONType
    {
        kJT_Null = 0,
        kJT_Boolean,
        kJT_Number,
        kJT_String,
        kJT_Array,
        kJT_Object
    };

    class JSONElement
    {
    public:
        typedef List<JSONElement> JSONList;
        typedef FlatMap<ConstString, JSONElement> JSONMap;

        JSONElement()
        {
            m_type = kJT_Null;
        }

        JSONElement(JSONType type)
        {
            m_type = type;
            allocate();
        }

        JSONElement(bool data)
        {
            m_type = kJT_Boolean;
            dataAs<bool>() = data;
        }

        JSONElement(int data)
        {
            m_type = kJT_Number;
            dataAs<double>() = static_cast<double>(data);
        }

        JSONElement(double data)
        {
            m_type = kJT_Number;
            dataAs<double>() = data;
        }

        JSONElement(const String &data)
        {
            m_type = kJT_String;
            new(&dataAs<String>()) String(data);
        }

        JSONElement(const char *str)
        {
            m_type = kJT_String;
            new(&dataAs<String>()) String(str);
        }

        JSONElement(const JSONElement &src);
        JSONElement(JSONElement &&src);

        ~JSONElement()
        {
            deallocate();
        }

        JSONElement &operator [] (int idx)
        {
            if(m_type == kJT_Array) {
                JSONList &lst = dataAs<JSONList>();
                mAssert(idx >= 0 && idx < lst.size(), "index is out of range");
                return lst[idx];
            } else {
                mAssert(m_type == kJT_Object, "not an array/object");
                JSONMap &map = dataAs<JSONMap>();
                mAssert(idx >= 0 && idx < map.size(), "index is out of range");
                return map.begin()[idx].value;
            }
        }

        const JSONElement &operator [] (int idx) const
        {
            if(m_type == kJT_Array) {
                const JSONList &lst = dataAs<JSONList>();
                mAssert(idx >= 0 && idx < lst.size(), "index is out of range");
                return lst[idx];
            } else {
                mAssert(m_type == kJT_Object, "not an array/object");
                const JSONMap &map = dataAs<JSONMap>();
                mAssert(idx >= 0 && idx < map.size(), "index is out of range");
                return map.begin()[idx].value;
            }
        }

        int size() const
        {
            if(m_type == kJT_Array)
                return dataAs<JSONList>().size();

            mAssert(m_type == kJT_Object, "not an array/object");
            return dataAs<JSONMap>().size();
        }

        void addElement(const JSONElement &src)
        {
            if(m_type == kJT_Array)
                dataAs<JSONList>().add(src);
            else {
                mAssert(m_type == kJT_Object, "not an array/object");
                mAssert(!src.m_name.isEmpty(), "can't add unnamed element to object");
                dataAs<JSONMap>().put(ConstString(src.m_name), src);
            }
        }

        void addElement(JSONElement &&src)
        {
            if(m_type == kJT_Array)
                dataAs<JSONList>().add(std::move(src));
            else {
                mAssert(m_type == kJT_Object, "not an array/object");
                mAssert(!src.m_name.isEmpty(), "can't add unnamed element to object");
                dataAs<JSONMap>().put(ConstString(src.m_name), std::move(src));
            }
        }

        void setName(const String &name)
        {
            mAssert(m_name.isEmpty(), "can't change name after it has been set");
            m_name = name;
        }

        bool has(const String &str) const
        {
            mAssert(m_type == kJT_Object, "not an object");
            return dataAs<JSONMap>().hasKey(ConstString(str));
        }

        bool has(const char *str) const
        {
            mAssert(m_type == kJT_Object, "not an object");
            return dataAs<JSONMap>().hasKey(ConstString(str));
        }

        bool has(const String &str, JSONType ofType) const
        {
            mAssert(m_type == kJT_Object, "not an object");
            const JSONElement *e;
            return dataAs<JSONMap>().getIfExists(ConstString(str), e) && e->m_type == ofType;
        }

        bool has(const char *str, JSONType ofType) const
        {
            mAssert(m_type == kJT_Object, "not an object");
            const JSONElement *e;
            return dataAs<JSONMap>().getIfExists(ConstString(str), e) && e->m_type == ofType;
        }

        const String &name() const
        {
            return m_name;
        }

        bool isNull() const
        {
            return m_type == kJT_Null;
        }

        bool asBool() const
        {
            mAssert(m_type == kJT_Boolean, "expected a boolean");
            return dataAs<bool>();
        }

        int asInt() const
        {
            mAssert(m_type == kJT_Number, "expected a number");
            return static_cast<int>(dataAs<double>());
        }

        double asDouble() const
        {
            mAssert(m_type == kJT_Number, "expected a number");
            return dataAs<double>();
        }

        const String &asString() const
        {
            mAssert(m_type == kJT_String, "expected a string");
            return dataAs<String>();
        }

        JSONElement &operator [] (const String &name)
        {
            mAssert(m_type == kJT_Object, "not an object");

            bool isNew;
            JSONElement &ret = dataAs<JSONMap>().get(ConstString(name), isNew);
            if(isNew)
                ret.setName(name);

            return ret;
        }

        JSONType type() const
        {
            return m_type;
        }

        bool isObject() const
        {
            return m_type == kJT_Object;
        }

        bool isArray() const
        {
            return m_type == kJT_Array;
        }

        bool isString() const
        {
            return m_type == kJT_String;
        }

        bool isNumber() const
        {
            return m_type == kJT_Number;
        }

        bool isBoolean() const
        {
            return m_type == kJT_Boolean;
        }

        JSONElement &operator = (const JSONElement &src);
        JSONElement &operator = (JSONElement &&src);

        JSONElement &operator = (bool data);
        JSONElement &operator = (int data);
        JSONElement &operator = (double data);
        JSONElement &operator = (const String &data);
        JSONElement &operator = (const char *data);

    private:
        void allocate();
        void deallocate();

        template <typename T> T &dataAs()
        {
            return *reinterpret_cast<T*>(m_data);
        }

        template <typename T> const T &dataAs() const
        {
            return *reinterpret_cast<const T*>(m_data);
        }

        JSONType m_type;
        String m_name;
        char m_data[sizeof(FlatMap<ConstString, int>)]; //Any type should do it; sizeof(FlatMap) depends of sizeof(List) which doesn't depend on the type
    };

    namespace json
    {
        bool parse(SSharedPtr<InputStream> src, JSONElement &dst, String &err);
        bool serializeCompact(SSharedPtr<OutputStream> out, JSONElement &src);
        bool serializeHumanReadable(SSharedPtr<OutputStream> out, JSONElement &src);
    }

}
