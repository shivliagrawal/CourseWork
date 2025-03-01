# this file is processed on each invocation of ksh

# skip the rest if the shell is non-interactive, i.e. is running a script
[[ "$-" != *i* ]] && return

PS1="`shorthostname` ! $ "
HISTSIZE=50
HISTFILE=$HOME/.sh_history	# pdksh doesn't set this by default

alias mail=mailx
