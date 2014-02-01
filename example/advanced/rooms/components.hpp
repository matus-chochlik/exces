/**
 *  @example advanced/rooms/components.hpp
 *  @brief Component registering.
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#ifndef EXCES_EXAMPLE_ADVANCED_ROOMS_COMPONENTS_HPP
#define EXCES_EXAMPLE_ADVANCED_ROOMS_COMPONENTS_HPP

#include <exces/manager.hpp>

#include "item_type.hpp"
#include "name_desc.hpp"
#include "physical.hpp"
#include "container.hpp"
#include "location.hpp"
#include "locking.hpp"
#include "input_output.hpp"
#include "actor.hpp"

EXCES_REG_FLYWEIGHT_COMPONENT(item_type)

EXCES_REG_COMPONENT(name)
EXCES_REG_COMPONENT(description)

EXCES_REG_COMPONENT(destructible)
EXCES_REG_COMPONENT(physical_object)
EXCES_REG_COMPONENT(gravity_modifier)

EXCES_REG_COMPONENT(container)

EXCES_REG_COMPONENT(location)
EXCES_REG_COMPONENT(portal)
EXCES_REG_FLYWEIGHT_COMPONENT(key)
EXCES_REG_COMPONENT(lockable)

EXCES_REG_COMPONENT(io)
EXCES_REG_COMPONENT(actor)

extern void use_components(void);

#endif // include guard
