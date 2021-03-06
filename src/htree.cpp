/**
    htree, merkle tree parallel hash computation
    Copyright (C) 2018 Adrien Devresse

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/


#include <iostream>
#include <algorithm>
#include <numeric>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <numeric>
#include <future>


#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#include <hadoken/format/format.hpp>
#include <hadoken/executor/thread_pool_executor.hpp>



#include <digestpp.hpp>

namespace fmt = hadoken::format;

using namespace digestpp;

typedef blake2b hasher_type;
constexpr std::size_t bytes_per_digest = 32;
typedef std::array<char, bytes_per_digest> digest_array;

constexpr std::size_t block_size = 1 << 24;  // 16Ki


void print_help(const char * progname){
    std::cerr << "Usage: " << progname << " [file]" << std::endl;
}


void check_file_exist(const std::string & filename){
    if(access(filename.c_str(), R_OK) != 0){
       throw std::system_error(errno, std::generic_category(),fmt::scat("file ", filename));
    }

}


constexpr char hexmap[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                           '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

template<typename T>
std::string byte_to_hex_str(const T & byte_array)
{
    const std::size_t len = byte_array.size();

    std::string s(len * 2, ' ');
    for (std::size_t i = 0; i < len; ++i) {
        s[2 * i]     = hexmap[(byte_array[i] & 0xF0) >> 4];
        s[2 * i + 1] = hexmap[byte_array[i] & 0x0F];
    }
    return s;
}


std::size_t get_file_size(int fd, const std::string & filename){

    struct stat st;
    if(fstat(fd, &st) != 0){
       throw std::system_error(errno, std::generic_category(),fmt::scat("file ", filename));
    }

    if(S_ISDIR(st.st_mode)){
        throw std::system_error(EISDIR, std::generic_category(), fmt::scat("file ", filename));
    }

    return st.st_size;
}

void test_sha256sum(const std::string & filename){

    hasher_type my_hash(256);

    std::ifstream ff(filename);

    my_hash.absorb(ff);

    std::vector<char> digest;
    my_hash.digest(std::back_inserter(digest));

 //   std::cout << byte_to_hex_str(digest) << " " << filename;

}


int open_filename(const std::string & filename){
    int fd = open(filename.c_str(), O_RDONLY);

    if(fd < 0){
        throw std::system_error(errno, std::generic_category(), fmt::scat("Unable to open ", filename, ""));
    }
    return fd;
}


void compute_leafs(std::vector<digest_array> & digests, int fd, std::size_t file_size){


   //for(std::size_t offset_id =0; offset_id < digests.size(); ++offset_id){
   auto digest_elem = [&](std::size_t offset_id){
        thread_local std::vector<char> buffer(block_size);

        do{
            std::size_t offset = offset_id * block_size;
            ssize_t nbytes = pread(fd, buffer.data(), block_size, offset);

            if(nbytes < 0){
                if(errno == EINTR || errno == EAGAIN){
                    continue;
                }
                throw std::system_error(errno, std::generic_category(), fmt::scat("pread error"));
            }else{
                hasher_type my_hash(256);
                my_hash.absorb(buffer.data(), std::size_t(nbytes));
                my_hash.digest((unsigned char*) digests[offset_id].data(), digests[offset_id].size());
                break;
            }

        }while(1);

       // std::cout << byte_to_hex_str(d) << " ";
    };

   std::vector<std::size_t> index(digests.size());
   std::iota(index.begin(), index.end(), std::size_t(0));


    if(file_size < block_size){
        std::for_each(index.begin(), index.end(), digest_elem);
    }else{

        hadoken::thread_pool_executor executor(std::thread::hardware_concurrency());

        std::vector<std::future<void>> futures;
        futures.reserve(index.size());

        for(auto i : index){
            futures.emplace_back(executor.twoway_execute([&digest_elem, i]() -> void{
                digest_elem(i);
            }));
        }


        for(auto & f : futures){
            f.get();
        }
    }
}


void reduce_leafs(std::vector<digest_array> & old_digests, std::vector<digest_array> & new_digests){
    const std::size_t old_digest_size = old_digests.size() * bytes_per_digest;
    const std::size_t new_size = (old_digest_size)  / block_size + (  ((old_digest_size)  % block_size) ? 1 : 0);

   // std::cerr << " - pass with    " << old_digests.size() << " objects " << std::endl;

    if(old_digests.size() == 1){
        old_digests.swap(new_digests);
        return;
    }

    new_digests.resize(new_size);

    for(std::size_t i =0; i < new_size; ++i){
        const std::size_t offset = i * block_size;
        const std::size_t read_size = std::min(block_size, old_digest_size - offset);

        hasher_type my_hash(256);
        my_hash.absorb(old_digests[0].data() + std::ptrdiff_t(offset), read_size);
        my_hash.digest((unsigned char*) new_digests[i].data(), bytes_per_digest);
    }


    old_digests.swap(new_digests);
    reduce_leafs(old_digests, new_digests);
}


template<typename Callback>
void hash_for_file(const std::string & filename, Callback digest_cb){
    //check_file_exist(filename);

    //std::cerr << " - compute blake2 merkle-tree " << std::endl;
    int fd = open_filename(filename);

    const std::size_t total_size = get_file_size(fd, filename);

    //std::cerr << " - file size " << total_size << std::endl;

    const std::size_t blocks = total_size / block_size + ( (total_size % block_size || total_size == 0) ? 1 : 0 );


    std::vector<digest_array> leafs(blocks), result;


    compute_leafs(leafs, fd, total_size);

    close(fd);

    reduce_leafs(leafs, result);

    if(result.size() != 1){
        throw std::invalid_argument(fmt::scat("Invalid tree root ", result.size()));
    }

    digest_cb(result[0]);
}

int main(int argc, char** argv){

    int ret = 0;
    if(argc <  2){
        print_help(argv[0]);
        exit(1);
    }


    for(int i =1; i < argc; ++i){
        try{
            const std::string filename(argv[i]);

            hash_for_file(filename, [&filename](const digest_array & result){
                std::cout << byte_to_hex_str(result) << " " << filename << std::endl;
            });

        }catch(std::exception & e){
            std::cerr << argv[0] << ": "<< e.what() << std::endl;
            ret = 1;
        }
    }
    std::flush(std::cout);


    return ret;
}







