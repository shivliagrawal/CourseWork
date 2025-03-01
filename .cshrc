# this file is processed on every csh invocation

# set PATH and MANPATH based on machine type
eval `/usr/local/bin/defaultpaths -csh`

# set default file/directory creation protection
umask 027

# skip the rest if the shell is non-interactive, i.e. is running a script
if ( ! $?prompt ) exit

set prompt = "`shorthostname` \! % "
set history = 50
set notify = on

alias mail mailx
