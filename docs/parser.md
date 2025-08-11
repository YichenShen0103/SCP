# Parser

## Overview

The Parser is the second component of the SCP compiler, responsible for analyzing the sequence of tokens produced by the Lexer and constructing an Abstract Syntax Tree (AST) according to the language's grammar. The SCP compiler provides two parsing algorithms:

1. **LL(1) Parser**: A top-down predictive parser using predictive parsing tables
2. **SLR Parser**: A bottom-up LR parser using Simple LR parsing tables

Both parsers support the same grammar and produce equivalent ASTs, providing flexibility in parsing strategy and educational comparison between different parsing approaches.

## Architecture Design

### Core Classes

#### `scp::parser::LL1Parser`
The main parser class that implements the LL(1) parsing algorithm with predictive parsing table.

#### `scp::parser::SLRParser`  
The main parser class that implements the SLR (Simple LR) parsing algorithm with action and goto tables.

#### `scp::core::AST`
Abstract Syntax Tree class that represents the parsed program structure.

#### `scp::core::TreeNode`
Parse tree node used during parsing process before converting to AST.

## Supported Grammar

The Parser implements an LL(1) grammar for a simple programming language with the following constructs:

### Terminals
- `identifier`: Variable names (e.g., `variable1`, `_temp`, `myVar`)
- `number`: Integer literals (e.g., `123`, `0`, `999`)
- `plus` (+): Addition operator
- `times` (*): Multiplication operator
- `assign` (<-): Assignment operator
- `left_paren` ((): Left parenthesis
- `right_paren` ()): Right parenthesis
- `semicolon` (;): Statement terminator
- `$`: End-of-file marker

### Non-terminals
- `Program`: Top-level program structure
- `StatementList`: Sequence of statements
- `Statement`: Individual assignment statement
- `Expression`: Arithmetic expressions with addition
- `Term`: Arithmetic terms with multiplication
- `Factor`: Basic expression components

### Production Rules

```
Program → StatementList
StatementList → Statement StatementList | ε
Statement → identifier assign Expression semicolon
Expression → Term Expression'
Expression' → plus Term Expression' | ε
Term → Factor Term'
Term' → times Factor Term' | ε
Factor → identifier | number | left_paren Expression right_paren
```

## Main Features

### LL(1) Parser API

```cpp
// Initialize parser
void Init();

// Set input string for parsing
void SetInput(const std::string &input);

// Parse input and return AST
auto Parse() -> std::shared_ptr<core::AST>;
```

### SLR Parser API

```cpp
// Constructor with program name
explicit SLRParser(std::string program_name);

// Initialize parser and build action/goto tables
void Init();

// Set input string for parsing
void SetInput(const std::string &input);

// Parse input and return AST
auto Parse() -> std::shared_ptr<core::AST>;

// Build AST from parse tree
auto BuildAST(const std::shared_ptr<core::TreeNode> &parse_tree) -> std::shared_ptr<core::AST>;

// Validate symbols
auto IsValidSymbol(const std::string &symbol) const -> bool;
```

### Utility Functions

**LL(1) Parser Utilities:**
```cpp
// Print parsing table for debugging
void PrintParseTable() const;

// Print current parsing stack state
void PrintParsingStack() const;

// Validate symbols and parse table entries
auto IsValidSymbol(const std::string &symbol) const -> bool;
auto HasParseTableEntry(const std::string &nonTerminal, const std::string &terminal) const -> bool;
```

**SLR Parser Utilities:**
```cpp
// Validate symbols
auto IsValidSymbol(const std::string &symbol) const -> bool;

// Convert token types to parser terminal strings
auto TokenTypeToString(core::TokenType type) -> std::string;

// Check if symbol is terminal
auto IsTerminal(const std::string &symbol, const std::unordered_set<std::string> &terminals) -> bool;
```

### AST Construction

**LL(1) Parser AST Construction:**
```cpp
// Build AST from parse tree
auto BuildAST(const std::shared_ptr<core::TreeNode> &parse_tree) -> std::shared_ptr<core::AST>;
```

**SLR Parser AST Construction:**
```cpp
// Build AST from parse tree (public interface)
auto BuildAST(const std::shared_ptr<core::TreeNode> &parse_tree) -> std::shared_ptr<core::AST>;

// Internal AST transformation methods
auto TransformToASTNode(const std::shared_ptr<core::TreeNode> &parse_node) -> std::shared_ptr<core::AST::ASTNode>;
auto CreateTerminalASTNode(const std::string &symbol) -> std::shared_ptr<core::AST::ASTNode>;
auto TransformProgram(const std::shared_ptr<core::TreeNode> &parse_node) -> std::shared_ptr<core::AST::ASTNode>;
auto TransformStatement(const std::shared_ptr<core::TreeNode> &parse_node) -> std::shared_ptr<core::AST::ASTNode>;
auto TransformExpression(const std::shared_ptr<core::TreeNode> &parse_node) -> std::shared_ptr<core::AST::ASTNode>;
auto TransformTerm(const std::shared_ptr<core::TreeNode> &parse_node) -> std::shared_ptr<core::AST::ASTNode>;
auto TransformFactor(const std::shared_ptr<core::TreeNode> &parse_node) -> std::shared_ptr<core::AST::ASTNode>;
void CollectStatements(const std::shared_ptr<core::TreeNode> &parse_node, const std::shared_ptr<core::AST::ASTNode> &root);
```

