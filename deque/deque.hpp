#ifndef SJTU_DEQUE_HPP
#define SJTU_DEQUE_HPP

#include "exceptions.hpp"

#include <cstddef>

namespace sjtu {
    const int blockSize = 300;

    template<class T>
    class deque {
        friend class iterator;
        friend class const_iterator;

    private:
        struct Node {
            int size;
            T **data;

            Node *prev;
            Node *next;

            Node() {
                size = 0;
                prev = next = NULL;
                data = new T *[blockSize + 1];
            }

            Node(const Node &other) {
                size = other.size;
                prev = next = NULL;

                data = new T *[blockSize + 1];
                for (int i = 0; i < other.size; ++i) {
                    data[i] = new T(*other.data[i]);
                }
            }

            Node &operator=(const Node &other) {
                if (this == &other)
                    return *this;

                clear();

                size = other.size;

                data = new T *[blockSize + 1];
                for (int i = 0; i < other.size; ++i)
                    data[i] = new T(*other.data[i]);

                return *this;
            }

            ~Node() {
                clear();
            }

            void clear() {
                for (int i = 0; i < size; ++i) {
                    delete data[i];
                }
                delete [] data;
                size = 0;
            }

            T &at(int idx) {
                if (idx < 0 || idx >= size)
                    throw index_out_of_bound();

                return *(data[idx]);
            }

            void insert(int idx, const T &value) {
                for (int i = size; i > idx; --i)
                    data[i] = data[i - 1];

                data[idx] = new T(value);
                size++;
            }

            void erase(int idx) {
                delete data[idx];

                for (size_t i = idx + 1; i < size; ++i)
                    data[i - 1] = data[i];

                size--;
            }

        };

        int tot;
        Node *head, *tail;

        void adjust(Node *node) {
            if (node == tail)
                return;

            if (node->size < blockSize / 2) {
                merge(node);
            }
            else if (node->size > blockSize) {
                split(node);
            }
        }

        void merge(Node *node) {
            while (node->next != tail && node->size + node->next->size <= blockSize) {
                while (node->next != tail && node->size == 0) {
                    Node *p = node;
                    node = node->next;
                    delete p;
                }

                Node *p = node->next;
                for (int i = node->size; i < node->size + p->size; ++i) {
                    node->data[i] = p->data[i - node->size];
                }
                node->size += p->size;
                p->size = 0;

                node->next = p->next;
                node->next->prev = node;

                delete p;
            }

            if (node->next != tail && node->size < blockSize / 2) {
                Node *p = node->next;
                int tmp = std::min(p->size, blockSize / 2 - node->size);

                for (int i = 0; i < tmp; ++i) {
                    node->data[node->size + i] = node->next->data[i];
                }

                node->size += tmp;
                int tmp2 = p->size - tmp;
                if (tmp2 == 0) {
                    node->next = p->next;
                    node->next->prev = node;
                    delete p;
                }
                else {
                    for (int i = 0; i < tmp2; ++i) {
                        p->data[i] = p->data[i + tmp];
                    }
                    p->size -= tmp;
                }
            }

        }

        void split(Node *node) {
            Node *p = new Node;
            p->prev = node;
            p->next = node->next;
            node->next->prev = p;
            node->next = p;

            int tmp = node->size / 2;
            for (int i = tmp; i < node->size; ++i) {
                p->data[i - tmp] = node->data[i];
            }
            p->size = node->size - tmp;
            node->size = tmp;
        }

    public:
        class const_iterator;

        class iterator {
            friend class deque;
            friend class const_iterator;
        private:
            deque *deq;
            Node *nod;
            int idx;

        public:
            iterator(deque *deq = NULL, Node *nod = NULL, int idx = 0) : deq(deq), nod(nod), idx(idx) {}

            iterator(const iterator &rhs) : deq(rhs.deq), nod(rhs.nod), idx(rhs.idx){}
            /**
             * return a new iterator which pointer n-next elements
             *   even if there are not enough elements, the behaviour is **undefined**.
             * as well as operator-
             */
            iterator operator+(const int &n) const {
                iterator ret = *this;
                if (n < 0) {
                    int dis = -n;
                    while (ret.nod->prev != NULL && ret.idx < dis) {
                        dis -= ret.idx + 1;
                        ret.nod = ret.nod->prev;
                        ret.idx = ret.nod->size - 1;
                    }
                    if (ret.idx < dis && ret.nod->prev == NULL) {
                        throw index_out_of_bound();
                    }
                    ret.idx -= dis;
                }
                else if (n > 0) {
                    int dis = n;
                    while (ret.nod->next != ret.deq->tail &&
                           (ret.idx >= ret.nod->size - dis)) {
                        dis -= ret.nod->size - ret.idx;
                        ret.idx = 0;
                        ret.nod = ret.nod->next;
                    }
                    if (ret.nod->next == NULL) {
                        throw index_out_of_bound();
                    }
                    ret.idx += dis;
                }
                return ret;
            }

