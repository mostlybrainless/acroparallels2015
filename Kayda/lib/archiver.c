


struct frame_flags{
	uint8_t 	blksize;
};


struct __attribute__((__packed__)) frame_header{
	uint32_t 			magic;
	struct frame_flags 	flags;
	uint64_t 			content_size;
	uint16_t 			file_mode;
	uint8_t 			checksum;
};



int archMain(struct arch_state st)
{
	Log(LOG_INFO, "Input \"%s\", output \"%s\"\n", st.finname, st.foutname);

	int ret;
	int findesc = -1;
	st.foutdesc = -1;
	st.finmap = (void*) -1;

	/* prepare files */
	findesc = open(st.finname, O_RDONLY);
	if (findesc == -1){
		ERROR("Failed to open input file: %s\n", strerror(errno));
		ret = 1;
		goto main_free;
	}

	struct stat filestat;
	if (fstat(findesc, &filestat) == -1){
		ERROR("Failed fstat: %s\n", strerror(errno));
		ret = 1;
		goto main_free;
	}
	st.finsize = filestat.st_size;
	st.finmode = filestat.st_mode & 07777; 	// except file type bits

	st.finmap = mmap(NULL, st.finsize, PROT_READ, MAP_SHARED, findesc, 0);
	if (st.finmap == (void *) -1){
		ERROR("Failed mmap: %s\n", strerror(errno));
		ret = 1;
		goto main_free;
	}

	/* create output file with default permissons */
	st.foutdesc = open(st.foutname, O_RDWR | O_CREAT | O_TRUNC, 0600);
	if (st.foutdesc == -1){
		ERROR("Failed to open output file: %s\n", strerror(errno));
		ret = 1;
		goto main_free;
	}

	if (st.action == ARCH_CREATE)
		ret = archCompress(st);
	else if (st.action == ARCH_EXTRACT)
		ret = archDecompress(st);

	if (ret != 0)
		if (unlink(st.foutname) == -1){
			ERROR("Failed to remove output file \"%s\": %s\n", 
					strerror(errno), st.foutname);
		}

	/* clean */
main_free:

	if (st.finmap != (void*) -1)
		munmap(st.finmap, st.finsize);
	if (findesc != -1)
		close(findesc);
	if (st.foutdesc != -1)
		close(st.foutdesc);

	return ret;
}


size_t getBlockSize(uint8_t blksize)
{
	return 1 << (blksize * 2 + 8);
}


int archCompress(struct arch_state st)
{
	struct frame_header head;
	XXH32_state_t hash_state;
	size_t size, block_size, buffer_size, read_size;
	void *buffer = NULL;
	size_t pos = 0;
	uint32_t seq_num = 0;
	uint32_t checksum;
	size_t out_size = 0;
	int ret;

	Log(LOG_FLOW, "archCompress() started\n");

	block_size = getBlockSize(st.block_size_id);
	buffer_size = LZ4_compressBound(block_size);
	buffer = (char*) malloc(buffer_size);
	if (!buffer){
		ERROR("Allocation error: %s\n", strerror(errno));
		return 1;
	}
	Log(LOG_INFO, "block_size=%u, buffer_size=%u\n", block_size, buffer_size);
	
	/* fill a frame header */
	head.magic = MAGIC_HEADER;
	head.flags.blksize = st.block_size_id;
	head.content_size = st.finsize;
	head.file_mode = st.finmode;
	head.checksum = (XXH32(&head, sizeof(head) - 1, 0) >> 8) & 0xFF;
	write(st.foutdesc, &head, sizeof(head));
	out_size += sizeof(head);

	XXH32_reset(&hash_state, 0);

	/* fill data blocks */
	while (pos < st.finsize){
		read_size = (block_size + pos < st.finsize) ? (block_size) : (st.finsize - pos);
		size = LZ4_compress(st.finmap + pos, buffer, read_size);
		XXH32_update(&hash_state, st.finmap + pos, read_size);
		write(st.foutdesc, &size, 4);
		write(st.foutdesc, &seq_num, 2);
		write(st.foutdesc, buffer, size);

		checksum = XXH32(buffer, size, 0);
		write(st.foutdesc, &checksum, 4);
		out_size += 4 + 2 + size + 4;

		Log(LOG_FLOW, "block #%u block_comp_size=%u, chksum=0x%X\n", 
			seq_num, size, checksum);

		seq_num++;
		pos += read_size;
	}
	/* last empty data block */
	size = 0;
	write(st.foutdesc, &size, 4);
	out_size += 4;

	/* write hash of raw input file */
	checksum = XXH32_digest(&hash_state);
	write(st.foutdesc, &checksum, 4);
	out_size += 4;
	st.foutsize = out_size;

	Log(LOG_FLOW, "frame_checksum=0x%X\n", checksum);
	Log(LOG_INFO, "Finish compression. %ubytes -> %ubytes. Wrote %u blocks.\n",
		st.finsize, out_size, seq_num);

	free(buffer);

	printf("File \"%s\" compressed successfully. %lu -> %lu bytes. Compress ratio = %.2f\n", 
			st.finname, st.finsize, st.foutsize, ((float)st.finsize)/st.foutsize);

	Log(LOG_FLOW, "archCompress() exit with success\n");

	return 0;
}


