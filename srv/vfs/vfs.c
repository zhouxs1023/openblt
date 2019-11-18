/* $Id: //depot/blt/srv/vfs/vfs.c#4 $
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

#include <blt/syscall.h>
#include <blt/namer.h>
#include <blt/error.h>
#include <blt/vfs.h>
#include <boot.h>
#include <string.h>

static char *init_message = "vfs: initialised";
static char *hello_message = "vfs: hello";
static char *error_message = "vfs: unknown command";

typedef struct
{
	char mount_point[MAX_DIR_LEN];
	int in_port, out_port;
} fs_conn;

fs_conn *mnted_fs[32];

int parse_command (char *ptr, char *type, char *dir, char *dev, char *options)
{
	int i;

	*dev = *dir = 0;
	if (!strncmp (ptr, "mount", 5))
		{
			ptr += 5;
			while (*ptr)
				{
					/* advance past the space */
					ptr++;

					if (!strncmp (ptr, "-t", 2))
						{
							ptr += 2;
							if (!*ptr)
								goto error;
							ptr++;
							i = 0;
							while ((*ptr != ' ') && (*ptr))
								{
									type[i++] = *ptr;
									ptr++;
								}
							type[i] = 0;
						}
					else if (!*dev)
						{
							i = 0;
							while ((*ptr != ' ') && (*ptr))
								{
									dev[i++] = *ptr;
									ptr++;
								}
							dev[i] = 0;
						}
					else if (!*dir)
						{
							i = 0;
							while ((*ptr != ' ') && (*ptr))
								{
									dir[i++] = *ptr;
									ptr++;
								}
							dir[i] = 0;
						}
				}
			if (*type && *dev && *dir)
				return VFS_MOUNT_MSG;
			else
				goto error;
		}
	else
		{
error:
			*type = *dir = *dev = *options = 0;
			return 0;
		}
}

int do_mount (char *type, char *dir, char *dev, char *options)
{
	int i;
	fs_conn *fs_ptr;

	if (!strcmp (type, "bootfs"))
		{
			/* we handle this one ourself */
			fs_ptr = (fs_conn *) malloc (sizeof (fs_conn));
			strcpy (fs_ptr->mount_point, dir);
			fs_ptr->in_port = fs_ptr->out_port = 0;
		}

	/* add the filesystem to the mount table */
	i = 0;
	while ((mnted_fs[i]) && (i < 32))
		i++;
	if (i < 32)
		{
			mnted_fs[i] = fs_ptr;
			return 1;
		}
	else
		{
			free (fs_ptr);
			return 0;
		}
}

int main (void)
{
	char *ptr, in_buf[80], out_buf[80], type[80], dir[80], dev[80], options[80];
	int cons_port, cons_pub_port, vfs_port, nh, i, boot_fs_len;
	boot_dir *boot_fs;
	msg_hdr_t msg_mh, mh;
	vfs_msg_t msg;

	/* open a connection to the console */
	nh = namer_newhandle ();
	while ((cons_pub_port = namer_find (nh, "console")) < 1)
		os_sleep(10);
	cons_port = port_create (cons_pub_port);

	/* say hello */
	msg_mh.src = nh;
	msg_mh.dst = cons_pub_port;
	msg_mh.data = init_message;
	msg_mh.size = strlen (init_message);
	(void) port_send (&msg_mh);

	/* get a public port and register ourself with the namer */
	vfs_port = port_create (0);
	(void) namer_register (nh, vfs_port, "vfs");

	/* find the location of the boot filesystem. */
	boot_fs = (boot_dir *) get_bootfs ();
	for (i = 0, boot_fs_len = 0; (boot_fs->bd_entry[i].be_type != BE_TYPE_NONE);
			i++)
		boot_fs_len += boot_fs->bd_entry[i].be_size;
	boot_fs_len *= 0x1000;

	/* initialise the array of mounted filesystems */
	for (i = 0; i < 32; i++)
		mnted_fs[i] = NULL;

	msg_mh.data = hello_message;
	msg_mh.size = strlen (hello_message);
	snprintf (out_buf, 80, "vfs: bootfs is at %x, length %x\n",
		(unsigned int) boot_fs, boot_fs_len);
	msg_mh.data = out_buf;
	msg_mh.size = strlen (out_buf);
	port_send (&msg_mh);

	/* main loop */
	for (;;)
		{
			mh.src = 0;
			mh.dst = vfs_port;
			mh.data = in_buf;
			mh.size = sizeof (in_buf);
			port_recv (&mh);
/*
			snprintf (out_buf, 80, "vfs: got message\n");
			msg_mh.data = out_buf;
			msg_mh.size = strlen (out_buf);
			port_send (&msg_mh);
*/
			switch (parse_command (in_buf, type, dir, dev, options))
				{
					case VFS_MOUNT_MSG:
						if (do_mount (type, dir, dev, options))
							snprintf (out_buf, 80, "vfs: mounted type %s from %s at %s\n",
								type, dev, dir);
						else
							snprintf (out_buf, 80, "vfs: mount failed\n");
						msg_mh.data = out_buf;
						msg_mh.size = strlen (out_buf);
						port_send (&msg_mh);
							break;
					default:
						snprintf (out_buf, 80, "vfs: error\n");
						msg_mh.data = out_buf;
						msg_mh.size = strlen (out_buf);
						port_send (&msg_mh);
						break;
				}
	}

	/* not reached */
	return 0;
}

