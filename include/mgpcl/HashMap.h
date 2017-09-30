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
#include "Config.h"
#include "Mem.h"
#include "List.h"
#include "Hasher.h"
#include <cmath>

namespace m
{
    template<typename K, typename V, class Hasher = DefaultHasher<K>> class MGPCL_PREFIX HashMap
    {
    public:
        class Pair
        {
        public:
            Pair(int h, const K &k) : key(k)
            {
                hash = h;
            }

            int hash; //Key hash
            K key;
            V value;
        };

        typedef List<Pair> Bucket;

    private:
        Bucket **m_buckets;
        int m_count;
        int m_numElems;

    public:
        class Iterator
        {
        public:
            Iterator(HashMap<K, V, Hasher> *src, bool rev = false)
            {
                m_parent = src;

                if(rev) {
                    m_cBucket = m_parent->m_count;
                    m_cPair = 0;
                } else {
                    m_cBucket = 0;
                    m_cPair = 0;

                    while(m_cBucket < m_parent->m_count && (m_parent->m_buckets[m_cBucket] == nullptr || m_parent->m_buckets[m_cBucket]->isEmpty()))
                        m_cBucket++;
                }
            }

            Iterator &operator++ ()
            {
                if(++m_cPair >= m_parent->m_buckets[m_cBucket]->size()) {
                    m_cPair = 0;

                    //Find the next non-empty bucket
                    do {
                        m_cBucket++;
                    } while(m_cBucket < m_parent->m_count && (m_parent->m_buckets[m_cBucket] == nullptr || m_parent->m_buckets[m_cBucket]->isEmpty()));
                }

                return *this;
            }

            Iterator operator++ (int)
            {
                Iterator ret(*this);
                ++(*this);

                return ret;
            }

            Iterator &operator-- ()
            {
                if(--m_cPair < 0) {
                    //Find the previous non-empty bucket
                    do {
                        m_cBucket--;
                    } while(m_cBucket >= 0 && (m_parent->m_buckets[m_cBucket] == nullptr || m_parent->m_buckets[m_cBucket]->isEmpty()));

                    if(m_cBucket < m_parent->m_count)
                        m_cPair = m_parent->m_buckets[m_cBucket]->size() - 1;
                }

                return *this;
            }

            Iterator operator-- (int)
            {
                Iterator ret(*this);
                --(*this);

                return ret;
            }

            Pair &operator * ()
            {
                return (*m_parent->m_buckets[m_cBucket])[m_cPair];
            }

            Pair *operator -> ()
            {
                return &((*m_parent->m_buckets[m_cBucket])[m_cPair]);
            }
            
            bool operator == (const Iterator &src)
            {
                return (m_parent == src.m_parent && m_cPair == src.m_cPair && m_cBucket == src.m_cBucket);
            }

            bool operator != (const Iterator &src)
            {
                return (m_parent != src.m_parent || m_cPair != src.m_cPair || m_cBucket != src.m_cBucket);
            }

        private:
            Iterator() {}

            HashMap<K, V, Hasher> *m_parent;
            int m_cPair;
            int m_cBucket;
        };

        HashMap()
        {
            m_count = 32;
            m_buckets = new Bucket*[m_count];
            m_numElems = 0;
            mem::zero(m_buckets, sizeof(Bucket*) * m_count);
        }

        HashMap(int bcount)
        {
            m_count = bcount;
            m_buckets = new Bucket*[m_count];
            m_numElems = 0;
            mem::zero(m_buckets, sizeof(Bucket*) * m_count);
        }

        HashMap(const HashMap<K, V, Hasher> &src)
        {
            m_count = src.m_count;
            m_numElems = src.m_numElems;
            m_buckets = new Bucket*[m_count];

            for(int i = 0; i < m_count; i++) {
                if(src.m_buckets[i] == nullptr)
                    m_buckets[i] = nullptr;
                else
                    m_buckets[i] = new Bucket(*src.m_buckets[i]);
            }
        }

        HashMap(HashMap<K, V, Hasher> &&src)
        {
            m_count = src.m_count;
            m_numElems = src.m_numElems;
            m_buckets = src.m_buckets;
            src.m_buckets = nullptr;
        }

        ~HashMap()
        {
            if(m_buckets != nullptr) {
                for(int i = 0; i < m_count; i++) {
                    if(m_buckets[i] != nullptr)
                        delete m_buckets[i];
                }

                delete[] m_buckets;
            }
        }

        HashMap<K, V, Hasher> &operator = (const HashMap<K, V, Hasher> &src)
        {
            for(int i = 0; i < m_count; i++) {
                if(m_buckets[i] != nullptr)
                    delete m_buckets[i];
            }

            if(m_count != src.m_count) {
                delete[] m_buckets;

                m_count = src.m_count;
                m_buckets = new Bucket*[m_count];
            }

            for(int i = 0; i < m_count; i++) {
                if(src.m_buckets[i] == nullptr)
                    m_buckets[i] = nullptr;
                else
                    m_buckets[i] = new Bucket(*src.m_buckets[i]);
            }

            m_numElems = src.m_numElems;
            return *this;
        }

        HashMap<K, V, Hasher> &operator = (HashMap<K, V, Hasher> &&src)
        {
            for(int i = 0; i < m_count; i++) {
                if(m_buckets[i] != nullptr)
                    delete m_buckets[i];
            }

            delete[] m_buckets;

            m_count = src.m_count;
            m_numElems = src.m_numElems;
            m_buckets = src.m_buckets;
            src.m_buckets = nullptr;
            return *this;
        }

        V &get(const K &key)
        {
            const int hc = Hasher::hash(key);
            Bucket **b = &m_buckets[abs(hc) % m_count];

            if(*b == nullptr) {
                *b = new Bucket;
                (**b).add(Pair(hc, key));

                m_numElems++;
                return (**b).last().value;
            }

            Bucket &bucket = **b;
            for(Pair &p : bucket) {
                if(p.hash == hc)
                    return p.value;
            }

            bucket.add(Pair(hc, key));
            m_numElems++;
            return bucket.last().value;
        }

        V get(const K &key) const
        {
            const int hc = Hasher::hash(key);
            Bucket **b = &m_buckets[abs(hc) % m_count];

            if(*b == nullptr)
                return V();

            Bucket &bucket = **b;
            for(Pair &p : bucket) {
                if(p.hash == hc)
                    return p.value;
            }

            return V();
        }

        V &operator[] (const K &key)
        {
            return get(key);
        }

        V operator[] (const K &key) const
        {
            return get(key);
        }

        bool hasKey(const K &key) const
        {
            const int hc = Hasher::hash(key);
            Bucket *b = m_buckets[abs(hc) % m_count];

            if(b == nullptr)
                return false;

            for(Pair &p : *b) {
                if(p.hash == hc)
                    return true;
            }

            return false;
        }

        void removeKey(const K &key, bool delBucket = true)
        {
            const int hc = Hasher::hash(key);
            Bucket **b = &m_buckets[abs(hc) % m_count];

            if(*b == nullptr)
                return;

            int idx = -1;
            Bucket &bucket = **b;

            for(int i = 0; i < bucket.size(); i++) {
                if(bucket[i].hash == hc) {
                    idx = i;
                    break;
                }
            }

            if(idx >= 0) {
                bucket.remove(idx);
                m_numElems--;

                if(bucket.isEmpty() && delBucket) {
                    delete *b;
                    *b = nullptr;
                }
            }
        }

        void doubleBuckets()
        {
            int nc = m_count << 1;
            Bucket **ray = new Bucket*[nc];
            mem::zero(ray, nc * sizeof(Bucket*));

            for(int i = 0; i < m_count; i++) {
                Bucket *b = m_buckets[i];

                if(b != nullptr) {
                    for(int j = 0; j < b->size(); j++)
                        manualInsert(ray, nc, b->get(j)); //No need to update m_numElems, this doesn't change anything...

                    delete b;
                }
            }

            delete[] m_buckets;
            m_buckets = ray;
            m_count = nc;
        }

        bool isEmpty() const
        {
            return m_numElems == 0;
        }

        int size() const
        {
            return m_numElems;
        }
        
        void clear()
        {
            for(int i = 0; i < m_count; i++) {
                if(m_buckets[i] != nullptr) {
                    delete m_buckets[i];
                    m_buckets[i] = nullptr;
                }
            }
        }

        Iterator begin()
        {
            return Iterator(this);
        }

        Iterator end()
        {
            return Iterator(this, true);
        }

    private:
        static void manualInsert(Bucket **ray, int cnt, Pair &pair)
        {
            //WARNING: DOES NOT UPDATE m_numElems !!
            Bucket **b = &ray[abs(pair.hash) % cnt];
            if(*b == nullptr)
                *b = new Bucket;

            (**b).add(pair);
        }
    };
}
