# MAVN Compiler

A compiler for **MAVN (MIPS Assembler of Higher Level)**, developed as a project for the **Fundamentals of Parallel Programming and Software Tools** course at the Faculty of Technical Sciences, University of Novi Sad.

The compiler translates programs written in a simplified high-level MIPS 32-bit assembly language into basic MIPS 32-bit assembly code.

## About

MAVN extends basic MIPS assembly with **logical register variables** (`r1`, `r2`, `r3`, ...), allowing programs to be written without directly managing a limited set of physical registers.

The compiler performs **register allocation** by analyzing variable lifetimes and constructing an interference graph. Logical registers are then mapped to a maximum of **four physical MIPS registers**:

* `$t0`
* `$t1`
* `$t2`
* `$t3`

The project implements a complete compilation pipeline, from lexical and syntactic analysis to MIPS code generation.

## Compilation Pipeline

```text
MAVN source (.mavn)
        │
        ▼
┌─────────────────────┐
│   Lexical Analysis  │
└─────────────────────┘
        │
        ▼
┌─────────────────────┐
│  Syntax Analysis    │
└─────────────────────┘
        │
        ▼
┌─────────────────────┐
│ Intermediate         │
│ Representation (IR) │
└─────────────────────┘
        │
        ▼
┌─────────────────────┐
│ Liveness Analysis   │
│   use / def / in /  │
│        out          │
└─────────────────────┘
        │
        ▼
┌─────────────────────┐
│ Interference Graph  │
└─────────────────────┘
        │
        ▼
┌─────────────────────┐
│ Register Allocation │
│   Graph Coloring    │
└─────────────────────┘
        │
        ▼
┌─────────────────────┐
│   Code Generation   │
└─────────────────────┘
        │
        ▼
   output.s (MIPS)
```

## Features

### Lexical Analysis

The compiler uses a **finite-state machine** to tokenize the input MAVN program.

It recognizes:

* Keywords and declarations
* Identifiers
* Register and memory variables
* Numeric constants
* Assembly instructions
* Operators and delimiters

### Syntax Analysis

A **recursive-descent parser** validates the token sequence according to the MAVN grammar and simultaneously builds the intermediate representation.

### Intermediate Representation

The compiler uses two main structures:

* `Variable` - represents memory and register variables
* `Instruction` - represents MAVN instructions together with their operands, control-flow information and liveness sets

### Liveness Analysis

For every instruction, the compiler calculates:

* `use`
* `def`
* `in`
* `out`

The analysis is performed iteratively over the control-flow graph until the sets converge.

### Interference Graph

An interference graph is constructed from the liveness information.

Each node represents a logical register variable, while an edge means that two variables are simultaneously live and therefore cannot be assigned to the same physical register.

### Register Allocation

Register allocation is implemented as a **graph-coloring algorithm** with a maximum of four colors.

The allocation consists of:

1. **Build** - construct the interference graph
2. **Simplify** - remove nodes with fewer than four neighbors
3. **Spill detection** - detect cases where allocation with four registers is not possible
4. **Select** - assign physical registers while rebuilding the graph

If the graph cannot be colored with four registers, the compiler reports a **register allocation spill**.

### MIPS Code Generation

After successful register allocation, the compiler generates a MIPS assembly file containing:

* `.data` section for memory variables
* `.text` section for instructions
* Function labels
* Mapped physical registers
* Translated MIPS instructions

The generated output is written to:

```text
output.s
```

## Supported Instructions

The compiler supports the instructions defined by the project specification:

```text
add
addi
sub
la
li
lw
sw
b
bltz
nop
```

The language was additionally extended with:

```text
and
or
beq
```

### Additional Instructions

* `and` - bitwise AND between two registers
* `or` - bitwise OR between two registers
* `beq` - conditional branch when two register values are equal

## Project Structure

```text
MAVN-Compiler/
│
├── doc/
│   ├── MIPS-instructions.pdf
│   ├── primer_dobre_dokumentacije.doc
│   └── projekat.pdf
│
├── examples/
│   ├── simple.mavn
│   ├── multiply.mavn
│   ├── extra.mavn
│   ├── loop.mavn
│   └── primer.mavn
│
└── src/
    ├── CodeGeneration.cpp
    ├── CodeGeneration.h
    ├── Constants.h
    ├── FiniteStateMachine.cpp
    ├── FiniteStateMachine.h
    ├── IR.h
    ├── LexicalAnalysis.cpp
    ├── LexicalAnalysis.h
    ├── LexicalAnalysis.sln
    ├── LexicalAnalysis.vcxproj
    ├── LexicalAnalysis.vcxproj.filters
    ├── LivenessAnalysis.cpp
    ├── LivenessAnalysis.h
    ├── ResourceAllocation.cpp
    ├── ResourceAllocation.h
    ├── SyntaxAnalysis.cpp
    ├── SyntaxAnalysis.h
    ├── Token.cpp
    ├── Token.h
    ├── Types.h
    └── main.cpp
```

## Testing

Several MAVN programs are included in the `examples/` directory.

| Test            | Purpose                                                       |
| :-------------- | :------------------------------------------------------------ |
| `simple.mavn`   | Basic memory/register operations and arithmetic instructions  |
| `multiply.mavn` | Tests register allocation failure and spill detection         |
| `extra.mavn`    | Tests a broad set of implemented instructions                 |
| `loop.mavn`     | Tests `and`, `or`, `beq`, labels and conditional control flow |
| `primer.mavn`   | Additional example program                                    |

The test cases cover lexical and syntax analysis, liveness analysis, interference graph construction, register allocation and MIPS code generation.

## Building and Running

The project is implemented in **C++** and can be built using **Visual Studio**.

Open:

```text
src/LexicalAnalysis.sln
```

Build the solution and run the compiler with a MAVN source file as input.

The compiler produces:

```text
output.s
```

containing the generated MIPS assembly code.

## Project Documentation

Additional project documentation, including the full project report and MIPS instruction reference, can be found in the [`doc/`](doc/) directory.

## 👤 Author

| Name and Surname | Index Number | Department            | Module                                           | Mentor           |
| :--------------- | :----------- | :-------------------- | :----------------------------------------------- | :--------------- |
| **Miloš Trišić** | RA 39/2023   | Computing and Control | Computer Engineering and Computer Communications | Teodora Novković |

---

This project was developed as part of the coursework at the Faculty of Technical Sciences, University of Novi Sad.
