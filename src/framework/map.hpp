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

		typedef typename Value::ID ID;
	
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

		ID insert(Value && value)
		{
			m_map.insert({m_next_id, value});
			return m_next_id++;
		}

		Value get(ID key)
		{
			return m_map.at(key);
		}

		void remove(ID key)
		{
			m_map.erase(key);
		}

		void clear()
		{
			m_map.clear();
		}

	private:

		std::map<ID, Value> m_map;
		ID m_next_id;

	};
}