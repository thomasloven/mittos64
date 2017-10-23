#MITTOS64

##Building
This project is set up to be built inside a Docker container.

The container can be built by running the `d` script in the projects root directory - provided Docker is installed on your computer.

When the docker container is built, commands can be run inside it through the `d` script. The script will run any arguments passed to it as commands inside the container.

E.g:

    d ls

If a previous command is running, the next command will be run inside the same container. Otherwise a new container will be started.


## Generating and iso file
A cdrom ISO file can be built by the command

    d mkiso

## Running the emulator
The project can be run in a qemu virtual machine inside the docker container by the command

    d emul

## Running the debugger
The gdb debugger can be run inside the docker container by the command

    d dbg

This assumes that a qemu session is already running.

### Debugger commands
Gdb is configured with a few special commands

    (gdb) q

Stops the qemu emulator and closes the debugger.

    (gdb) reset

Reboots the qemu virtual machine.

    (gdb) reg

Displays information about cpu register contents from the emulator. Note that this is slightly different from the built-in gdb command `info registers`.


### Bypassing the docker container
Most scripts and makefiles in the project will check for wether they are run inside the container by looking for the environment variable $MITTOS64. If not present, execution will stop.

If you know what you're doing, this check can be bypassed by defining the environment variable MITTOS64. I really don't recommend it, though.
