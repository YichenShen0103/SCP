#pragma once

#include <fstream>
#include <list>
#include <memory>
#include <string>
#include <utility>

#include "cgen/runtime_environment.h"
#include "core/type.h"

namespace scp::core {

/**
 * Enum class for AST node types
 */
enum class ASTNodeType { ROOT, IDENTIFIER, NUMBER, PLUS, TIMES, ASSIGN, STRING };

/**
 * Struct representing a node in the abstract syntax tree (AST).
 */
struct TreeNode {
  /* The value of the tree node */
  std::string val_;
  /* The children of the tree node */
  std::list<std::shared_ptr<TreeNode>> children_;

  /**
   * Constructor for a tree node.
   * @param v The value of the node.
   */
  explicit TreeNode(std::string v) : val_(std::move(v)) {}

  /**
   * Add a child to the tree node.
   * @param child The child node to add.
   */
  void AddChild(const std::shared_ptr<TreeNode> &child) { children_.push_front(child); }
};

/**
 * Abstract Syntax Tree (AST) representation.
 */
class AST {
 public:
  /**
   * AST node representation.
   */
  class ASTNode {
   public:
    /**
     * Constructor for an AST node.
     * @param type The type of the node.
     * @param value The value of the node.
     */
    explicit ASTNode(ASTNodeType type, std::string value) : value_(std::move(value)), type_(type) {}

    /**
     * Destructor for an AST node.
     */
    ~ASTNode() = default;

    /**
     * Add a child to the AST node.
     * @param child The child node to add.
     */
    void AddChild(const std::shared_ptr<ASTNode> &child) { children_.push_back(child); }

    /**
     * Set the type of the AST node.
     * @param type The new type of the node.
     */
    void SetType(ASTNodeType type) { type_ = type; }

    /**
     * Set the value of the AST node.
     * @param value The new value of the node.
     */
    void SetValue(const std::string &value) { value_ = value; }

    // Getters
    /**
     * Get the type of the AST node.
     * @return The type of the node.
     */
    auto GetType() const -> ASTNodeType { return type_; }

    /**
     * Get the value of the AST node.
     * @return The value of the node.
     */
    auto GetValue() const -> const std::string & { return value_; }

    /**
     * Get the children of the AST node.
     * @return The children of the node.
     */
    auto GetChildren() const -> const std::list<std::shared_ptr<ASTNode>> & { return children_; }

    /**
     * Check type of the AST node.
     * @param environment The current type environment.
     * @return Type of the AST node.
     */
    auto TypeCheck(const std::shared_ptr<TypeEnvironment> &environment, bool &has_bug) const -> Type;

    /**
     * Generate code for the AST node.
     * @param runtime The runtime environment.
     * @return The generated code as a string.
     */
    auto GenerateCode(const std::shared_ptr<cgen::RuntimeEnvironment> &runtime) const -> std::string;

   private:
    /* The value of the AST node */
    std::string value_;
    /* The type of the AST node */
    ASTNodeType type_;
    /* The children of the AST node */
    std::list<std::shared_ptr<ASTNode>> children_;

    /**
     * Determine the runtime type of an expression for code generation.
     * @param runtime The runtime environment.
     * @return The runtime type of the expression.
     */
    auto GetRuntimeType(const std::shared_ptr<cgen::RuntimeEnvironment> &runtime) const -> Type;
  };

  /**
   * Constructor for the AST.
   * @param program_name The name of the program.
   */
  explicit AST(std::string program_name) : name_(std::move(program_name)), root_(nullptr) {}

  /**
   * Constructor for the AST.
   * @param name The name of the program.
   * @param root The root node of the AST.
   */
  explicit AST(std::string name, std::shared_ptr<ASTNode> root) : name_(std::move(name)), root_(std::move(root)) {}

  /**
   * Constructor for the AST using a .ast file
   * @param file The .ast file
   */
  explicit AST(std::ifstream &file);

  /**
   * Destructor for the AST.
   */
  ~AST() = default;

  /**
   * Set the root node of the AST.
   * @param root The new root node.
   */
  void SetRoot(std::shared_ptr<ASTNode> root) { root_ = std::move(root); }

  /**
   * Get the root node of the AST.
   * @return The root node.
   */
  auto GetRoot() const -> std::shared_ptr<ASTNode> { return root_; }

 private:
  /* The name of the program */
  std::string name_;
  /* The root node of the AST */
  std::shared_ptr<ASTNode> root_;
};

}  // namespace scp::core
