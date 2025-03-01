# this file is processed only when zsh is running as a login (top-level) shell
# and should contain commands to be run once per session, e.g. setting
# environment variables and terminal-specific settings

# set PATH and MANPATH based on machine type
eval `/usr/local/bin/defaultpaths`

# set default file/directory creation protection
umask 027

[ -f ${ZDOTDIR:-$HOME}/.zshrc ] && . ${ZDOTDIR:-$HOME}/.zshrc

EDITOR=vi; export EDITOR
PAGER=less; export PAGER
