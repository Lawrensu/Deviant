#pragma once

#include <cstdlib>
#include <iostream>

// Doubly linked list: the action history, one command per node
// Rewind removes from the tail and Fable reads backward from it, the reveal replays forward from the head
template <typename T>
class DoublyLinkedList
{
private:
    struct Node
    {
        T value;
        Node* prev;
        Node* next;
    };

public:
    // Iterator: walks the list both ways without exposing the nodes
    class Iterator
    {
    public:
        Iterator(Node* start) : current(start) {}

        T& operator*() const
        {
            return current->value;
        }

        // Step forward, toward the tail
        Iterator& operator++()
        {
            current = current->next;
            return *this;
        }

        // Step backward, toward the head
        Iterator& operator--()
        {
            current = current->prev;
            return *this;
        }

        bool operator!=(const Iterator& other) const
        {
            return current != other.current;
        }

    private:
        Node* current;   // nullptr is end(), past either end
    };

    DoublyLinkedList() : head(nullptr), tail(nullptr), count(0) {}

    // Free every node, the values themselves are not deleted
    ~DoublyLinkedList()
    {
        Node* current = head;

        while (current != nullptr)
        {
            Node* next = current->next;
            delete current;
            current = next;
        }
    }

    // Copies would share nodes and free them twice
    DoublyLinkedList(const DoublyLinkedList&) = delete;
    DoublyLinkedList& operator=(const DoublyLinkedList&) = delete;

    // Add at the tail
    void pushBack(const T& value)
    {
        Node* node = new Node{ value, tail, nullptr };

        if (head == nullptr)
        {
            head = node;
        }
        else
        {
            tail->next = node;
        }

        tail = node;
        count++;
    }

    // Remove the tail and return its value, used by rewind
    T popBack()
    {
        if (tail == nullptr)
        {
            std::cerr << "DoublyLinkedList: popBack on an empty list\n";
            std::abort();
        }

        Node* removed = tail;
        T value = removed->value;

        // The previous node becomes the tail, or the list is now empty
        tail = removed->prev;

        if (tail == nullptr)
        {
            head = nullptr;
        }
        else
        {
            tail->next = nullptr;
        }

        delete removed;
        count--;
        return value;
    }

    const T& back() const
    {
        if (tail == nullptr)
        {
            std::cerr << "DoublyLinkedList: back on an empty list\n";
            std::abort();
        }

        return tail->value;
    }

    int size() const
    {
        return count;
    }

    // Forward from the head
    Iterator begin() const
    {
        return Iterator(head);
    }

    Iterator end() const
    {
        return Iterator(nullptr);
    }

    // Backward from the tail, -- until end()
    Iterator last() const
    {
        return Iterator(tail);
    }

private:
    Node* head;
    Node* tail;
    int count;
};
