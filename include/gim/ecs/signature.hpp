#pragma once

#include <cassert>
#include <map>
#include <memory>
#include <string_view>
#include <typeinfo>

namespace gim::ecs {
class Signature {
  private:
	std::map<std::string_view, bool> signature;

  public:
	Signature() { signature.clear(); }

	template <typename T> auto set() -> void {
		std::string_view typeName = typeid(T).name();

		// Assert that the component has not been registered before.
		assert(signature.find(typeName) == signature.end() &&
			   "Registering component type more than once.");

		signature.insert({typeName, true});
	}

	template <typename T> auto unset() -> void {
		std::string_view typeName = typeid(T).name();
		signature.insert({typeName, false});
	}

	template <typename T> auto get() -> bool {
		std::string_view typeName = typeid(T).name();
		if (signature.find(typeName) == signature.end()) {
			return false;
		}
		return signature.at(typeName);
	}

	auto reset() -> void { signature.clear(); }

	auto empty() -> bool { return signature.empty(); }

	auto subsetOf(std::shared_ptr<Signature> other) -> bool {
		for (auto const &[key, value] : signature) {
			if (other->signature.find(key) == other->signature.end()) {
				return false;
			}
			if (value != other->signature.at(key)) {
				return false;
			}
		}
		return true;
	}

	auto operator&(Signature const &other) -> Signature {
		Signature result;
		for (auto const &[key, value] : signature) {
			if (other.signature.find(key) != other.signature.end()) {
				result.signature.insert(
					{key, value && other.signature.at(key)});
			}
		}
		return result;
	}

	auto operator==(Signature const &other) -> bool {
		for (auto const &[key, value] : signature) {
			if (other.signature.find(key) == other.signature.end()) {
				return false;
			}
			if (value != other.signature.at(key)) {
				return false;
			}
		}
		return true;
	}

	auto operator!=(Signature const &other) -> bool {
		return !(*this == other);
	}

	auto operator=(Signature const &other) -> Signature & {
		signature = other.signature;
		return *this;
	}
};
} // namespace gim::ecs