## Usage Examples

### LL(1) Parser Usage

```cpp
#include "parser/ll1_parser.h"

scp::parser::LL1Parser parser;

// Parse simple assignment
parser.SetInput("result <- 123 + 456;");
auto ast = parser.Parse();

// Parse complex expression
parser.SetInput("result <- (a + b) * factor;");
auto complex_ast = parser.Parse();

// Parse multiple statements
parser.SetInput("a <- 10; b <- 20; c <- a + b;");
auto program_ast = parser.Parse();
```

### SLR Parser Usage

```cpp
#include "parser/slr_parser.h"

scp::parser::SLRParser parser("MyProgram");

// Parse simple assignment
parser.SetInput("result <- 123 + 456;");
auto ast = parser.Parse();

// Parse complex expression  
parser.SetInput("result <- (a + b) * factor;");
auto complex_ast = parser.Parse();

// Parse multiple statements
parser.SetInput("a <- 10; b <- 20; c <- a + b;");
auto program_ast = parser.Parse();
```

### Input Examples

#### Simple Assignment
```
result <- 42;
```

#### Arithmetic Expression
```
result <- (variable1 + variable2) * factor;
```

#### Multiple Statements
```
a <- 10;
b <- 20;
c <- a + b;
result <- c * 2;
```

#### Operator Precedence
```
result <- 1 + 2 * 3;  // Parsed as: result <- 1 + (2 * 3)
```

## Technical Implementation

### LL(1) Parsing Algorithm

The LL(1) parser uses a predictive parsing approach with the following components:

- **Parsing Table**: Pre-computed table mapping (non-terminal, terminal) pairs to production rules
- **Parsing Stack**: Stack-based algorithm for tracking parsing state
- **Lookahead**: Single token lookahead for decision making

### SLR Parsing Algorithm

The SLR (Simple LR) parser uses a bottom-up parsing approach with the following components:

- **Action Table**: Maps (state, terminal) pairs to parsing actions (SHIFT, REDUCE, ACCEPT, REJECT)
- **Goto Table**: Maps (state, non-terminal) pairs to next state transitions
- **Parsing Stack**: Stack containing symbols and parse trees along with state information
- **LR(0) Items**: Core items used to construct the parsing automaton

#### SLR Parser Actions

The SLR parser supports four types of actions:

```cpp
enum class ActionType { 
  SHIFT,   // Push token onto stack and transition to new state
  REDUCE,  // Reduce by production rule  
  ACCEPT,  // Input successfully parsed
  REJECT   // Parsing error
};
```

#### Action and Goto Tables

The action table determines what action to take given the current state and input terminal:
```cpp
std::unordered_map<int, std::unordered_map<std::string, Action>> action_table_;
```

The goto table determines the next state after reducing by a production:
```cpp
std::unordered_map<int, std::unordered_map<std::string, int>> goto_table_;
```

#### SLR Parsing Algorithm Flow

1. **Initialization**: Push initial state (0) onto stack
2. **Main Loop**:
   - Look up action in action table using current state and input terminal
   - **SHIFT**: Push terminal and new state onto stack, advance input
   - **REDUCE**: Pop RHS symbols from stack, push LHS non-terminal, consult goto table
   - **ACCEPT**: Parsing successful, return AST
   - **REJECT**: Report parsing error
3. **AST Construction**: Build AST during reduce actions using parse tree nodes

### Parsing Table Structure

**LL(1) Parsing Table:**
The parsing table is implemented as a nested hash map:
```cpp
std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>> parse_table_;
```

Example entries:
- `parse_table_["Expression"]["identifier"] = {"Term", "Expression'"}`
- `parse_table_["Expression'"]["plus"] = {"plus", "Term", "Expression'"}`
- `parse_table_["Expression'"]["semicolon"] = {"ε"}`

**SLR Action and Goto Tables:**
The SLR parser uses separate action and goto tables:

```cpp
// Action table: (state, terminal) -> Action
std::unordered_map<int, std::unordered_map<std::string, Action>> action_table_;

// Goto table: (state, non-terminal) -> next_state  
std::unordered_map<int, std::unordered_map<std::string, int>> goto_table_;
```

