/*
* (C) 2015,2017 Jack Lloyd
* (C) 2015 Simon Warta (Kullo GmbH)
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include <botan/exceptn.h>
#include <botan/internal/filesystem.h>
#include <algorithm>

#if defined(BOTAN_TARGET_OS_HAS_STL_FILESYSTEM_MSVC) && defined(BOTAN_BUILD_COMPILER_IS_MSVC)
  #include <filesystem>
#elif defined(BOTAN_HAS_BOOST_FILESYSTEM)
  #include <boost/filesystem.hpp>
#elif defined(BOTAN_TARGET_OS_HAS_POSIX1)
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <dirent.h>
  #include <deque>
  #include <memory>
  #include <functional>
#elif defined(BOTAN_TARGET_OS_HAS_WIN32)
  #define NOMINMAX 1
  #define _WINSOCKAPI_ // stop windows.h including winsock.h
  #include <windows.h>
  #include <deque>
  #include <memory>
#endif

namespace Botan {

namespace {

#if defined(BOTAN_TARGET_OS_HAS_STL_FILESYSTEM_MSVC) && defined(BOTAN_BUILD_COMPILER_IS_MSVC)
std::vector<std::string> impl_stl_filesystem(const std::string& dir)
   {
#if (_MSVC_LANG >= 201703L)
   using namespace std::filesystem;
#else
   using namespace std::tr2::sys;
#endif

   std::vector<std::string> out;

   path p(dir);

   if(is_directory(p))
      {
      for(recursive_directory_iterator itr(p), end; itr != end; ++itr)
         {
         if(is_regular_file(itr->path()))
            {
            out.push_back(itr->path().string());
            }
         }
      }

   return out;
   }

#elif defined(BOTAN_HAS_BOOST_FILESYSTEM)

std::vector<std::string> impl_boost_filesystem(const std::string& dir_path)
{
   namespace fs = boost::filesystem;

   std::vector<std::string> out;

   for(fs::recursive_directory_iterator dir(dir_path), end; dir != end; ++dir)
      {
      if(fs::is_regular_file(dir->path()))
         {
         out.push_back(dir->path().string());
         }
      }

   return out;
}

#elif defined(BOTAN_TARGET_OS_HAS_POSIX1)

std::vector<std::string> impl_readdir(const std::string& dir_path)
   {
   std::vector<std::string> out;
   std::deque<std::string> dir_list;
   dir_list.push_back(dir_path);

   while(!dir_list.empty())
      {
      const std::string cur_path = dir_list[0];
      dir_list.pop_front();

      std::unique_ptr<DIR, std::function<int (DIR*)>> dir(::opendir(cur_path.c_str()), ::closedir);

      if(dir)
         {
         while(struct dirent* dirent = ::readdir(dir.get()))
            {
            const std::string filename = dirent->d_name;
            if(filename == "." || filename == "..")
               continue;
            const std::string full_path = cur_path + "/" + filename;

            struct stat stat_buf;

            if(::stat(full_path.c_str(), &stat_buf) == -1)
               continue;

            if(S_ISDIR(stat_buf.st_mode))
               dir_list.push_back(full_path);
            else if(S_ISREG(stat_buf.st_mode))
               out.push_back(full_path);
            }
         }
      }

   return out;
   }

#elif defined(BOTAN_TARGET_OS_HAS_WIN32)

std::vector<std::string> impl_win32(const std::string& dir_path)
   {
   std::vector<std::string> out;
   std::deque<std::string> dir_list;
   dir_list.push_back(dir_path);

   while(!dir_list.empty())
      {
      const std::string cur_path = dir_list[0];
      dir_list.pop_front();

      WIN32_FIND_DATAA find_data;
      HANDLE dir = ::FindFirstFileA((cur_path + "/*").c_str(), &find_data);

      if(dir != INVALID_HANDLE_VALUE)
         {
         do
            {
            const std::string filename = find_data.cFileName;
            if(filename == "." || filename == "..")
               continue;
            const std::string full_path = cur_path + "/" + filename;

            if(find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
               {
               dir_list.push_back(full_path);
               }
            else
               {
               out.push_back(full_path);
               }
            }
         while(::FindNextFileA(dir, &find_data));
         }

      ::FindClose(dir);
      }

   return out;
}
#endif

}

bool has_filesystem_impl()
   {
#if defined(BOTAN_TARGET_OS_HAS_STL_FILESYSTEM_MSVC) && defined(BOTAN_BUILD_COMPILER_IS_MSVC)
   return true;
#elif defined(BOTAN_HAS_BOOST_FILESYSTEM)
   return true;
#elif defined(BOTAN_TARGET_OS_HAS_POSIX1)
   return true;
#elif defined(BOTAN_TARGET_OS_HAS_WIN32)
   return true;
#else
   return false;
#endif
   }

std::vector<std::string> get_files_recursive(const std::string& dir)
   {
   std::vector<std::string> files;

#if defined(BOTAN_TARGET_OS_HAS_STL_FILESYSTEM_MSVC) && defined(BOTAN_BUILD_COMPILER_IS_MSVC)
   files = impl_stl_filesystem(dir);
#elif defined(BOTAN_HAS_BOOST_FILESYSTEM)
   files = impl_boost_filesystem(dir);
#elif defined(BOTAN_TARGET_OS_HAS_POSIX1)
   files = impl_readdir(dir);
#elif defined(BOTAN_TARGET_OS_HAS_WIN32)
   files = impl_win32(dir);
#else
   BOTAN_UNUSED(dir);
   throw No_Filesystem_Access();
#endif

   std::sort(files.begin(), files.end());

   return files;
   }

}
