#include <memory>
#include <functional>
#include <cmath>
#include <iostream>
#include <exception>
#include <algorithm>

template <class Key>
class not_found_exception;

namespace details
{

    enum COLOR
    {
        RED,
        BLACK
    };

    enum WAYS
    {
        LR = 12,
        RL = 21,
        RR = 22,
        LL = 11
    };

    template <class Data>
    struct Node
    {
        std::weak_ptr<Node> parent;
        std::shared_ptr<Node> left, right;
        COLOR color;
        Data data;

        Node(
            std::shared_ptr<Node> parent,
            std::shared_ptr<Node> left,
            std::shared_ptr<Node> right,
            COLOR color,
            Data data)
            : parent(parent), left(left), right(right), color(color), data(data) {}
    };

    template <class Key, class T, class Compare>
    class RedBlackTree
    {
    private:
        using value_type = std::pair<const Key, T>;

        std::shared_ptr<Node<value_type>> root = nullptr;

        void recolor(std::shared_ptr<Node<value_type>> node)
        {
            node->color = (node->color == COLOR::RED) ? COLOR::BLACK : COLOR::RED;
        }

        COLOR checkSiblingColor(std::shared_ptr<Node<value_type>> node, value_type value) const
        {
            node = node->parent;
            if (Compare{}(node->data.first, value.first))
                node = node->right;
            else
                node = node->left;
            if (node == nullptr)
                return COLOR::BLACK;
            return node->color;
        }

        WAYS way(std::shared_ptr<Node<value_type>> node, value_type value) const
        {
            int RES = 0;
            for (int i = 1; i >= 0; --i)
            {
                if (Compare{}(node->data.first, value.first))
                {
                    RES += std::pow(10, i) * 1;
                    node = node->left;
                }
                else
                {
                    RES += std::pow(10, i) * 2;
                    node = node->right;
                }
            }

            return WAYS(RES);
        }
        std::shared_ptr<Node<value_type>> leftRotation(std::shared_ptr<Node<value_type>> node, value_type value) // nodegrandpa
        {
            auto newRoot = node->right;
            node->right = newRoot->left;
            if (node->right != nullptr)
                node->right->parent = node;
            newRoot->left = node;
            if (node->parent == nullptr)
            {
                root = newRoot;
                newRoot->parent = nullptr;
            }
            else
            {
                if (Compare{}(node->parent->data.first, value.first))
                    node->parent->left = newRoot;
                else
                    node->parent->right = newRoot;
                newRoot->parent = node->parent;
            }
            node->parent = newRoot;
            return newRoot;
        }
        std::shared_ptr<Node<value_type>> rightRotation(std::shared_ptr<Node<value_type>> node, value_type value)
        {
            auto newRoot = node->left;
            node->left = newRoot->right;
            if (node->left != nullptr)
                node->left->parent = node;
            newRoot->right = node;
            if (node->parent == nullptr)
            {
                root = newRoot;
                newRoot->parent = nullptr;
            }
            else
            {
                if (Compare{}(node->parent->data.first, value.first))
                    node->parent->left = newRoot;
                else
                    node->parent->right = newRoot;
                newRoot->parent = node->parent;
            }
            node->parent = newRoot;
            return newRoot;
        }

    protected:
        std::shared_ptr<Node<value_type>> leftmost() const
        {
            if (root == nullptr)
                nullptr;
            auto tmp = root;
            while (tmp->left)
                tmp = tmp->left;
            return tmp;
        }

        std::shared_ptr<Node<value_type>> rightmost() const
        {
            if (root == nullptr)
                nullptr;
            auto tmp = root;
            while (tmp->right)
                tmp = tmp->right;
            return tmp;
        }

