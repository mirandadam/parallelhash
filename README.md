# parallelhash

Parallelhash is a multithreaded command line utility for hashing a file.
It supports algorithms md5, sha1, sha256 and sha512 and can perform multiple simultaneous hash calculations.
It's initial goal is to be faster at hashing than dcfldd.

Features:
* faster than dcfldd
* md5, sha1, sha256, sha512
* simultaneous hash calculations
* configurable partial hashes (compute a digest for each file segment)
* write output to separate log files
