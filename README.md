libmutton
========

A distributed bitmap index and query engine for embedding or overlaying on top of other databases


### Example Schema

```
keyspace user_events
row_id -> [product, user]
columns -> event_name : counter

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
* All index addresses spaces are 128 bit
* Index chunks are 256 bytes
* Offsets are 16 bytes (64 bits)
* If the index value can't be transformed to a 128bit unsigned int, it must be hashed by [cityhash](https://code.google.com/p/cityhash/).
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

### Query Language

The most basic operation is a slice, or all rows who have a 1 bit set in the named index

Give me all users that have visted the site:
```(slice "visit")```

Give me all users that have visited the site in 2013:
```(slice "visit_2013")```

Give me all users that have visited the site in May of 2013:
```(slice "visit_2013_05")```

Give me all users that have not visited the site in May of 2013:
```(not (slice "visit_2013_05"))```

Give me all users that have visited the site in May of 2013 more than 6 times:
```(slice "visit_2013_05" (range 6 0))```

Give me all users that have generated more than 10k of revenue:
```(slice "revenue" (range 10000 0))```

Give me all users that have generated more than 10k of revenue, or visited more than 150 times:
```(or (slice "revenue" (range 10000 0)) (slice "visit" (range 150 0)))```

Give me all users that have hit a mobpub ad campaign in 2013 and made a purchase:
```(and (slice "ad_campaign_2013" (regex "mopub.*")) (slice "revenue_2013"))```

**NOTE:** Still need to figure out API for selecting which fields to return to the caller
