#include "../headers/compressor.h"

int main(int argc, char** argv)
{
	my_log_init(MESSAGE, "logs.txt");
	if(argc < 2)
	{
		my_log(ERROR, "Input file name hasn't been found.");
		return 0;
	}

	char file_name[256] = {0};
	strcpy(file_name, argv[1]);

	char* file_type = NULL;
	char dot = '.';
	file_type = strtok(argv[1], &dot);

	if(strcmp(file_type, file_name))
	{
		file_type = strtok(NULL, &dot);
	}

	int rezult = 0;
	if(strcmp(file_type, "mgz"))
	{
		rezult = my_compress(file_name);
		my_log(MESSAGE, "File %s was compressed with result: %s", file_name, rezult == 0 ? "failed" : "success");
	}
	else
	{
		rezult = my_decompress(file_name);
		my_log(MESSAGE, "File %s was decompressed with result: %s", file_name, rezult == 0 ? "failed" : "success");
	}

	my_log_end();
	return 0;
}
