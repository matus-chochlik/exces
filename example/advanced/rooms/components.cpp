/**
 *  @example advanced/rooms/components.cpp
 *  @brief Instantiation of component-related exces functions
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include "components.hpp"
#include <exces/implement.hpp>

void use_components(void)
{
	EXCES_INSTANTIATE(manager);
	EXCES_INSTANTIATE(implicit_manager);
	EXCES_INSTANTIATE(classification<entity>);
}
