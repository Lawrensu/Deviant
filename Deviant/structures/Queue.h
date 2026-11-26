#pragma once

#include <cstdlib>
#include <iostream>

// Queue: first in, first out, the turn order
// The actor at the front takes its turn, then rejoins at the back
template <typename T>
class Queue
{
private:
    struct Node
    {
        T value;
        Node* next;
    };

public:
    Queue() : head(nullptr), tail(nullptr), count(0) {}

    // Free every node, the values themselves are not deleted
    ~Queue()
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
    Queue(const Queue&) = delete;
    Queue& operator=(const Queue&) = delete;

    // Join at the back
    void enqueue(const T& value)
    {
        Node* node = new Node{ value, nullptr };

        if (tail == nullptr)
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

    // Leave from the front and return the value
    T dequeue()
    {
        if (head == nullptr)
        {
            std::cerr << "Queue: dequeue on an empty queue\n";
            std::abort();
        }

        Node* removed = head;
        T value = removed->value;

        // The next node becomes the front, or the queue is now empty
        head = removed->next;

        if (head == nullptr)
        {
            tail = nullptr;
        }

        delete removed;
        count--;
        return value;
    }

    const T& front() const
    {
        if (head == nullptr)
        {
            std::cerr << "Queue: front on an empty queue\n";
            std::abort();
        }

        return head->value;
    }

    bool isEmpty() const
    {
        return count == 0;
    }

    int size() const
    {
        return count;
    }

private:
    Node* head;   // front, the next to leave
    Node* tail;   // back, the last to join
    int count;
};
