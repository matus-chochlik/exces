/**
 *  @file exces/storage.hpp
 *  @brief Implements component storage
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef EXCES_STORAGE_1212101457_HPP
#define EXCES_STORAGE_1212101457_HPP

#include <exces/group.hpp>
#include <exces/metaprog.hpp>

#include <cassert>

namespace exces {

// Interface for component storage vectors of component_storage
template <typename Component>
struct component_storage_vector
{
	typedef std::size_t component_key;

	virtual ~component_storage_vector(void){ }

	virtual Component& at(component_key) = 0;

	virtual void reserve(std::size_t) = 0;

	virtual component_key store(Component&&) = 0;

	virtual component_key replace(component_key, Component&&) = 0;

	virtual component_key copy(component_key) = 0;

	virtual void add_ref(component_key) = 0;

	virtual bool release(component_key) = 0;
};

template <typename Group>
struct component_storage_init;

template <typename Group>
struct component_storage_cleanup;

template <typename Group = default_group>
class component_storage
{
private:
	struct _make_csv_ptr
	{
		template <typename C>
		struct apply
		{
			typedef component_storage_vector<
				typename std::remove_reference<C>::type
			>* type;
		};
	};

	typedef typename mp::as_tuple<
		typename mp::transform<
			components< Group >,
			_make_csv_ptr
		>::type
	>::type _store_type;

	_store_type _store;

	friend struct component_storage_init<Group>;
	friend struct component_storage_cleanup<Group>;

	template <typename Component>
	component_storage_vector<Component>& _store_of(void)
	{
		component_storage_vector<Component>* _pcsv = mp::get<
			component_id<Component, Group>::value>(_store);
		assert(_pcsv != nullptr);
		return *_pcsv;
	}
public:
	component_storage(const component_storage&) = delete;
	component_storage(void);
	~component_storage(void);

	/// The component key type
	typedef std::size_t component_key;

	/// Return a special NULL component key value
	static component_key null_key(void)
	{
		return component_key(-1);
	}

	/// Reserves space for n instances of Component
	template <typename Component>
	void reserve(std::size_t n)
	{
		_store_of<Component>()
			.reserve(n);
	}

	/// Access the specified Component type by its key
	template <typename Component>
	Component& access(component_key key)
	{
		return _store_of<Component>()
			.at(key);
	}

	/// Stores the specified component and returns
	template <typename Component>
	component_key store(Component&& component)
	{
		return _store_of<Component>()
			.store(std::move(component));
	}

	/// Replaces the value of Component at the specified key
	template <typename Component>
	component_key replace(component_key key, Component&& component)
	{
		return _store_of<Component>()
			.replace(key,std::move(component));
	}

	/// Copies the component at the specified key returns the new key
	template <typename Component>
	component_key copy(component_key key)
	{
		return _store_of<Component>()
			.copy(key);
	}

	/// Adds reference to the component at the specified key
	template <typename Component>
	void add_ref(component_key key)
	{
		_store_of<Component>()
			.add_ref(key);
	}

	/// Releases (removes reference to) the component at the specified key
	template <typename Component>
	bool release(component_key key)
	{
		return _store_of<Component>()
			.release(key);
	}

	template <typename Component>
	void mark_write(component_key key)
	{
		// TODO
	}
};

} // namespace exces

#endif //include guard

