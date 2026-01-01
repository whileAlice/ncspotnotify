# ncspotnotify

<img
  width="400"
  height="142"
  alt="image"
  src="https://github.com/user-attachments/assets/ab0672bc-ce88-4153-9556-b15b763fe290"
/>

It's a desktop notifier for [ncspot](https://github.com/hrkfdn/ncspot),
which uses [dunst](https://github.com/dunst-project/dunst)'s `dunstify`.

There's still a bit to go, but there's more done than not. It's rough
around the edges and might need some tinkering to make it work elsewhere.

## Compiling

Just `make`.

By default, it will use clang. I haven't tested it much yet with other
compilers.

It also needs `libcurl-devel` or your distro's equivalent.

## Usage

Run `ncspotnotify`.

If you want to see what it's doing, there are  `--verbose` and `--debug`
flags (the latter is automatically added with `make run`).

## Features

So far, it polls ncspot's UNIX socket (the path is hardcoded in `config.h`
at the moment), parses the JSON and builds a notification out of it,
including the album cover downloaded with `libcurl` and progress bar.

Right now, only `dunstify` is hardcoded as the default notifier.
I plan to make it more flexible in the future and include an ncspot
controller as well.

It's multithreaded mostly because I think it's neat.

Press `ctrl+c` to quit.
