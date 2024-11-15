#include <iostream>
using namespace std;

template<typename T>
struct HASH_NODE{
private:

    template<typename U>
    struct Node{
        string key;       
        T value;         
        Node<U>* next;    

        Node(const string& key, const T& value) : key(key), value(value), next(nullptr) {}
    };

    Node<T>* head;    
    Node<T>* tail;   

public:
    HASH_NODE() : head(nullptr), tail(nullptr) {}

    bool isEmpty(){
        return head == nullptr;
    }

    T get(const string& key){

        Node<T>* tempNode = head;
        while (tempNode && tempNode->key != key) tempNode = tempNode->next;
        if (tempNode && tempNode->key == key)
            return tempNode->value;
        throw runtime_error("Key not found");
    }

    void push_back(const string& key, const T& value){

        Node<T>* newNode = new Node<T>(key, value);
        if (isEmpty()){
            head = newNode;
            tail = newNode;
            return;
        }
        tail->next = newNode;
        tail = newNode;
    }

    void remove(const string& key){
        if (isEmpty()) return;

        if (head->key == key){
            Node<T>* temp = head;
            head = head->next;
            delete temp;

            if (head == nullptr) tail = nullptr;

            return;
        }

        Node<T>* prev = head;
        Node<T>* current = head->next;

        while (current != nullptr){

            if (current->key == key){

                prev->next = current->next;
                if (current == tail) tail = prev;
                delete current;
                return;
            }
            prev = current;
            current = current->next;
        }
    }

    void print(){
        if (isEmpty()) return;
        Node<T>* currentNode = head;
        while (currentNode){
            cout << "[" << currentNode->key << ": " << currentNode->value << "] ";
            currentNode = currentNode->next;
        }
        cout << endl;
    }
};


template<typename T>
struct HASH{
private:
    uint32_t size_of_hash;
    HASH_NODE<T>* arr;
    uint32_t currentSize;

    uint32_t hash(const string& value){
        uint32_t hash = 5381;
        for (char c : value)
        {
            hash = ((hash << 5) + hash) + c;
        }
        return hash % size_of_hash;
    }

public:

    HASH(){
        arr = new HASH_NODE<T>[20];
        size_of_hash = 20;
        currentSize = 0;
    }

    HASH(int capacityValue){
        arr = new HASH_NODE<T>[capacityValue];  
        size_of_hash = capacityValue;
        currentSize = 0;
    }

    void HSET(const string& key, const T& value){
        uint32_t index = hash(key); 
        try {
            T currentValue = arr[index].get(key);
            arr[index].remove(key);  
            arr[index].push_back(key, value);  
        } catch (const runtime_error&) {
            arr[index].push_back(key, value);
            currentSize += 1;  
        }
    }

    T HGET(const string& key){
        uint32_t index = hash(key);  
        return arr[index].get(key);  
    }

    void HDEL(const string& key){
        uint32_t index = hash(key); 
        arr[index].remove(key);  
        currentSize -= 1;  
    }

    uint32_t size(){
        return currentSize;
    }

    void print(){
        for (uint32_t i = 0; i < size_of_hash; ++i){
            cout << "Bucket " << i << ": ";
            arr[i].print();
        }
    }
};