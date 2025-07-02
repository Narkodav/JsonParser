#pragma once
#include <cstddef>
#include <array>

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86)
#include <immintrin.h>
#define HAS_SSE2
#if defined(__AVX2__) || (defined(_MSC_VER) && defined(__AVX2__))
#define HAS_AVX2
#endif
#endif

#ifdef _MSC_VER
#include <intrin.h>
#define CTZ32(x) _tzcnt_u32(x)
#define CTZ16(x) _tzcnt_u16(x)
#elif defined(__GNUC__) || defined(__clang__)
#define CTZ32(x) __builtin_ctz(x)
#define CTZ16(x) __builtin_ctz(x)
#else
namespace
{
    inline int CTZ16_FUNC(uint16_t x) {
        int count = 0;
        while ((x & 1) == 0 && x != 0) { x >>= 1; count++; }
        return count;
    }
    inline int CTZ32_FUNC(uint32_t x) {
        int count = 0;
        while ((x & 1) == 0 && x != 0) { x >>= 1; count++; }
        return count;
    }
}
#define CTZ32(x) CTZ32_FUNC(x)
#define CTZ16(x) CTZ16_FUNC(x)
#endif

#ifdef HAS_AVX2
template<typename Container>
inline size_t skipWhitespacesSIMD32(const Container& input, size_t i) {
    const size_t size = input.size();

    static const __m256i ws_space = _mm256_set1_epi8(' ');
    static const __m256i ws_tab = _mm256_set1_epi8('\t');
    static const __m256i ws_cr = _mm256_set1_epi8('\r');
    static const __m256i ws_lf = _mm256_set1_epi8('\n');
    __m256i chunk;
    uint32_t mask;

    while (i + 32 <= size) {
        chunk = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&input[i]));

        mask = ~_mm256_movemask_epi8(_mm256_or_si256(
            _mm256_or_si256(_mm256_cmpeq_epi8(chunk, ws_space), _mm256_cmpeq_epi8(chunk, ws_tab)),
            _mm256_or_si256(_mm256_cmpeq_epi8(chunk, ws_cr), _mm256_cmpeq_epi8(chunk, ws_lf))));

        if (mask != 0) {
            return i + CTZ32(mask);
        }

        i += 32;
    }

    while (i < size && (input[i] == ' ' || input[i] == '\t' || input[i] == '\r' || input[i] == '\n')) {
        ++i;
    }

    return i;
};
#endif

#ifdef HAS_SSE2
template<typename Container>
inline size_t skipWhitespacesSIMD16(const Container& input, size_t i) {
    const size_t size = input.size();

    static const __m128i ws_space = _mm_set1_epi8(' ');
    static const __m128i ws_tab = _mm_set1_epi8('\t');
    static const __m128i ws_cr = _mm_set1_epi8('\r');
    static const __m128i ws_lf = _mm_set1_epi8('\n');
    __m128i chunk;
    uint16_t mask;

    while (i + 16 <= size) {
        chunk = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&input[i]));

        mask = ~_mm_movemask_epi8(_mm_or_si128(
            _mm_or_si128(_mm_cmpeq_epi8(chunk, ws_space), _mm_cmpeq_epi8(chunk, ws_tab)),
            _mm_or_si128(_mm_cmpeq_epi8(chunk, ws_cr), _mm_cmpeq_epi8(chunk, ws_lf))));

        if (mask != 0) {
            return i + CTZ16(mask);
        }

        i += 16;
    }
    while (i < size && (input[i] == ' ' || input[i] == '\t' || input[i] == '\r' || input[i] == '\n')) {
        ++i;
    }

    return i;
};
#endif

template<typename Container>
inline size_t skipWhitespacesScalar(const Container& input, size_t i) {
    const size_t size = input.size();
    while (i < size && (input[i] == ' ' || input[i] == '\t' || input[i] == '\r' || input[i] == '\n')) {
        ++i;
    }
    return i;
}

template<typename Container>
inline size_t skipWhitespaces(const Container& input, size_t i) {
#ifdef HAS_AVX2
    return skipWhitespacesSIMD32(input, i);
#elif (#defined HAS_SSE2)
    return skipWhitespacesSIMD16(input, i);
#else 
    return skipWhitespacesScalar(input, i);
#endif
}