/**
 *  @file exces/storage.hpp
 *  @brief Implements component storage
 *
 *  Copyright 2012-2013 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef EXCES_STORAGE_1212101457_HPP
#define EXCES_STORAGE_1212101457_HPP

#include <exces/group.hpp>
#include <exces/metaprog.hpp>

#include <vector>

namespace exces {

template <typename Group = default_group>
class component_storage
{
private:
	template <typename Component>
	struct _component_entry
	{
		Component _component;

		_component_entry(Component&& component)
		 : _component(std::move(component))
		{ }
	};

	template <typename Component>
	struct _component_vector
	 : public std::vector<_component_entry<Component> >
	{
	};

	struct _add_component_vector_mf
	{
		template <typename T>
		struct apply
		{
			typedef _component_vector<
				typename std::remove_reference<T>::type
			> type;
		};
	};

	typedef typename mp::as_tuple<
		typename mp::transform<
			components< Group >,
			_add_component_vector_mf
		>::type
	>::type _store_type;

	_store_type _store;

	template <typename Component>
	_component_vector<Component>& _store_of(void)
	{
		return mp::get<component_id<Component>::value>(_store);
	}
public:
	typedef std::size_t key_t;

	static key_t null_key(void)
	{
		return key_t(-1);
	}

	template <typename Component>
	Component& access(key_t key)
	{
		return _store_of<Component>()[key]._component;
	}

	template <typename Component>
	key_t store(Component&& component)
	{
		_component_vector<Component>& v = _store_of<Component>();
		key_t result = v.size();
		v.push_back(std::move(component));
		add_ref<Component>(result);
		return result;
	}

	template <typename Component>
	key_t copy(key_t key)
	{
		return store<Component>(Component(access<Component>(key)));
	}

	template <typename Component>
	void add_ref(key_t key)
	{
		// TODO:
	}

	template <typename Component>
	void release(key_t key)
	{
		// TODO:
	}
};

} // namespace exces

#endif //include guard

