git-lard
========

This is a native rewrite of [git-fat](https://github.com/jedbrown/git-fat)
utility. In short, it is used to keep large binary files out of git
repositories. For a more detailed description consult git-fat documentation.

git-lard vs git-fat
-------------------

The main downside of git-fat is its speed. Initial checkout of a couple
thousand binary files can take hours, especially on Windows machines. git-fat
is written in python and interacts with git using git's command line
interface. This requires spawning multiple processes and parsing text data
sent through pipes.

In contrast, git-lard is written in C/C++ and can use git internal structures
to get the needed data without any unnecessary processing. All is done in a
single process. The only external binary being run is rsync.

The aforementioned initial checkout takes only a few seconds in git-lard, the
limiting factor being I/O throughput.
