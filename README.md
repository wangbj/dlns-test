# dlns-test
a test case for linker namespace

run.c: an launcher to inject DSOs.
preload.c: a mini loader using LD_PRELOAD -> dlmopen
dlns.c: DSO being `dlmopen`-ed
app.c: app being injected with DSOs.

running `make test` should reproduce the issue. (need to run `ulimit -c unlimited` to get core dump).
