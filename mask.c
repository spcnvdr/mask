/*****************************************************************************
 * Copyright 2020 spcnvdr <spcnvdrr@protonmail.com>                          *
 *                                                                           *
 * Redistribution and use in source and binary forms, with or without        *
 * modification, are permitted provided that the following conditions        *
 * are met:                                                                  *
 *                                                                           *
 * 1. Redistributions of source code must retain the above copyright notice, *
 * this list of conditions and the following disclaimer.                     *
 *                                                                           *
 * 2. Redistributions in binary form must reproduce the above copyright      *
 * notice, this list of conditions and the following disclaimer in the       *
 * documentation and/or other materials provided with the distribution.      *
 *                                                                           *
 * 3. Neither the name of the copyright holder nor the names of its          *
 * contributors may be used to endorse or promote products derived from      *
 * this software without specific prior written permission.                  *
 *                                                                           *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS       *
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT         *
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR     *
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT      *
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,    *
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED  *
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR    *
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF    *
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING      *
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS        *
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.              *
 *                                                                           *
 * A program that calculates information for a subnet given either an IP     *
 * address in CIDR notation or an IP address and subnet mask.                *
 *****************************************************************************/
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


/** Convert a string in decimal to long
 *  @param str the string to convert
 *  @returns the number as a long integer
 *
 */
long str_to_long(const char *str){
	char *endptr;
	long result;

	errno = 0;
	result = strtol(str, &endptr, 10);

	if((errno == ERANGE && (result == LONG_MAX || result == LONG_MIN))
			|| (errno != 0 && result == 0)){
		perror("strtol");
		exit(EXIT_FAILURE);
	}

	if(endptr == str){
		fprintf(stderr, "strtol: No digits were found\n");
		exit(EXIT_FAILURE);
	}

	return(result);
}


/** Print class of an IPv4 address
 *  @param addr ip address as an unsigned integer
 */
void print_class(uint32_t addr){
	uint32_t ip = ntohl(addr);

	printf("%-12s ", "Class:");
    if(ip >= 0xf0000000 && ip <= 0xfffffffe){
		printf("Class E");
	}
	else if(ip >= 0xe0000000 && ip <= 0xefffffff){
		printf("Class D, Multicast");
	}
	else if(ip >= 0xc0000000 && ip <= 0xdfffffff){
		printf("Class C");

		if(ip >= 0xc0000200 && ip <= 0xc00002ff){
			printf(", TEST-NET-1");
		}
		else if(ip >= 0xc6336400 && ip <= 0xc63364ff){
			printf(", TEST-NET-2");
		}
		else if(ip >= 0xcb007100 && ip <= 0xcb0071ff){
			printf(", TEST-NET-3");
		}
		else if(ip >= 0xc0a80000 && ip <= 0xc0a8ffff){
		    	printf(", Private");
		    }
	}
	else if(ip >= 0x80000000 && ip <= 0xbfffffff){
		printf("Class B");

		if(ip >= 0xac100000 && ip <= 0xac1fffff){
		    	printf(", Private");
		    }
		else if(ip >= 0xa9fe0000 && ip <= 0xa9feffff){
	    	printf(", APIPA");
	    }
	}
    /* ip >= 0x00000000 && ip <= 0x7fffffff */
	else if(ip <= 0x7fffffff){
		printf("Class A");

	    if(ip >= 0x0a000000 && ip <= 0x0affffff){
	    	printf(", Private");
	    }
	    else if(ip >= 0x7f000000 && ip <= 0x7f1fffff){
	    	printf(", Loopback");
	    }
	    else if(ip <= 0x00ffffff){
	    	/* ip >= 0x00000000 && ip <= 0x00ffffff*/
	    	printf(", Local");
	    }
	}
	else{
		printf("Other");
	}
    putchar('\n');
}

/** Determine if subnet mask is valid
 *  @param netmask the subnet mask as an unsigned int (big-endian)
 *  @returns 1 (true) is valid, else 0
 *
 */
int is_valid_mask(uint32_t netmask){
	uint32_t i, j, t;
	t = ntohl(netmask);
	i = ~t;
	j = i+1;
	return(((i&j) == 0));
}


