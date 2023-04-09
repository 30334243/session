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
					if (auto it{this->sessions.find(key_first)}; it != this->sessions.cend()) {
						ret = std::make_tuple(VectorHasher()(it->first), it->second);
					} else if (auto it{this->sessions.find(key_second)};
								  it != this->sessions.cend()) {
						ret = std::make_tuple(VectorHasher()(it->first), it->second);
					} else {
						this->sessions[key_first] = false;
						this->sessions[key_second] = true;
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
							if (auto it{this->sessions.find(key_first)};
								 it != this->sessions.cend()) {
								write(it->second);
							} else if (auto it{this->sessions.find(key_second)};
										  it != this->sessions.cend()) {
								write(it->second);
							} else {
								auto new_dst{
									create(std::to_string(VectorHasher()(key_first)))};
								this->sessions[key_first] = new_dst;
								this->sessions[key_second] = new_dst;
							}
						};
					};
				}
			// GET MAP
			std::unordered_map<std::vector<uint8_t>, Type, VectorHasher> const&
				GetMap() const {
					return this->sessions;
				}
		// PRIVATE
		private:
			// INTEGRALL TO VECTOR
			template<int F, int S, class T,
				typename = std::enable_if_t<std::is_integral_v<T>>>
					static auto integralToVec(Elm<T>& var) {
						std::vector<uint8_t> ret(sizeof(T) * 2);
						std::copy((uint8_t*)&std::get<F>(var),
									 (uint8_t*)&std::get<F>(var) + sizeof(T),
									 ret.begin());
						std::copy((uint8_t*)&std::get<S>(var),
									 (uint8_t*)&std::get<S>(var) + sizeof(T),
									 ret.begin() + sizeof(T));
						return ret;
					}
			// INTEGRALL TO VECTOR
			template<int F, int S, class T,
				typename = std::enable_if_t<std::is_integral_v<T>>>
					static constexpr auto integralToVec() {
						return [] (Elm<T>& var) {
							return integralToVec<F,S,T>(var);
						};
					}
			// STRING TO VECTOR
			template<int F, int S>
				static auto strToVec(Elm<std::string>& str) {
					std::vector<uint8_t> ret(str.first.size() + str.second.size());
					std::copy(std::get<F>(str).data(),
								 std::get<F>(str).data() + std::get<F>(str).size(),
								 ret.begin());
					std::copy(std::get<S>(str).data(),
								 std::get<S>(str).data() + std::get<S>(str).size(),
								 ret.begin() + std::get<F>(str).size());
					return ret;
				}
			// STRING TO VECTOR
			template<int F, int S>
				static auto strToVec() {
					return [] (Elm<std::string>& str) {
						return strToVec<F,S>(str);
					};
				}
			// POINTER TO VECTOR
			template<int F, int S, class T,
				typename = std::enable_if_t<std::is_pointer_v<T>>>
					static auto ptrToVec(Ptr<T>& ptr) {
						std::vector<uint8_t> ret(ptr.first.second + ptr.second.second);
						std::copy(std::get<F>(ptr).first,
									 std::get<F>(ptr).first + std::get<F>(ptr).second,
									 ret.begin());
						std::copy(std::get<S>(ptr).first,
									 std::get<S>(ptr).first + std::get<S>(ptr).second,
									 ret.begin() + std::get<F>(ptr).second);
						return ret;
					}
			// POINTER TO VECTOR
			template<int F, int S, class T,
				typename = std::enable_if_t<std::is_pointer_v<T>>>
					static auto ptrToVec() {
						return [] (Ptr<T>& ptr) {
							return ptrToVec<F,S,T>(ptr);
						};
					}
			// SWAP
			template<int F, int S>
				static auto swap(var_t& val) {
					return std::visit(overloaded{
						integralToVec<F,S,int>(),
						integralToVec<F,S,char>(),
						ptrToVec<F,S,uint8_t*>(),
						strToVec<F,S>(),
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
					std::vector<uint8_t> original{transform<0,1>(vec)};
					std::vector<uint8_t> reverse{transform<1,0>(vec)};
					return std::make_tuple(original, reverse);
				}
			// SESSIONS
			std::unordered_map<std::vector<uint8_t>, Type, VectorHasher> sessions{};
	};
}
#endif
