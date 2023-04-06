#ifndef SESSION_INTERFACE_HPP
#define SESSION_INTERFACE_HPP

#include <cstddef>
#include <vector>
#include <cstdint>
#include <variant>
#include <iostream>

namespace Session {
	// CONSTANTS
	static constexpr int kNoSwap{0};
	static constexpr int kSwap{1};
	static constexpr int kMaxElm{0xFFFF};
	// USING ELEMENT
	template<class T>
		using Elm = std::pair<T,T>;
	// USING POINTER
	template<class T, typename = std::enable_if_t<std::is_pointer_v<T>>>
		using Ptr = std::pair<std::pair<T, size_t>, std::pair<T, size_t>>;
	// USING VAR
	using var_t = std::variant<Elm<int>, Elm<char>, Elm<std::string>,
			Ptr<uint8_t*>>;
	// USING RET
	using ret_t = std::vector<var_t>;
	// OVERLOADED
	template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
	template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;
	// VECTOR HASHER
	struct VectorHasher {
		size_t operator()(std::vector<uint8_t> const& v) const {
			size_t hash{v.size()};
			for(auto const &i : v) {
				hash ^=
					(size_t)i +
					0x9e3779b97f4a7c15LLU +
					(hash << 12) +
					(hash >> 4);
			}
			return hash;
		}
	};
}
#endif
