
# ghi

  Install a project on GitHib by running its `make install` script.  Basically, clib-install(1) without the `package.json` requirement.

## Installation

  Expects [libcurl](http://curl.haxx.se/libcurl/) to be installed and linkable.

  With [clib](https://github.com/clibs/clib), do:

    $ clib install stephenmathieson/ghi

  With git, do:

    $ git clone https://github.com/stephenmathieson/ghi.git /tmp/ghi
    $ cd /tmp/ghi
    $ make && [sudo] make install

## Usage

```
$ ./ghi --help

  Usage: ghi [options] <repo ...>

  Options:

    -V, --version                 output program version
    -h, --help                    output help information
    -o, --out <dir>               set the download directory
    -s, --show-output             show install output
    -c, --command <command>       use a custom command
```

## Examples

  You may install a repo at a specific version, branch, etc:

```sh
$ [sudo] ./ghi visionmedia/git-extras@1.6.0
       fetch : https://github.com/visionmedia/git-extras/archive/1.6.0.tar.gz
       untar : /tmp/git-extras-visionmedia.tar.gz
   installed : visionmedia/git-extras@1.6.0
```

  Installing multiple repos will kick off a thread for each.  This makes installs a bit faster:

```sh
$ ./ghi stephenmathieson/git-{standup,sync,upstream} visionmedia/watch -o ./repos
       fetch : https://github.com/stephenmathieson/git-standup/archive/master.tar.gz
       fetch : https://github.com/stephenmathieson/git-sync/archive/master.tar.gz
       fetch : https://github.com/stephenmathieson/git-upstream/archive/master.tar.gz
       fetch : https://github.com/visionmedia/watch/archive/master.tar.gz
       untar : ./repos/git-sync-stephenmathieson.tar.gz
   installed : stephenmathieson/git-sync
       untar : ./repos/watch-visionmedia.tar.gz
       untar : ./repos/git-standup-stephenmathieson.tar.gz
       untar : ./repos/git-upstream-stephenmathieson.tar.gz
   installed : stephenmathieson/git-standup
   installed : stephenmathieson/git-upstream
   installed : visionmedia/watch

```

  Install someuser/somerepo using command `./configure && make && make install`:

```sh
$ ./ghi someuser/somerepo --command "./configure && make && make install"
  [ ... ]
```

## Updating

  ghi can install itself, so for updates, just run `ghi stephenmathieson/ghi` ;)

## License

  MIT