# Termux Battery Checkup

this just depends on termux-api (specifically termux-battery-status).

It reads the command (termux battery), process it with sed, save the result to cache file, and prints it.

If the cache time is still valid, it only reads the cache file.

## Building and compiling

You need to have Termux and Termux:Api installed. Then you can start doing:

```sh
make
```

You also need working gcc and make. The program is saved as `main` so execute it and/or saves it to your bin path.

## License

Licensed in MIT
