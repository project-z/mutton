libprz
========

A distributed bitmap index and query engine for embedding or overlaying on top of other databases


### Example Schema

```
keyspace user_events
row_id -> [product, user]
colmns -> event_name : counter

[prod1, user1] : {login : 1}
[prod1, user1] : {landing_page : 2}
[prod1, user1] : {signup : 1}

[prod1, user2] : {login : 1}
[prod1, user2] : {landing_page : 2}
[prod1, user2] : {signup : 1}

[prod1, user2] : {login : 2}
[prod1, user2] : {landing_page : 7}
[prod1, user2] : {signup : 2}
```

### Indexes

* Indexes are stored on disk in [leveldb](https://code.google.com/p/leveldb/)
* All index addresses spaces are 64 bit
* Index chunks are 256 bytes
* Offsets are 16 bytes (64 bits)
* If the index value can't be transformed to a 64bit unsigned int, it must be hashed by [cityhash](https://code.google.com/p/cityhash/).
* In level db all keys are contained within the same namespace. Need to prepend byte sequence to keys to partition keyspaces. Partition byte sequence are 16 bits (2 bytes).
* Fields [prod, event] are encodeded as [bytes] where the first uint16 is the size of the byte sequence
* Because of hash collsion we can't rely upon hash values alone to find unique IDs. Anytime a hash is employed we must keep a dictionary mapping hash values to actual value.

### hash -> table row_id inverted index

```
[partition][table][hash_val][row_id] : NULL
[2][bytes][16][bytes] : NULL
```


### Range/equality encoded bitslice index

```
[partition][field1 bytes][value][offset] : [index chunk 0x00]
[partition][field1 bytes][value][offset] : [index chunk 0x01]

[partition][field2 bytes][value][offset] : [index chunk 0x01]
[partition][field2 bytes][value][offset] : [index chunk 0x03]

[2][bytes][16][16] : [256]
```
