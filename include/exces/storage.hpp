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
#include <cassert>

namespace exces {

template <typename Group = default_group>
class component_storage
{
private:
	template <typename Component>
	struct _component_entry
	{
		// negative reference count (if < 0)
		// or the next free component entry
		// in the vector
		int _neg_rc_or_nf;
		Component _component;

		_component_entry(Component&& component)
		 : _neg_rc_or_nf(0)
		 , _component(std::move(component))
		{ }
	};

	template <typename Component>
	class _component_vector
	 : public std::vector<_component_entry<Component> >
	{
	private:
		int _next_free;
	public:
		typedef std::size_t key_t;

		_component_vector(void)
		 : _next_free(-1)
		{ }

		key_t store(Component&& component)
		{
			key_t result;
			if(_next_free >= 0)
			{
				result = key_t(_next_free);
				this->at(result)._component =
					std::move(component);
				_next_free = this->at(result)._neg_rc_or_nf;
				this->at(result)._neg_rc_or_nf = -1;
			}
			else
			{
				result = this->size();
				this->push_back(std::move(component));
				this->back()._neg_rc_or_nf = -1;
			}
			return result;
		}

		void add_ref(key_t key)
		{
			assert(this->at(key)._neg_rc_or_nf < 0);
			--this->at(key)._neg_rc_or_nf;
		}

		void release(key_t key)
		{
			assert(this->at(key)._neg_rc_or_nf < 0);
			if(++this->at(key)._neg_rc_or_nf == 0)
			{
				this->at(key)._neg_rc_or_nf = _next_free;
				_next_free = int(key);

			}
		}
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
		return _store_of<Component>().store(std::move(component));
	}

	template <typename Component>
	key_t copy(key_t key)
	{
		return store<Component>(Component(access<Component>(key)));
	}

	template <typename Component>
	void add_ref(key_t key)
	{
		_store_of<Component>().add_ref(key);
	}

	template <typename Component>
	void release(key_t key)
	{
		_store_of<Component>().release(key);
	}
};

} // namespace exces

#endif //include guard

