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

namespace m
{

    template<typename K, typename V> class FlatMap
    {
    public:
        class Pair
        {
        public:
            Pair(const K &k, const V &v) : key(k), value(v)
            {
            }

            Pair(const K &k) : key(k)
            {
            }

            K key;
            V value;

        private:
            Pair()
            {
            }
        };

        typedef Pair *Iterator;
        typedef const Pair *CIterator;

        Iterator begin()
        {
            return m_data.begin();
        }

        Iterator end()
        {
            return m_data.end();
        }

        CIterator begin() const
        {
            return m_data.begin();
        }

        CIterator end() const
        {
            return m_data.end();
        }

        void put(const K &key, const V &value)
        {
            if(m_data.isEmpty())
                m_data.add(Pair(key, value));
            else {
                int begin = 0;
                int end = m_data.size();

                do {
                    int pos = (begin + end) / 2;

                    if(key == m_data[pos].key) {
                        m_data[pos].value = value;
                        return;
                    } else if(key > m_data[pos].key)
                        begin = pos + 1;
                    else
                        end = pos;
                } while(begin != end);

                m_data.insert(begin, Pair(key, value));
            }
        }

        V &operator [] (const K &key)
        {
            if(m_data.isEmpty()) {
                m_data.add(Pair(key));
                return m_data.last().value;
            } else {
                int begin = 0;
                int end = m_data.size();

                do {
                    int pos = (begin + end) / 2;

                    if(key == m_data[pos].key)
                        return m_data[pos].value;
                    else if(key > m_data[pos].key)
                        begin = pos + 1;
                    else
                        end = pos;
                } while(begin != end);

                m_data.insert(begin, Pair(key));
                return m_data[begin].value;
            }
        }

        bool hasKey(const K &key) const
        {
            if(m_data.isEmpty())
                return false;
            else {
                int begin = 0;
                int end = m_data.size();

                do {
                    int pos = (begin + end) / 2;

                    if(key == m_data[pos].key)
                        return true;
                    else if(key > m_data[pos].key)
                        begin = pos + 1;
                    else
                        end = pos;
                } while(begin != end);

                return false;
            }
        }

        bool removeKey(const K &key)
        {
            if(m_data.isEmpty())
                return false;
            else {
                int begin = 0;
                int end = m_data.size();

                do {
                    int pos = (begin + end) / 2;

                    if(key == m_data[pos].key) {
                        m_data.remove(pos);
                        return true;
                    } else if(key > m_data[pos].key)
                        begin = pos + 1;
                    else
                        end = pos;
                } while(begin != end);

                return false;
            }
        }

        bool isEmpty() const
        {
            return m_data.isEmpty();
        }

        int size() const
        {
            return m_data.size();
        }

        void clear()
        {
            m_data.clear();
        }

        void cleanup()
        {
            m_data.cleanup();
        }

    private:
        List<Pair> m_data;
    };

}
