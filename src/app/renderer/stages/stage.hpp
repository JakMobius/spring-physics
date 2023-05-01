#pragma once

#include "etna/vk-wrappers/command-buffer/vk-command-buffer.hpp"

class Stage {
public:
    virtual ~Stage() = default;

    virtual void cleanup_pipeline() = 0;
    virtual void create_pipeline() = 0;
    virtual void record_command_buffer(VK::CommandBuffer& command_buffer) = 0;
    virtual void prepare_for_frame() {}
};