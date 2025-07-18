#include "vk_deletionManager.hpp"

namespace VxEngine {

void DeletionManager::delete_objects() {
    while (!_deletionStack.empty()) {
        _deletionStack.top()();
        _deletionStack.pop();
    }
}

} // namespace VxEngine