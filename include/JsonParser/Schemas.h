#pragma once
#include "JsonParser/Value.h"

#include <vector>
#include <array>
#include <unordered_set>

namespace Json
{
    struct PrimitiveSchema {
        template<typename T>
        static T read(const Value& json) {
			switch(json.getType()) {
				case Value::Type::String:
					if constexpr (std::convertible_to<Value::String, T>) {
						return json.asString();
					}
					else throw std::runtime_error("Json is not convertible to String");
					break;
				case Value::Type::Bool:
					if constexpr (std::convertible_to<bool, T>) {
						return json.asBool();
					}
					else throw std::runtime_error("Json is not convertible to Bool");
					break;
				case Value::Type::Integer:
					if constexpr (std::convertible_to<int64_t, T>) {
						return json.asInteger();
					}
					else throw std::runtime_error("Json is not convertible to Integer");
					break;
				case Value::Type::Number:
					if constexpr (std::convertible_to<double, T>) {
						return json.asNumber();
					}
					else throw std::runtime_error("Json is not convertible to Number");
					break;
				default:
					throw std::runtime_error("Json Value has wrong type");
					break;
			}
        }

        template<typename T>
        static Value write(const T& val) {
            return val;
        }
    };

    template<Detail::FixedString Name, typename Schema>
    struct NamedSchema {
        static constexpr std::string_view s_name = Name.data;

        template<typename T>
		static auto read(const Value& json) {
            return Schema::template read<T>(json);
        }

        template<typename T>
		static Value write(const T& val) {
            return Schema::write(val);
        }
    };

    template<auto Pointer, Detail::FixedString Name, typename Schema>
    struct StructMemberSchema : NamedSchema<Name, Schema> {
        static constexpr auto s_pointer = Pointer;
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
		static T read(const Value& json)  {
			if(!json.isObject()) throw std::runtime_error("Json Value has wrong type");
			const auto& object = json.asObject();
            T result;
			visit(result, [&]<typename Member>(auto& field) {
				auto it = object.find(Member::s_name);
				if(it == object.end()) 
					throw std::runtime_error(std::string("No value named ") + Member::s_name.data() + " in Json");
                field = Member::template read<std::remove_cvref_t<decltype(field)>>(it->second);
			});
            return result;
		}

		template<typename T>
		static Value write(const T& val) {
            Value json = Value::object();
            auto& object = json.asObject();
			visit(val, [&]<typename Member>(auto& field) {
                Value v = Member::write(field);
                object.emplace(std::make_pair(Member::s_name, std::move(v)));
			});
            return json;
        }
    };

    template<typename T>
    struct ContainerTraits {};

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
		static T read(const Value& json) {
            using ValueType = typename ContainerTraits<T>::ValueType;
			if(!json.isArray()) throw std::runtime_error("Json Value has wrong type");
			const auto& arr = json.asArray();
            T result;
			ContainerTraits<T>::resize(result, arr.size());
			for(size_t i = 0; i < arr.size(); ++i) {
                ContainerTraits<T>::add(result, Schema::template read<ValueType>(arr[i]), i);
			}
            return result;
        }

		template<typename T>
		static Value write(const T& val) {
            Value json = Value::array();
            auto& arr = json.asArray();
            arr.resize(ContainerTraits<T>::size(val));
            size_t i = 0;
			for(const auto& field : val) {
                arr[i] = Schema::write(field); ++i;
			}
            return json;
        }
    };

    template<typename T, Detail::FixedString Name, typename Schema>
    struct TypedSchema : NamedSchema<Name, Schema> {
        using ValueType = T;
    };

    template<typename... SchemasT>
    struct ConstructorSchema {
        using Values = std::tuple<typename SchemasT::ValueType...>;
        static constexpr std::size_t s_size = sizeof...(SchemasT);

        template<typename T>
        static T read(const Value& json) {
            switch(json.getType()) {
                case Value::Type::Object: {
                        const auto& object = json.asObject();
                        return T(
                            SchemasT::template read<typename SchemasT::ValueType>(
                                object.at(SchemasT::s_name.data())
                            ) ...
                        );
                    }
                    break;
                case Value::Type::Array: {
                        size_t i = 0;
                        const auto& arr = json.asArray();
                        if(arr.size() != s_size) throw std::runtime_error(
                            "Json Value array size doesn't match target array size");
                        return T(
                            SchemasT::template read<typename SchemasT::ValueType>(
                                arr[i++]
                            )...
                        );
                    }
                    break;
                default:
                    throw std::runtime_error("Json Value has wrong type");
            }
        }
    };

    template<Detail::FixedString TypeField, typename... SchemasT>
    struct PolymorphicSchema {
        using Schemas = std::tuple<SchemasT...>;
        static constexpr std::size_t s_size = sizeof...(SchemasT);

		template<typename T>
		static T read(const Value& json)  {
			if(!json.isObject()) throw std::runtime_error("Json Value has wrong type");
			const auto& object = json.asObject();
			auto type = object.find(TypeField.data);
			if(type == object.end()) 
                throw std::runtime_error(std::string("No type field named ") + TypeField.data + " in Json");
			if(!type->second.isString()) 
                throw std::runtime_error(std::string("Type name filed must be a string"));
            const auto& typeName = type->second.asString();

            T result;
            (
                (std::strcmp(typeName.data(), SchemasT::s_name.data()) == 0 &&
                (result = std::make_unique<typename SchemasT::ValueType>(
                    SchemasT::template read<typename SchemasT::ValueType>(json)), true)) || ...
                || (throw std::runtime_error("Unknown object type: " + type->second.asString()), true)
            );
            return result;
		}
    };

    template<auto Val, Detail::FixedString Name>
    struct EnumValSchema {
        static constexpr std::string_view s_name = Name.data;
        static constexpr auto s_value = Val;
        using ValueType = decltype(s_value);
    };

    template<typename... SchemasT>
    struct EnumSchema {
        template<typename T>
        static T read(const Value& json) {
			switch(json.getType()) {
                case Value::Type::String: {
                        T result;
                        (
                            (std::strcmp(json.asString().data(), SchemasT::s_name.data()) == 0 &&
                            (result = SchemasT::s_value, true)) || ... 
                            || (throw std::runtime_error("Unknown enum value"), true)
                        );
                        return result;
                    } break;
				case Value::Type::Integer:
                    return static_cast<T>(json.asInteger());
					break;
				default:
					throw std::runtime_error("Json Value has wrong type");
					break;
			}
        }
    };
}