Example action table entries:
- `action_table_[0]["identifier"] = Action(SHIFT, 7)`
- `action_table_[1]["$"] = Action(ACCEPT)`
- `action_table_[9]["plus"] = Action(REDUCE, 0, {"Term"}, "Expression")`

Example goto table entries:
- `goto_table_[0]["Program"] = 1`
- `goto_table_[6]["Expression"] = 8`

### Algorithm Flow

1. **Initialization**: 
   - Initialize parsing stack with end marker (`$`) and start symbol (`Program`)
   - Set up lexer with input string

2. **Parsing Loop**:
   - Get current token from lexer
   - Examine top of parsing stack
   - If top is terminal: match with current token
   - If top is non-terminal: use parsing table to find production rule
   - Apply production rule by replacing non-terminal with production symbols

3. **Epsilon Handling**:
   - Special handling for epsilon (ε) productions
   - No stack operations for epsilon productions

4. **Error Recovery**:
   - Detailed error messages for parsing failures
   - Validation of parse table entries and symbols

### AST Construction

The parser constructs an Abstract Syntax Tree through a two-phase process:

1. **Parse Tree Generation**: Build parse tree during parsing process
2. **AST Transformation**: Convert parse tree to AST by eliminating unnecessary nodes

#### AST Node Types

```cpp
enum class ASTNodeType {
  ROOT,        // Program root
  IDENTIFIER,  // Variable names
  NUMBER,      // Numeric literals
  PLUS,        // Addition operations
  TIMES,       // Multiplication operations
  ASSIGN,      // Assignment statements
};
```

### Operator Precedence and Associativity

The grammar naturally handles operator precedence:
- **Multiplication (`*`)**: Higher precedence (handled in `Term`)
- **Addition (`+`)**: Lower precedence (handled in `Expression`)
- **Left Associativity**: Achieved through left-recursive elimination using prime non-terminals

## Error Handling

The parser provides comprehensive error handling:

- **Token Mismatch**: When terminal symbols don't match current token
- **Missing Productions**: When no production rule exists for (non-terminal, terminal) pair
- **Unexpected EOF**: When input ends unexpectedly
- **Parse Table Errors**: When accessing undefined entries

Error messages include context information:
```cpp
"Parsing error: expected 'semicolon', got 'identifier' with value 'x'"
"No production rule for symbol 'Expression' and terminal 'times'"
```

## Test Coverage

Both parsers have extensive test coverage including:

### Common Test Cases
- **Basic Functionality Tests**: Simple assignments and expressions
- **Complex Expression Tests**: Nested parentheses, operator precedence
- **Multiple Statement Tests**: Programs with multiple assignments
- **Error Case Tests**: Invalid syntax, missing operators, unbalanced parentheses
- **Edge Case Tests**: Empty programs, whitespace handling
- **AST Validation Tests**: Correct AST structure generation

### LL(1) Parser Specific Tests
- **Predictive Parsing Tests**: Parsing table validation
- **Left Recursion Elimination**: Grammar transformation verification
- **FIRST/FOLLOW Set Tests**: Set computation validation

### SLR Parser Specific Tests  
- **Action Table Tests**: Shift/reduce action validation
- **Goto Table Tests**: State transition verification
- **LR(0) Item Tests**: Item set construction validation
- **Conflict Resolution Tests**: Shift/reduce and reduce/reduce conflict handling

### Test File Examples

Test files located in `test/data/code/`:
- `expression.scpl`: Complex arithmetic expressions
- `multiple_statements.scpl`: Multi-statement programs
- `operator_precedence.scpl`: Precedence validation
- `parentheses.scpl`: Nested parentheses
- `error_*.scpl`: Various error conditions

### Performance Comparison Tests

The test suite includes performance benchmarks comparing LL(1) and SLR parsing:
- **Parsing Speed**: Time complexity validation
- **Memory Usage**: Stack depth and memory consumption
- **Error Recovery**: Error handling performance

## Performance Characteristics

### LL(1) Parser Performance
- **Linear Time Complexity**: O(n) parsing time for n tokens
- **Predictive Parsing**: No backtracking required
- **Efficient Table Lookup**: Constant time parsing decisions
- **Memory Efficient**: Stack-based approach with minimal memory overhead

### SLR Parser Performance
- **Linear Time Complexity**: O(n) parsing time for n tokens  
- **Bottom-up Parsing**: Handles left recursion naturally
- **State Machine**: Efficient DFA-based state transitions
- **Stack Operations**: Constant time push/pop operations

### Comparison

| Aspect                | LL(1) Parser            | SLR Parser                |
| --------------------- | ----------------------- | ------------------------- |
| **Parsing Direction** | Top-down                | Bottom-up                 |
| **Grammar Support**   | Requires transformation | Natural left recursion    |
| **Table Size**        | Smaller parsing table   | Larger action/goto tables |
| **Memory Usage**      | Lower stack depth       | Higher stack usage        |
| **Error Detection**   | Earlier error detection | Later error detection     |
| **Grammar Class**     | LL(1) subset            | SLR subset of LR(1)       |

