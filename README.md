## Design Notes

- The reason why `u64` is used for EntityID is to avoid narrowing. We use
  `std::vector::size` for getting unique EntityIDs, which outputs `u64`.


# Game-as-a-service Architecture

The game engine uses a game-as-a-service architecture. In this architecture, there are two components: the platform component and the game component. Each must be built separately. The platform component is the main entry point of the program, and dynamically loads the game component. From this point onwards, we shall refer to the "game component" as the game module.

The main idea behind this architecture is that for every new platform we would like to support, we can write a new implementation of the platform component, and the game module would remain the same. The game module is our game as a mathematical object, insulated from the non-mathematical nature of operating systems.

Currently, we only have one implementation for the platform component, which uses SDL3. Conveniently, SDL3 works for all operating systems used by most people, so can get away with just using that as our sole implementation for the platform component. It is still beneficial to write platform components tailor-made to a specific operating system (or more broadly, a platform), as we can squeeze the full capability of that specific operating system without having to worry about compatibility with another operating system.

The greatest benefit of this architecture is that while the game is running, the game module can be replaced with a new game module, also known as hot reloading.

# Components Overview


# Building the Project (OUTDATED)
Make sure you have CMAKE and some C++ Compiler installed
1. Clone the project
2. Create a `/build/` directory
3. Open a terminal in that `/build/` directory
4. Run `cmake ../`
5. Run `make `
6. Fix the issues that will inevitably ensue from steps 4 and or 5.


