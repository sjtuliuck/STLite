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

template<
	class Key,
	class T,
	class Compare = std::less<Key>
> class map {
    friend class iterator;
    friend class const_iterator;
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
        node *father;
        node *lc, *rc;

        node(value_type *v = NULL) : data(v) {
            father = lc = rc = NULL;
        }
        ~node() {
            if (data != NULL)
                delete data;
        }
    };

    node *root;
    size_t _size;

    bool islc(node *t) const {
        if (t == root)
            return false;
        return t->father->lc == t;
    }

    node *prev(node *t) const{
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

    node *next(node *t) const{
        if (t->rc != NULL) {
            t = t->rc;
            while (t->lc != NULL) {
                t = t->lc;
            }
            return t;
        }
        else {
            while (!islc(t) && t != root) {
                t = t->father;
            }
            return t->father;
        }
    }

    node *insert(const value_type &x, node *&t, node *fa) {
        node *ret;
        if (t == NULL) {
            t = new node(new value_type(x));
            t->father = fa;
            return t;
        }
        else if (cmp(x.first, t->data->first)) {
            ret = insert(x, t->lc, t);
        }
        else if (cmp(t->data->first, x.first)) {
            ret = insert(x, t->rc, t);
        }

        return ret;
    }

    void swap(node *&x, node *&y) {
        node tx = *x, ty = *y;
        tx.data = ty.data = NULL;

        if (x == y->father) {
            x->father = y;
            if (y == tx.lc) {
                y->lc = x;
                y->rc = tx.rc;
            }
            else {
                y->rc = x;
                y->lc = tx.lc;
            }
        }
        else {
            x->father = ty.father;
            if (y == y->father->lc) {
                y->father->lc = x;
            }
            else {
                y->father->rc = x;
            }
            y->lc = tx.lc;
            y->rc = tx.rc;
        }
        x->lc = ty.lc;
        x->rc = ty.rc;
        y->father = tx.father;

        if (x->lc) x->lc->father = x;
        if (x->rc) x->rc->father = x;
        if (y->lc) y->lc->father = y;
        if (y->rc) y->rc->father = y;

        node *t;
        t = x;
        x = y;
        y = t;
    }

    void remove(const Key &key, node *&t) {
        if (t == NULL)
            return;

        if (cmp(key, t->data->first)) {
            remove(key, t->lc);
        }
        else if (cmp(t->data->first, key)) {
            remove(key, t->rc);
        }
        else if (t->lc != NULL && t->rc != NULL) {
//            node *tmp = t->rc;
//            while (tmp->lc != NULL) {
//                tmp = tmp->lc;
//            }
//            if (t->data != NULL)
//                delete t->data;
//            t->data = new value_type(*(tmp->data));
//            remove(tmp->data->first, t->rc);

            node *tmp = t->lc;
            while (tmp->rc != NULL) {
                tmp = tmp->rc;
            }
//            if (t->data != NULL)
//                delete t->data;
//            t->data = new value_type(*(tmp->data));
            swap(t, tmp);
            remove(tmp->data->first, t->lc);
        }
        else {
            node *oldNode = t;
            if (t->lc != NULL)
                t = t->lc;
            else
                t = t->rc;

            if (t != NULL) {
                t->father = oldNode->father;
            }
            oldNode->lc = oldNode->rc = NULL;
            if (oldNode)
                delete oldNode;
        }

    }

    node *find(const Key &key, node *t) const {
        if (t == NULL)
            return t;
        if (!(cmp(key, t->data->first) || cmp(t->data->first, key)))
            return t;

        if (cmp(key, t->data->first))
            return find(key, t->lc);
        else
            return find(key, t->rc);
    }

    void makeEmpty(node *t) {
        if (t == NULL)
            return;

        makeEmpty(t->lc);
        makeEmpty(t->rc);

        delete t;
        t = NULL;
    }

public:
	class const_iterator;
	class iterator {
	    friend class map;
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
                while (nod->rc != NULL) {
                    nod = nod->rc;
                }
            }
            else {
                nod = mp->prev(nod);
            }

            return ret;
		}
		/**
		 * TODO --iter
		 */
		iterator & operator--() {
            if (*this == mp->begin())
                throw invalid_iterator();

            if (*this == mp->end()) {
                nod = mp->root;
                while (nod->rc != NULL) {
                    nod = nod->rc;
                }
            } else {
                nod = mp->prev(nod);
            }

            return *this;
		}
		/**
		 * a operator to check whether two iterators are same (pointing to the same memory).
		 */
		value_type & operator*() const {
            if (nod == NULL || nod->data == NULL)
                throw invalid_iterator();
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
                while (nod->rc != NULL) {
                    nod = nod->rc;
                }
            }
            else {
                nod = mp->prev(nod);
            }

            return ret;
        }
        /**
         * TODO --iter
         */
        const_iterator & operator--() {
            if (*this == mp->cbegin())
                throw invalid_iterator();

            if (*this == mp->cend()) {
                nod = mp->root;
                while (nod->rc != NULL) {
                    nod = nod->rc;
                }
            }
            else {
                nod = mp->prev(nod);
            }

            return *this;
        }
        /**
         * a operator to check whether two iterators are same (pointing to the same memory).
         */
        const value_type & operator*() const {
            if (nod == NULL || nod->data == NULL)
                throw invalid_iterator();
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

private:
    void copy(node *&t, node *other, node *fa) {
        if (other == NULL)
            return;

        t = new node(new value_type(*(other->data)));
        t->father = fa;

        if (other->lc)
            copy(t->lc, other->lc, t);
        if (other->rc)
            copy(t->rc, other->rc, t);
    }

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
	    copy(root, other.root, NULL);
	    _size = other._size;
	}
	/**
	 * TODO assignment operator
	 */
	map & operator=(const map &other) {
        if (this == &other)
            return *this;

	    clear();
	    copy(root, other.root, NULL);
	    _size = other._size;
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
        node *t = find(key, root);
        if (t == NULL)
            throw index_out_of_bound();
        return t->data->second;
	}
	const T & at(const Key &key) const {
        node *t = find(key, root);
        if (t == NULL)
            throw index_out_of_bound();
        return t->data->second;
	}
	/**
	 * TODO
	 * access specified element 
	 * Returns a reference to the value that is mapped to a key equivalent to key,
	 *   performing an insertion if such key does not already exist.
	 */
	T & operator[](const Key &key) {
	    node *tmp = find(key, root);

        if (tmp == NULL) {
            ++_size;
            tmp = insert(value_type(key, T()), root, NULL);
        }

        return tmp->data->second;
	}
	/**
	 * behave like at() throw index_out_of_bound if such key does not exist.
	 */
	const T & operator[](const Key &key) const {
	    return at(key);
	}
	/**
	 * return a iterator to the beginning
	 */
	iterator begin() {
        node *tmp = root;
        if (tmp == NULL)
            return iterator(this, NULL);
        while (tmp->lc != NULL) {
            tmp = tmp->lc;
        }
	    return iterator(this, tmp);
	}
	const_iterator cbegin() const {
        node *tmp = root;
        if (tmp == NULL)
            return const_iterator(this, NULL);
        while (tmp->lc != NULL) {
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
	    node *tmp = find(value.first, root);

	    if (tmp == NULL) {
	        ++_size;
	        node *t = insert(value, root, NULL);
	        return pair<iterator, bool>(iterator(this, t), true);
	    }
	    else
	        return pair<iterator, bool>(iterator(this, tmp), false);
	}
	/**
	 * erase the element at pos.
	 *
	 * throw if pos pointed to a bad element (pos == this->end() || pos points an element out of this)
	 */
	void erase(iterator pos) {
	    if (pos == end() || pos.mp != this)
	        throw invalid_iterator();

//	    Key key = pos->first;
	    if (find(pos->first, root) == NULL)
            throw invalid_iterator();


	    remove(pos->first, root);
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
	    if (find(key, root) == NULL)
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
	    node *tmp = find(key, root);

	    if (tmp == NULL)
	        return end();
	    else
	        return iterator(this, tmp);
	}
	const_iterator find(const Key &key) const {
        node *tmp = find(key, root);

        if (tmp == NULL)
            return cend();
        else
            return const_iterator(this, tmp);
	}
};

}

#endif
