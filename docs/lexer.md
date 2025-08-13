# Lexer

## Overview

The Lexer is the first component of the SCP compiler, responsible for converting input source code strings into a sequence of tokens. It uses Deterministic Finite Automata (DFA) to recognize different types of lexical units.

## Architecture Design

### Core Classes

#### `scp::lexer::Lexer`
The main lexical analyzer class that provides both streaming and batch tokenization functionality.

#### `scp::lexer::DeterministicFiniteAutomata` 
DFA implementation class used to recognize tokens with specific patterns.

#### `scp::core::Token`
Token class that contains token type and value.

## Supported Token Types

The Lexer currently supports the following 9 token types:

| Token Type    | Description             | Regular Expression         | Examples                            |
| ------------- | ----------------------- | -------------------------- | ----------------------------------- |
| `IDENTIFIER`  | Identifier              | `^[A-Za-z_][A-Za-z0-9_]*$` | `variable1`, `_temp`, `myVar`       |
| `NUMBER`      | Number                  | `^[0-9]+$`                 | `123`, `0`, `999`                   |
| `STRING`      | String literal          | `^".*"$`                   | `"hello"`, `"hello world"`, `"123"` |
| `PLUS`        | Plus operator           | `^\+$`                     | `+`                                 |
| `TIMES`       | Multiplication operator | `^\*$`                     | `*`                                 |
| `LEFT_PAREN`  | Left parenthesis        | `^\($`                     | `(`                                 |
| `RIGHT_PAREN` | Right parenthesis       | `^\)$`                     | `)`                                 |
| `ASSIGN`      | Assignment operator     | `^<-$`                     | `<-`                                |
| `SEMICOLON`   | Semicolon               | `^;$`                      | `;`                                 |

## Main Features

### Streaming API

```cpp
// Set input string
void SetInput(const std::string &input);

// Get next token
auto Next() -> std::optional<core::Token>;

// Check if there are more tokens
auto HasNext() const -> bool;

// Reset to beginning position
void Reset();
```

### Batch Processing API

```cpp
// Tokenize entire input at once (legacy method)
auto Tokenize(const std::string &input) -> std::vector<core::Token>;
```

## Usage Examples

### Basic Usage

```cpp
#include "lexer/lexer.h"

scp::lexer::Lexer lexer;

// Streaming processing
lexer.SetInput("variable1 <- 123 + 456;");
while (lexer.HasNext()) {
    auto token = lexer.Next();
    if (token) {
        std::cout << "Token: " << core::ToString(token->GetType()) 
                  << ", Value: " << token->GetValue() << std::endl;
    }
}

// Batch processing
auto tokens = lexer.Tokenize("result <- (a + b) * 2;");
for (const auto& token : tokens) {
    // Process each token
}
```

### Output Example

For input: `message <- "hello world";`

Output tokens:
```
Token: IDENTIFIER, Value: message
Token: ASSIGN, Value: <-
Token: STRING, Value: "hello world"
Token: SEMICOLON, Value: ;
```

For input: `variable1 <- 123 + 456;`

Output tokens:
```
Token: IDENTIFIER, Value: variable1
Token: ASSIGN, Value: <-
Token: NUMBER, Value: 123
Token: PLUS, Value: +
Token: NUMBER, Value: 456
Token: SEMICOLON, Value: ;
```

## Technical Implementation

### DFA Architecture

The Lexer maintains a separate DFA for each token type:

- **Parallel Processing**: All DFAs process input characters simultaneously
- **Longest Match**: Selects the DFA that matches the longest string
- **Priority Handling**: When match lengths are equal, selects based on predefined priority

### State Management

- `input_`: Current input string
- `current_pos_`: Current processing position
- `dfa_list_`: List of all DFAs
- `survival_list_`: Tracks which DFAs are still active

### Algorithm Flow

1. **Initialization**: Reset all DFAs to initial state
2. **Character Processing**:
   - Skip whitespace characters
   - Feed current character to all active DFAs
   - Remove DFAs that cannot continue
3. **Token Recognition**:
   - When no DFA can continue, select the longest matching accepting DFA
   - Create corresponding Token object
4. **Position Update**: Update current position, continue to next token

## Whitespace Handling

The Lexer automatically skips the following whitespace characters:
- Space (` `)
- Tab (`\t`)
- Newline (`\n`)
- Carriage return (`\r`)

## Error Handling

- When encountering unrecognizable characters, the lexer stops processing
- Uses `std::optional` return values to indicate successful token retrieval
- Provides `HasNext()` method to check if more tokens are available for processing

## Test Coverage

The Lexer has comprehensive test coverage, including:

- **Basic Functionality Tests**: Recognition of various token types
- **Edge Case Tests**: Empty input, single characters, special character combinations
- **Streaming Tests**: `SetInput`, `Next`, `HasNext`, `Reset` functionality
- **Complex Expression Tests**: Real code snippet tokenization
- **File Input Tests**: Reading and processing from test files

## Performance Characteristics

- **Efficient DFA Implementation**: Uses state transition tables for performance optimization
- **Streaming Processing**: Supports incremental processing of large files
- **Memory Management**: Uses smart pointers to manage DFA lifecycles

## Extensibility

The current architecture makes it easy to add new token types:

1. Add new type to `TokenType` enum
2. Create corresponding DFA setup function
3. Initialize new DFA in `InitializeDFAs()`
4. Add corresponding test cases

## Dependencies

- `core/token.h`: Token type definitions
- `lexer/dfa.h`: DFA implementation
- Standard library: `<memory>`, `<optional>`, `<string>`, `<vector>`

## File Structure

```
include/lexer/
├── lexer.h          # Lexer class declaration
└── dfa.h           # DFA class declaration

src/lexer/
├── lexer.cpp       # Lexer implementation
└── dfa.cpp         # DFA implementation

test/
├── lexer_test.cpp  # Unit tests
└── data/           # Test data files
    ├── expression.scpl
    ├── sample_input_clean.scpl
    └── ...
```

This lexer implementation provides a stable and efficient lexical analysis foundation for the SCP compiler, supporting both streaming and batch processing modes, with good extensibility and test coverage.
