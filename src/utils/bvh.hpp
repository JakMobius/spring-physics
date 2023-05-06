#pragma once

#include "vec3.hpp"

template<typename Leaf, typename Impl>
struct LazyBVH {
    struct Node {
        Vec3f min;
        Vec3f max;
        Leaf leaf;
        std::unique_ptr<Node> left = nullptr;
        std::unique_ptr<Node> right = nullptr;
        bool lazy = false;
        std::vector<Leaf> lazy_leaves;
    };

    std::unique_ptr<Node> m_root = nullptr;

    void build(std::vector<Leaf>& surface_mesh) {
        m_root = std::make_unique<Node>();
        build_lazy(m_root.get(), surface_mesh);
    }

    void build_lazy(Node* node, std::vector<Leaf>& leaves) {
        if (leaves.empty())
            return;

        Vec3f min = Impl::lower_bound(leaves.begin(), leaves.end());
        Vec3f max = Impl::upper_bound(leaves.begin(), leaves.end());

        node->min = min;
        node->max = max;

        if (leaves.size() == 1) {
            node->leaf = leaves[0];
            return;
        } else {
            node->leaf = nullptr;
        }

        node->lazy_leaves = leaves;
        node->lazy = true;
    }

    void build(Node* node) {
        std::vector<Leaf>& leaves = node->lazy_leaves;

        int best_axis = -1;
        float best_volume = std::numeric_limits<float>::infinity();

        for (int i = 0; i < 3; i++) {
            float volume = get_split_volume(leaves, i);
            if (volume < best_volume) {
                best_volume = volume;
                best_axis = i;
            }
        }

        std::vector<Leaf> left_leaves;
        std::vector<Leaf> right_leaves;

        // Partition

        std::sort(leaves.begin(), leaves.end(), [best_axis](Leaf a, Leaf b) {
            return Impl::compare(a, b, best_axis);
        });

        for (int i = 0; i < leaves.size(); i++) {
            if (i < leaves.size() / 2) {
                left_leaves.push_back(leaves[i]);
            } else {
                right_leaves.push_back(leaves[i]);
            }
        }

        if (left_leaves.empty()) {
            left_leaves = right_leaves;
            right_leaves.clear();
        }

        if (right_leaves.empty()) {
            right_leaves = left_leaves;
            left_leaves.clear();
        }

        node->lazy = false;
        node->lazy_leaves.clear();

        node->left = std::make_unique<Node>();
        node->right = std::make_unique<Node>();

        build_lazy(node->left.get(), left_leaves);
        build_lazy(node->right.get(), right_leaves);
    }

    void query(Node* node, Vec3f min, Vec3f max, std::vector<Leaf>& result) {
        if (node &&
            node->max.x >= min.x && node->min.x <= max.x &&
            node->max.y >= min.y && node->min.y <= max.y &&
            node->max.z >= min.z && node->min.z <= max.z) {

            if (node->leaf) {
                result.push_back(node->leaf);
            }
            if(node->lazy) {
                build(node);
            }

            query(node->left.get(), min, max, result);
            query(node->right.get(), min, max, result);
        }
    }

    float get_split_volume(std::vector<Leaf>& leaves, int axis) {
        std::sort(leaves.begin(), leaves.end(), [axis](Leaf a, Leaf b) {
            return Impl::compare(a, b, axis);
        });

        float median = leaves.size() / 2;

        Vec3f left_min = Impl::lower_bound(leaves.begin(), leaves.begin() + median);
        Vec3f left_max = Impl::upper_bound(leaves.begin(), leaves.begin() + median);

        Vec3f right_min = Impl::lower_bound(leaves.begin() + median, leaves.end());
        Vec3f right_max = Impl::upper_bound(leaves.begin() + median, leaves.end());

        Vec3f left_size = left_max - left_min;
        Vec3f right_size = right_max - right_min;

        float left_volume = left_size[0] * left_size[1] * left_size[2];
        float right_volume = right_size[0] * right_size[1] * right_size[2];

        return left_volume + right_volume;
    }
};