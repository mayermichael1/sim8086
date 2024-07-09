# SIM8086

Does not simulate actual CPU work.
This "simulator" does disassemble 8086 machine code.

Currently supports:

- mov from register to register
- mov between accumulator and memory
- mov immediate to register
- mov from register to memory or memory to register
- mov from and to direct address
- mov immediate to memory

# TODOs

- [x] MOV to and from Memory
- [x] MOV immediate to register
- [x] MOV to and from ax
- [x] signed memory displacements
- [x] MOV immediate to memory
- [x] refactor code
- [ ] notion of where a register actually is
- [ ] restructure the way instructions are read in
