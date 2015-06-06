///////////////////////////////////////////////////////////////////////////////
//
// Copyright 2011-2014 Felipe Magno de Almeida.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// See http://www.boost.org/libs/foreach for documentation
//

#include <dfd/filters.hpp>

#include <cstdlib>

namespace dfd { namespace parsers {

void nit(unsigned char const* buf, std::size_t count, int fd, dfd::filters& filters)
{
  std::cout << "NIT packet" << std::endl;
  filters.remove_filter(fd);
  
}

} }
