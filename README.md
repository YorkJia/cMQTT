A groovy mqtt client library in ANSI C
======================================
Overview
========
cMQTT is a free software library to implement the MQTT Client.

The functions included in the library have been derived from the MQTT Protocol Reference Guide which can be obtained from [link](https://mqtt.org)

The license of cMQTT is LGPL v2.1 or later.

The library design refer to Aliyun IOT SDK,so it is very stable.

The library is written in C and designed to run on Linux, Mac OS X, FreeBSD, ARM embed platform and Windows.

Installation
============
you can compile the library or direct put the source code into your project.

If you want to compile the library,just run the usual dance,

1.  ./configure.sh

2.	make isntall

the default compile is gcc, if your platform need cross compile(example: arm-linux-gnueabihf),just run:

1.	./confgure.sh arm-linux-gnueabihf

2.	make install

After compile the bibrary, in current director,will create libcMQTT director,you need copy `libcMQTT.so` to 
your platform(ex. linux platform is /usr/lib)

Testing
=======
Some tests are provided in test directory, you can freely edit the source code to fit your needs.

See test/README for a description of each program.

For a quick test of cMQTT, you can run the following programs in two shells:

./single-client
./multi-client

Attention, before run the programs, you need start a MQTT broker, the example programs configure params:

username: admin
password: password

host: 127.0.0.1
port: 1883



