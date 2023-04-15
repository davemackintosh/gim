#include <doctest/doctest.h>
#include <gim/ecs/ecs.hpp>
#include <gim/ecs/signature.hpp>

using namespace gim::ecs;

class TestComponent : public IComponent {
  public:
	int x;

	TestComponent(int x) : x(x) {}
};

class AnotherComponent : public IComponent {
  public:
	int x;

	AnotherComponent(int x) : x(x) {}
};

class YetAnotherComponent : public IComponent {
  public:
	int x;

	YetAnotherComponent(int x) : x(x) {}
};

class OneMoreComponent : public IComponent {
  public:
	int x;

	OneMoreComponent(int x) : x(x) {}
};

TEST_CASE("signature") {
	Signature s1;
	Signature s2;
	Signature s3;
	Signature s4;

	s1.set<TestComponent>();
	s2.set<TestComponent>();
	s3.set<TestComponent>();
	s4.set<TestComponent>();

	s1.set<AnotherComponent>();
	s2.set<AnotherComponent>();
	s3.set<AnotherComponent>();

	s1.set<YetAnotherComponent>();
	s2.set<YetAnotherComponent>();

	s1.set<OneMoreComponent>();

	CHECK(s1 == s2);
	CHECK(s1 != s3);
	CHECK(s1 != s4);
	CHECK(s2 != s3);
	CHECK(s2 != s4);
	CHECK(s3 != s4);

	CHECK(s1.get<TestComponent>());
	CHECK(s1.get<AnotherComponent>());
	CHECK(s1.get<YetAnotherComponent>());
	CHECK(s1.get<OneMoreComponent>());

	CHECK(s2.get<TestComponent>());
	CHECK(s2.get<AnotherComponent>());
	CHECK(s2.get<YetAnotherComponent>());
	CHECK(!s2.get<OneMoreComponent>());

	CHECK(s3.get<TestComponent>());
	CHECK(s3.get<AnotherComponent>());
	CHECK(!s3.get<YetAnotherComponent>());
	CHECK(!s3.get<OneMoreComponent>());

	CHECK(s4.get<TestComponent>());
	CHECK(!s4.get<AnotherComponent>());
	CHECK(!s4.get<YetAnotherComponent>());
	CHECK(!s4.get<OneMoreComponent>());
}
