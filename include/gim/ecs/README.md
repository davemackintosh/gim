# ECS

The Entity Component Systems manager is a way to segregate the data used to power the game though it's entire lifecyle.

# Contigious Memory.

C++ guarantees that all `vector` memory is contigious which helps solve the data locality problem. How the data is arranged is decribed below

![ECS memory layout](https://user-images.githubusercontent.com/1430657/232319359-406153ba-1625-49f8-b557-090cbb0a8140.png)

## Entities and signatures

These are special types to this ECS and are pre-allocated to the size permitted at ECS instantiation time. This affects runtime memory consumption as the different between preallocating 10 uint32_t and Signature classes will be far less than preallocating 1'000'000+ respectively.

# Memory ownership.

*You* are owner of the memory for component instances only, the ECS class is the owner of System & Entity memory. Your components are created as `std::shared_ptr` shared, smart pointers so are automatically cleaned up when not in use. Entities are a simple unsigned long `uint32_t` and are distributed across a packed array of `uint32_t`s.

Systems are created internally from type information as `std::unique_ptr` smart pointers and are also cleaned up automatically at the end of their use.
