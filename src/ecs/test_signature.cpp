#include <doctest/doctest.h>
#include <gim/ecs/component_array.hpp>
#include <gim/ecs/ecs.hpp>
#include <gim/ecs/signature.hpp>

using namespace gim::ecs;

class TestComponent : public IComponent {
  public:
	int x;

	explicit TestComponent(int x) : x(x) {}
};

class AnotherComponent : public IComponent {
  public:
	int x;

	explicit AnotherComponent(int x) : x(x) {}
};

class YetAnotherComponent : public IComponent {
  public:
	int x;

	explicit YetAnotherComponent(int x) : x(x) {}
};

class OneMoreComponent : public IComponent {
  public:
	int x;

	explicit OneMoreComponent(int x) : x(x) {}
};

TEST_CASE("signature") {
	auto s1 = std::make_shared<Signature>();
	s1->set<TestComponent>();
	s1->set<AnotherComponent>();
	s1->set<YetAnotherComponent>();
	s1->set<OneMoreComponent>();

	auto s2 = std::make_shared<Signature>();
	s2->set<TestComponent>();
	s2->set<AnotherComponent>();
	s2->set<YetAnotherComponent>();

	auto s3 = std::make_shared<Signature>();
	s3->set<TestComponent>();
	s3->set<AnotherComponent>();

	auto s4 = std::make_shared<Signature>();
	s4->set<TestComponent>();

	CHECK(s1->subsetOf(s1) == true);
	CHECK(s2->subsetOf(s2) == true);
	CHECK(s3->subsetOf(s3) == true);
	CHECK(s4->subsetOf(s4) == true);
}
