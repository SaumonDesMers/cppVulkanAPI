#pragma once

#include "defines.hpp"

#include <vulkan/vulkan.h>

#include <stdexcept>
#include <map>

namespace LIB_NAMESPACE
{
	template<typename Value>
	class Map
	{
	
	public:

		Map():
			m_map(),
			m_next_id(1)
		{
		}

		~Map()
		{
		}

		Map(Map& map) = delete;
		Map(Map&& map) = delete;
		Map & operator=(Map& map) = delete;
		Map & operator=(Map&& map) = delete;

		uint64_t insert(Value && value)
		{
			m_map.insert(std::make_pair(m_next_id, std::move(value)));
			return m_next_id++;
		}

		Value & get(uint64_t key) { return m_map.at(key); }
		const Value & get(uint64_t key) const { return m_map.at(key); }

		void remove(uint64_t key) { m_map.erase(key); }

		void replace(uint64_t key, Value && value)
		{
			m_map.erase(key);
			m_map.insert(std::make_pair(key, std::move(value)));
		}

		auto begin() { return m_map.begin(); }
		auto end() { return m_map.end(); }

		auto cbegin() const { return m_map.cbegin(); }
		auto cend() const { return m_map.cend(); }

		static const uint64_t no_id = 0;

	private:

		std::map<uint64_t, Value> m_map;
		uint64_t m_next_id;

	};
}