            iterator operator-(const int &n) const {
                return operator+(-n);
            }

            // return th distance between two iterator,
            // if these two iterators points to different vectors, throw invaild_iterator.
            int operator-(const iterator &rhs) const {
                if (deq != rhs.deq) {
                    throw invalid_iterator();
                }
                else if (nod == rhs.nod) {
                    return idx - rhs.idx;
                }
                else {
                    int dis = rhs.nod->size - rhs.idx;
                    Node *p = rhs.nod->next;
                    while (p != NULL && p != nod) {
                        dis += p->size;
                        p = p->next;
                    }
                    if (p != NULL) {
                        return dis + idx;
                    }
                    else {
                        p = nod->next;
                        dis = nod->size - idx;
                        while (p != NULL && p != rhs.nod) {
                            dis += p->size;
                            p = p->next;
                        }
                        if (p != NULL) {
                            return -(dis + rhs.idx);
                        }
                        else {
                            throw invalid_iterator();
                        }
                    }
                }
            }

            iterator operator+=(const int &n) {
                *this = (*this) + n;
                return *this;
            }

            iterator operator-=(const int &n) {
                *this = (*this) - n;
                return *this;
            }

            /**
             * TODO iter++
             */
            iterator operator++(int) {
                iterator ret = *this;

                if (idx == nod->size - 1) {
                    idx = 0;
                    if (nod->next == NULL) {
                        throw invalid_iterator();
                    }
                    else {
                        nod = nod->next;
                    }
                }
                else {
                    idx++;
                }

                return ret;
            }

            /**
             * TODO ++iter
             */
            iterator &operator++() {
                (*this)++;
                return *this;
            }

            /**
             * TODO iter--
             */
            iterator operator--(int) {
                iterator ret = *this;
                if (idx == 0) {
                    if (nod->prev == NULL) {
                        throw invalid_iterator();
                    }
                    else {
                        nod = nod->prev;
                        idx = nod->size - 1;
                    }
                } else {
                    idx--;
                }
                return ret;
            }

            /**
             * TODO --iter
             */
            iterator &operator--() {
                (*this)--;
                return *this;
            }

            /**
             * TODO *it
             */
            T &operator*() const {
                return nod->at(idx);
            }

            /**
             * TODO it->field
             */
            T *operator->() const noexcept {
                return nod->data[idx];
            }

            /**
             * a operator to check whether two iterators are same (pointing to the same memory).
             */
            bool operator==(const iterator &rhs) const {
                if (deq != rhs.deq)
                    return false;

                return (nod == rhs.nod && idx == rhs.idx) ||
                       (nod->next == NULL && rhs.nod->next == NULL) ||
                       (nod->next == rhs.nod && idx == nod->size && rhs.nod->next == NULL);
            }

            bool operator==(const const_iterator &rhs) const {
                if (deq != rhs.deq)
                    return false;

                return (nod == rhs.nod && idx == rhs.idx) ||
                       (nod->next == NULL && rhs.nod->next == NULL) ||
                       (nod->next == rhs.nod && idx == nod->size && rhs.nod->next == NULL);
            }

            /**
             * some other operator for iterator.
             */
            bool operator!=(const iterator &rhs) const {
                return !(*this == rhs);
            }

            bool operator!=(const const_iterator &rhs) const {
                return !(*this == rhs);
            }
        };

        class const_iterator {
            friend class deque;
            friend class iterator;
            // it should has similar member method as iterator.
            //  and it should be able to construct from an iterator.
        private:
            const deque *deq;
            Node *nod;
            int idx;
        public:
            const_iterator(const deque *deq = NULL, Node *nod = NULL, int idx = 0) : deq(deq), nod(nod), idx(idx) {}

            const_iterator(const const_iterator &other) : deq(other.deq), nod(other.nod), idx(other.idx) {
            }

            const_iterator(const iterator &other) : deq(other.deq), nod(other.nod), idx(other.idx) {
            }

            const_iterator operator+(const int &n) const {
                const_iterator ret = *this;
                if (n < 0) {
                    int dis = -n;
                    while (ret.nod->prev != NULL && ret.idx < dis) {
                        dis -= ret.idx + 1;
                        ret.nod = ret.nod->prev;
                        ret.idx = ret.nod->size - 1;
                    }
                    if (ret.idx < dis && ret.nod->prev == NULL) {
                        throw index_out_of_bound();
                    }
                    ret.idx -= dis;
                }
                else if (n > 0) {
                    int dis = n;
                    while (ret.nod->next != ret.deq->tail &&
                           (ret.idx >= ret.nod->size - dis)) {
                        dis -= ret.nod->size - ret.idx;
                        ret.idx = 0;
                        ret.nod = ret.nod->next;
                    }
                    if (ret.nod->next == NULL) {
                        throw index_out_of_bound();
                    }
                    ret.idx += dis;
                }
                return ret;
            }

