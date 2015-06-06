///////////////////////////////////////////////////////////////////////////////
//
// Copyright 2011-2014 Felipe Magno de Almeida.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// See http://www.boost.org/libs/foreach for documentation
//

#ifndef DFD_PARSERS_PAT_HPP
#define DFD_PARSERS_PAT_HPP

#include <dfd/filters.hpp>
#include <dfd/programs.hpp>
#include <dfd/carousels.hpp>

#include <gts/sections/extended_section.hpp>
#include <gts/dsmcc/download_info_indication.hpp>
#include <gts/dsmcc/download_server_initiate.hpp>
#include <gts/dsmcc/sections/user_network_message_section.hpp>
#include <gts/dsmcc/biop/file_message.hpp>
#include <gts/dsmcc/biop/directory_message.hpp>

#include <cstdlib>
#include <vector>

namespace dfd { namespace parsers {

typedef gts::sections::extended_section<unsigned char const*> extended_section_type;
typedef gts::dsmcc::sections::user_network_message_section<unsigned char const*>
 section_un_type;
typedef section_un_type::user_network_message_iterator user_network_message_iterator;
typedef user_network_message_iterator::deref_type user_network_type;
typedef gts::dsmcc::download_info_indication<typename user_network_type::base_iterator> download_info_indication;
typedef gts::dsmcc::download_server_initiate<typename user_network_type::base_iterator> download_server_initiate;
typedef gts::dsmcc::biop::file_message<std::vector<unsigned char>::const_iterator> file_message;
typedef gts::dsmcc::biop::directory_message<std::vector<unsigned char>::const_iterator> directory_message;

void pat(unsigned char const* buffer, std::size_t size, int fd, dfd::filters& filters
         , dfd::programs& program, dfd::carousels& carousels);
void nit(unsigned char const* buffer, std::size_t size, int fd, dfd::filters& filters);
void pmt(unsigned char const* buffer, std::size_t size, int fd, dfd::filters& filters
         , dfd::programs& program, __u16 pid, dfd::carousels& carousels);
void ait(unsigned char const* buffer, std::size_t size, int fd, dfd::filters& filters
         , dfd::programs& program, __u16 pmt_pid, __u16 dsmcc_pid
         , dfd::carousels& carousels);
void dsmcc(unsigned char const* buffer, std::size_t size, int fd, dfd::filters& filters
           , dfd::programs& program, __u16 pmt_pid, __u16 dsmcc_pid
           , dfd::carousels& carousels);
void dsmcc_un_messages(extended_section_type extended_section, int fd, dfd::filters& filters
                       , dfd::programs& programs, __u16 pmt_pid, __u16 dsmcc_pid);
void dsmcc_dii(download_info_indication dii, int fd, dfd::filters& filters
               , dfd::programs& programs, __u16 pmt_pid, __u16 dsmcc_pid);
void dsmcc_dsi(download_server_initiate dii, int fd, dfd::filters& filters
               , dfd::programs& programs, __u16 pmt_pid, __u16 dsmcc_pid);
void dsmcc_ddb(extended_section_type extended_section, int fd, dfd::filters& filters
               , dfd::programs& programs, __u16 pmt_pid, __u16 dsmcc_pid
               , dfd::carousels& carousels);
void dsmcc_module(unsigned int download_id, dfd::module const& module, int fd, dfd::filters& filters
                  , dfd::carousels& carousels);
std::vector<unsigned char>::const_iterator
dsmcc_file(file_message const& file, int fd, dfd::filters& filters
           , unsigned int download_id, unsigned int module_id
           , boost::iterator_range<std::vector<unsigned char>::const_iterator> object_key
           , dfd::carousels& carousels);
std::vector<unsigned char>::const_iterator
dsmcc_directory(directory_message const& directory, int fd, dfd::filters& filters
                , unsigned int download_id, unsigned int module_id
                , boost::iterator_range<std::vector<unsigned char>::const_iterator> object_key
                , dfd::carousels& carousels);
std::vector<unsigned char>::const_iterator
dsmcc_gateway(directory_message const& directory, int fd, dfd::filters& filters
              , unsigned int download_id, unsigned int module_id
              , boost::iterator_range<std::vector<unsigned char>::const_iterator> object_key
              , dfd::carousels& carousels);

} }

#endif
