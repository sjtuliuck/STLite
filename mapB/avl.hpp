/**
 * implement a container like std::map
 */
#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

// only for std::less<T>
#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"

namespace sjtu {
    inline size_t max(size_t a, size_t b) {
        if (a > b)
            return a;
        else return b;
    }

    template<
            class Key,
            class T,
            class Compare = std::less<Key>
    > class map {
    public:
        /**
         * the internal type of data.
         * it should have a default constructor, a copy constructor.
         * You can use sjtu::map as value_type by typedef.
         */
        typedef pair<const Key, T> value_type;
        /**
         * see BidirectionalIterator at CppReference for help.
         *
         * if there is anything wrong throw invalid_iterator.
         *     like it = map.begin(); --it;
         *       or it = map.end(); ++end();
         */
    private:
        Compare cmp;
        struct node{
            value_type *data;
            node *lc, *rc;
            node *father;
            size_t h;

            node(value_type *v = NULL) : data(v) {
                lc = rc = NULL;
                father = NULL;
                h = 1;
            }

            ~node() {
                if (data)
                    delete data;
            }

            size_t height(const node *t) {
                if (t == NULL)
                    return 0;
                return t->h;
            }

            void update() {
                h = max(height(lc), height(rc)) + 1;
            }
        };

        size_t height(const node *t) {
            if (t == NULL)
                return 0;
            return t->h;
        }

        node *root;
        size_t _size;

        void LL(node *&t) {
            if (t->lc == NULL)
                return;
            node *t1 = t->lc;

            t->lc = t1->rc;
            if (t1->rc)
                t1->rc->father = t;
            if (t1->rc)
                t1->rc = t;

            t->father = t1;

            t->update();
            t1->update();

            t = t1;
        }

        void RR(node *&t) {
            if (t->rc == NULL)
                return;
            node *t1 = t->rc;

            t->rc = t1->lc;
            if (t1->lc)
                t1->lc->father = t;
            if (t1->lc)
                t1->lc = t;
            t->father = t1;

            t->update();
            t1->update();

            t = t1;
        }

        void LR(node *&t) {
            RR(t->lc);
            LL(t);
        }

        void RL(node *&t) {
            LL(t->rc);
            RR(t);
        }


        node *insert(const value_type &v, node *&t, node *fa) {
            //*ret!!!
            node *ret;
            if (t == NULL) {
                t = new node(new value_type(v));
                t->father = fa;
                return t;
            }
            else if (cmp(v.first, t->data->first)) {
                ret = insert(v, t->lc, t);
                if (height(t->lc) - height(t->rc) == 2) {
                    if (cmp(v.first, t->lc->data->first))
                        LL(t);
                    else
                        LR(t);
                }
            }
            else if (cmp(t->data->first, v.first)) {
                ret = insert(v, t->rc, t);
                if (height(t->rc) - height(t->lc) == 2) {
                    if (cmp(t->rc->data->first, v.first))
                        RR(t);
                    else
                        RL(t);
                }
            }

            t->update();
            return ret;
        }

        //P218 yellow
        bool remove(const value_type &v, node *&t) {
            bool stop = false;
            int subTree;

            if (t == NULL)
                return true;
            if (cmp(v.first, t->data->first)) {
                stop = remove(v, t->lc);
                subTree = 0;
            }
            else if (cmp(t->data->first, v.first)) {
                stop = remove(v, t->rc);
                subTree = 1;
            }
            else if (t->lc != NULL && t->rc != NULL) {
                node *tmp = t->rc;
                while (t->lc != NULL) {
                    t = t->lc;
                }
                if (t->data)
                    delete t->data;
                t->data = new value_type(*(tmp->data));
                stop = remove(*(tmp->data), t->rc);
                subTree = 1;
            }
            else {
                node *oldNode = t;
                t = (t->lc != NULL) ? t->lc : t->rc;
                if (t != NULL)
                    t->father = oldNode->father;
                delete oldNode;
                return false;
            }

            //TODO
            if (stop)
                return true;

            int bf;
            switch(subTree) {
                case 0:
                    bf = height(t->lc) - height(t->rc) + 1;
                    if (bf == 0)    return true;
                    if (bf == 1)    return false;
                    if (bf == -1) {
                        int bfr = t->height(t->rc->lc) - height(t->rc->rc);
                        switch(bfr) {
                            case 0:
                                RR(t);
                                return true;
                            case -1:
                                RR(t);
                                return false;
                            default:
                                RL(t);
                                return false;
                        }
                    }
                    break;

                case 1:
                    bf = height(t->lc) - height(t->rc) - 1;
                    if (bf == 0)    return true;
                    if (bf == -1)   return false;
                    if (bf == 1) {
                        int bfl = height(t->lc->lc) - height(t->lc->rc);
                        switch(bfl) {
                            case 0:
                                LL(t);
                                return true;
                            case 1:
                                LL(t);
                                return false;
                            default:
                                LR(t);
                                return false;
                        }
                    }
            }

        }

        void makeEmpty(node *t) {
            if (t == NULL)
                return;

            makeEmpty(t->lc);
            makeEmpty(t->rc);
            delete t;
        }

        bool islc(node *t) const {
            if (t == root)
                return false;
            return t->father->lc == t;
        }

        bool isrc(node *t) const {
            if (t == root)
                return false;
            return t->father->rc == t;
        }

        node *prev(node *t) const {
            if (t->lc != NULL) {
                t = t->lc;
                while (t->rc != NULL) {
                    t = t->rc;
                }
                return t;
            }
            else {
                while (islc(t) && t != root) {
                    t = t->father;
                }
                return t->father;
            }
        }

        node *next(node *t) const {

            if (t->rc != NULL) {
                t = t->rc;
                while (t->lc != NULL) {
                    t = t->lc;
                }
                return t;
            }
            else {
                while (isrc(t) && t != root) {
                    t = t->father;
                }
                return t->father;
            }
        }

    public:
        class const_iterator;
        class iterator {
            friend class map;
            friend class const_iterator;
        private:
            /**
             * TODO add data members
             *   just add whatever you want.
             */
             map *mp;
             node *nod;

        public:
            iterator(map *m = NULL, node *n = NULL) : mp(m), nod(n) {
                // TODO

            }
            iterator(const iterator &other) : mp(other.mp), nod(other.nod) {
                // TODO
            }
            /**
             * return a new iterator which pointer n-next elements
             *   even if there are not enough elements, just return the answer.
             * as well as operator-
             */
            /**
             * TODO iter++
             */
            iterator operator++(int) {
                if (*this == mp->end())
                    throw invalid_iterator();


                iterator ret = *this;
                nod = mp->next(nod);
                return ret;
            }
            /**
             * TODO ++iter
             */
            iterator & operator++() {
                if (*this == mp->end())
                    throw invalid_iterator();

                nod = mp->next(nod);
                return *this;
            }
            /**
             * TODO iter--
             */
            iterator operator--(int) {
                if (*this == mp->begin())
                    throw invalid_iterator();

                iterator ret = *this;
                if (*this == mp->end()) {
                    nod = mp->root;

                    while (nod->rc != NULL)
                        nod = nod->rc;
                }
                else
                    nod = mp->prev(nod);
                return ret;
            }
            /**
             * TODO --iter
             */
            iterator & operator--() {
                if (*this == mp->begin())
                    throw invalid_iterator();

                if (*this == mp->end())
                {
                    nod = mp->root;
                    while (nod->rc != NULL)
                        nod = nod->rc;
                }
                else
                    nod = mp->prev(nod);
                return *this;
            }
            /**
             * a operator to check whether two iterators are same (pointing to the same memory).
             */
            value_type & operator*() const {
                return *(nod->data);
            }
            bool operator==(const iterator &rhs) const {
                return mp == rhs.mp && nod == rhs.nod;
            }
            bool operator==(const const_iterator &rhs) const {
                return mp == rhs.mp && nod == rhs.nod;
            }
            /**
             * some other operator for iterator.
             */
            bool operator!=(const iterator &rhs) const {
                return !(mp == rhs.mp && nod == rhs.nod);
            }
            bool operator!=(const const_iterator &rhs) const {
                return !(mp == rhs.mp && nod == rhs.nod);
            }

            /**
             * for the support of it->first.
             * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
             */
            value_type* operator->() const noexcept {
                return nod->data;
            }
        };
        class const_iterator {
            friend class map;
            friend class const_iterator;
            // it should has similar member method as iterator.
            //  and it should be able to construct from an iterator.
        private:
            // data members.
            const map *mp;
            node *nod;
        public:
            const_iterator(const map *m = NULL, node *n = NULL) : mp(m), nod(n) {
                // TODO
            }
            const_iterator(const const_iterator &other) : mp(other.mp), nod(other.nod) {
                // TODO
            }
            const_iterator(const iterator &other) : mp(other.mp), nod(other.nod) {
                // TODO
            }
            // And other methods in iterator.
            /**
             * TODO iter++
             */
            const_iterator operator++(int) {
                if (*this == mp->cend())
                    throw invalid_iterator();

                const_iterator ret = *this;
                nod = mp->next(nod);
                return ret;
            }
            /**
             * TODO ++iter
             */
            const_iterator & operator++() {
                if (*this == mp->cend())
                    throw invalid_iterator();

                nod = mp->next(nod);
                return *this;
            }
            /**
             * TODO iter--
             */
            const_iterator operator--(int) {
                if (*this == mp->cbegin())
                    throw invalid_iterator();

                const_iterator ret = *this;
                if (*this == mp->cend()) {
                    nod = mp->root;

                    while (nod->rc != NULL)
                        nod = nod->rc;
                }
                else
                    nod = mp->prev(nod);
                return ret;
            }
            /**
             * TODO --iter
             */
            const_iterator & operator--() {
                if (*this == mp->cbegin())
                    throw invalid_iterator();

                if (*this == mp->cend())
                {
                    nod = mp->root;
                    while (nod->rc != NULL)
                        nod = nod->rc;
                }
                else
                    nod = mp->prev(nod);
                return *this;
            }
            /**
             * a operator to check whether two iterators are same (pointing to the same memory).
             */
            const value_type & operator*() const {
                return *(nod->data);
            }
            bool operator==(const iterator &rhs) const {
                return mp == rhs.mp && nod == rhs.nod;
            }
            bool operator==(const const_iterator &rhs) const {
                return mp == rhs.mp && nod == rhs.nod;
            }
            /**
             * some other operator for iterator.
             */
            bool operator!=(const iterator &rhs) const {
                return !(mp == rhs.mp && nod == rhs.nod);
            }
            bool operator!=(const const_iterator &rhs) const {
                return !(mp == rhs.mp && nod == rhs.nod);
            }

            /**
             * for the support of it->first.
             * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
             */
            const value_type* operator->() const noexcept {
                return nod->data;
            }
        };



    public:
        /**
         * TODO two constructors
         */
        map() {
            root = NULL;
            _size = 0;
        }
        map(const map &other) {
            root = NULL;
            _size = 0;

            for (const_iterator it = other.cbegin(); it != other.cend(); ++it) {
                insert(*it);
            }
        }
        /**
         * TODO assignment operator
         */
        map & operator=(const map &other) {
            if (this == &other)
                return *this;
            clear();
            root = NULL;
            _size = 0;

            for (const_iterator it = other.cbegin(); it != other.cend(); ++it) {
                insert(*it);
            }

            return *this;
        }
        /**
         * TODO Destructors
         */
        ~map() {
            clear();
        }
        /**
         * TODO
         * access specified element with bounds checking
         * Returns a reference to the mapped value of the element with key equivalent to key.
         * If no such element exists, an exception of type `index_out_of_bound'
         */
        T & at(const Key &key) {
            iterator it = find(key);
            if (it == end())
                throw index_out_of_bound();
            return it->second;
        }
        const T & at(const Key &key) const {
            const_iterator it = find(key);
            if (it == cend())
                throw index_out_of_bound();
            return it->second;
        }
        /**
         * TODO
         * access specified element
         * Returns a reference to the value that is mapped to a key equivalent to key,
         *   performing an insertion if such key does not already exist.
         */
        T & operator[](const Key &key) {
            node *t = root;

            while (t != NULL && (cmp(key, t->data->first) || cmp(t->data->first, key))) {
                if (cmp(key, t->data->first))
                    t = t->lc;
                else
                    t = t->rc;
            }

            if (t != NULL)
                return t->data->second;

            t = insert(value_type(key, T()), root, NULL);
            return t->data->second;
        }
        /**
         * behave like at() throw index_out_of_bound if such key does not exist.
         */
        const T & operator[](const Key &key) const {
            at(key);
        }
        /**
         * return a iterator to the beginning
         */
        iterator begin() {
            node *tmp = root;
            while (tmp->lc != nullptr) {
                tmp = tmp->lc;
            }

            return iterator(this, tmp);
        }
        const_iterator cbegin() const {
            node *tmp = root;
            while (tmp->lc != nullptr) {
                tmp = tmp->lc;
            }

            return const_iterator(this, tmp);
        }
        /**
         * return a iterator to the end
         * in fact, it returns past-the-end.
         */
        iterator end() {
            return iterator(this, NULL);
        }
        const_iterator cend() const {
            return const_iterator(this, NULL);
        }
        /**
         * checks whether the container is empty
         * return true if empty, otherwise false.
         */
        bool empty() const {
            return _size == 0;
        }
        /**
         * returns the number of elements.
         */
        size_t size() const {
            return _size;
        }
        /**
         * clears the contents
         */
        void clear() {
            _size = 0;
            makeEmpty(root);
            root = NULL;
        }
        /**
         * insert an element.
         * return a pair, the first of the pair is
         *   the iterator to the new element (or the element that prevented the insertion),
         *   the second one is true if insert successfully, or false.
         */
        pair<iterator, bool> insert(const value_type &value) {
            iterator it = find(value.first);

            if (it == end()) {
                ++_size;
                return pair<iterator, bool>(iterator(this, insert(value, root, NULL)), true);
            }
            else
                return pair<iterator, bool>(it, false);
        }
        /**
         * erase the element at pos.
         *
         * throw if pos pointed to a bad element (pos == this->end() || pos points an element out of this)
         */
        void erase(iterator pos) {
            if (pos == this->end() || pos.mp != this)
                throw invalid_iterator();

            Key key = pos->first;
            if (find(key) == end())
                throw invalid_iterator();

            remove(*pos, root);
            --_size;
        }
        /**
         * Returns the number of elements with key
         *   that compares equivalent to the specified argument,
         *   which is either 1 or 0
         *     since this container does not allow duplicates.
         * The default method of check the equivalence is !(a < b || b > a)
         */
        size_t count(const Key &key) const {
            if (find(key) == cend())
                return 0;
            else
                return 1;
        }
        /**
         * Finds an element with key equivalent to key.
         * key value of the element to search for.
         * Iterator to an element with key equivalent to key.
         *   If no such element is found, past-the-end (see end()) iterator is returned.
         */
        iterator find(const Key &key) {
            node *t = root;

            while (t != NULL && (cmp(key, t->data->first) || cmp(t->data->first, key))) {
                if (cmp(key, t->data->first))
                    t = t->lc;
                else
                    t = t->rc;
            }

            if (t == NULL)
                return end();
            else
                return iterator(this, t);

        }

        const_iterator find(const Key &key) const {
            node *t = root;

            while (t != NULL && (cmp(key, t->data->first) || cmp(t->data->first, key))) {
                if (cmp(key, t->data->first))
                    t = t->lc;
                else
                    t = t->rc;
            }

            if (t == NULL)
                return cend();
            else
                return const_iterator(this, t);
        }
    };

}

#endif
