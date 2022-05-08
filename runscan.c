#include <stdio.h>
#include "ext2_fs.h"
#include "read_ext2.h"
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

int digitCounter(int count) {
	int numDigits = 1;
	while (count != 0) {
		numDigits++;
		count /= 10;
	}
	return numDigits;
}

int makeFile(char* p, int digits, int node) {
	size_t nameLength = (strlen(p) + digits + 12);
	char* filename = malloc(nameLength) ;
	snprintf(filename, nameLength, "%s/file-%u.jpg", p, node);
	int file_ptr = open(filename, O_WRONLY | O_TRUNC | O_CREAT, 0600);
	return file_ptr;
}

int main(int argc, char **argv) {

    	if (argc != 3) {
		printf("expected usage: ./runscan inputfile outputfile\n");
		exit(0);
	}

	// Check args and create output directory
	DIR *dir = opendir(argv[2]);
	if (dir != NULL) {
		//ERROR - already exists
		printf("ERROR\n");
		exit(1);
	}
	mkdir(argv[2], 0725); // Creates the new output directory

	// Opens disk images
	int fd;
	fd = open(argv[1], O_RDONLY);

	ext2_read_init(fd); // Prints info about the blocks and nodes in input_file


	struct ext2_super_block super;
	struct ext2_group_desc group;

	read_super_block(fd, 0, &super); //prints uper block info
	read_group_desc(fd, 0, &group); //prints group desc info

	int isJPG[inodes_per_group];

	for (uint y = 0; y < inodes_per_group; y++) {
		isJPG[y] = 0;
	}

	//printf("There are %u inodes in an inode table block and %u blocks in the idnode table\n", inodes_per_block, itable_blocks);
	//iterate the first inode block
	off_t start_inode_table = locate_inode_table(0, &group);
    for (unsigned int i = 0; i < inodes_per_group; i++) { //inodes_per_group

            //printf("inode %u: \n", i);
            struct ext2_inode *inode = malloc(sizeof(struct ext2_inode));
            read_inode(fd, 0, start_inode_table, i, inode);
	    /* the maximum index of the i_block array should be computed from i_blocks / ((1024<<s_log_block_size)/512)
			 * or once simplified, i_blocks/(2<<s_log_block_size)
			 * https://www.nongnu.org/ext2-doc/ext2.html#i-blocks
			 */
			// unsigned int i_blocks = inode->i_blocks/(2<<super.s_log_block_size);
            //printf("number of blocks %u\n", i_blocks);
            /*printf("Is directory? %s \n Is Regular file? %s\n",
               S_ISDIR(inode->i_mode) ? "true" : "false",
               S_ISREG(inode->i_mode) ? "true" : "false");*/
			

			// check the first block of every node to see if it is a jpg
			char buffer[1024];
			lseek(fd,BLOCK_OFFSET(inode->i_block[0]), SEEK_SET);
			read(fd,buffer,1024);

			//check for jpg numbers
			int is_jpg = 0;
			if (buffer[0] == (char)0xff && buffer[1] == (char)0xd8 && buffer[2] == (char)0xff &&
					(buffer[3] == (char)0xe0 || buffer[3] == (char)0xe1 || buffer[3] == (char)0xe8)) {
				is_jpg = 1;
			}

			if (is_jpg == 1) {
				// sizing for multi-digit inode nums
				int numDigits = digitCounter(i);

				int file_ptr = makeFile(argv[2], numDigits, i);

				// And a copy to rename later
				size_t newNameLength = (strlen(argv[2]) + numDigits + 12);
				char* newFilename = malloc(newNameLength);
				snprintf(newFilename, newNameLength, "%s/file-%uex.jpg", argv[2], i);
				int new_file_ptr = open(newFilename, O_WRONLY | O_TRUNC | O_CREAT, 0600);

				// Get filesize from the inode
				uint filesize = inode->i_size; 
				
				uint bytes_read;
				uint block_id;
				for (bytes_read = 0, block_id = 0; bytes_read < filesize && bytes_read < block_size * EXT2_NDIR_BLOCKS; bytes_read = bytes_read + block_size) {
					
					uint j;
					uint y;
					uint x;
					x = filesize - bytes_read;
					y = block_size;
					if (x>y) {
						j = y;
					} else { j = x;}
					write(file_ptr, buffer, j);
					write(new_file_ptr, buffer, j);
					uint fileReadBytes = bytes_read + j;
					int endFile = fileReadBytes >= filesize;
					if (!endFile) {
						block_id++;
						lseek(fd,BLOCK_OFFSET(inode->i_block[block_id]), SEEK_SET);
						read(fd,buffer,block_size);
					}
				}


				if (bytes_read < filesize) {

					uint ind_buffer[block_size / sizeof(uint)];

					lseek(fd,BLOCK_OFFSET(inode->i_block[EXT2_IND_BLOCK]), SEEK_SET);
					read(fd,ind_buffer,block_size);

					lseek(fd,BLOCK_OFFSET(ind_buffer[0]), SEEK_SET);
					read(fd,buffer,block_size);

					for (block_id = 0; bytes_read < filesize && block_id < (block_size / sizeof(uint)); bytes_read = bytes_read + block_size) {

						uint j;
						uint x;
						uint y;
						x = filesize - bytes_read;
						y = block_size;
						if (x>y) {
							j = y;
						} else { j = x;}

						write(file_ptr, buffer, j);
						write(new_file_ptr, buffer, j);

						uint fileReadBytes = bytes_read + j;
						int endFile = fileReadBytes >= filesize;
						if (!endFile) {
							block_id++;
							lseek(fd,BLOCK_OFFSET(ind_buffer[block_id]), SEEK_SET);
							read(fd,buffer,block_size);
						}
					}
				} 

				if (bytes_read < filesize) { // Double Indirect Points
					
					uint first_buffer[block_size / sizeof(uint)];
					uint second_buffer[block_size / sizeof(uint)];

					lseek(fd,BLOCK_OFFSET(inode->i_block[EXT2_DIND_BLOCK]), SEEK_SET);
					read(fd,first_buffer,block_size);

					uint first_layer_id;
					for (first_layer_id = 0; first_layer_id < (block_size / sizeof(int)) && bytes_read < filesize; first_layer_id++) {
						lseek(fd,BLOCK_OFFSET(first_buffer[first_layer_id]), SEEK_SET);
						read(fd,second_buffer,block_size);
						lseek(fd,BLOCK_OFFSET(second_buffer[0]), SEEK_SET);
						read(fd,buffer,block_size);

						for (block_id = 0; bytes_read < filesize && block_id < (block_size / sizeof(int)); bytes_read = bytes_read + block_size) {
							uint j, x, y;
							x = filesize - bytes_read;
							y = block_size;
							if (x>y) {
								j = y;
							} else { j = x;}

							write(file_ptr, buffer, j);
							write(new_file_ptr, buffer, j);

							uint fileReadBytes = bytes_read + j;
							int endFile = fileReadBytes >= filesize;
							if (!endFile) {
								block_id++;
								lseek(fd,BLOCK_OFFSET(second_buffer[block_id]), SEEK_SET);
								read(fd,buffer,block_size);
							}
						}
					}
				}

				isJPG[i] = 42;
			}
            free(inode);
        }
	

	start_inode_table = locate_inode_table(0, &group);
    for (unsigned int i = 0; i < inodes_per_group; i++) {

		struct ext2_inode *inode = malloc(sizeof(struct ext2_inode));
		read_inode(fd, 0, start_inode_table, i, inode);
		char dir_buffer[block_size];
		if (S_ISDIR(inode->i_mode)) { // inode is a directory
				lseek(fd,BLOCK_OFFSET(inode->i_block[0]), SEEK_SET);
				read(fd,dir_buffer,block_size);
				uint curr_offset = 24;
				struct ext2_dir_entry_2* dentry = (struct ext2_dir_entry_2*) & (dir_buffer[curr_offset]);
				for (; curr_offset < block_size; dentry = (struct ext2_dir_entry_2*) & (dir_buffer[curr_offset])) {
				
					int name_len = dentry->name_len & 0xFF;
					if (name_len <= 0) break;
					else {
						char name [EXT2_NAME_LEN];
						strncpy(name, dentry->name, name_len);
						name[name_len] = '\0';


						struct ext2_inode *curr_inode = malloc(sizeof(struct ext2_inode));
						read_inode(fd, 0, start_inode_table, i, curr_inode);

						if (isJPG[dentry->inode] == 42) {
							int numDigits = digitCounter(dentry->inode);

							int directory_chars = (strlen(argv[2]) + 1);
						
							int oldname_chars = (directory_chars + numDigits + 12);
							size_t oldname_len = oldname_chars * sizeof(char);
							char* oldname = malloc(oldname_len);
							snprintf(oldname, oldname_len, "%s/file-%uex.jpg", argv[2], dentry->inode);

							int newname_chars = directory_chars + dentry->name_len +1;
							size_t newname_len = newname_chars * sizeof(char);
							char*newname = malloc(newname_len);
							snprintf(newname, newname_len, "%s/%s", argv[2], dentry->name);

							rename(oldname, newname);
						}

						curr_offset = curr_offset + name_len + sizeof(dentry->inode) + sizeof(dentry->rec_len) + sizeof(dentry->name_len) + sizeof(dentry->file_type);

						// byte aligment
						if (curr_offset % 4 != 0) {
							curr_offset = curr_offset + (4 - (curr_offset % 4));
						}
					}

				}
			}
		}
}
