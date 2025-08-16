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
1. LL(1) Parsing
2. SLR Parsing

Generate a pointer to the Abstract Syntax Tree for following processing.

### Semantic Analysis

We mainly do a type check in this section using a recursive descent on the AST.

### Code Generation

We use a 1-register stack machine to implement code generator, while providing several library function.


## Usage and Demo

**Download Spim**

Use your favourite package manager to download the MIPS simulator `spim`. If you're using MacOS, you can do this by:

```sh
$ brew install spim
```

**Compile the project**

```sh
$ cd build
$ cmake ..
$ make
```

**Write a simple demo**
```sh
$ cat demo/demo.scpl
stdout <- "please tell me your name: ";
name <- stdin;
stdout <- "hello " + name + "!\n";
```

**Compile the demo and run**

We assume you've export `build/bin` into environmental variable.

```sh
$ scpc hello.scpl -o hello.s
$ spim -quiet hello.s
please tell me your name: chiri
hello, chiri!
```