#pragma once

#include <deque>
#include <functional>
#include <stack>

// Simple class to manage the deletion of vulkan objects.
// Maintains an internal queue of functions to delete vulkan objects
// Based on VKGuide's VulkanDeletionManager

// TODO, Use a map of object types to their deletion functions 
// to avoid storing unnecesary duplicate functions.

namespace VxEngine {

class DeletionManager {
public:
    DeletionManager() = default;
    ~DeletionManager() = default;

    void push_function(std::function<void()> function) {
        _deletionStack.push(function);
    }

    void delete_objects();

private:
    std::stack<std::function<void()>> _deletionStack; // Stack of deletion functions.
};

} // namespace VxEngine