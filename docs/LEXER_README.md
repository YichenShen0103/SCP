# Lexer Implementation Documentation

## Overview
This lexer implementation provides tokenization support for two types of tokens:
- **IDENTIFIER**: Variables, function names, etc. (matches `^[A-Za-z_][A-Za-z0-9_]*$`)
- **NUMBER**: Integer literals (matches `^[0-9]+$`)

## Files Created/Modified
- `include/lexer.h` - Lexer class header file
- `src/lexer.cc` - Lexer class implementation
- `CMakeLists.txt` - Updated to include lexer compilation

## Usage Example

### Pipeline (Streaming) Interface (Recommended)
```cpp
#include "lexer.h"
#include "token.h"

int main() {
    // Create lexer instance
    Lexer lexer;
    
    // Set input for streaming tokenization
    std::string input = "hello 123 world_var 456";
    lexer.setInput(input);
    
    // Process tokens one by one using pipeline approach
    while (lexer.hasNext()) {
        auto token = lexer.next();
        if (token.has_value()) {
            std::cout << toString(token->getType()) 
                      << " = \"" << token->getValue() << "\"" << std::endl;
        }
    }
    
    return 0;
}
```

### Legacy Batch Interface (For Compatibility)
```cpp
#include "lexer.h"
#include "token.h"

int main() {
    // Create lexer instance
    Lexer lexer;
    
    // Tokenize input string (legacy approach)
    std::string input = "hello 123 world_var 456";
    std::vector<Token> tokens = lexer.tokenize(input);
    
    // Process tokens
    for (const Token& token : tokens) {
        std::cout << toString(token.getType()) 
                  << " = \"" << token.getValue() << "\"" << std::endl;
    }
    
    return 0;
}
```

## Features

### Pipeline (Streaming) Architecture
- **Non-blocking Processing**: Uses pipeline approach for real-time token processing
- **Memory Efficient**: Processes one token at a time instead of loading all tokens into memory
- **Streaming Interface**: `setInput()`, `next()`, `hasNext()`, and `reset()` methods for stream-like processing
- **Legacy Support**: Maintains backward compatibility with batch `tokenize()` method

### Token Recognition
- **Identifiers**: Must start with letter or underscore, followed by letters, digits, or underscores
- **Numbers**: Sequences of digits (0-9)

### Whitespace Handling
- Automatically skips spaces, tabs, newlines, and carriage returns
- Treats whitespace as token separators

### Error Handling
- Gracefully handles invalid characters by skipping them
- Outputs warnings for unrecognized characters
- Uses longest match principle for tokenization

### DFA Integration
- Uses the existing `DeterministicFiniteAutomata` class for token recognition
- Each token type has its own DFA instance
- Follows the same pattern as `dfa_test.cpp`

## Implementation Details

### Class Structure
```cpp
class Lexer {
private:
    // Input string and current position for streaming
    std::string input_;
    size_t current_pos_;
    
    std::unique_ptr<DeterministicFiniteAutomata> number_dfa_;
    std::unique_ptr<DeterministicFiniteAutomata> identifier_dfa_;
    std::vector<DeterministicFiniteAutomata*> dfa_list_;
    std::vector<bool> survival_list_;
    
public:
    Lexer();
    
    // Pipeline (streaming) interface
    void setInput(const std::string& input);
    std::optional<Token> next();
    bool hasNext() const;
    void reset();
    
    // Legacy batch interface
    std::vector<Token> tokenize(const std::string& input);
};
```

### DFA Setup
- **Number DFA**: 2 states, transitions on digits 0-9
- **Identifier DFA**: 2 states, initial transitions on letters/underscore, loop transitions on alphanumeric/underscore

### Pipeline Processing Algorithm
1. **Input Setup**: `setInput()` stores the input string and resets position to 0
2. **Token Streaming**: `next()` processes characters from current position:
   - Skip whitespace
   - Initialize all DFAs for next token
   - Feed characters to all active DFAs using pipeline approach
   - Track the longest valid token match
   - Return `std::optional<Token>` when token is found or `std::nullopt` when done
3. **State Management**: `hasNext()` checks for remaining tokens, `reset()` restarts from beginning
4. **Legacy Support**: `tokenize()` uses streaming interface internally for batch processing

## Test Results

The lexer successfully handles various test cases using both pipeline and legacy interfaces:

### Pipeline Interface Test
```
Input: "hello123 world 456" 
Token 1: IDENTIFIER = "hello123"
Token 2: IDENTIFIER = "world" 
Token 3: NUMBER = "456"
```

### Legacy Interface Test  
```
Input: "aa 123" -> [id="aa", int="123"]
Input: "ababc 4a56b7" -> [id="ababc", int="4", id="a56b7"] 
Input: "xyz_789" -> [id="xyz_789"]
Input: "123abc" -> [int="123", id="abc"]
Input: "_" -> [id="_"]
Input: "0" -> [int="0"]
```

### Reset Functionality Test
```
First token: abc
After reset, first token: abc
```

## Building and Testing

### Using CMake
```bash
cd /path/to/compiler
rm -rf build && cmake -B build -S . && cd build && make
./lexer                    # Run basic test
./lexer_extended_test     # Run comprehensive batch test
./lexer_stream_test       # Run pipeline/streaming interface test
```

### Direct Compilation
```bash
g++ -std=c++17 -I include -o test lexer_simple_test.cpp src/lexer.cc src/dfa.cc
./test
```

## Integration with Existing Code

The lexer is designed to work seamlessly with the existing DFA implementation and follows the same patterns used in `test/dfa_test.cpp`. It uses the same alphabet definitions and transition setup approach as demonstrated in the test file.

### Key Architectural Changes
- **Pipeline Architecture**: Replaced blocking batch processing with streaming pipeline approach
- **Memory Efficiency**: Processes tokens on-demand instead of storing all tokens in memory
- **Enhanced Interface**: Added streaming methods (`setInput`, `next`, `hasNext`, `reset`) while maintaining legacy `tokenize` method
- **State Management**: Maintains internal position tracking for continuous token processing
