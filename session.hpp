#ifndef SESSION_HPP
#define SESSION_HPP

#include <unordered_map>
#include <utility>
#include <typeinfo>
#include <functional>
#include <memory>
#include <fstream>
#include "session_interface.hpp"

namespace Session {
// SESSION
template<class Type = bool>
	class Session {
		// PUBLIC
		public:
			// GET CONVERSATION ID
			template<class... Tail>
				auto GetConversationId(Tail&&... tail) {
					std::tuple<size_t, bool> ret{};
					auto [key_first, key_second] = transform(std::forward<Tail>(tail)...);
					if (auto it{m_sessions.find(key_first)}; it != m_sessions.cend()) {
						ret = std::make_tuple(VectorHasher()(it->first), it->second);
					} else if (auto it{m_sessions.find(key_second)};
								  it != m_sessions.cend()) {
						ret = std::make_tuple(VectorHasher()(it->first), it->second);
					} else {
						m_sessions[key_first] = false;
						m_sessions[key_second] = true;
						ret = std::make_tuple(VectorHasher()(key_first), false);
					}
					return ret;
				};
			// WRITE TO FILE
			template<class Create>
				auto WriteToFile(Create create) {
					return [this, create] (auto write) {
						return [this, create, write] (auto&&... ts) {
							std::tuple<size_t, bool> ret{};
							auto [key_first, key_second] =
								transform(std::forward<decltype(ts)>(ts)...);
							if (auto it{m_sessions.find(key_first)};
								 it != m_sessions.cend()) {
								write(it->second);
							} else if (auto it{m_sessions.find(key_second)};
										  it != m_sessions.cend()) {
								write(it->second);
							} else {
								auto new_dst{
									create(std::to_string(VectorHasher()(key_first)))};
								m_sessions[key_first] = new_dst;
								m_sessions[key_second] = new_dst;
							}
						};
					};
				}
			// GET MAP
			std::unordered_map<std::vector<uint8_t>, Type, VectorHasher> const&
				GetMap() const {
					return m_sessions;
				}
		// PRIVATE
		private:
			// INTEGRALL TO VECTOR
			template<int IdxF, int IdxS, class T,
				typename = std::enable_if_t<std::is_integral_v<T>>>
					static auto integral_to_vec(Elm<T>& var) {
						std::vector<uint8_t> ret(sizeof(T) * 2);
						std::copy((uint8_t*)&std::get<IdxF>(var),
									 (uint8_t*)&std::get<IdxF>(var) + sizeof(T),
									 ret.begin());
						std::copy((uint8_t*)&std::get<IdxS>(var),
									 (uint8_t*)&std::get<IdxS>(var) + sizeof(T),
									 ret.begin() + sizeof(T));
						return ret;
					}
			// INTEGRALL TO VECTOR
			template<int IdxF, int IdxS, class T,
				typename = std::enable_if_t<std::is_integral_v<T>>>
					static constexpr auto integral_to_vec() {
						return [] (Elm<T>& var) {
							return integral_to_vec<IdxF, IdxS, T>(var);
						};
					}
			// STRING TO VECTOR
			template<int IdxF, int IdxS>
				static auto str_to_vec(Elm<std::string>& str) {
					std::vector<uint8_t> ret(str.first.size() + str.second.size());
					std::copy(std::get<IdxF>(str).data(),
								 std::get<IdxF>(str).data() + std::get<IdxF>(str).size(),
								 ret.begin());
					std::copy(std::get<IdxS>(str).data(),
								 std::get<IdxS>(str).data() + std::get<IdxS>(str).size(),
								 ret.begin() + std::get<IdxF>(str).size());
					return ret;
				}
			// STRING TO VECTOR
			template<int IdxF, int IdxS>
				static auto str_to_vec() {
					return [] (Elm<std::string>& str) {
						return str_to_vec<IdxF, IdxS>(str);
					};
				}
			// POINTER TO VECTOR
			template<int IdxF, int IdxS, class T,
				typename = std::enable_if_t<std::is_pointer_v<T>>>
					static auto ptr_to_vec(Ptr<T>& ptr) {
						std::vector<uint8_t> ret(ptr.first.second + ptr.second.second);
						std::copy(std::get<IdxF>(ptr).first,
									 std::get<IdxF>(ptr).first + std::get<IdxF>(ptr).second,
									 ret.begin());
						std::copy(std::get<IdxS>(ptr).first,
									 std::get<IdxS>(ptr).first + std::get<IdxS>(ptr).second,
									 ret.begin() + std::get<IdxF>(ptr).second);
						return ret;
					}
			// POINTER TO VECTOR
			template<int IdxF, int IdxS, class T,
				typename = std::enable_if_t<std::is_pointer_v<T>>>
					static auto ptr_to_vec() {
						return [] (Ptr<T>& ptr) {
							return ptr_to_vec<IdxF, IdxS, T>(ptr);
						};
					}
			// SWAP
			template<int F, int S>
				static auto swap(var_t& val) {
					return std::visit(overloaded{
						integral_to_vec<F, S, int>(),
						integral_to_vec<F, S, char>(),
						ptr_to_vec<F, S, uint8_t*>(),
						str_to_vec<F, S>(),
					}, val);
				}
			// TRNASFORM
			template<int F, int S>
				auto transform(std::vector<var_t>& vec) {
					std::vector<uint8_t> ret{};
					for (auto& val: vec) {
						auto const vec{swap<F, S>(val)};
						ret.insert(ret.cend(), vec.cbegin(), vec.cend());
					}
					return ret;
				}
			// TRANSFORM
			template<class... Ts>
				auto transform(Ts&&... ts) {
					ret_t vec{std::forward<Ts>(ts)...};
					return std::make_tuple(transform<0,1>(vec), transform<1,0>(vec));
				}
			// SESSIONS
			std::unordered_map<std::vector<uint8_t>, Type, VectorHasher> m_sessions{};
	};
}
#endif
