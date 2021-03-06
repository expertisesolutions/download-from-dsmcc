///////////////////////////////////////////////////////////////////////////////
//
// Copyright 2011-2014 Felipe Magno de Almeida.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// See http://www.boost.org/libs/foreach for documentation
//

#include <dfd/filters.hpp>
#include <dfd/programs.hpp>
#include <dfd/parsers/parsers.hpp>

#include <gts/dsmcc/biop/message_header.hpp>
#include <gts/dsmcc/biop/message_subheader.hpp>
#include <gts/dsmcc/biop/file_message.hpp>

namespace dfd { namespace parsers {

void dsmcc_module(unsigned download_id, dfd::module const& module, int fd, dfd::filters& filters
                  , dfd::carousels& carousels)
{
  std::cout << "Must parse module" << std::endl;
  std::vector<unsigned char> buffer;
  for(std::vector<dfd::block>::const_iterator first = module.blocks.begin()
        , last = module.blocks.end(); first != last; ++first)
  {
    buffer.insert(buffer.end(), first->raw.begin(), first->raw.end());
  }

  std::cout << "module size: " << buffer.size() << std::endl;

  std::vector<unsigned char>::const_iterator first = buffer.begin()
    , last = buffer.end();
  while(first != last)
  {
  typedef gts::dsmcc::biop::message_header<std::vector<unsigned char>::const_iterator> message_header_type;
  typedef message_header_type::magic_iterator magic_iterator;
  typedef message_header_type::biop_version_iterator biop_version_iterator;
  typedef message_header_type::byte_order_iterator byte_order_iterator;
  typedef message_header_type::message_type_iterator message_type_iterator;
  typedef message_header_type::message_size_iterator message_size_iterator;
  typedef message_header_type::end_iterator header_end_iterator;

  std::vector<unsigned char>::const_iterator old_first = first;
  bool found_magic = false;
  while (std::distance(first, last) >= 4 && !found_magic)
  {
    unsigned char const biop_magic[4] = {0x42, 0x49, 0x4F, 0x50};
    if(std::equal(biop_magic, biop_magic + 4, first))
    {
      found_magic = true;
      break;
    }
    first++;
  }
  if(!found_magic)
  {
    std::cout << "No next message" << std::endl;
    return;
  }
  else
    std::cout << "skipped " << std::distance(old_first, first) << " bytes" << std::endl;
  message_header_type message_header(first, last);
  magic_iterator magic_it = message_header.begin();
  assert(*magic_it == 0x42494F50);

  message_size_iterator message_size_it = gts::iterators::next<4>(magic_it);
  std::cout << "message_size " << *message_size_it << std::endl;

  header_end_iterator header_end_it = gts::iterators::next(message_size_it);

  typedef gts::dsmcc::biop::message_subheader<std::vector<unsigned char>::const_iterator> message_subheader_type;
  typedef message_subheader_type::object_key_iterator object_key_iterator;
  typedef message_subheader_type::object_kind_iterator object_kind_iterator;
  typedef message_subheader_type::object_info_iterator object_info_iterator;
  typedef message_subheader_type::service_context_list_iterator service_context_list_iterator;
  typedef message_subheader_type::end_iterator subheader_end_iterator;

  message_subheader_type message_subheader(header_end_it.base(), last);

  object_key_iterator object_key_it = message_subheader.begin();
  object_kind_iterator object_kind_it = gts::iterators::next(object_key_it);
  object_info_iterator object_info_it = gts::iterators::next(object_kind_it);
  typename object_kind_iterator::deref_type kind = *object_kind_it;
  if(boost::distance(kind) == 4)
  {
    std::vector<unsigned char>::const_iterator parse_end = first;
    const char *file_kind = "fil"
      , *directory_kind = "dir"
      , *service_gateway_kind = "srg";
    if(std::equal(kind.begin(), kind.end(), file_kind))
    {
      std::cout << "is a file" << std::endl;
      file_message file(object_info_it.base(), last);
      parse_end = dsmcc_file(file, fd, filters, download_id, module.id, *object_key_it, carousels);
      assert(std::distance(first, parse_end) == *message_size_it+12 || parse_end == last);
    }
    else if(std::equal(kind.begin(), kind.end(), directory_kind))
    {
      subheader_end_iterator message_subheader_end = gts::iterators::next<2>(object_info_it);
      std::cout << "is a directory" << std::endl;
      directory_message directory(message_subheader_end.base(), last);
      parse_end = dsmcc_directory(directory, fd, filters, download_id, module.id, *object_key_it, carousels);
      assert(std::distance(first, parse_end) == *message_size_it+12 || parse_end == last);
    }
    else if(std::equal(kind.begin(), kind.end(), service_gateway_kind))
    {
      std::cout << "is a service gateway" << std::endl;
      subheader_end_iterator message_subheader_end = gts::iterators::next<2>(object_info_it);
      std::cout << "is a directory" << std::endl;
      directory_message directory(message_subheader_end.base(), last);
      parse_end = dsmcc_gateway(directory, fd, filters, download_id, module.id, *object_key_it, carousels);
      assert(std::distance(first, parse_end) == *message_size_it+12 || parse_end == last);
    }
    else
    {
      std::cout << "Unknown kind" << std::endl;
    }
  }
  std::advance(first, (std::min<std::size_t>)(*message_size_it+12
                                              , std::distance<std::vector<unsigned char>::const_iterator>
                                              (first, last)));
  }
}

} }
