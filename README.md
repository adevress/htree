# HTREE

## SUMMARY

htree is a message digest tool (like md5sum, sha256sum) but fully multi-threaded.

This make significantly faster the creation of checksum for large files.

## USAGE

	htree [file1] [file...] [filen]


## EXAMPLE

```bash
ls -lh bigtarball.tar.gz 
-rw-rw-r-- 1 X X 2,4G mars  27 21:38 bigtarball.tar.gz


time htree bigtarball.tar.gz 
22dbbb9fc312070f987789d7fc3ed605529434d2da5823e3c058ce8c749cbd85 bigtarball.tar.gz

real	0m1,717s
user	0m11,328s
sys	0m0,696s

time md5sum bigtarball.tar.gz 
34cbee16ad07f1aa6e912d4c7c42d98a  bigtarball.tar.gz

real	0m3,940s
user	0m3,749s
sys	0m0,192s

```

## ALGORITHM

htree uses by default a decomposition in 16MiB block size and a [Merkle tree](https://en.wikipedia.org/wiki/Merkle_tree) associated with the [blake2b](https://en.wikipedia.org/wiki/BLAKE_(hash_function)) hash function.

The implementation of htree is simple and fits in a single < 300 lines file.


## DEPENDENCIES

None. C++11 compatible compiler

Embedded component : [hadoken](https://github.com/adevress/hadoken), [digestpp](https://github.com/kerukuro/digestpp)


## LICENSE

GPLv3

