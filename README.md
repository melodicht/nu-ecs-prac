## Design Notes

- The reason why `u64` is used for EntityID is to avoid narrowing. We use
  `std::vector::size` for getting unique EntityIDs, which outputs `u64`.

# Building the Project
Make sure you have CMAKE and some C++ Compiler installed
1. Clone the project
2. Create a `/build/` directory
3. Open a terminal in that `/build/` directory
4. Run `cmake ../`
5. Run `make `
6. Fix the issues that will inevitably ensue.