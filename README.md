# Simple Compiler for Practice

> I'm trying to build a very simple compiler and this is only a project for practice purpose, so I'd like to call it SCP (Simple Compiler for Practice). You can refer to `docs/*.md` to see all the features it provide.

## Overall Design Principle

1. Using pipeline model.
2. Keep it simple, but supporting as more baseline algorithms as possible. 

## Components

### Lexical Analysis

Using a deterministic finite automata to implement regular expression match. Call next() in parser will generate a token retrived from input.

### Parsing

We support several implement:
1. LL(1) Parsing (currently building...)
