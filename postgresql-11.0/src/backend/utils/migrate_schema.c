#include "utils/migrate_schema.h"
#include "storage/shmem.h"

/* flag to indicate if a query is a part of a migration */
bool migrateflag = false;

/* MigrateAggHashTable: global hash table */
HTAB *MigrateAggHashTable = NULL;

/* LocalAggHashTable: local hash table */
HTAB *LocalIPAggHashTable0 = NULL;
HTAB *LocalIPAggHashTable1 = NULL;

uint32 count_inprogress = 0;
uint64 tuplemigratecount = 0;

void
InitMigrateAggHashTable(void)
{
	HASHCTL		ctl;
	int 		size;

	memset(&ctl, 0, sizeof(ctl));

	ctl.keysize = sizeof(MigrateAggHashKey);
	ctl.entrysize = sizeof(MigrateAggHashValue);
	ctl.num_partitions = NUM_MIGRATE_AGG_PARTITIONS;
	//ctl.hash = tag_hash;

	size = MIGRATE_AGG_HASH_TABLE_SIZE + NUM_MIGRATE_AGG_PARTITIONS;

	MigrateAggHashTable = ShmemInitHash("Shared Migrate Agg Hash Table",
                                        100000, size, &ctl,
                                        HASH_ELEM | HASH_BLOBS | HASH_PARTITION);

	printf ("MigrateAggHashTable created.\n");
}


void
InitLocalIPAggHashTable(void)
{
	HASHCTL		ctl;
	int 		psize;

	memset(&ctl, 0, sizeof(ctl));

	ctl.keysize = sizeof(LocalAggHashKey);
	ctl.entrysize = sizeof(LocalAggHashValue);

	psize = 50;
	LocalIPAggHashTable0 = hash_create("Local Agg Hash Table For In-Progress Tuples",
										psize, &ctl,
										HASH_ELEM | HASH_BLOBS);

	LocalIPAggHashTable1 = hash_create("Local Agg Hash Table For In-Progress Tuples",
										psize, &ctl,
										HASH_ELEM | HASH_BLOBS);
}

uint32
MigrateAggHashCode(MigrateAggHashKey *tagPtr)
{
    return get_hash_value(MigrateAggHashTable, (void *) tagPtr);
}

bool
migrateagghashtable_lookup(uint32 k1, uint32 k2, uint32 k3, uint8 *hval)
{
	MigrateAggHashKey key;
	MigrateAggHashValue *hvalue;
	bool found;
	LWLock *partitionLock;
	uint32 hashcode;

	key.key1 = k1;
	key.key2 = k2;
	key.key3 = k3;

	/* determine its hash code and partition lock ID */
	hashcode = MigrateAggHashCode(&key);
	partitionLock = MigrateAggPartitionLock(hashcode);

	LWLockAcquire(partitionLock, LW_SHARED);

	hvalue = (MigrateAggHashValue *) hash_search_with_hash_value(MigrateAggHashTable,
							    				(void *)&key, hashcode, HASH_FIND, &found);

	if (found)
	{
		*hval = hvalue->migrateByte;
	}

	LWLockRelease(partitionLock);

	return found;
}

bool
migrateagghashtable_insert(uint32 k1, uint32 k2, uint32 k3, uint8 *hval, bool setMigrate)
{
	MigrateAggHashKey key;
	MigrateAggHashValue *hvalue;
	bool found;
	LWLock *partitionLock;
	uint32 hashcode;

	key.key1 = k1;
	key.key2 = k2;
	key.key3 = k3;

	/* determine its hash code and partition lock ID */
	hashcode = MigrateAggHashCode(&key);
	partitionLock = MigrateAggPartitionLock(hashcode);


	LWLockAcquire(partitionLock, LW_EXCLUSIVE);



	hvalue = (MigrateAggHashValue *) hash_search_with_hash_value(MigrateAggHashTable,
								    			(void *)&key, hashcode, HASH_ENTER, &found);

	if (found)
	{
		if (!setMigrate)
			*hval = hvalue->migrateByte;
		else
			hvalue->migrateByte = *hval;
	}
	else
		hvalue->migrateByte = *hval;

	LWLockRelease(partitionLock);

	return found;
}

bool
localagghashtable_ip_lookup(uint32 k1, uint32 k2, uint32 k3, uint8 *hval, uint8 id)
{
	LocalAggHashKey key;
	LocalAggHashValue *hvalue;
	bool found;

	key.key1 = k1;
	key.key2 = k2;
	key.key3 = k3;

	if (id == 0) {
		hvalue = (LocalAggHashValue *) hash_search(LocalIPAggHashTable0,
													(void *)&key, HASH_FIND, &found);
	} else {
		hvalue = (LocalAggHashValue *) hash_search(LocalIPAggHashTable1,
													(void *)&key, HASH_FIND, &found);
	}

	if (found)
		*hval = hvalue->migrateByte;


	return found;
}

void
localagghashtable_ip_insert(uint32 k1, uint32 k2, uint32 k3, uint8 hval, uint8 id)
{
	LocalAggHashKey key;
	LocalAggHashValue *hvalue;
	bool found;

	key.key1 = k1;
	key.key2 = k2;
	key.key3 = k3;

	if (id == 0) {
		hvalue = (LocalAggHashValue *) hash_search(LocalIPAggHashTable0,
													(void *)&key, HASH_ENTER, &found);
	} else {
		hvalue = (LocalAggHashValue *) hash_search(LocalIPAggHashTable1,
													(void *)&key, HASH_ENTER, &found);
	}

	hvalue->migrateByte = hval;
}
