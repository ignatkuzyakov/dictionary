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
    template <class Key, class T, class Compare = std::greater<Key>>
    class dictionary
    {
    private:
        template <class Data> class Node;

    public:
        class Iterator;

        using key_type = Key;
        using dicted_type = T;
        using value_type = std::pair<const key_type, dicted_type>;
        using node_ptr = std::shared_ptr<Node<value_type>>;
        using Myt_ = dictionary<Key, T, Compare>;
        using key_compare = Compare;
        using reference = value_type &;
        using const_reference = const value_type &;
        using pointer = std::shared_ptr<value_type>;
        using const_pointer = const std::shared_ptr<value_type>;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;

        using iterator = Iterator;
        using const_iterator = const iterator;

        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    private:
        enum COLOR { RED, BLACK };

        enum WAYS { LR = 12, RL = 21, RR = 22, LL = 11 };

        node_ptr root = nullptr;

        static constexpr Compare compare{};

        template <class Data>
        struct Node
        {
            std::weak_ptr<Node> parent__;
            std::shared_ptr<Node> left__, right__;
            COLOR color;
            Data data;

            Node(
                std::shared_ptr<Node> parent,
                std::shared_ptr<Node> left,
                std::shared_ptr<Node> right,
                COLOR color,
                Data data)
                : parent__(parent), left__(left), right__(right), color(color), data(data) {}
        };

    public:
        class Iterator
        {

            node_ptr iter;
            node_ptr lastelem; // TODO rm

        public:
            using difference_type = std::ptrdiff_t;
            using iterator_category = std::bidirectional_iterator_tag;
            using pointer = std::shared_ptr<value_type>;
            using reference = value_type &;

            node_ptr lefter(node_ptr node) const
            {
                if (!node) return nullptr;
                while (node->left__)
                    node = node->left__;
                return node;
            }

            node_ptr righter(node_ptr node) const
            {
                if (!node) return nullptr;
                while (node->right__)
                    node = node->right__;
                return node;
            }

        public:
            Iterator() = default;
            explicit Iterator(node_ptr node) : iter(node) {}

            reference operator*() const { return iter->data; }

            Iterator &operator++()
            {
                if (iter->right__ != nullptr)
                {
                    iter = iter->right__;
                    iter = lefter(iter);
                }
                else
                {
                    lastelem = iter;
                    while ((iter->parent__.lock() != nullptr) && compare(iter->data.first, iter->parent__.lock()->data.first))
                        iter = iter->parent__.lock();
                    iter = iter->parent__.lock();
                }
                return *this;
            }
            Iterator operator++(int)
            {
                auto tmp = *this;
                this->operator++();
                return tmp;
            }

            Iterator &operator--()
            {
                if (iter == nullptr)
                    iter = lastelem;
                else if (iter->left__ != nullptr)
                {
                    iter = iter->left__;
                    iter = righter(iter);
                }
                else
                {
                    while ((iter->parent__.lock() != nullptr) && !compare(iter->data.first, iter->parent__.lock()->data.first))
                        iter = iter->parent__.lock();
                    iter = iter->parent__.lock();
                }
                return *this;
            }

            Iterator operator--(int)
            {
                auto tmp = *this;
                this->operator--();
                return tmp;
            }

            bool operator==(const Iterator &rhs) const { return iter == rhs.iter; }
            bool operator!=(const Iterator &rhs) const { return iter != rhs.iter; }
        };

    private:
        void recolor(node_ptr node)
        {
            node->color = (node->color == COLOR::RED) ? COLOR::BLACK : COLOR::RED;
        }

        COLOR checkSiblingColor(node_ptr node, value_type value) const
        {
            node = node->parent__.lock();
            if (compare(node->data.first, value.first))
                node = node->right__;
            else
                node = node->left__;
            if (node == nullptr)
                return COLOR::BLACK;
            return node->color;
        }

        WAYS way(node_ptr node, value_type value) const
        {
            int RES = 0;
            for (int i = 1; i >= 0; --i)
            {
                if (compare(node->data.first, value.first))
                {
                    RES += std::pow(10, i) * 1;
                    node = node->left__;
                }
                else
                {
                    RES += std::pow(10, i) * 2;
                    node = node->right__;
                }
            }

            return WAYS(RES);
        }
        node_ptr leftRotation(node_ptr node, value_type value) // nodegrandpa
        {
            auto newRoot = node->right__;
            node->right__ = newRoot->left__;
            if (node->right__ != nullptr)
                node->right__->parent__ = node;
            newRoot->left__ = node;
            if (!node->parent__.lock())
            {
                root = newRoot;
                newRoot->parent__.reset();
            }
            else
            {
                if (compare(node->parent__.lock()->data.first, value.first))
                    node->parent__.lock()->left__ = newRoot;
                else
                    node->parent__.lock()->right__ = newRoot;
                newRoot->parent__ = node->parent__.lock();
            }
            node->parent__ = newRoot;
            return newRoot;
        }
        node_ptr rightRotation(node_ptr node, value_type value)
        {
            auto newRoot = node->left__;
            node->left__ = newRoot->right__;
            if (node->left__ != nullptr)
                node->left__->parent__ = node;
            newRoot->right__ = node;
            if (!node->parent__.lock())
            {
                root = newRoot;
                newRoot->parent__.reset();
            }
            else
            {
                if (compare(node->parent__.lock()->data.first, value.first))
                    node->parent__.lock()->left__ = newRoot;
                else
                    node->parent__.lock()->right__ = newRoot;
                newRoot->parent__ = node->parent__.lock();
            }
            node->parent__ = newRoot;
            return newRoot;
        }

        std::pair<node_ptr, node_ptr *const> find(node_ptr node, const value_type &value) const
        {
            while (node != nullptr)
            {
                if (compare(node->data.first, value.first))
                {
                    if (node->left__ == nullptr)
                        return {node, &node->left__};
                    node = node->left__;
                }
                else if (compare(value.first, node->data.first))
                {
                    if (node->right__ == nullptr)
                        return {node, &node->right__};
                    node = node->right__;
                }
                else
                    break;
            }
            return {node, nullptr};
        }

        node_ptr leftmost() const
        {
            auto node = root;
            while (node->left__)
                node = node->left__;
            return node;
        }

        node_ptr rightmost() const
        {
            auto node = root;
            while (node->right__)
                node = node->right__;
            return node;
        }

    private:
        size_type size_;

    public:
        std::pair<iterator, bool> insert(value_type &&value)
        {
            if (root == nullptr)
            {
                root = std::make_shared<Node<value_type>>(nullptr, nullptr, nullptr, COLOR::BLACK, value);
                return {iterator(root), true};
            }

            auto tmp = find(root, value);
            node_ptr node = tmp.first;
            *tmp.second = std::make_shared<Node<value_type>>(node, nullptr, nullptr, COLOR::RED, value);
            auto insertPlace = *tmp.second;

            while (node != root)
            {
                if (node->color == COLOR::BLACK) break;
                // Red Red conflict
                if (checkSiblingColor(node, value) == COLOR::RED)
                {
                    node = node->parent__.lock();
                    recolor(node->right__);
                    recolor(node->left__);

                    if (node == root) break;
                    else
                    {
                        recolor(node);
                        node = node->parent__.lock();
                    }
                }
                else
                {
                    node = node->parent__.lock(); // grandpa

                    switch (way(node, value))
                    {
                    case WAYS::LR:
                        node = node->left__;
                        node = leftRotation(node, value);
                        node = node->parent__.lock();
                        node = rightRotation(node, value);
                        recolor(node);
                        recolor(node->right__);
                        break;
                    case WAYS::RL:
                        node = node->right__;
                        node = rightRotation(node, value);
                        node = node->parent__.lock();
                        node = leftRotation(node, value);
                        recolor(node);
                        recolor(node->left__);
                        break;
                    case WAYS::RR:
                        node = leftRotation(node, value);
                        recolor(node);
                        recolor(node->left__);
                        break;
                    case WAYS::LL:
                        node = rightRotation(node, value);
                        recolor(node);
                        recolor(node->right__);
                        break;
                    }
                    break;
                }
            }
            ++size_;
            return {iterator(insertPlace), true};
        }

        std::pair<iterator, bool> is_set(const Key &key) const
        {
            value_type val = {key, T{}};
            auto checker = find(root, val);
            if (checker.second == nullptr)
                return {iterator(checker.first), true};
            else
                return {iterator(*checker.second), false};
        }

    public:
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
        iterator begin() const { return iterator(leftmost()); }
        iterator end() const { return iterator(nullptr); }

        T get(const Key &key) const
        {
            auto result = is_set(key);
            if (result.second)
                return (*result.first).second;
            else
                throw not_found_exception<Key>(key);
        }

        T &operator[](const Key &key)
        {
            auto result = is_set(key);
            if (result.second)
                return (*result.first).second;
            else
            {
                auto node = insert({key, {}});
                auto &&tmp = (*node.first).second;
                ++size_;
                return tmp;
            }
        }
    };

}

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
    details::dictionary<int, int> dict;
    details::dictionary<int, int> dictFromInit = {{10, 1}, {292, 3}, {8282, 2}};

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

    details::dictionary<int, int>::iterator it = dict.begin();

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
