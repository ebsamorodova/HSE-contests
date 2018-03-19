#include <iostream>
#include <list>
#include <vector>
#include <stdexcept>
#include <type_traits>

template<class KeyType, class ValueType, typename Hash = std::hash<KeyType> >

class HashMap {
private:
    using Node = std::pair<const KeyType, ValueType>;
    using TableList = std::list<Node>;
    using TableType = std::vector<TableList>;

    size_t sz;
    Hash hasher;
    TableType table;

    HashMap(size_t _sz, Hash _hasher = Hash()) : sz(0), hasher(_hasher), table(_sz) {}

public:
    HashMap(Hash _hasher = Hash()) : HashMap(2, _hasher) {}

    size_t size() const {
        return sz;
    }

    bool empty() const {
        return sz == 0;
    }

    Hash hash_function() const {
        return hasher;
    }

    struct iterator {
        typename TableType::iterator it_vector, it_end;
        typename TableList::iterator it_list;

        iterator() {}

        iterator(typename TableType::iterator _vector, typename TableType::iterator _end, typename TableList::iterator _list) :
            it_vector(_vector)
            , it_end(_end)
            , it_list(_list)
            {}

        Node& operator *() const {
            return *it_list;
        }

        Node* operator ->() const {
            return it_list.operator->();
        }

        iterator& operator ++() {
            it_list++;
            while (it_vector != it_end && it_list == it_vector->end()) {
                ++it_vector;
                if (it_vector != it_end) {
                    it_list = it_vector->begin();
                }
            }
            return *this;
        }

        iterator operator ++(int) {
            iterator it = *this;
            ++(*this);
            return it;
        }

        bool operator == (const iterator &other) const {
            if (it_vector == it_end)
                return other.it_vector == other.it_end;
            else
                return it_vector == other.it_vector && it_list == other.it_list;
        }

        bool operator != (const iterator &other) const {
            return !(*this == other);
        }
    };

    iterator begin() {
        if (empty()) {
            return iterator();
        }
        auto it = table.begin();
        while (it != table.end() && it->empty()) {
            ++it;
        }
        return iterator(it, table.end(), it->begin());
    }

    iterator end() {
        return iterator();
    }

    struct const_iterator {
        typename TableType::const_iterator it_vector, it_end;
        typename TableList::const_iterator it_list;

        const_iterator() {}

        const_iterator(typename TableType::const_iterator _vector, typename TableType::const_iterator _end, typename TableList::const_iterator _list) :
            it_vector(_vector)
            , it_end(_end)
            , it_list(_list)
            {}

        const Node& operator *() const {
            return *it_list;
        }

        const Node* operator ->() const {
            return it_list.operator->();
        }

        const_iterator& operator ++() {
            it_list++;
            while (it_vector != it_end && it_list == it_vector->end()) {
                ++it_vector;
                if (it_vector != it_end) {
                    it_list = it_vector->begin();
                }
            }
            return *this;
        }

        const_iterator operator ++(int) {
            const_iterator it = *this;
            ++(*this);
            return it;
        }

        bool operator == (const const_iterator &other) const {
            if (it_vector == it_end)
                return other.it_vector == other.it_end;
            else
                return it_vector == other.it_vector && it_list == other.it_list;
        }

        bool operator != (const const_iterator &other) const {
            return !(*this == other);
        }
    };

    const_iterator begin() const {
        if (empty()) {
            return const_iterator();
        }
        auto it = table.begin();
        while (it != table.end() && it->empty()) {
            ++it;
        }
        return const_iterator(it, table.end(), it->begin());
    }

    const_iterator end() const {
        return const_iterator();
    }

    iterator find(const KeyType &key) {
        int pos = (hasher(key)) % table.size();
        auto it = table[pos].begin();
        while (it != table[pos].end() && !(it->first == key)) {
            ++it;
        }
        if (it != table[pos].end()) {
            return iterator(table.begin() + pos, table.end(), it);
        } else {
            return iterator();
        }
    }

    const_iterator find(const KeyType &key) const {
        int pos = (hasher(key)) % table.size();
        auto it = table[pos].begin();
        while (it != table[pos].end() && !(it->first == key)) {
            ++it;
        }
        if (it != table[pos].end()) {
            return const_iterator(table.begin() + pos, table.end(), it);
        } else {
            return const_iterator();
        }
    }

    ValueType& operator [] (const KeyType &key) {
        if (find(key) == iterator()) {
            this->insert({key, ValueType()});
        }
        return (find(key))->second;
    }

    const ValueType& at(const KeyType &key) const {
        if (find(key) == const_iterator()) {
            throw std::out_of_range("no key");
        }
        return (find(key))->second;
    }

    iterator erase(const KeyType &key) {
        if (sz * 6 < table.size()) {
            rehash(sz * 2 + 1);
        }
        iterator it = find(key), cur_it = it;
        if (it != iterator()) {
            cur_it++;
            (*it.it_vector).erase(it.it_list);
            --sz;
        }
        return cur_it;
    }

    std::pair<iterator, bool> insert(const Node &node) {
        iterator it = this->find(node.first);
        if (it != this->end()) {
            return {it, false};
        } else {
            int pos = (hasher(node.first)) % table.size();
            table[pos].push_back(node);
            sz++;
            if (sz * 2 > table.size()) {
                rehash(sz*4 + 1);
            }
            return {this->find(node.first), true};
        }

    }

   template <typename Iterator>
    HashMap(Iterator _begin, Iterator _end, Hash _hasher = Hash()) : HashMap(_hasher) {
        for (auto it = _begin; it != _end; ++it) {
            this->insert(*it);
        }
    }

    HashMap(std::initializer_list<Node> lst, Hash _hasher = Hash()) : HashMap(lst.begin(), lst.end(), _hasher) {}

    void clear() {
        table.clear();
        sz = 0;
        table.resize(2);
    }

    HashMap& operator = (const HashMap &other) {
        if (this != &other) {
            this->clear();
            this->rehash(other.table.size());
            for (auto it = other.begin(); it != other.end(); ++it) {
                this->insert(*it);
            }
        }
        return *this;
    }

    void rehash(size_t new_sz) {
        TableType new_table(new_sz);
        if (!this->empty()) {
            for (iterator it = this->begin(); it != this->end(); ++it) {
                Node &node = *it;
                int pos = (hasher(node.first)) % new_sz;
                new_table[pos].push_back(node);
            }
        }
        table = std::move(new_table);
    }
};

