## production jffs2 file system 

 ### arm-linx-gcc version 4.3.2 

 ### Compile busybox
   - busybox version [busybox-1.20.0][1]
   - modify busybox Makfile 

``` protobuf
   ARCH  ?= arm
   CROSS_COMPILE ?= arm-linux-   
```
   - make  
   - `make CONFIG_PREFIX=dir_path_install` 

### glibc lib 

 - ...

### structure file system 

 - ...

  [1]: https://busybox.net/