/** Determine if an IPv4 address is valid
 *  @param ip the IPv4 address to check
 *  @returns 1 if valid, 0 if invalid
 */
int is_valid_ip(const char *ip){
	size_t i;
	long j;
	char *temp, *nip;

	if(ip == NULL)
		return(0);

	/* Create a copy so as not to modify original string */
	if((nip = strdup(ip)) == NULL){
		perror("strdup");
		exit(EXIT_FAILURE);
	}


	/* Make sure only digits are in the IP address */
	for(i = 0; i < strlen(nip); i++){
		if(ip[i] != '.' && !isdigit(ip[i])){
			free(nip);
			return(0);
		}
	}

	i = 0;
	temp = strtok(nip, ".");

	/* Check that all octets are a valid number */
	if(temp == NULL){
		return(0);
	} else {
		while(temp){
			i++;
			j = str_to_long(temp);
			if(j > 255 || j < 0){
				free(nip);
				return(0);
			}

			temp = strtok(NULL, ".");
		}
	}

	/* Make sure there are 4 octets */
	if(i != 4){
		free(nip);
		return(0);
	}

	free(nip);
	return(1);
}


/** Splits an IP address in CIDR notation to IP and net bits
 *  @param cidr is a string IP in CIDR notation, e.g. 192.168.1.1/24
 *  @param net an argument to hold the number of net bits .e.g 24
 *  @returns The IP address portion of the CIDR IP
 *
 */
char *split_cidr(char* cidr, char **net){
	char *ip  = strtok(cidr, "/");
	*net = strtok(NULL, "-");
	return(ip);
}


/** Increment an IP address by 1
 *  @param ip the IP address to increment
 *  @returns the incremented IP in big endian form (network)
 *
 */
uint32_t increment_ip(uint32_t ip){
	uint32_t temp = ntohl(ip);
	temp++;
	return(htonl(temp));
}


/** Increment an IP address by 1
 *  @param ip the IP address to increment
 *  @returns the incremented IP in big endian form (network)
 *
 */
uint32_t decrement_ip(uint32_t ip){
	uint32_t temp = ntohl(ip);
	temp--;
	return(htonl(temp));
}


/** Convert an IP address in integer format to a string
 *  @param ip the IP ass an integer
 *  @returns a dynamically allocated string representation of the IP address
 *  NOTE: It is the caller's responsibility to free the buffer after use!
 *
 */
char *ip_to_str(uint32_t ip){
	char *ipstr;
	struct in_addr ipaddr;

	if((ipstr = malloc(20 * sizeof(char))) == NULL){
		perror("malloc");
		exit(EXIT_FAILURE);
	}
	ipaddr.s_addr = ip;
	strncpy(ipstr, inet_ntoa(ipaddr), 20);
	return(ipstr);

}


/** Convert an IPv4 address in a string to an unsigned integer
 *  @param addr the IPv4 address to convert
 *  @returns the IP address as a 32-bit unsigned integer in big endian
 *  (network byte-order)
 *
 */
uint32_t str_to_ip(const char *addr){
	struct sockaddr_in sa;

	// store the IP address in sa:
	inet_pton(AF_INET, addr, &(sa.sin_addr));
	uint32_t ip = sa.sin_addr.s_addr;
	return(ip);
}


/** Convert a CIDR value from string to unsigned integer
 *  @param cidr the value to convert e.g. "24"
 *  @returns the CIDR number as a uint64_t or exit on failure
 *
 */
uint32_t cidr_to_int(const char *cidr){
	char *endptr;
	unsigned long result;
	errno = 0;

	result = strtoul(cidr, &endptr, 10);

	/* If there was an error converting cidr to a unsigned long, bail */
	if((errno == ERANGE && (result == ULONG_MAX))
	          || (errno != 0 && result == 0)){
		perror("strtoul");
		exit(EXIT_FAILURE);
	}
	if(result < 1 || result > 32){
		fprintf(stderr, "Error: Invalid CIDR value (1-32)\n");
		exit(EXIT_FAILURE);
	}

	return(result);
}


