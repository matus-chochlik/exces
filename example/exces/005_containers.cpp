/**
 *  @example excess/005_containers.cpp
 *  @brief Introductory example showing the basic usage of exces.
 *
 *  Copyright 2012-2013 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#include <exces/exces.hpp>

#include <iostream>

struct named
{
	std::string name;

	named(std::string n)
	 : name(n)
	{ }
};
EXCES_REG_COMPONENT(named)

struct container
{
	float max_size;
	float max_weight;

	float cur_size;
	float cur_weight;

	float size_coef;
	float weight_coef;

	bool growable;

	container(float max_s, float max_w, bool g, float sc = 1.0f, float wc = 1.0f)
	 : max_size(max_s)
	 , max_weight(max_w)
	 , cur_size(0.0f)
	 , cur_weight(0.0f)
	 , size_coef(sc)
	 , weight_coef(wc)
	 , growable(g)
	{ }

};
EXCES_REG_COMPONENT(container)

struct physical
{
	float size;
	float weight;

	physical(float s, float w)
	 : size(s)
	 , weight(w)
	{ }
};
EXCES_REG_COMPONENT(physical)

struct contained
{
	exces::entity container;

	contained(exces::entity c)
	 : container(c)
	{ }
};
EXCES_REG_COMPONENT(contained)

class physical_system
{
private:
	typedef exces::manager<> manager;
	typedef typename manager::entity_key entity_key;

	manager& _manager;
public:
	physical_system(manager& m)
	 : _manager(m)
	{ }

	float get_weight(entity_key k)
	{
		auto e = _manager.get_entity(k);
		auto ep = _manager.ref<physical>(k);
		float w = ep->weight;

		auto ec = _manager.ref<container>(k);
		if(ec)
		{
			float wc = ec->weight_coef;
			
			for(auto r = _manager.select_with<contained>(); !r.empty(); r.next())
			{
				if(r.front_component<contained>().read().container == e)
					w += wc * get_weight(r.front());
			}
		}
		return w;
	}

	float get_weight(exces::entity e)
	{
		return get_weight(_manager.get_key(e));
	}

	float get_size(entity_key k)
	{
		auto e = _manager.get_entity(k);
		auto ep = _manager.ref<physical>(k);
		float s = ep->size;

		auto ec = _manager.ref<container>(k);
		if(ec)
		{
			if(ec->growable)
			{
				float sc = ec->size_coef;
				
				for(auto r = _manager.select_with<contained>(); !r.empty(); r.next())
				{
					if(r.front_component<contained>().read().container == e)
						s += sc * get_size(r.front());
				}
			}
		}
		return s;
	}
};

class container_system
{
private:
	typedef exces::manager<> manager;
	typedef typename manager::entity_key entity_key;

	manager& _manager;
public:
	container_system(manager& m)
	 : _manager(m)
	{ }


	void put_into(entity_key cont, entity_key item)
	{
		auto cc = _manager.ref<container>(cont);
		auto ip = _manager.ref<physical>(item);

		assert(cc);

		if(ip)
		{
			cc->cur_size += ip->size;
			cc->cur_weight += ip->weight;
		}
		_manager.add(item, contained(_manager.get_entity(cont)));
	}

	void put_into(exces::entity cont, exces::entity item)
	{
		put_into(_manager.get_key(cont), _manager.get_key(item));
	}

	void get_from(entity_key cont, entity_key item)
	{
		auto cc = _manager.ref<container>(cont);
		auto ip = _manager.ref<physical>(item);

		assert(cc);

		if(ip)
		{
			cc->cur_size -= ip->size;
			cc->cur_weight -= ip->weight;
		}
		_manager.remove<contained>(item);
	}

	void get_from(exces::entity cont, exces::entity item)
	{
		get_from(_manager.get_key(cont), _manager.get_key(item));
	}

	void move_between(entity_key from, entity_key into, entity_key item)
	{
		get_from(from, item);
		put_into(into, item);
	}

	void move_between(exces::entity from, exces::entity into, exces::entity item)
	{
		move_between(_manager.get_key(from), _manager.get_key(into), _manager.get_key(item));
	}
};

class listing_system
{
private:
	typedef exces::manager<> manager;
	typedef typename manager::entity_key entity_key;
	typedef exces::entity entity;

	manager& _manager;
	physical_system& _phy_sys;
public:
	listing_system(manager& m, physical_system& ps)
	 : _manager(m)
	 , _phy_sys(ps)
	{ }

	struct contained_printer
	{
		entity e;

		void operator()(named& n, contained& c) const
		{
			if(c.container == e)
			{
				std::cout << n.name << " ";
			}
		}
	};

	struct printer
	{
		physical_system& ps;

		void operator()(manager& m, entity_key k, entity e, named& n) const
		{
			std::cout << n.name << ":";
			if(m.has<container>(k))
			{
				std::cout << " contains { ";
				contained_printer cp = { e };
				// for each entity with the specified components
				m.for_each_c<named, contained>(cp);
				std::cout << "}";
			}
			if(m.has<physical>(k))
			{
				std::cout << " weight: " << ps.get_weight(k);
				std::cout << " size: " << ps.get_size(k);
			}
			std::cout << std::endl;
		}
	};

	void print(void)
	{
		printer p = { _phy_sys };
		// for each entity with the specified components
		// passing also the rererence to the manager, entity
		// key and entity to the functor
		_manager.for_each_mkec<named>(p);
	}
};

int main(void)
{
	using namespace exces;

	manager<> m;

	container_system cs(m);
	physical_system ps(m);

	listing_system ls(m, ps);
	
	entity crate, bag, anvil, dagger, ring;

	m.add(crate, named("big_crate"), container(400.0, 1500.0, false), physical(410.0, 12.0));
	m.add(bag, named("magic_bag"), container(10.0, 50.0, true, 0.4f, 0.1f), physical(2.0, 1.0));
	m.add(anvil, named("heavy_anvil"), physical(3.0, 20.0));
	m.add(dagger, named("dagger"), physical(0.4, 0.5));
	m.add(ring, named("ring"), physical(0.03, 0.1));

	cs.put_into(crate, bag);
	cs.put_into(crate, anvil);
	cs.put_into(bag, dagger);
	cs.put_into(bag, ring);

	std::cout << "----------------------" << std::endl;

	ls.print();
	std::cout << "----------------------" << std::endl;
	std::cout << " moving anvil to bag" << std::endl;
	std::cout << "----------------------" << std::endl;

	cs.move_between(crate, bag, anvil);

	ls.print();
	std::cout << "----------------------" << std::endl;
	
	return 0;
}

