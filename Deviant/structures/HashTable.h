#pragma once

#include <iostream>
#include <string>

// Hash table: the combination lookup, a chain's fragment ids in order ("f03+f07+f01") to the barrier it breaks
// Separate chaining, polynomial string hash, the bucket array doubles when the load factor passes 0.75
template <typename V>
class HashTable
{
private:
    struct Entry
    {
        std::string key;
        V value;
        Entry* next;   // next entry in the same bucket
    };

public:
    static const int INITIAL_BUCKET_COUNT = 8;
    static constexpr double MAX_LOAD_FACTOR = 0.75;

    HashTable() : bucketCount(INITIAL_BUCKET_COUNT), count(0)
    {
        buckets = new Entry*[bucketCount];

        for (int i = 0; i < bucketCount; i++)
        {
            buckets[i] = nullptr;
        }
    }

    // Free every entry in every bucket, then the bucket array
    ~HashTable()
    {
        for (int i = 0; i < bucketCount; i++)
        {
            Entry* current = buckets[i];

            while (current != nullptr)
            {
                Entry* next = current->next;
                delete current;
                current = next;
            }
        }

        delete[] buckets;
    }

    // Copies would share entries and free them twice
    HashTable(const HashTable&) = delete;
    HashTable& operator=(const HashTable&) = delete;

    // Add a key, or replace its value when the key is already there
    void insert(const std::string& key, const V& value)
    {
        int index = hash(key);
        Entry* current = buckets[index];

        while (current != nullptr)
        {
            if (current->key == key)
            {
                current->value = value;
                return;
            }

            current = current->next;
        }

        // New key: add it to the front of its bucket's chain
        buckets[index] = new Entry{ key, value, buckets[index] };
        count++;

        // Too full: double the buckets so chains stay short
        if (loadFactor() > MAX_LOAD_FACTOR)
        {
            rehash();
        }
    }

    // Pointer to the value, nullptr when the key is missing
    V* find(const std::string& key)
    {
        // Only the key's own bucket is searched
        int index = hash(key);
        Entry* current = buckets[index];

        while (current != nullptr)
        {
            if (current->key == key)
            {
                return &current->value;
            }

            current = current->next;
        }

        return nullptr;
    }

    // True when the key was found and removed
    bool remove(const std::string& key)
    {
        int index = hash(key);
        Entry* previous = nullptr;
        Entry* current = buckets[index];

        while (current != nullptr)
        {
            if (current->key == key)
            {
                // Unlink: either the bucket starts at the next entry, or the previous entry skips over it
                if (previous == nullptr)
                {
                    buckets[index] = current->next;
                }
                else
                {
                    previous->next = current->next;
                }

                delete current;
                count--;
                return true;
            }

            previous = current;
            current = current->next;
        }

        return false;
    }

    // Free every entry, the buckets stay and are all empty again
    void clear()
    {
        for (int i = 0; i < bucketCount; i++)
        {
            Entry* current = buckets[i];

            while (current != nullptr)
            {
                Entry* next = current->next;
                delete current;
                current = next;
            }

            buckets[i] = nullptr;
        }

        count = 0;
    }

    // Entries per bucket
    double loadFactor() const
    {
        return (double)count / bucketCount;
    }

    // Demo only: every bucket and the keys chained in it
    void print() const
    {
        std::cout << "  buckets=" << bucketCount << " entries=" << count << " load factor=" << loadFactor() << "\n";

        for (int i = 0; i < bucketCount; i++)
        {
            std::cout << "  [" << i << "]";
            Entry* current = buckets[i];

            while (current != nullptr)
            {
                std::cout << " -> " << current->key;
                current = current->next;
            }

            std::cout << "\n";
        }
    }

private:
    Entry** buckets;   // each bucket is the head of a chain, nullptr when empty
    int bucketCount;
    int count;

    // Polynomial hash: h = h * 31 + character, unsigned so overflow wraps instead of going negative
    int hash(const std::string& key) const
    {
        unsigned int h = 0;

        for (char c : key)
        {
            h = h * 31 + (unsigned char)c;
        }

        return (int)(h % bucketCount);
    }

    // Double the buckets and move every entry to its bucket in the new array
    void rehash()
    {
        Entry** oldBuckets = buckets;
        int oldBucketCount = bucketCount;

        bucketCount = bucketCount * 2;
        buckets = new Entry*[bucketCount];

        for (int i = 0; i < bucketCount; i++)
        {
            buckets[i] = nullptr;
        }

        // The index depends on bucketCount, so every entry is hashed again
        for (int i = 0; i < oldBucketCount; i++)
        {
            Entry* current = oldBuckets[i];

            while (current != nullptr)
            {
                Entry* next = current->next;
                int index = hash(current->key);
                current->next = buckets[index];
                buckets[index] = current;
                current = next;
            }
        }

        delete[] oldBuckets;
    }
};
