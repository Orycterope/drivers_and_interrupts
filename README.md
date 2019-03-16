# PS/2 Keyboard driver

Re-implementation of the PS/2 keyboard driver for the linux kernel.

#### Goals

The main goals of this school project is to find out how to write your own device driver for the linux kernel, 
figure out how to handle interrupts, and interact with a port-mapped device controller.

#### Keylogger

Call it malicious, our driver logs every key pressed/released by the user in a linked list, 
and reports them in `/dev/keylogger`.

#### Interrupt handling

We divide the interrupt handling in a top and bottom half. 

The top half reads the scankey from the keyboard controller, gets the time of the event, and returns.
As it runs in interrupt context, it must be fast and is very restricted in what it can do.

The bottom half, which has no such constraints, gets the event generated by the top half, 
translates it to the key which was pressed/released, formats the time in a readable format,
and adds it to the linked list of every logged key event.

#### Accessing the log

When a read is made to `/dev/keylogger`, the driver copies the contents of its linked list 
to the user-provided buffer. 

Since the file will often be accessed by multiple readers at the same time,
we must make sure that our driver implements correct locking mechanisms around the linked list,
even in the interrupt handler.

##### Final grade

**111 / 100**