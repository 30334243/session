#ifndef SESSION_HPP
#define SESSION_HPP

#include <unordered_map>
#include <utility>
#include <vector>
#include <cstdint>
#include <cstddef>
#include <variant>
#include <typeinfo>
#include <iostream>
#include <functional>

// SESSION
class Session {
	public:
		// USING ELM
		template<class T>
			using Elm = std::pair<T,T>;
		// CONSTRUCTOR
		explicit Session() : m_key_first(kMaxElm), m_key_second(kMaxElm) {}
		// GET CONVERSATION ID
		template<class... Tail>
			std::tuple<size_t, bool> GetConversationId(Tail const&... tail) {
				std::tuple<size_t, bool> ret{};
				transform<kNoSwap>(m_key_first, tail...);
				transform<kSwap>(m_key_second, tail...);
				if (auto it{m_sessions.find(m_key_first)}; it != m_sessions.cend()) {
					ret = std::make_tuple(VectorHasher()(it->first), it->second);
				} else if (auto it{m_sessions.find(m_key_second)};
							  it != m_sessions.cend()) {
					ret = std::make_tuple(VectorHasher()(it->first), it->second);
				} else {
					m_sessions[m_key_first] = false;
					m_sessions[m_key_second] = true;
					ret = std::make_tuple(VectorHasher()(m_key_first), false);
				}
				return ret;
			};
	private:
		// CONSTANTS
		static constexpr int kNoSwap{0};
		static constexpr int kSwap{1};
		static constexpr int kMaxElm{0xFFFF};
		// USING VAR
		using var_t = std::variant<Elm<int>, Elm<char>, Elm<std::string>>;
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
	// INTEGRALL TO VECTOR
	template<class T, int IdxF, int IdxS>
		static auto integral_to_vec(std::vector<uint8_t>& key,
											 size_t const offset, Elm<T>& var) {
			std::copy((uint8_t*)&std::get<IdxF>(var),
						 (uint8_t*)&std::get<IdxF>(var) + sizeof(T),
						 key.data() + offset);
			std::copy((uint8_t*)&std::get<IdxS>(var),
						 (uint8_t*)&std::get<IdxS>(var) + sizeof(T),
						 key.data() + offset + sizeof(T));
			return sizeof(T) * 2;
		}
	// INTEGRALL TO VECTOR
	template<int IdxF, int IdxS, class T,
		typename = std::enable_if_t<std::is_integral_v<T>>>
			static constexpr auto integral_to_vec(std::vector<uint8_t>& key,
															  size_t const offset) {
				return [&key, offset] (Elm<T>& var) {
					return integral_to_vec<T, IdxF, IdxS>(key, offset, var);
				};
			}
	// STRING TO VECTOR
	template<int IdxF, int IdxS>
		static auto str_to_vec(std::vector<uint8_t>& key,
									  size_t const offset,
									  Elm<std::string>& str) {
			std::copy(std::get<IdxF>(str).data(),
						 std::get<IdxF>(str).data() + std::get<IdxF>(str).size(),
						 key.data() + offset);
			std::copy(std::get<IdxS>(str).data(),
						 std::get<IdxS>(str).data() + std::get<IdxS>(str).size(),
						 key.data() + offset + std::get<IdxF>(str).size());
			return str.first.size() + str.second.size();
		}

	// STRING TO VECTOR
	template<int IdxF, int IdxS>
		static auto str_to_vec(std::vector<uint8_t>& key, size_t const offset) {
			return [&key, offset] (Elm<std::string>& str) {
				return str_to_vec<IdxF, IdxS>(key, offset, str);
			};
		}
	// SWAP
	template<int F, int S>
		static auto swap(std::vector<uint8_t>& key,
							  size_t const offset, var_t& val) {
			return std::visit(overloaded{
				integral_to_vec<F, S, int>(key, offset),
					integral_to_vec<F, S, char>(key, offset),
					str_to_vec<F, S>(key, offset),
			}, val);
		}
	// TRANSFORM
	template<int Swap, class... Ts>
		void transform(std::vector<uint8_t>& key, Ts const&... ts) {
			ret_t vec{ts...};
			size_t offset{};
			for (auto& val: vec) {
				size_t sz{};
				if constexpr (!Swap) {
					sz = swap<0,1>(key, offset, val);
				} else {
					sz = swap<1,0>(key, offset, val);
				}
				offset += sz;
			}
		}
	// SESSIONS
	std::unordered_map<std::vector<uint8_t>, bool, VectorHasher> m_sessions{};
	// INTERNAL VECTOR
	std::vector<uint8_t> m_key_first{};
	std::vector<uint8_t> m_key_second{};
};
#endif
