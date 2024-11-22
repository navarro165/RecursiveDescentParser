# Recursive Descent Parser

**Course:** CSE340  
**Project 2: Parsing**  

### Description  
This project implements a **top-down recursive descent parser** to process a language defined by a given grammar. It includes:  
- **Lexical Analysis:** Tokenizing input, handling comments, and ignoring whitespace.  
- **Parsing:** Building a parser for the specified grammar.  
- **Scoping and Symbol Table Management:** Resolving variable declarations and references using lexical scoping rules.  

The parser resolves assignments to their specific scope and outputs either resolved assignments or a syntax error.  

### Features  
1. **Lexical Analysis:**  
   - Supports tokens like `ID`, `PUBLIC`, `PRIVATE`, `EQUAL`, and more.  
   - Skips spaces and comments (`// ...`).  

2. **Grammar Support:**  
   - Nested scopes.  
   - Public and private variable access rules.  
   - Resolves variable references using scoping rules.  

3. **Output:**  
   - Outputs resolved assignments (e.g., `test.a = test.b`).  
   - Prints `Syntax Error` if input is invalid.  

### Files  
- **Source Code:**  
  - `lexer.cc` (main implementation)  
  - `lexer.h`, `inputbuf.cc`, `inputbuf.h`  
- **Testing:**  
  - `tests/` (test cases)  
  - `run_tests.sh` (automated test runner)  

### Build and Run  
1. **Compile:**  
   ```bash
   clang++ -std=c++11 -Wall *.cc -o lexer