            const_iterator operator-(const int &n) const {
                return operator+(-n);
            }

            // return th distance between two iterator,
            // if these two iterators points to different vectors, throw invaild_iterator.
            int operator-(const const_iterator &rhs) const {
                if (deq != rhs.deq) {
                    throw invalid_iterator();
                }
                else if (nod == rhs.nod) {
                    return idx - rhs.idx;
                }
                else {
                    int dis = rhs.nod->size - rhs.idx;
                    Node *p = rhs.nod->next;
                    while (p != NULL && p != nod) {
                        dis += p->size;
                        p = p->next;
                    }
                    if (p != NULL) {
                        return dis + idx;
                    }
                    else {
                        p = nod->next;
                        dis = nod->size - idx;
                        while (p != NULL && p != rhs.nod) {
                            dis += p->size;
                            p = p->next;
                        }
                        if (p != NULL) {
                            return -(dis + rhs.idx);
                        }
                        else {
                            throw invalid_iterator();
                        }
                    }
                }
            }

            const_iterator operator+=(const int &n) {
                *this = (*this) + n;
                return *this;
            }

            const_iterator operator-=(const int &n) {
                *this = (*this) - n;
                return *this;
            }

            /**
             * TODO iter++
             */
            const_iterator operator++(int) {
                const_iterator ret = *this;

                if (idx == nod->size - 1) {
                    idx = 0;
                    if (nod->next == NULL) {
                        throw invalid_iterator();
                    }
                    else {
                        nod = nod->next;
                    }
                }
                else {
                    idx++;
                }

                return ret;
            }

            /**
             * TODO ++iter
             */
            const_iterator &operator++() {
                (*this)++;
                return *this;
            }

            /**
             * TODO iter--
             */
            const_iterator operator--(int) {
                const_iterator ret = *this;
                if (idx == 0) {
                    if (nod->prev == NULL) {
                        throw invalid_iterator();
                    }
                    else {
                        nod = nod->prev;
                        idx = nod->size - 1;
                    }
                } else {
                    idx--;
                }
                return ret;
            }

            /**
             * TODO --iter
             */
            const_iterator &operator--() {
                (*this)--;
                return *this;
            }

            /**
             * TODO *it
             */
            const T &operator*() const {
                return nod->at(idx);
            }

            /**
             * TODO it->field
             */
            const T *operator->() const noexcept {
                return nod->data[idx];
            }

            /**
             * a operator to check whether two iterators are same (pointing to the same memory).
             */
            bool operator==(const iterator &rhs) const {
                if (deq != rhs.deq)
                    return false;

                return (nod == rhs.nod && idx == rhs.idx) ||
                       (nod->next == NULL && rhs.nod->next == NULL) ||
                       (nod->next == rhs.nod && idx == nod->size && rhs.nod->next == NULL);
            }

            bool operator==(const const_iterator &rhs) const {
                if (deq != rhs.deq)
                    return false;

                return (nod == rhs.nod && idx == rhs.idx) ||
                       (nod->next == NULL && rhs.nod->next == NULL) ||
                       (nod->next == rhs.nod && idx == nod->size && rhs.nod->next == NULL);
            }

            /**
             * some other operator for iterator.
             */
            bool operator!=(const iterator &rhs) const {
                return !(*this == rhs);
            }

            bool operator!=(const const_iterator &rhs) const {
                return !(*this == rhs);
            }
        };

        /**
         * TODO Constructors
         */
        deque() {
            tot = 0;

            head = new Node();
            tail = new Node();

            head->next = tail;
            tail->prev = head;
        }

        deque(const deque &other) {
            tot = other.tot;

            head = new Node(*other.head);
            tail = new Node();
            Node *p = head;
            Node *q = other.head->next;
            while (q != other.tail) {
                p->next = new Node(*q);
                p->next->prev = p;
                p = p->next;
                q = q->next;
            }
            p->next = tail;
            tail->prev = p;

        }

        /**
         * TODO Deconstructor
         */
        ~deque() {
            clear();
            delete head;
            delete tail;
        }

        /**
         * TODO assignment operator
         */
        deque &operator=(const deque &other) {
            if (this == &other)
                return *this;

            clear();
            tot = other.tot;
            delete head;
            head = new Node(*other.head);

            Node *p = head, *q = other.head->next;
            while (q != other.tail) {
                p->next = new Node(*q);
                p->next->prev = p;
                p = p->next;
                q = q->next;
            }
            p->next = tail;
            tail->prev = p;

            return *this;
        }

