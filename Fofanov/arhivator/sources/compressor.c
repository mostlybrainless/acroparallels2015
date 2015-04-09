#include "../headers/compressor.h"

int my_compress(char* file_name)
{
	int status = 0;
	int fd = open(file_name, O_RDWR);
	if(fd == -1) {
		my_log(ERROR, "File %s can't be opened.", file_name);
		return 0;
	}

	struct stat file_data;
	status = stat(file_name, &file_data);
	if(status == 0) {
		my_log(MESSAGE, "Info about file %s was gotten success.", file_name);
	} else {
		my_log(ERROR, "Info about file %s wasn't gotten.", file_name);
		return 0;
	}

	void* memory_mapped_file = NULL;
	memory_mapped_file = mmap(NULL, (size_t)file_data.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if(memory_mapped_file == (void*)-1) {
		my_log(ERROR, "File %s wasn't mapped with error %d.", file_name, errno);
		return 0;
	} else {
		my_log(MESSAGE, "File %s was mapped success.", file_name);
	}

	ulong dest_lenght = (ulong)(file_data.st_size + ADD_BYTES);
	void* memory_mapped_buffer = calloc(dest_lenght, sizeof(char));
	
	if(memory_mapped_buffer == NULL) {
		my_log(ERROR, "Memory allocated error.");
		return 0;
	}

	status = compress((Bytef*)(memory_mapped_buffer), &dest_lenght, (Bytef*)(memory_mapped_file), (uLong)file_data.st_size);
	
	if(status == 0) {
		my_log(MESSAGE, "Data compress was made success.");
	} else {
		my_log(ERROR, "Data compress return error %d.", status);
	}

	status = munmap(memory_mapped_file, (size_t)file_data.st_size);
	
	if(status == -1) {
		my_log(ERROR, "Unmap return error %d", errno);
	}

	close(fd);	

	char old_file_name[NAME_SIZE] = {0};
	strcpy(old_file_name, file_name);
	char* new_file_name = strtok(old_file_name, ".");
	strcat(new_file_name, ".mgz");

	fd = open(new_file_name, O_RDWR|O_CREAT, S_IRWXU|S_IRWXG|S_IRWXO);
        if(fd == -1) {
                my_log(ERROR, "File %s can't be opened.", new_file_name);
                return 0;
        }

        write(fd, memory_mapped_buffer, dest_lenght);
        write(fd, (void*)&file_data.st_size, sizeof(int));
	close(fd);

	return 1;
}

int my_decompress(char* file_name)
{
	int status = 0;
	int fd = open(file_name, O_RDWR);
	if(fd == -1)
	{
		my_log(ERROR, "File %s can't be opened.", file_name);
		return 0;
	}

	struct stat file_data;
	status = stat(file_name, &file_data);
        if(status == 0) {
                my_log(MESSAGE, "Info about file %s was gotten success.", file_name);
        } else {
                my_log(ERROR, "Info about file %s wasn't gotten.", file_name);
                return 0;
        }

	void* memory_mapped_file = NULL;
	memory_mapped_file = mmap(NULL, (size_t)(file_data.st_size - sizeof(int)), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
        if(memory_mapped_file == (void*)-1) {
                my_log(ERROR, "File %s wasn't mapped with error %d.", file_name, errno);
                return 0;
        } else {
                my_log(MESSAGE, "File %s was mapped success.", file_name);
        }
	
	ulong dest_lenght = 0;
	lseek(fd, -(sizeof(int)), SEEK_END);
	read(fd, (void*)&dest_lenght, sizeof(int));

	void* memory_mapped_buffer = calloc(dest_lenght, sizeof(char));
        if(memory_mapped_buffer == NULL) {
                my_log(ERROR, "Memory allocated error.");
                return 0;
        }

	status = uncompress((Bytef*)memory_mapped_buffer, &dest_lenght, (Bytef*)memory_mapped_file, (ulong)(file_data.st_size - sizeof(int)));
        if(status == 0) {
                my_log(MESSAGE, "Data compress was made success.");
        } else {
                my_log(ERROR, "Data compress return error %d.", status);
        }

	status = munmap(memory_mapped_file, (size_t)(file_data.st_size - sizeof(int)));
        if(status == -1) {
                my_log(ERROR, "Unmap return error %d", errno);
        }

	close(fd);
	
        char old_file_name[NAME_SIZE] = {0};
        strcpy(old_file_name, file_name);
        char* new_file_name = strtok(file_name, ".");

	fd = open(new_file_name, O_RDWR|O_CREAT, S_IRWXU|S_IRWXG|S_IRWXO);
        if(fd == -1) {
                my_log(ERROR, "File %s can't be opened.", new_file_name);
                return 0;
        }

        write(fd, memory_mapped_buffer, dest_lenght);
	close(fd);	

	return 1;
}
