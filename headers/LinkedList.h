#pragma once
#include <iostream>
#include <string>
#include <stdexcept>
#include <fstream>
#include <cmath>
#include <cstring> 
#include <sstream>
#include <cstdint>
using namespace std;


template<typename T>
struct Node {
    T value;
    Node<T>* next;
    Node<T>* prev;

    Node(T value) : value(value), next(nullptr), prev(nullptr) {}
};

template<typename T>
struct LinkedList {
private:

    int currentSize;
    Node<T>* head;
    Node<T>* tail;

public:

    LinkedList(){
        currentSize = 0;
        head = nullptr;
        tail = nullptr;
    }


    bool is_empty() {
        return head == nullptr;
    }           

    void addhead(T name) {
        Node<T>* newnode = new Node<T>(name);

        if (head == nullptr) {
            head = newnode;
            tail = newnode;
        } else {
            newnode->next = head;
            head->prev = newnode;
            head = newnode;
        }
        currentSize++;
    } 

    void addtail(T name) {
        Node<T>* newnode = new Node<T>(name);

        if (head == nullptr) {
            head = newnode;
            tail = newnode;
        } else {
            tail->next = newnode;
            newnode->prev = tail;
            tail = newnode;
        }
        currentSize++;
    } 

    void delhead() {
        if (is_empty()) {
            throw runtime_error("LinkedList is empty");
        }
        Node<T>* next_node = head->next;

        if (next_node != nullptr) {
            next_node->prev = nullptr;
        } else {
            tail = nullptr;
        }

        delete head;
        head = next_node;
        currentSize--;
    }   

    void deltail() {
        if (is_empty()) {
            throw runtime_error("LinkedList is empty");
        }

        Node<T>* prev_node = tail->prev;

        if (prev_node != nullptr) {
            prev_node->next = nullptr;
        } else {
            head = nullptr;
        }

        delete tail;
        tail = prev_node;
        currentSize--;
    }             

    void remove(T name) {
        if (is_empty()) {
            throw runtime_error("LinkedList is empty");
        }

        Node<T>* current = head;
        bool found = false;

        while (current != nullptr) {

            if (current->value == name) {

                if (current == head) {
                    head = current->next;

                    if (head != nullptr) {
                        head->prev = nullptr;
                    } else {
                        tail = nullptr;
                    }
                } else if (current == tail) {
                    tail = current->prev;
                    tail->next = nullptr;
                } else {
                    current->prev->next = current->next; // The next node for the previous one
                    current->next->prev = current->prev; // The previous node for the next one
                }
                currentSize--;
                delete current; // Deleting the node
                found = true;
                break;
            }
            current = current->next;
        }

        if (!found) {
            cout << "Element \"" << name << "\" not found in LinkedList!!!" << "\n";
        }
    }      

    bool search(T name) {
        if (is_empty()) {   
            return false;
            throw runtime_error("LinkedList is empty");
        }

        Node<T>* current = head;

        while (current != nullptr) {

            if (current->value == name) {
                cout << "Element \"" << current->value << "\" was found in LinkedList!!!" << "\n";
                return true;
            }
            current = current->next;
        }
        return false;
        throw runtime_error("Element not found in LinkedList!!!");
    }

    auto get(int index) {
        if (is_empty()) {
            throw runtime_error("LinkedList is empty");
        }

        Node<T>* current = head;
        int currentIndex = 0;

        while (current != nullptr) {
            if (currentIndex == index) {
                
                return current->value;
            }
            current = current->next;
            currentIndex++;
        }
        throw runtime_error("Element not found in LinkedList!!!");
    }

    int size(){
        return currentSize;
    }

    bool contains(const T& value) const {
        Node<T>* current = head;
        while (current != nullptr) {
            if (current->value == value) {
                return true;
            }
            current = current->next;
        }
        return false;
    }


    void print() {
        Node<T>* current = head;
        while (current != nullptr) {
            cout << current->value << " ";
            current = current->next;
        }
        cout << "\n";
    }    
};