#pragma once
#include <stdint.h>
#include <vector>

namespace AnA
{
    namespace Spatial
    {
        template <uint32_t K, typename T, typename P, uint32_t offset = 0>
        class KDTree
        {
        public:
            KDTree(std::vector<P>& _points) : root(), points(_points) {}
            ~KDTree()
            {
                freeNode(root);
            }
        private:
            struct Node
            {
                uint32_t index;
                Node* left;
                Node* right;
                Node(uint32_t idx) : index(idx), left(), right(){}
            };

            Node* root;
            std::vector<P>& points;

            Node* insert(Node* node, uint32_t index, uint32_t depth)
            {
                if (!node)
                    return new Node(index);

                uint32_t cd = depth % K;
                if (reinterpret_cast<T*>(&points[index] + offset)[cd] <
                    reinterpret_cast<T*>(&points[node->index] + offset)[cd])
                    node->left = insert(node->left, index, depth + 1);
                else
                    node->right = insert(node->right, index, depth + 1);

                return node;
            }

            bool search(Node* node, uint32_t index, uint32_t depth) const
            {
                if (!node)
                    return false;
                if (node->index == index)
                    return true;

                int cd = depth % K;
                if (reinterpret_cast<T*>(&points[index] + offset)[cd] <
                    reinterpret_cast<T*>(&points[node->index] + offset)[cd])
                    return search(node->left, index, depth);
                else
                    return search(node->right, index, depth);
            }

            void freeNode(Node* node)
            {
                if (!node)
                    return;

                freeNode(node->left);
                freeNode(node->right);

                delete node;
            }
        };
    }
}
