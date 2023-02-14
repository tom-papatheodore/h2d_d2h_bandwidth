## H2D-D2H Bandwidth

This test measures host-to-device and device-to-host bandwidth using 50 iterations of `hipMemcpy` in each direction. 

### Build

To build on Crusher or Frontier:
```text
$ source setup_environment.sh
$ make
```

### Usage

```text
$ ./bandwidth --help
----------------------------------------------------------------
Usage: ./bandwidth [OPTIONS]

--loop_count=<value>,  -l:       Iterations of timed bw loop
                                 <value> should be an integer > 1
                                 (default is 50)

--help,                -h:       Show help
----------------------------------------------------------------
```
