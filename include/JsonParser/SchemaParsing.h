#pragma once
#include "JsonParser/Value.h"

#include <vector>
#include <array>
#include <unordered_set>

namespace Json
{
    template<typename Schema, typename T>
    concept ReadSchemaFor = requires(const Json::Value& json, T& value) {
        { Schema::read(json, value) } -> std::same_as<void>;
    };

    template<typename Schema, typename T>
    concept WriteSchemaFor = requires(Json::Value& json, const T& value) {
        { Schema::write(json, value) } -> std::same_as<void>;
    };

    struct PrimitiveSchema {
        template<typename T>
        static void read(const Value& json, T& val) {
			switch(json.getType()) {
				case Value::Type::String:
					if constexpr (std::convertible_to<Value::String, T>) {
						val = json.asString();
					}
					else throw std::runtime_error("Json is not convertible to String");
					break;
				case Value::Type::Bool:
					if constexpr (std::convertible_to<bool, T>) {
						val = json.asBool();
					}
					else throw std::runtime_error("Json is not convertible to Bool");
					break;
				case Value::Type::Integer:
					if constexpr (std::convertible_to<int64_t, T>) {
						val = json.asInteger();
					}
					else throw std::runtime_error("Json is not convertible to Integer");
					break;
				case Value::Type::Number:
					if constexpr (std::convertible_to<double, T>) {
						val = json.asNumber();
					}
					else throw std::runtime_error("Json is not convertible to Number");
					break;
				default:
					throw std::runtime_error("Json Value has wrong type");
					break;
			}
        }

        template<typename T>
        static void write(Value& json, const T& val) {
            json = val;
        }
    };

    template<Detail::FixedString Name, auto Pointer, typename Schema>
    struct StructMemberSchema {
        static constexpr auto s_name = Name;
        static constexpr auto s_pointer = Pointer;

		template<typename T>
		static void read(const Value& json, T& val) requires ReadSchemaFor<Schema, T> {
            Schema::read(json, val);
        }

		template<typename T>
		static void write(Value& json, const T& val) requires WriteSchemaFor<Schema, T> {
            Schema::write(json, val);
        }
    };

    template<typename... MembersT>
    struct StructSchema {
        using Members = std::tuple<MembersT...>;

        static constexpr std::size_t size = sizeof...(MembersT);

        template<typename Object, typename Fn>
        static void visit(Object& object, Fn&& fn) {
            (fn.template operator()<MembersT>(object.*MembersT::s_pointer), ...);
        }

		template<typename T>
		static void read(const Value& json, T& val)  {
			if(!json.isObject()) throw std::runtime_error("Json Value has wrong type");
			const auto& object = json.asObject();

			visit(val, [&]<typename Member>(auto& field) {
				auto it = object.find(Member::s_name);
				if(it == object.end()) 
					throw std::runtime_error(std::string("No value named ") + Member::s_name.data + " in Json");
				Member::read(it->second, field);
			});
		}

		template<typename T>
		static void write(Value& json, const T& val) {
            json = Value::object();
            auto& object = json.asObject();
			visit(val, [&]<typename Member>(auto& field) {
                Value v;
                Member::write(v, field);
                object.emplace(std::make_pair(Member::s_name, std::move(v)));
			});
        }
    };

    template<typename T>
    struct ContainerTraits;

    template<typename T, typename Alloc>
    struct ContainerTraits<std::vector<T, Alloc>> {
        using ArrayType = std::vector<T, Alloc>;
        using ValueType = T;

        static void resize(ArrayType& a, size_t size) {
            a.resize(size);
        }

        template<typename U>
        static void add(ArrayType& a, U&& v, size_t index) {
            a[index] = std::forward<U>(v);
        }

        static size_t size(const ArrayType& a) { return a.size(); }
    };

    template<typename T, std::size_t N>
    struct ContainerTraits<std::array<T, N>> {
        using ArrayType = std::array<T, N>;
        using ValueType = T;

        static void resize(ArrayType& a, size_t size) {
            // In fixed size containers this is just an assert
            if(a.size() != size) throw std::runtime_error(
                "Json Value array size doesn't match target array size");
        }

        template<typename U>
        static void add(ArrayType& a, U&& v, size_t index) {
            a[index] = std::forward<U>(v);
        }

        static size_t size(const ArrayType& a) { return a.size(); }
    };

    template<typename T, typename Hash, typename Comp, typename Alloc>
    struct ContainerTraits<std::unordered_set<T, Hash, Comp, Alloc>> {
        using ArrayType = std::unordered_set<T, Hash, Comp, Alloc>;
        using ValueType = T;

        static void resize(ArrayType& a, size_t size) {
            a.reserve(size);
        }

        template<typename U>
        static void add(ArrayType& a, U&& v, size_t) {
            a.emplace(std::forward<U>(v));
        }

        static size_t size(const ArrayType& a) { return a.size(); }
    };

    template<typename Schema>
    struct ContainerSchema {
		template<typename T>
		static void read(const Value& json, T& val) requires ReadSchemaFor<Schema, T>  {
			if(!json.isArray()) throw std::runtime_error("Json Value has wrong type");
			const auto& arr = json.asArray();

			ContainerTraits<T>::resize(val, arr.size());
			for(size_t i = 0; i < arr.size(); ++i) {
                typename ContainerTraits<T>::ValueType v;
                Schema::read(arr[i], v);
                ContainerTraits<T>::add(val, std::move(v), i);
			}
        }

		template<typename T>
		static void write(Value& json, const T& val) requires WriteSchemaFor<Schema, T>  {
            json = Value::array();
            auto& arr = json.asArray();
            arr.resize(ContainerTraits<T>::size(val));
            size_t i = 0;
			for(const auto& field : val) {
                Schema::write(arr[i], field); ++i;
			}
        }
    };
}