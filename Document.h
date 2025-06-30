#pragma once
#include "ValueTypes/Value.h"
#include <memory>
#include <vector>

namespace Json
{
	class Parser;

	class Document
	{
		friend class Parser;
	private:
		std::vector<std::unique_ptr<Value>> m_roots;  // Multiple JSON documents (JSON Lines)
		
	public:
		Document() = default;
		Document(const Document&) = default;
		Document& operator=(const Document&) = default;
		Document(Document&&) = default;
		Document& operator=(Document&&) = default;

		bool isEmpty() const { return m_roots.empty(); }

		const auto& roots() const { return m_roots; };
	};
}