        /**
         * access specified element with bounds checking
         * throw index_out_of_bound if out of bound.
         */
        T &at(const size_t &pos) {
            Node *p = head;
            int n = pos;

            while (p != tail && n >= p->size) {
                n -= p->size;
                p = p->next;
            }

            if (p == tail)
                throw index_out_of_bound();

            return p->at(n);
        }

        const T &at(const size_t &pos) const {
            Node *p = head;
            int n = pos;

            while (p != tail && n >= p->size) {
                n -= p->size;
                p = p->next;
            }

            if (p == tail)
                throw index_out_of_bound();

            return p->at(n);
        }

        T &operator[](const size_t &pos) {
            return at(pos);
        }

        const T &operator[](const size_t &pos) const {
            return at(pos);
        }

        /**
         * access the first element
         * throw container_is_empty when the container is empty.
         */
        const T &front() const {
            if (empty())
                throw container_is_empty();

            return head->at(0);
        }

        /**
         * access the last element
         * throw container_is_empty when the container is empty.
         */
        const T &back() const {
            if (empty())
                throw container_is_empty();

            return tail->prev->at(tail->prev->size - 1);
        }

        /**
         * returns an iterator to the beginning.
         */
        iterator begin() {
            if (tot == 0)
                return end();
            else
                return iterator(this, head, 0);
        }

        const_iterator cbegin() const {
            if (tot == 0)
                return cend();
            else
                return const_iterator(this, head, 0);
        }

        /**
         * returns an iterator to the end.
         */
        iterator end() {
            return iterator(this, tail, 0);
        }

        const_iterator cend() const {
            return const_iterator(this, tail, 0);
        }

        /**
         * checks whether the container is empty.
         */
        bool empty() const {
            return tot == 0;
        }

        /**
         * returns the number of elements
         */
        size_t size() const {
            return tot;
        }

        /**
         * clears the contents
         */
        void clear() {
            Node *p = head->next;
            Node *q;
            delete head;
            head = new Node();
            head->next = tail;
            tail->prev = head;
            while (p->next) {
                q = p->next;
                delete p;
                p = q;
            }
            tot = 0;
        }

        /**
         * inserts elements at the specified locat on in the container.
         * inserts value before pos
         * returns an iterator pointing to the inserted value
         *     throw if the iterator is invalid or it point to a wrong place.
         */
        iterator insert(iterator pos, const T &value) {
            if (this != pos.deq)
                throw invalid_iterator();

            if (pos.nod->prev != NULL && pos.idx == 0) {
                pos = iterator(this, pos.nod->prev, pos.nod->prev->size);
            }

            adjust(pos.nod);

            while (pos.nod != tail && pos.idx > pos.nod->size) {
                pos.idx -= pos.nod->size;
                pos.nod = pos.nod->next;
            }

            if (pos.nod == tail)
                throw index_out_of_bound();

            pos.nod->insert(pos.idx, value);

            tot++;
            return pos;
        }

        /**
         * removes specified element at pos.
         * removes the element at pos.
         * returns an iterator pointing to the following element, if pos pointing to the last element, end() will be returned.
         * throw if the container is empty, the iterator is invalid or it points to a wrong place.
         */
        iterator erase(iterator pos) {
            if (this != pos.deq || pos.nod == tail)
                throw invalid_iterator();

            while (pos.idx < 0) {
                pos.nod = pos.nod->prev;
                pos.idx += pos.nod->size;
            }

            adjust(pos.nod);

            while (pos.nod != tail && pos.idx >= pos.nod->size) {
                pos.idx -= pos.nod->size;
                pos.nod = pos.nod->next;
            }
            if (pos.nod == tail)
                throw index_out_of_bound();


            pos.nod->erase(pos.idx);
            if (pos.idx == pos.nod->size) {
                pos.idx = 0;
                pos.nod = pos.nod->next;
            }
            tot--;

            return pos;
        }

        /**
         * adds an element to the end
         */
        void push_back(const T &value) {
            insert(iterator(this, tail->prev, tail->prev->size), value);
        }

        /**
         * removes the last element
         *     throw when the container is empty.
         */
        void pop_back() {
            if (empty())
                throw container_is_empty();

            erase(iterator(this, tail->prev, tail->prev->size - 1));
        }

        /**
         * inserts an element to the beginning.
         */
        void push_front(const T &value) {
            insert(iterator(this, head, 0), value);
        }

        /**
         * removes the first element.
         *     throw when the container is empty.
         */
        void pop_front() {
            if (empty())
                throw container_is_empty();

            erase(iterator(this, head, 0));
        }
    };

}

#endif
