# openblt
OpenBLT: An Overview
by Brian J. Swetland
OpenBLT is a microkernel operating system for PC's based on the Intel 80386 or later CPUs. Some aspects of OpenBLT were inspired by Andy Valencia's VSTa OS, Mach, assorted UNIX variants, and the author's deranged imagination. The acronym BLT expands to "Brian's Lightweight Tasker" or "Bacon, Lettuce, and Threads". The word Open was tacked on for no particular reason.

The OpenBLT kernel is responsible for managing system memory, providing IPC facilities, and creation, destruction, and scheduling threads of control.

Below, we'll examine Tasks, Aspaces, and Ports (the basic objects managed by the OpenBLT kernel) in terms of their use in and appearance to userland programs.

Tasks

Tasks in OpenBLT are single threads of control. Each task is associated with an address space, within which it runs. Tasks may terminate themselves using the

void  os_terminate(int status);
system call. A task may start another task (sharing the same address space) using the
int   os_thread(void *addr);
system call. The new task begins execution at the specified address and a 4K stack is created for it. [ os_thread() will probably take a stack pointer address in the next revision ]
Address Spaces

Address spaces are currently not readily manipulated from userland. In the near future, shared memory system calls should allow the creation of new address spaces and sharing memory between address spaces. A thread may request that its address space be expanded using the os_brk() system call:

int   os_brk(int addr);
Ports

IPC (interprocess communication) in OpenBLT occurs when one task sends a message to another task via a port. Ports are 'owned' by one task (the task that created them). Messages are send from one port to another port (thus a port must exist for the sending task to send from). A port may be restricted so that it can only receive messages from one specified other port [ future versions will likely allow a list of allowed senders ].

All messages sent or received via an OpenBLT port are described with a message header. The header indicates the source port, destination port, size of message, pointer to message data (or receive data buffer), and flags (to allow for future expansion -- non-blocking IO, scatter/gather IO, etc)

typedef struct {
    int flags;
    int src;
    int dst;
    int size;
    void *data;    
} msg_hdr_t;
int port_create(uint32 restrict);
Create a new port owned by the running thread. Only allow messages from port 'restrict' to be received. If restrict is 0, messages from any source will be received. Will return ERR_MEMORY if the system lacks the resources to create the port, otherwise the port number will be returned.

int port_destroy(uint32 port);
Destroy an existing port. Returns ERR_RESOURCE if port is not a valid port or if there are other ports slaved to this port. Returns ERR_PERMISSION if the running thread is not the owner of this port. Returns ERR_NONE upon success/

int port_send(msg_hdr_t *mh);
Send the message in mh->data, of length mh->size from port mh->src to port mh->dst. Returns mh->size upon success, ERR_MEMORY if there is a bounds error, ERR_RESOURCE if either the source or destination ports do not exist, ERR_PERMISSION if the source is not owned by the running thread or the destination is not sendable from the source port.

int port_recv(msg_hdr_t *mh);
Receive a message (in buffer mh->data, max length mh->size) from port mh->dst. Upon success, mh->src will be set to the sender, mh->dst will be set to the destination if it was a slaved port, and the number of received bytes will be returned. If the data or header are out of bounds, ERR_MEMORY is returned. If the destination port does not exist ERR_RESOURCE is returned. If the running thread does not own the destination port, ERR_PERMISSION is returned. This call will block if no messages are available, unless the port is set to NOWAIT, in which case ERR_WOULDBLOCK is returned.

int port_option(uint32 port, int opt, int arg);
Modify port options. The two functions below are wrappers around port_option()

int port_slave(uint32 master, uint32 slave);
Cause the master port to receive all messages sent to the slave port. If master is 0, the slave is released from bondage. Returns ERR_NONE upon success, ERR_PERMISSION if the master and slave are not both owned by the running thread, or ERR_RESOURCE if the master or slave are not valid ports.

int port_set_restrict(uint32 port, uint32 restrict);
Change the restriction on a port. Returns ERR_NONE on success, ERR_RESOURCE if the port does not exits, or ERR_PERMISSION if the running thread does not own the port. A restriction of 0 allows any port to send to this port.

Finding other tasks to talk to

Since port numbers are generated dynamically by the kernel and tend to be different depending on the order tasks are started, etc, there needs to be a mechanism for locating device drivers or just other tasks to talk to. The kernel automatically creates port 1 (the uberport) and gives ownership of this port to the first task created. This task is the namer, a service that allows drivers to register ports under specific names and allows tasks that wish to find these drivers to look up the port that is associated with a specific name.

The utility functions listed below are part of the OpenBLT libc and provide convenient ways to access the namer.

int namer_newhandle(void);
int namer_delhandle(int nh);
int namer_register(int nh, int port, char *name);
int namer_find(int nh, char *name);
Connections between Connectionless ports

Ports in OpenBLT are connectionless. Each port_send() specifies a source and destination port. Tasks may send from any port they own to any port they are allowed to write to. Ports may be writable to be any other port or restricted to just one other port. [ multiple port restrict lists will be in a later version ]

The general procedure for providing a service in OpenBLT is

Create an unrestricted port
Register this port with the namer under a descriptive name (eg ne2000, console, etc)
When one task (a client) wishes to connect to a service (the server), the following occurs:
The client looks up the servers unrestricted port using the namer
The client creates a port that is restricted to the server's unrestricted port
The client sends a message to the servers unrestricted port requesting a connection
If the server accepts the connection, it will send a response back to the client's port notifying it that the connection is accepted and possibly providing a different port than the unrestricted port that the client should send future messages too.
[ a toolkit in libc will streamline this process ]
Handling Devices

The security model in OpenBLT is still being designed. Most likely there will be a task that is a "security manager" which will be able to give permissions (for accession IO devices, etc) to tasks that need them. The two syscalls described below are sufficient for simple device drivers in the initial (overly trusting) version of OpenBLT:

void os_handle_irq(int irq);
Ask the kernel to make this task eligible to process the specified IRQ. The kernel will also allow the task IO access.

void os_sleep_irq(void);
Suspend this task until the IRQ it is registered to handle is triggered. When that IRQ is triggered, the task will be scheduled (preempting the currently running task). The IRQ will be ignored by the kernel when the task is no sleeping in os_sleep_irq().

Startup
On startup, OpenBLT creates initializes its internal memory and resource management, sets up kernel space at the 2gb line (0x8000000), creates an address space and task for each userland program that it can find. An idle task is created (to give the scheduler something to schedule, should all other tasks be sleeping on something). The kernel schedules the first task in the run queue to execute and the fun starts.
