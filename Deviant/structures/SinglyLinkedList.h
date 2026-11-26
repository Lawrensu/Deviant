#pragma once

// Singly linked list: the fragment inventory, in the order fragments were taken
// Fragments are added at the end and removed from anywhere, and only ever read front to back
template <typename T>
class SinglyLinkedList
{
private:
    struct Node
    {
        T value;
        Node* next;
    };

public:
    // Iterator: walks the list front to back without exposing the nodes
    class Iterator
    {
    public:
        Iterator(Node* start) : current(start) {}

        T& operator*() const
        {
            return current->value;
        }

        // Step to the next node
        Iterator& operator++()
        {
            current = current->next;
            return *this;
        }

        bool operator!=(const Iterator& other) const
        {
            return current != other.current;
        }

    private:
        Node* current;   // nullptr is end()
    };

    SinglyLinkedList() : head(nullptr), tail(nullptr), count(0) {}

    // Free every node, the values themselves are not deleted
    ~SinglyLinkedList()
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
    SinglyLinkedList(const SinglyLinkedList&) = delete;
    SinglyLinkedList& operator=(const SinglyLinkedList&) = delete;

    // Add at the end, the tail pointer makes this O(1)
    void pushBack(const T& value)
    {
        Node* node = new Node{ value, nullptr };

        if (head == nullptr)
        {
            head = node;
            tail = node;
        }
        else
        {
            tail->next = node;
            tail = node;
        }

        count++;
    }

    // Remove the first match, true when found
    bool remove(const T& value)
    {
        Node* previous = nullptr;
        Node* current = head;

        while (current != nullptr)
        {
            if (current->value == value)
            {
                // Unlink: either the head moves on, or the previous node skips over it
                if (previous == nullptr)
                {
                    head = current->next;
                }
                else
                {
                    previous->next = current->next;
                }

                // Removing the last node moves the tail back
                if (current == tail)
                {
                    tail = previous;
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

    bool contains(const T& value) const
    {
        Node* current = head;

        while (current != nullptr)
        {
            if (current->value == value)
            {
                return true;
            }

            current = current->next;
        }

        return false;
    }

    int size() const
    {
        return count;
    }

    Iterator begin() const
    {
        return Iterator(head);
    }

    Iterator end() const
    {
        return Iterator(nullptr);
    }

private:
    Node* head;
    Node* tail;
    int count;
};