/** Calculate the subnet mask from a CIDR number
 *  @param cidr the  CIDR number, e.g. 24, valid numbers are from 1 to 32
 *  @returns the subnet mask as an unsigned integer in network byte-order
 *
 */
uint32_t cidr_to_netmask(const char *cidr){
	uint32_t net;

	net = cidr_to_int(cidr);
	uint32_t subnet = (0xFFFFFFFF << (32 - net)) & 0xFFFFFFFF;
	subnet = htonl(subnet);
	return(subnet);

}


/** Calculate a wildcard mask from a subnet mask
 *  @param netmask the subnet mask of the network
 *  @returns the wildcard mask as an unsigned integer
 *
 */
uint32_t netmask_to_wildcard(uint32_t netmask){
	uint32_t wildcard = 0xffffffff & (~netmask);
	return(wildcard);
}


/** Calculate the CIDR number from a netmask
 *  @param netmask the subnet mask
 *  @returns the CIDR number for a given subnet mask
 *
 */
uint32_t netmask_to_cidr(uint32_t netmask){
	uint32_t count = 0;

	while(netmask){
		netmask &= netmask - 1;
		++count;
	}

	return(count);
}


/** Calculate the First IP address in a subnet
 *  @param ip the IP address in the given subnet
 *  @param netmask the subnet mask of the network
 *  @returns the first IP address as an unsigned integer
 *
 */
uint32_t first_ip(uint32_t ip, uint32_t netmask){
	uint32_t first = ip & netmask;
	return(first);

}


/** Calculate the last IP address in a subnet
 *  @param ip the IP address in the given subnet
 *  @param netmask the subnet mask of the network
 *  @returns the last IP address as an unsigned integer
 *
 */
uint32_t last_ip(uint32_t ip, uint32_t netmask){
	uint32_t last = 0xffffffff & (ip | (~netmask));
	return(last);

}


/** Calculate the number of IP addresses in a subnet from CIDR
 *  @param cidr the subnet mask in CIDR notation (valid 1-32)
 *  @param returns number of IP addresses in a subnet
 *
 */
uint32_t total_addrs(uint32_t cidr){
	uint32_t host, addrs = 1;
	host = 32 - cidr;

	while(host != 0){
		addrs *= 2;
		host--;
	}

	return(addrs);

}


/** Print information from an address in CIDR notation
 *  @param addr the ip address and subnet mask in CIDR (e.g. 192.168.1.0/24)
 *
 */
void print_from_cidr(char *addr){
	char *ip, *wildcard, *net = NULL;
	char *fstr, *lstr, *fustr, *lustr, *nstr;
	uint32_t ipaddr, netmask, first, last;
	uint32_t fuse, luse, total;


	ip = split_cidr(addr, &net);

	/* Check IP and CIDR number */
	if(net == NULL){
		fprintf(stderr, "Error: Missing CIDR or subnet mask!\n");
		exit(EXIT_FAILURE);
	}

	if(!is_valid_ip(ip)){
		fprintf(stderr, "Error: Invalid IP address!\n");
		exit(EXIT_FAILURE);
	}

	ipaddr = str_to_ip(ip);
	netmask = cidr_to_netmask(net);
	wildcard = ip_to_str(netmask_to_wildcard(netmask));
	total = total_addrs(cidr_to_int(net));

	/* Subtract two addresses for the network and broadcast */
	total -=2 ;

	first = first_ip(ipaddr, netmask);
	last = last_ip(ipaddr, netmask);
	fuse = increment_ip(first);
	luse = decrement_ip(last);

	nstr = ip_to_str(netmask);
	fstr = ip_to_str(first);
	lstr = ip_to_str(last);
	fustr = ip_to_str(fuse);
	lustr = ip_to_str(luse);

	printf("%-12s %s\n", "IP Address:", ip);
	printf("%-12s %s = %s\n", "Subnet:", nstr, net);
	printf("%-12s %s\n", "Wildcard:", wildcard);
	printf("%-12s %s -> %s\n", "IP Range:", fstr, lstr);
	printf("%-12s %s\n", "Host Min:", fustr);
	printf("%-12s %s\n", "Host Max:", lustr);
	printf("%-12s %u\n", "Hosts:", total);
	print_class(ipaddr);


	free(nstr);
	free(wildcard);
	free(fstr);
	free(lstr);
	free(fustr);
	free(lustr);

}


