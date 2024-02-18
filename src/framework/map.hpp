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
			m_next_id(0)
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

		void remove(uint64_t key)
		{
			m_map.erase(key);
		}

		void clear()
		{
			m_map.clear();
		}

	private:

		std::map<uint64_t, Value> m_map;
		uint64_t m_next_id;

	};
}