int archDecompress(struct arch_state st)
{
	struct frame_header head;
	XXH32_state_t hash_state;
	void *buffer = NULL;
	void *src_read, *foutmap;
	size_t size, block_size, out_size;
	uint32_t seq_num = 0;
	uint32_t checksum, written_checksum;
	int ret;

	Log(LOG_FLOW, "archDecompress() started\n");

	/* read frame header */
	src_read = st.finmap;
	memcpy(&head, src_read, sizeof(head));
	checksum = (XXH32(&head, sizeof(head) - 1, 0) >> 8) & 0xFF;
	if (checksum != head.checksum){
		Log(LOG_ERROR, "Frame header checksum: written=0x%X, computed=0x%X\n",
			head.checksum, checksum);
		ret = 1;
		goto decomp_clean;
	}
	src_read += sizeof(head);
	st.foutsize = head.content_size;

	block_size = getBlockSize(head.flags.blksize);
	buffer = malloc(block_size);
	if (!buffer){
		ERROR("Allocation error: %s\n", strerror(errno));
		ret = 1;
		goto decomp_clean;
	}

	Log(LOG_FLOW, "frame header magic=0x%X\n", head.magic);
	Log(LOG_FLOW, "frame header chksum=0x%X\n", head.checksum);
	Log(LOG_INFO, "comp_size=%u, content_size=%u, block_size=%u\n", 
		st.finsize, head.content_size, block_size);

	/* set output file size and mmap it to memory */
	if (ftruncate(st.foutdesc, st.foutsize)){
		ERROR("Fail ftrunk output file: %s\n", strerror(errno));
		ret = 1;
		goto decomp_clean;
	}
	foutmap = mmap(NULL, st.foutsize, PROT_READ | PROT_WRITE, 
				   MAP_SHARED, st.foutdesc, 0);
	if (foutmap == (void *) -1){
		ERROR("Fail mmap output file: %s\n", strerror(errno));
		ret = 1;
		goto decomp_clean;
	}

	XXH32_reset(&hash_state, 0);

	/* read data blocks */
	while (1){
		memcpy(&size, src_read, 4);
		src_read += 4;
		/* chack for last empty block */
		if (size == 0)
			break;
		memcpy(&seq_num, src_read, 2);
		src_read += 2;

		out_size = LZ4_decompress_safe(src_read, foutmap + seq_num*block_size, 
									   size, block_size);
		checksum = XXH32(src_read, size, 0);
		src_read += size;
		XXH32_update(&hash_state, foutmap + seq_num*block_size, out_size);

		memcpy(&written_checksum, src_read, 4);
		src_read += 4;
		if (checksum != written_checksum){
			ERROR("Block #%u checksum mismatch: written=0x%X, computed=0x%X\n",
				  seq_num, written_checksum, checksum);
			ret = 1;
			goto decomp_clean;
		}

		Log(LOG_FLOW, "block #%u, comp_size=%u, chksum=0x%X\n", 
			seq_num, size, checksum);
	}
	/* check hash of decompressed file */
	memcpy(&written_checksum, src_read, 4);
	src_read += 4;
	checksum = XXH32_digest(&hash_state);
	if (checksum != written_checksum){
		ERROR("Content checksum mismatch: written=0x%X, computed=0x%X\n",
			written_checksum, checksum);
		ret = 1;
		goto decomp_clean;
	}
	Log(LOG_FLOW, "content_checksum=0x%X\n", checksum);

	/* set saved permissions */
	if (fchmod(st.foutdesc, head.file_mode) != 0)
		ERROR("Failed setting permissions: %s\n", strerror(errno));

	printf("File \"%s\" decompressed successfully. %lu -> %lu bytes.\n", 
			st.finname, st.finsize, st.foutsize);

	Log(LOG_FLOW, "archDecompress() exit with success\n", 1);
	ret = 0;

decomp_clean:

	free(buffer);
	if (foutmap != (void *) -1)
		munmap(foutmap, st.foutsize);

	return ret;
}

