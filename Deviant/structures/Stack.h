#pragma once

#include <cstdlib>
#include <iostream>

// Stack: last in, first out, on a fixed capacity array
// Used for the fragment chain (the chain limit is the capacity) and the directory tree traversal
template <typename T, int Capacity>
class Stack
{
public:
    Stack() : count(0) {}

    // Add on top, false when full
    bool push(const T& value)
    {
        // Full, refuse the push
        if (count == Capacity)
        {
            return false;
        }

        items[count] = value;
        count++;
        return true;
    }

    // Remove the top and return it
    T pop()
    {
        if (count == 0)
        {
            std::cerr << "Stack: pop on an empty stack\n";
            std::abort();
        }

        count--;
        return items[count];
    }

    const T& top() const
    {
        if (count == 0)
        {
            std::cerr << "Stack: top on an empty stack\n";
            std::abort();
        }

        return items[count - 1];
    }

    // Read any item, the bottom is 0, used to build the combination key in order
    const T& at(int index) const
    {
        if (index < 0 || index >= count)
        {
            std::cerr << "Stack: at(" << index << ") is outside the stack\n";
            std::abort();
        }

        return items[index];
    }

    bool isEmpty() const
    {
        return count == 0;
    }

    bool isFull() const
    {
        return count == Capacity;
    }

    int size() const
    {
        return count;
    }

private:
    T items[Capacity];
    int count;   // also the index of the next free slot
};
