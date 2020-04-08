# Mask - A IPv4 subnet calculator

Mask is an IPv4 subnet calculator that prints statistics about 
a subnet given an IP addess in CIDR notation or an IP address 
and subnet mask. It will print the subnet mask and CIDR number,
wildcard mask, the range of IP addresses, the first useable adress,
the last usable address, the number of usable IP addresses, and 
the class of IP address.  


**Getting Started**

Get a copy of the source code and change into the mask directory.

    cd mask

From the source directory, simply run make to build the program

    make

Then run the program with the -h or -? options for help.

    ./mask -h

An example of using mask to calculate statistics about a subnet

    ./mask 192.168.1.1/24

An alternative is to provide the subnet mask instead of a CIDR 
number

    ./mask 192.168.1.1 255.255.255.0



**To-Do**

- [ ] Add an option to print IP addresses in binary 
- [ ] Add some color to the output


**License**

This project is licensed under the 3-Clause BSD License also known as the
*"New BSD License"* or the *"Modified BSD License"*. A copy of the license
can be found in the LICENSE file. A copy can also be found at the
[Open Source Institute](https://opensource.org/licenses/BSD-3-Clause)
