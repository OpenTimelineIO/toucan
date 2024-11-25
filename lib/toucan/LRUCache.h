// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#pragma once

#include <cstdint>
#include <map>
#include <vector>

namespace toucan
{
    //! Least recently used (LRU) cache.
    template<typename T, typename U>
    class LRUCache
    {
    public:
        //! \name Size
        ///@{

        size_t getMax() const;
        size_t getSize() const;
        size_t getCount() const;
        float getPercentage() const;

        void setMax(size_t);

        ///@}

        //! \name Contents
        ///@{

        bool contains(const T& key) const;
        bool get(const T& key, U& value) const;

        void add(const T& key, const U& value, size_t size = 1);
        void remove(const T& key);
        void clear();

        std::vector<T> getKeys() const;
        std::vector<U> getValues() const;

        ///@}

    private:
        void _maxUpdate();

        size_t _max = 10000;
        std::map<T, std::pair<U, size_t> > _map;
        mutable std::map<T, int64_t> _counts;
        mutable int64_t _counter = 0;
    };
}

#include <toucan/LRUCacheInline.h>
