#pragma once

#include <array>
#include <bitset>
#include <cstdint>
// I don't know why this is required but tests were failing due to a missing
// template.
#include <iostream>

namespace gim::ecs {

const int ECS_MAX_ENTITIES = 5000;
const int ECS_MAX_COMPONENTS = 32;

typedef uint32_t Entity;
typedef std::bitset<ECS_MAX_COMPONENTS> Signature;

// A Component is just a placeholder class which is used to identify
// a component type in the ComponentArray and ComponentManager.
class IComponent {
  public:
	virtual ~IComponent() = default;
};

namespace TESTING {
// This is a test component which is used in the tests.
class TestComponent : public virtual IComponent {
  private:
	int value;

  public:
	TestComponent(int value) { this->value = value; };
	int getValue() { return value; };
};
} // namespace TESTING
} // namespace gim::ecs