/** Print information from an address in CIDR notation
 *  @param addr the ip address
 *  @param subnet subnet mask
 *
 */
void print_from_netmask(char *addr, char *subnet){
	char *wildcard;
	char *fstr, *lstr, *fustr, *lustr, *nstr;
	uint32_t ipaddr, netmask, first, last, cidr;
	uint32_t fuse, luse, total;


	/* Check IP and CIDR number */
	if(subnet == NULL || !is_valid_ip(subnet)){
		fprintf(stderr, "Error: Missing subnet mask!\n");
		exit(EXIT_FAILURE);
	}

	if(!is_valid_ip(addr)){
		fprintf(stderr, "Error: Invalid IP address!\n");
		exit(EXIT_FAILURE);
	}

	ipaddr = str_to_ip(addr);
	netmask = str_to_ip(subnet);

	if(!is_valid_ip(subnet) || !is_valid_mask(netmask)){
		fprintf(stderr, "Error: Invalid subnet mask!\n");
		exit(EXIT_FAILURE);
	}

	cidr = netmask_to_cidr(netmask);
	wildcard = ip_to_str(netmask_to_wildcard(netmask));
	total = total_addrs(cidr);

	/* Subtract two addresses for the network and broadcast */
	total -=2 ;

	first = first_ip(ipaddr, netmask);
	last = last_ip(ipaddr, netmask);
	fuse = increment_ip(first);
	luse = decrement_ip(last);

	nstr = ip_to_str(netmask);
	fstr = ip_to_str(first);
	lstr = ip_to_str(last);
	fustr = ip_to_str(fuse);
	lustr = ip_to_str(luse);

	printf("%-12s %s\n", "IP Address:", addr);
	printf("%-12s %s = %u\n", "Subnet:", nstr, cidr);
	printf("%-12s %s\n", "Wildcard:", wildcard);
	printf("%-12s %s -> %s\n", "IP Range:", fstr, lstr);
	printf("%-12s %s\n", "Host Min:", fustr);
	printf("%-12s %s\n", "Host Max:", lustr);
	printf("%-12s %u\n", "Hosts:", total);
	print_class(ipaddr);


	free(nstr);
	free(wildcard);
	free(fstr);
	free(lstr);
	free(fustr);
	free(lustr);
}


/** Print a banner because its cool
 *
 */
void banner(void){
	puts("                     _    ");
	puts(" _ __ ___   __ _ ___| | __");
	puts("| '_ ` _ \\ / _` / __| |/ /");
	puts("| | | | | | (_| \\__ \\   < ");
	puts("|_| |_| |_|\\__,_|___/_|\\_\\");
	puts("");

}


/** Print help information
 *
 */
void usage(void){
	fprintf(stderr, "Usage: mask ADDRESS/CIDR\n");
	fprintf(stderr, "  or: mask ADDRESS SUBNET_MASK\n");
	fprintf(stderr, "Calculate statistics about an IPv4 subnet.\n\n");
	fprintf(stderr, "e.g. mask 192.168.1.3/24\n");
	fprintf(stderr, "  or: mask 192.168.1.3 255.255.255.0\n");
	fprintf(stderr, "\n");
	exit(EXIT_FAILURE);
}


int main(int argc, char *argv[]){
	int opt;
	char *ip, *nm;

	/* Handle -h and -? arguments and future arguments*/
	while((opt = getopt(argc, argv, "h?")) != -1){
		switch(opt){
			case '?':
				/* Fall through */
			case 'h':
				/* Fall through */
			default:
				usage();
				break;
		}
	}

	ip = argv[optind];

	/* No args were entered */
	if(ip == NULL)
		usage();

	banner();

	if(argc == 3){
		nm = argv[++optind];
		print_from_netmask(ip, nm);
	}
	else{
		print_from_cidr(ip);
	}

	return(EXIT_SUCCESS);
}
