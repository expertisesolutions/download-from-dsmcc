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

#include <gts/sections/program_association_section.hpp>
#include <gts/sections/extended_section.hpp>
#include <gts/partial_descriptor_sequence.hpp>
#include <gts/nit_private_sequence.hpp>
#include <gts/sections/private_section.hpp>
#include <gts/sections/program_map_section.hpp>

#include <boost/range.hpp>
#include <boost/bind.hpp>

#include <cstdlib>

namespace dfd { namespace parsers {

void pmt(unsigned char const* buf, std::size_t count, int fd, dfd::filters& filters
         , dfd::programs& programs, __u16 pid, dfd::carousels& carousels)
{
  std::cout << "PMT packet" << std::endl;
  filters.remove_filter(fd);

  program prg (pid);

  typedef gts::sections::private_section<unsigned char const*> private_section_type;

  typedef private_section_type::section_syntax_indicator_iterator section_syntax_indicator_iterator;
  typedef private_section_type::private_indicator_iterator private_indicator_iterator;
  typedef private_section_type::private_section_length_iterator private_section_length_iterator;
  typedef private_section_type::private_data_iterator private_data_iterator;
  typedef private_section_type::end_iterator end_iterator;

  private_section_type private_section(buf, buf + count);
  section_syntax_indicator_iterator
    section_syntax_indicator = gts::iterators::next(private_section.begin ());

  if(*section_syntax_indicator == 1) // section syntax indicator
  {
    std::cout << "section syntax indicator == 1" << std::endl;
    private_indicator_iterator private_indicator = gts::iterators::next(section_syntax_indicator);
    if(*private_indicator == 0)
    {
      private_section_length_iterator private_section_length = gts::iterators::next(private_indicator);
      std::cout << "private_section_length: " << *private_section_length << std::endl;

      private_data_iterator private_data_it = gts::iterators::next(private_section_length);
      typedef private_data_iterator::deref_type private_data_type;
      private_data_type private_data = *private_data_it;
      std::cout << "size of private data: " << std::distance(private_data.begin (), private_data.end ())
                << std::endl;

      typedef gts::sections::program_map_section<unsigned char const*> pmt_private_type;
      pmt_private_type pmt_private (buf, buf+count);
      typedef pmt_private_type::program_number_iterator program_number_iterator;
      typedef pmt_private_type::version_number_iterator version_number_iterator;
      typedef pmt_private_type::current_next_indicator_iterator current_next_indicator_iterator;
      typedef pmt_private_type::section_number_iterator section_number_iterator;
      typedef pmt_private_type::last_section_number_iterator last_section_number_iterator;
      typedef pmt_private_type::pcr_pid_iterator pcr_pid_iterator;
      typedef pmt_private_type::program_info_length_iterator program_info_length_iterator;
      typedef pmt_private_type::descriptor_repetition_iterator descriptor_repetition_iterator;
      typedef pmt_private_type::program_map_repetition_iterator program_map_repetition_iterator;
      typedef pmt_private_type::end_iterator end_iterator;

      program_number_iterator program_number = ++ ++ ++ ++pmt_private.begin ();
      std::cout << "broadcasting_program_number_identifier: "
                << std::hex << *program_number << std::dec << std::endl;
      version_number_iterator version_number = gts::iterators::next(program_number);
      std::cout << "version_number: " << *version_number << std::endl;
      current_next_indicator_iterator current_next = gts::iterators::next(version_number);
      std::cout << "current_next: " << *current_next << std::endl;
      section_number_iterator section_number = gts::iterators::next(current_next);
      std::cout << "section_number: " << *section_number << std::endl;
      last_section_number_iterator last_section_number = gts::iterators::next(section_number);
      std::cout << "last_section_number: " << *last_section_number << std::endl;
      pcr_pid_iterator pcr_pid = gts::iterators::next(last_section_number);
      std::cout << "PCR_PID: 0x" 
                << std::hex << *pcr_pid << std::dec << std::endl;
      program_info_length_iterator program_info_length = gts::iterators::next(pcr_pid);
      std::cout << "program_info_length: " << *program_info_length << std::endl;
      descriptor_repetition_iterator descriptor_repetition = gts::iterators::next(program_info_length);
      typedef descriptor_repetition_iterator::deref_type descriptor_range;
      descriptor_range descriptors = *descriptor_repetition;
      for(boost::range_iterator<descriptor_range>::type first = descriptors.begin ()
            , last = descriptors.end (); first != last; ++first)
      {
        std::cout << "one descriptor" << std::endl;
      }
      program_map_repetition_iterator program_map_repetition = gts::iterators::next(descriptor_repetition);
      typedef program_map_repetition_iterator::deref_type program_map_range;
      program_map_range program_maps = *program_map_repetition;
      for(boost::range_iterator<program_map_range>::type first = program_maps.begin ()
            , last = program_maps.end (); first != last; ++first)
      {
        std::cout << "program_map" << std::endl;
        typedef boost::range_value<program_map_range>::type program_map_type;
        program_map_type program_map = *first;
        typedef program_map_type::stream_type_iterator stream_type_iterator;
        typedef program_map_type::elementary_pid_iterator elementary_pid_iterator;
        typedef program_map_type::es_info_length_iterator es_info_length_iterator;
        typedef program_map_type::descriptor_repetition_iterator descriptor_repetition_iterator;
        
        stream_type_iterator stream_type = program_map.begin ();
        std::cout << " stream_type: 0x" << std::hex << *stream_type << std::dec << std::endl;
        elementary_pid_iterator elementary_pid = gts::iterators::next(stream_type);
        std::cout << " elementary_pid: 0x"
                  << std::hex << *elementary_pid << std::dec << std::endl;

        if((*stream_type >= 0x8 && *stream_type <= 0x0D)
           || (*stream_type >= 0x17 && *stream_type <= 0x19))
        {
          std::cout << "DSMCC stream" << std::endl;
          dfd::filters::table_filter f(*elementary_pid);
          filters.add_table_filter(f, boost::bind(&dfd::parsers::dsmcc, _1, _2, _3, boost::ref(filters)
                                                  , boost::ref(programs), pid, *elementary_pid
                                                  , boost::ref(carousels))
                                   , DMX_IMMEDIATE_START|DMX_CHECK_CRC);
        }
        else if(*stream_type == 0x5)
        {
          std::cout << "AIT stream" << std::endl;
          dfd::filters::table_filter f(*elementary_pid);
          programs.add_program(prg);
          filters.add_table_filter(f, boost::bind(&dfd::parsers::ait, _1, _2, _3, boost::ref(filters)
                                                  , boost::ref(programs), pid, *elementary_pid
                                                  , boost::ref(carousels))
                                   , DMX_IMMEDIATE_START|DMX_CHECK_CRC);
        }
      }
    }
  }
  else
    std::cout << "section syntax indicator == 0 to PMT" << std::endl;
}

} }
