#!/bin/sh

eval `/usr/local/bin/defaultpaths -sh`

eval `xrdb -global -symbols | sed -ne '/=/!d;s/^-D//;p'`

host=`/usr/local/bin/shorthostname | sed -e 'h;s/.//;x;s/\(.\).*/\1/;y/abcdefghijklmnopqrstuvwxyz/ABCDEFGHIJKLMNOPQRSTUVWXYZ/;G;s/\n//'`

# For OpenWindows apps
if [ -d /usr/openwin ]; then
	OPENWINHOME=/usr/openwin; export OPENWINHOME
	HELPPATH=$OPENWINHOME/lib/help; export HELPPATH
	PATH=${PATH}:/usr/openwin/bin
fi

# load .Xresources and .Xdefaults into the X resource database
for i in $HOME/.Xresources $HOME/.Xdefaults; do
	[ -f $i ] && xrdb -merge $i
done

# do these in a subshell so they don't become zombies on exit
(
	xscreensaver &
	fvwm &				# window manager
	xterm -geometry 80x24-0+257 -T "$host" -n "$host" &
)

# only grab the actual console when we're running on a local display
[ "$SERVERHOST" != "lore.cs.purdue.edu" -a "$SERVERHOST" = "$CLIENTHOST" ] && consflag=-C
# kill this xterm to end your session
exec xterm $consflag -geometry 80x14-0+15 -T console -n console -fg darkslategray -bg burlywood3
