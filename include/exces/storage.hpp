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
#include <exces/threads.hpp>
#include <exces/metaprog.hpp>

#include <cassert>

namespace exces {

struct component_access_read_only { };
struct component_access_read_write { };

template <typename Component>
inline typename mp::if_c<
	std::is_reference<Component>::value &&
	not(std::is_const<Component>::value),
	component_access_read_write,
	component_access_read_only
>::type get_component_access(void)
{
	return typename mp::if_c<
		std::is_reference<Component>::value &&
		not(std::is_const<Component>::value),
		component_access_read_write,
		component_access_read_only
	>::type();
}
 
// Interface for component storage vectors of component_storage
template <typename Group, typename Component>
struct component_storage_vector : lock_intf
{
	typedef component_locking<Group, Component> locking;
	typedef typename locking::shared_lock shared_lock;
	typedef typename locking::unique_lock unique_lock;

	typedef std::size_t component_key;

	virtual ~component_storage_vector(void){ }

	virtual shared_lock read_lock(void) = 0;
	virtual unique_lock write_lock(void) = 0;

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
				Group,
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
	component_storage_vector<Group, Component>& _store_of(void)
	{
		component_storage_vector<Group, Component>* _pcsv = mp::get<
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

	/// Returns an unlocked shared lock postponing cleanup of components
	template <typename Component>
	poly_lock lifetime_lock(void)
	{
		return poly_lock(&_store_of<Component>());
	}

	/// Returns an unlocked shared lock for read-only operations on Component
	template <typename Component>
	typename component_locking<Group, Component>::shared_lock
	read_lock(component_key = null_key())
	{
		return _store_of<Component>()
			.read_lock();
	}

	/// Same as read_lock(key)
	template <typename Component>
	typename component_locking<Group, Component>::shared_lock
	access_lock(component_access_read_only, component_key = null_key())
	{
		return _store_of<Component>()
			.read_lock();
	}

	/// Returns an unlocked unique lock for read/write operations on Component
	template <typename Component>
	typename component_locking<Group, Component>::unique_lock
	write_lock(component_key = null_key())
	{
		return _store_of<Component>()
			.write_lock();
	}

	/// Same as write lock
	template <typename Component>
	typename component_locking<Group, Component>::unique_lock
	access_lock(component_access_read_write, component_key = null_key())
	{
		return _store_of<Component>()
			.write_lock();
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

