#pragma once

namespace Json
{
    class Value;

    namespace Detail {
        template <size_t N>
        struct FixedString {
            char data[N];

            constexpr FixedString(const char (&str)[N]) {
                for (size_t i = 0; i < N; ++i)
                    data[i] = str[i];
            }

            constexpr operator std::string_view() const {
                return {data, N - 1};
            }
        };
    }

    struct PrimitiveSchema {
        using PrimitiveTrait = void;
    };

    template<Detail::FixedString Name, auto Pointer, typename Schema>
    struct MemberSchema
    {
        static constexpr auto s_name = Name;
        static constexpr auto s_pointer = Pointer;

        using ElementSchema = Schema;
    };

    template<typename... MembersT>
    struct StructSchema {
        using Members = std::tuple<MembersT...>;

        static constexpr std::size_t size = sizeof...(MembersT);

        template<typename Object, typename Fn>
        static void visit(Object& object, Fn&& fn) {
            (fn.template operator()<MembersT>(object.*MembersT::s_pointer), ...);
        }
    };

    template<typename ElementSchemaT>
    struct ArraySchema {
        using ElementSchema = ElementSchemaT;
    };

    template<typename T>
    concept StructSchemaConcept = requires {
        typename T::Members;
    };

    template<typename T>
    concept ArraySchemaConcept = requires {
        typename T::ElementSchema;
    };

    template<typename T>
    concept PrimitiveSchemaConcept = requires {
        typename T::PrimitiveTrait;
    };

    template<typename T>
    struct ContainerTraits;

    template<typename T, typename Alloc>
    struct ContainerTraits<std::vector<T, Alloc>> {
        using ArrayType = std::vector<T, Alloc>;

        static void resize(ArrayType& v, size_t size) {
            v.resize(size);
        }

        static T& at(ArrayType& v, size_t index) {
            return v[index];
        }
    };

    template<typename T, std::size_t N>
    struct ContainerTraits<std::array<T, N>> {
        using ArrayType = std::array<T, N>;

        static void resize(ArrayType& a, size_t size) {
            // In fixed size containers this is just an assert
            if(a.size() != size) throw std::runtime_error(
                "Json Value array size doesn't match target array size");
        }

        static T& at(ArrayType& a, size_t index) {
            return a[index];
        }
    };
}