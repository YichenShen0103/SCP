#pragma once

#include <list>
#include <memory>
#include <string>
#include <utility>

namespace scp::core {

enum class ASTNodeType {
  ROOT,
  IDENTIFIER,
  NUMBER,
  PLUS,
  TIMES,
  ASSIGN,
};

struct TreeNode {
  std::string val_;
  std::list<std::shared_ptr<TreeNode>> children_;

  explicit TreeNode(std::string v) : val_(std::move(v)) {}

  void AddChild(const std::shared_ptr<TreeNode> &child) { children_.push_front(child); }
};

class AST {
 public:
  class ASTNode {
   public:
    explicit ASTNode(ASTNodeType type, std::string value) : value_(std::move(value)), type_(type) {}
    ~ASTNode() = default;

    void AddChild(const std::shared_ptr<ASTNode> &child) { children_.push_back(child); }
    void SetType(ASTNodeType type) { type_ = type; }
    void SetValue(const std::string &value) { value_ = value; }

    // Getters
    auto GetType() const -> ASTNodeType { return type_; }
    auto GetValue() const -> const std::string & { return value_; }
    auto GetChildren() const -> const std::list<std::shared_ptr<ASTNode>> & { return children_; }

   private:
    std::string value_;
    ASTNodeType type_;
    std::list<std::shared_ptr<ASTNode>> children_;
  };

  explicit AST(std::string program_name) : name_(std::move(program_name)), root_(nullptr) {}
  explicit AST(std::string name, std::shared_ptr<ASTNode> root) : name_(std::move(name)), root_(std::move(root)) {}
  ~AST() = default;

  void SetRoot(std::shared_ptr<ASTNode> root) { root_ = std::move(root); }
  auto GetRoot() const -> std::shared_ptr<ASTNode> { return root_; }

 private:
  std::string name_;
  std::shared_ptr<ASTNode> root_;
};

}  // namespace scp::core
