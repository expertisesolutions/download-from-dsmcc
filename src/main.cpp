///////////////////////////////////////////////////////////////////////////////
//
// Copyright 2011-2014 Felipe Magno de Almeida.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// See http://www.boost.org/libs/foreach for documentation
//

#include <dfd/frontend.hpp>
#include <dfd/filters.hpp>
#include <dfd/parsers/parsers.hpp>

#include <boost/program_options.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/bind.hpp>

#include <sys/stat.h>
#include <fcntl.h>
#include <sstream>
#include <algorithm>
#include <iterator>

void save_directory(boost::filesystem::path path, dfd::directory_object const& directory
                    , dfd::carousels const& carousels)
{
  for(std::vector<dfd::directory_bind>::const_iterator
        bind_first = directory.binds.begin()
        , bind_last = directory.binds.end()
        ;bind_first != bind_last; ++bind_first)
  {
    dfd::file_object const* file = carousels.carousels_.find(bind_first->carousel_id)
      ->second.find_file(bind_first->object_key, bind_first->module_id);
    std::string filename (bind_first->names.id.begin(), bind_first->names.id.end());
    filename.erase(std::remove(filename.begin(), filename.end(), '\0'));
    
    if(file)
    {
      std::cout << "Found file " << filename << std::endl;
      boost::filesystem::ofstream fstream(path / filename);
      std::copy(file->content.begin(), file->content.end()
                , std::ostream_iterator<unsigned char>(fstream));
    }
    else
    {
      dfd::directory_object const* object = carousels.carousels_.find(bind_first->carousel_id)
        ->second.find_directory(bind_first->object_key, bind_first->module_id);
      if(object)
      {
        boost::filesystem::path child_directory = path / filename;
        boost::filesystem::create_directory(child_directory);
        save_directory(child_directory, *object, carousels);
      }
      else
        std::cout << "Couldn't find file or directory named " << filename
                  << " with carousel: " << bind_first->carousel_id
                  << " and module_id " << bind_first->module_id << std::endl;
    }
  }
}

int main(int argc, char* argv[])
{
  unsigned int frequency = 0, adapter = 0;
  boost::filesystem::path directory;
  bool dvbv5 = false;
  {
    boost::program_options::options_description descriptions("Allowed options");
    descriptions.add_options()
      ("help", "produce this help message")
      ("directory", boost::program_options::value<std::string>(), "Directory to download DSMCC to")
      ("frequency", boost::program_options::value<unsigned int>(), "Frequency of channel")
      ("adapter", boost::program_options::value<unsigned int>(), "Adapter number")
      ("dvbv5", boost::program_options::value<bool>(), "If should use DVBv5 for frontend (default: use dvbv3)")
      ;

    boost::program_options::variables_map vm;
    boost::program_options::store(boost::program_options::parse_command_line(argc, argv, descriptions), vm);
    boost::program_options::notify(vm);
      
    if(vm.count("help") > 0)
    {
      std::cout << descriptions << std::endl;
      return 1;
    }
    if(!vm.count("frequency"))
    {
      std::cout << "You must specify the frequency (e.g. 599142857)" << std::endl;
      return 1;
    }
    if(!vm.count("directory"))
    {
      std::cout << "You must specify a directory to download the DSMCC to" << std::endl;
      return 1;
    }

    directory = vm["directory"].as<std::string>();
    if(!boost::filesystem::exists(directory))
    {
      std::cout << "Path " << directory << " doesn't exist" << std::endl;
      return 1;
    }
    frequency = vm["frequency"].as<unsigned int>();

    if(vm.count("adapter"))
    {
      adapter = vm["adapter"].as<unsigned int>();
    }

    if(vm.count("dvbv5") && vm["dvbv5"].as<bool>())
      dvbv5 = true;
  }  

  std::cout << "Opening /dev/dvb/adapter" << adapter << "/frontend0" << std::endl;
  std::string adapter_devname;
  {
    std::stringstream s;
    s << "/dev/dvb/adapter" << adapter;
    adapter_devname = s.str();
  }
  int frontend_fd = 0;
  {
    const std::string frontend_devname = adapter_devname + "/frontend0";
    frontend_fd = open(frontend_devname.c_str(), O_RDWR);
    if(frontend_fd < 0)
    {
      std::cout << "Couldn't open frontend device " << frontend_devname << " for reading and writing" << std::endl;
      return -1;
    }
  }

  try
  {
    dfd::frontend frontend (frontend_fd);
    if(dvbv5)
      frontend.tune_dvb5(frequency);
    else
      frontend.tune_dvb3(frequency);

    dfd::programs programs;
    dfd::carousels carousels;

    dfd::filters filters(adapter_devname + "/demux0");
    {
      dfd::filters::table_filter pat(0);
      filters.add_table_filter(pat, boost::bind(&dfd::parsers::pat, _1, _2, _3, boost::ref(filters)
                                                , boost::ref(programs), boost::ref(carousels))
                               , DMX_IMMEDIATE_START|DMX_ONESHOT|DMX_CHECK_CRC);
    }

    filters.run();

    std::cout << "Finished. Carousels: " << carousels.carousels_.size() << std::endl;
    unsigned int files = 0, directories = 0;
    for(std::map<unsigned int, dfd::carousel>::const_iterator
          first = carousels.carousels_.begin()
          , last = carousels.carousels_.end()
          ; first != last;++first)
    {
      files += first->second.file_objects.size();
      directories += first->second.directory_objects.size();
    }
    std::cout << "Files: " << files << " directories: " << directories << std::endl;

    directories = 0;
    for(std::map<unsigned int, dfd::carousel>::const_iterator
          first = carousels.carousels_.begin()
          , last = carousels.carousels_.end()
          ;first != last;++first)
    {
      std::stringstream s;
      s << "directory" << directories;
      boost::filesystem::path child_directory = directory / s.str();
      boost::filesystem::create_directory(child_directory);
      dfd::directory_object const& gateway = first->second.gateway;
      save_directory(child_directory, gateway, carousels);
    }
  }
  catch(std::exception const& e)
  {
    std::cout << "Exception caught " << e.what() << std::endl;
  }
}
