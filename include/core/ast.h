#pragma once

#include <memory>
#include <vector>

namespace scp::core {

struct TreeNode {
  int val_;
  std::vector<std::shared_ptr<TreeNode>> children_;

  explicit TreeNode(int v) : val_(v) {}

  void AddChild(const std::shared_ptr<TreeNode> &child) { children_.push_back(child); }
};

class AST {};

}  // namespace scp::core