## Grammar Properties

### Grammar Support

Both LL(1) and SLR parsers support the same context-free grammar, but use different parsing strategies:

**LL(1) Grammar Requirements:**
- No left recursion (requires grammar transformation)
- Left factored
- FIRST and FOLLOW sets are disjoint

**SLR Grammar Requirements:**  
- Left recursion allowed (natural expression of operator precedence)
- No reduce/reduce conflicts in SLR parsing table
- No shift/reduce conflicts in SLR parsing table

### Original Grammar (for SLR)

```
Program → StatementList
StatementList → Statement StatementList | ε
Statement → identifier assign Expression semicolon
Expression → Expression plus Term | Term
Term → Term times Factor | Factor  
Factor → identifier | number | left_paren Expression right_paren
```

### Transformed Grammar (for LL(1))

```
Program → StatementList
StatementList → Statement StatementList | ε  
Statement → identifier assign Expression semicolon
Expression → Term Expression'
Expression' → plus Term Expression' | ε
Term → Factor Term'
Term' → times Factor Term' | ε
Factor → identifier | number | left_paren Expression right_paren
```

### FIRST and FOLLOW Sets

**FIRST Sets:**
- FIRST(Program) = {identifier, ε}
- FIRST(Expression) = {identifier, number, left_paren}
- FIRST(Term) = {identifier, number, left_paren}
- FIRST(Factor) = {identifier, number, left_paren}

**FOLLOW Sets:**
- FOLLOW(Program) = {$}
- FOLLOW(Expression) = {semicolon, right_paren}
- FOLLOW(Term) = {plus, semicolon, right_paren}
- FOLLOW(Factor) = {times, plus, semicolon, right_paren}

## Extensibility

The parser architecture supports easy extension:

### Adding New Operators
1. Add terminal to lexer token types
2. Update grammar production rules
3. Extend parsing table with new entries
4. Add corresponding AST node types
5. Update AST transformation methods

### Adding New Statement Types
1. Extend `Statement` production rules
2. Add new non-terminals as needed
3. Update parsing table
4. Implement AST transformation for new constructs

## Dependencies

- `core/ast.h`: AST data structures
- `core/token.h`: Token definitions from lexer
- `lexer/lexer.h`: Lexer integration
- `constant/ast_constant.h`: AST-related constants
- `constant/error_messages.h`: Error message definitions
- Standard library: `<memory>`, `<stack>`, `<unordered_map>`, `<unordered_set>`, `<vector>`

## File Structure

```
include/parser/
├── ll1_parser.h         # LL1Parser class declaration
└── slr_parser.h         # SLRParser class declaration

src/parser/
├── ll1_parser.cpp       # LL1Parser implementation
└── slr_parser.cpp       # SLRParser implementation

include/core/
└── ast.h               # AST class declarations

docs/
├── ll1_grammar.txt     # LL(1) grammar specification  
├── slr_grammar.txt     # SLR grammar specification
└── parser.md           # This documentation

test/
├── ll1_parser_test.cpp # LL(1) parser unit tests
├── slr_parser_test.cpp # SLR parser unit tests
└── data/
    ├── code/           # Test input files
    └── ast/            # Expected AST outputs
```

## Integration with Lexer

Both parsers seamlessly integrate with the lexer:

- **Token Stream Processing**: Consume tokens one at a time using `lexer_.Next()`
- **Lookahead Management**: Single token lookahead for parsing decisions
- **EOF Handling**: Proper handling of end-of-file conditions
- **Error Coordination**: Consistent error reporting between lexer and parser

## Parser Selection Guidelines

### Choose LL(1) Parser When:
- Learning top-down parsing concepts
- Need early error detection
- Working with simple, naturally LL(1) grammars
- Memory usage is critical
- Want smaller parsing tables

### Choose SLR Parser When:
- Learning bottom-up parsing concepts
- Working with left-recursive grammars
- Need to handle operator precedence naturally
- Want more powerful grammar class support
- Educational comparison with LL(1) approach

## Conclusion

The SCP compiler's dual parser implementation provides a comprehensive foundation for syntax analysis, offering both top-down (LL(1)) and bottom-up (SLR) parsing strategies. Both parsers:

- Support the same programming language grammar
- Generate equivalent Abstract Syntax Trees
- Provide robust error handling and reporting
- Maintain linear time complexity O(n)
- Include comprehensive test coverage
- Offer clear separation of concerns and extensible architecture

This implementation serves as an excellent educational resource for understanding different parsing techniques while providing a robust foundation for the SCP compiler's syntax analysis phase. The choice between parsers depends on specific requirements, learning objectives, and the nature of the grammar being parsed.