        std::pair<std::shared_ptr<Node<value_type>>, bool> insert(value_type &&value)
        {
            if (root == nullptr)
            {
                root = std::make_shared<Node<value_type>>(nullptr, nullptr, nullptr, COLOR::BLACK, value);
                return {root, true};
            }
            auto tmp = root;
            std::shared_ptr<Node<value_type>> insertPlace;

            while (tmp != nullptr)
            {
                if (Compare{}(tmp->data.first, value.first))
                {
                    if (tmp->left == nullptr)
                        break;
                    tmp = tmp->left;
                }
                else if (Compare{}(value.first, tmp->data.first))
                {
                    if (tmp->right == nullptr)
                        break;
                    tmp = tmp->right;
                }
                else
                    return {nullptr, false}; // NO multi value | MB soon
            }

            if (Compare{}(tmp->data.first, value.first))
                insertPlace = tmp->left = std::make_shared<Node<value_type>>(tmp, nullptr, nullptr, COLOR::RED, value);
            else
                insertPlace = tmp->right = std::make_shared<Node<value_type>>(tmp, nullptr, nullptr, COLOR::RED, value);

            while (tmp != root)
            {
                if (tmp->color == COLOR::BLACK)
                    break;
                // tmp - parent
                // Red Red conflict
                if (checkSiblingColor(tmp, value) == COLOR::RED)
                {
                    tmp = tmp->parent;
                    recolor(tmp->right);
                    recolor(tmp->left);

                    if (tmp == root)
                        break;
                    else
                    {
                        recolor(tmp);
                        tmp = tmp->parent;
                    }
                }
                else
                {
                    tmp = tmp->parent; // grandpa

                    switch (way(tmp, value))
                    {
                    case WAYS::LR:
                        tmp = tmp->left;
                        tmp = leftRotation(tmp, value);
                        tmp = tmp->parent;
                        tmp = rightRotation(tmp, value);
                        recolor(tmp);
                        recolor(tmp->right);
                        break;
                    case WAYS::RL:
                        tmp = tmp->right;
                        tmp = rightRotation(tmp, value);
                        tmp = tmp->parent;
                        tmp = leftRotation(tmp, value);
                        recolor(tmp);
                        recolor(tmp->left);
                        break;
                    case WAYS::RR:
                        tmp = leftRotation(tmp, value);
                        recolor(tmp);
                        recolor(tmp->left);
                        break;
                    case WAYS::LL:
                        tmp = rightRotation(tmp, value);
                        recolor(tmp);
                        recolor(tmp->right);
                        break;
                    }
                    break;
                }
            }
            return {insertPlace, true};
        }

        std::pair<std::shared_ptr<Node<value_type>>, bool> is_set(const Key &key) const
        {
            auto tmp = root;
            while (tmp != nullptr)
            {
                if (Compare{}(tmp->data.first, key))
                    tmp = tmp->left;
                else if (Compare{}(key, tmp->data.first))
                    tmp = tmp->right;
                else
                    return {tmp, true};
            }
            return {{nullptr}, false};
        }
    };

    template <typename Data, typename Compare>
    class RedBlackTreeIter
    {
    public:
        using value_type = Data;
        using difference_type = std::ptrdiff_t;
        using pointer = std::shared_ptr<value_type>;
        using reference = value_type &;
        using iterator_category = std::bidirectional_iterator_tag;

    private:
        std::shared_ptr<Node<value_type>> iter;
        std::shared_ptr<Node<value_type>> endminus1;

    public:
        RedBlackTreeIter() = default;
        explicit RedBlackTreeIter(std::shared_ptr<Node<value_type>> node) : iter(node) {}

        reference operator*() const { return iter->data; }

        RedBlackTreeIter &operator++()
        {
            if (iter->right != nullptr)
            {
                iter = iter->right;

                while (iter->left != nullptr)
                    iter = iter->left;
            }
            else
            {
                endminus1 = iter;
                while ((iter->parent != nullptr) && Compare{}(iter->data.first, iter->parent->data.first))
                    iter = iter->parent;

                iter = iter->parent;
            }
            return *this;
        }
        RedBlackTreeIter operator++(int)
        {
            auto tmp = *this;
            this->operator++();
            return tmp;
        }

        RedBlackTreeIter &operator--()
        {
            if (iter == nullptr)
            {
                iter = endminus1;
                return *this;
            }
            if (iter->left != nullptr)
            {
                iter = iter->left;

                while (iter->right != nullptr)
                    iter = iter->right;
            }
            else
            {
                while ((iter->parent != nullptr) && !Compare{}(iter->data.first, iter->parent->data.first))
                    iter = iter->parent;

                iter = iter->parent;
            }
            return *this;
        }

