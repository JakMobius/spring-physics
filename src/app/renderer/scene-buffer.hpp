#pragma once

#include "../../base/smart-buffer/smart-buffer-factory.hpp"
#include "../../base/smart-buffer/smart-buffer.hpp"
#include "../../utils/buffer-range.hpp"
#include "etna/vk-wrappers/descriptors/sets/vk-descriptor-set-array.hpp"
#include "etna/vk-wrappers/descriptors/vk-storage-buffer-descriptor.hpp"
#include <memory>
#include <utility>
#include <vector>

class SceneStorageBuffer {
    VK::Device* m_device{};
    SmartBufferFactory m_factory{};
    std::unique_ptr<SmartBuffer> m_buffer;
    std::vector<float> m_storage{};

    BufferRange m_dirty_range{};
    int m_gpu_buffer_size = -1;
    VkDeviceSize m_offset_alignment = 0;
    VkDeviceSize m_size_alignment = 0;

    VK::DescriptorSetArray* m_descriptor_set_array = nullptr;
    int m_descriptor_set_index = -1;
    int m_binding = -1;

  public:
    SceneStorageBuffer(VK::Device* device, SmartBufferFactory factory)
        : m_device(device), m_factory(std::move(factory)) {
        VkPhysicalDeviceProperties physicalDeviceProperties{};
        vkGetPhysicalDeviceProperties(m_device->get_physical_device()->get_handle(), &physicalDeviceProperties);

        m_offset_alignment = physicalDeviceProperties.limits.optimalBufferCopyOffsetAlignment;
        m_size_alignment = physicalDeviceProperties.limits.nonCoherentAtomSize;
    };

    void ensure_size() {
        if (!m_buffer || m_gpu_buffer_size != m_storage.size()) {
            m_gpu_buffer_size = m_storage.size();
            if (m_gpu_buffer_size == 0) {
                m_dirty_range.clear();
                return;
            }

            m_factory.set_size(m_gpu_buffer_size * sizeof(float));
            m_buffer = std::make_unique<SmartBuffer>(m_factory.create(m_device));
            m_dirty_range.extend(0, m_gpu_buffer_size);

            rebind();
        }
    }

    void rebind() {
        if (m_descriptor_set_array && m_buffer) {
            VK::StorageBufferDescriptor storage_buffer_descriptor(m_buffer->get_buffer(), 0, m_gpu_buffer_size * sizeof(float));
            m_descriptor_set_array->bind_descriptor(m_descriptor_set_index, m_binding, storage_buffer_descriptor);
        }
    }

    bool is_dirty() {
        return m_dirty_range.size() > 0;
    }

    void update(VK::CommandBuffer& command_buffer) {
        if (m_dirty_range.is_empty()) {
            return;
        }

        int from = std::max(m_dirty_range.m_from, 0);
        int to = std::min(m_dirty_range.m_to, (int)m_storage.size());

        int offset = from * (int)sizeof(float);
        int size = (to - from) * (int)sizeof(float);

        VkDeviceSize aligned_offset = offset & ~(m_offset_alignment - 1);
        VkDeviceSize aligned_size = (size + (offset - aligned_offset) + (m_size_alignment - 1)) & ~(m_size_alignment - 1);

        VkDeviceSize max_size = (int)m_storage.size() * sizeof(float) - aligned_offset;

        aligned_size = std::min(aligned_size, max_size);

        m_buffer->update_data(command_buffer, aligned_offset, aligned_size, (char*)m_storage.data() + aligned_offset);

        m_dirty_range.clear();
    }

    SmartBuffer* get_buffer() {
        return m_buffer.get();
    }

    std::vector<float>& get_storage() {
        return m_storage;
    }

    void mark_dirty(int from, int to) {
        assert(from >= 0);
        assert(to >= 0);
        assert(from <= to);
        assert(to <= m_storage.size());
        m_dirty_range.extend(from, to);
    }

    void bind(VK::DescriptorSetArray* descriptor_set_array, int descriptor_set_index, int binding) {
        m_descriptor_set_array = descriptor_set_array;
        m_descriptor_set_index = descriptor_set_index;
        m_binding = binding;
        rebind();
    }
};