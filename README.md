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
- [x] notion of where a register actually is
- [x] segment registers are missing
- [x] read_value_from_register can be replaced by a read_value_from_operand
- [ ] restructure the way instructions are read in
- [ ] make a architecture structure right now memory and registers are seperate