        RedBlackTreeIter operator--(int)
        {
            auto tmp = *this;
            this->operator--();
            return tmp;
        }

        bool operator==(const RedBlackTreeIter &rhs) { return iter == rhs.iter; }
        bool operator!=(const RedBlackTreeIter &rhs) { return iter != rhs.iter; }
    };

}

template <class Key, class T, class Compare = std::greater<Key>>
class dictionary : public details::RedBlackTree<Key, T, Compare>
{
public:
    using Myt_ = dictionary<Key, T, Compare>;
    using key_type = Key;
    using dicted_type = T;
    using value_type = std::pair<const key_type, dicted_type>;
    using key_compare = Compare;
    using reference = value_type &;
    using const_reference = const value_type &;
    using pointer = std::shared_ptr<value_type>;
    using const_pointer = const std::shared_ptr<value_type>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using iterator = details::RedBlackTreeIter<value_type, Compare>;
    using const_iterator = details::RedBlackTreeIter<const value_type, Compare>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
    size_type size_;

public:
    using RBT = details::RedBlackTree<Key, T, Compare>;

    dictionary() 
        : size_(0) {}

    dictionary(std::initializer_list<value_type> &&list)
    {
        for (auto x : list)
            if (insert(std::move(x)).second)
                ++size_;
    }

    size_type size() const noexcept { return size_; }
    bool empty() const noexcept { return !size_; }
    iterator begin() const { return iterator(RBT::leftmost()); }
    iterator end() const { return iterator(nullptr); }

    T get(const Key &key) const
    {
        auto result = RBT::is_set(key);
        if (result.second)
            return result.first->data.second;
        else
            throw not_found_exception<Key>(key);
    }

    T &operator[](const Key &key)
    {
        auto result = RBT::is_set(key);
        if (result.second)
            return result.first->data.second;
        else
        {
            auto &&tmp = RBT::insert({key, {}}).first->data.second;
            ++size_;
            return tmp;
        }
    }

    std::pair<iterator, bool> insert(value_type &&value)
    {
        auto result = RBT::insert(std::forward<value_type>(value));
        if (result.second)
            ++size_;
        return {iterator(result.first), result.second};
    }

    std::pair<iterator, bool> is_set(const Key &key) const
    {
        auto result = RBT::is_set(key);
        return {iterator(result.first), result.second};
    }
};

template <class Key>
class not_found_exception : public std::exception
{
    Key key;

public:
    not_found_exception(Key key) : key(key) {}
    virtual const Key &get_key() const noexcept;
};

template <class Key>
const Key &not_found_exception<Key>::get_key() const noexcept { return key; }

int main(int argc, char const *argv[])
{
    dictionary<int, int> dict;
    dictionary<int, int> dictFromInit = {{10, 1}, {292, 3}, {8282, 2}};

    for (const auto [key, value] : dictFromInit)
        std::cout << "key: " << key << " value: " << value << std::endl;

    dict.insert({10, 1});
    dict.insert({18, 1});
    dict.insert({7, 1});
    dict.insert({15, 4});
    dict.insert({16, 1});
    dict.insert({30, 1});
    dict.insert({25, 1});
    dict.insert({40, 1});
    dict.insert({60, 1});
    dict.insert({2, 1});
    dict.insert({1, 1});

    dict.insert({30, 1}); // no multivalues
    dictionary<int, int>::iterator it = dict.begin();

    dict[1000] = 3;
    dict[1000] = 20;

    std::cout << dict.get(1000) << std::endl;

    while (it != dict.end())
        std::cout << (*(it++)).first << ", ";

    std::cout << std::endl;

    --it;
    while (it != dict.begin())
        std::cout << (*(it--)).second << ", ";
    std::cout << std::endl;

    std::for_each(dict.begin(), dict.end(), [](auto &&elem)
                  { elem.second += elem.second; });

    for (const auto [key, value] : dict)
        std::cout << "key: " << key << " value: " << value << std::endl;
    std::cout << std::boolalpha << dict.is_set(1000).second << std::endl;
    std::cout << dict.size() << '\n';

    return 0;
}
