/* $Id: //depot/blt/include/blt/vfs.h#2 $
**
** Copyright 1998 Sidney Cammeresi
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions, and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions, and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _BLT_VFS_H_
#define _BLT_VFS_H_

#include <blt/types.h>

#define MAX_TYPE_LEN      32
#define MAX_DIR_LEN       255

#define VFS_MOUNT_MSG     1


typedef struct
{
} bootfs_mount_params;

union mount_params
{
	bootfs_mount_params bootfs;
};

typedef struct
{
	char type[MAX_TYPE_LEN], dir[MAX_DIR_LEN];
	int flags;
	union mount_params params;
} mount_msg_t;

typedef struct
{
	char dir[MAX_DIR_LEN];
	int flags;
} unmount_msg_t;

typedef struct
{
	int type;
	union
	{
		mount_msg_t mount_msg;
		unmount_msg_t unmount_msg;
	} data;
} vfs_msg_t;

/* wrappers to ipc */

typedef struct
{
	void *data;
	size_t size;
	char *type, *name;
} *handle_t;

typedef struct
{
} res_info;

/* these work on both forks */
int open (const char *path, int flags); /* returns an fd */
int close (int fd);
int creat (const char *path, const char *creator, const char *type);

/* these are fork-specific */
int new_fork (int fd, const char *name);
int num_forks (int fd);
char *get_fork_name (int fd, int fork);
int read_fork (int fd, int fork, void *buf, size_t numbytes);
int write_fork (int fd, int fork, const void *buf, size_t numbytes);

/* these don't */
int df_creat (int fd);
size_t df_read (int fd, void *buf, size_t numbytes);
size_t df_write (int fd, const void *buf, size_t numbytes);

int rf_creat (int fd);
int rf_num_types (int fd);
const char *rf_get_type (int fd, int type_index);
/*res_info *rf_get_resource_info (int fd, char *type, char *name);
int rf_set_resource_info (handle_t h, res_info *info);*/
handle_t rf_get_indexed_resource (int fd, int index);
handle_t rf_get_named_resource (int fd, char *type, char *name);
int rf_res_size (handle_t h);
void *rf_get_ptr (handle_t h);
int rf_changed_resource (handle_t h);
handle_t rf_new_resource (char *type, char *name, size_t size);
int rf_add_resource (int fd, handle_t h);
int rf_remove_resource (handle_t h);
void rf_update (int fd);
void rf_update_all (void);
int rf_write_resource (handle_t h);

/* wrappers to df_* for ease of user */
size_t read (int fd, void *buf, size_t numbytes);
size_t write (int fd, const void *buf, size_t numbytes);

/* directory operations */
int opendir (const char *path); /* returns a dd */
int closedir (int dd);
int searchdir (int dd, const char *creator, const char *type);
int mkdir (const char *path);

#endif

