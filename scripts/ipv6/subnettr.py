#!/usr/bin/env python3

import csv,hashlib,os,time

#### Subnettr by Xuu <jon@xuu.cc>
#
#  Subnettr is a script for generating dns records suitable for use in BIND9. 
#  It will attempt to create delegate as much as possible to smaller subnets for reverse dns.
#  It does this by using RFC2317 CNAME records. 
#
#  The following script is used to generate the input csv files that the script consumes. 
#  In this example i have a symlink to the data directory of the mtn repo. 
#
#  Modify to your own purposes. 
#
# ~~~
#  #!/bin/bash
#  cd ~/ddns/data
#  mtn sync mtn://mtn.xuu.dn42
#  mtn update
#  REVISION=`mtn status|grep Revision|cut -d ' ' -f 2`
#
#  cd ~/ddns/
#  grep nserver data/inet6num/* | sed -E 's_:nserver:[\ \t]+_,_' | sed -E 's_[\ \t]+_,_' | sed s/_/,/ | sed s_data/inet6num/__ > inet6.txt
#  grep nserver data/inetnum/* | sed -E 's_:nserver:[\ \t]+_,_' | sed -E 's_[\ \t]+_,_' | sed s/_/,/ | sed s_data/inetnum/__ > inet.txt
#  grep nserver data/dns/* | sed -E 's_:nserver:[\ \t]+_,_' | sed -E 's_[\ \t]+_,_' | sed s/_/,/ | sed s_data/dns/__ > dns.txt
#
#  export SUBNETTR_CONTACT=xuu.dn42.us
#  export SUBNETTR_PERSON=XUU-DN42
#  export SUBNETTR_PRIMARY='souris.root.dn42'
#  export SUBNETTR_REVISION=$REVISION
#
#  ./subnettr.py
#
#  sudo service bind9 reload
# ~~~
#
####

version=os.getenv('SUBNETTR_VERSION', '0.0')
contact=os.getenv('SUBNETTR_CONTACT', 'dummy.example.com')
primary=os.getenv('SUBNETTR_PRIMARY', 'dummy.root.dn42')
person=os.getenv('SUBNETTR_PERSON', 'DUMMY-DN42')
revision=os.getenv('SUBNETTR_REVISION', '0000000000000000000000000000000000000000')

UTC_DT = int(time.time())

def is_ipv4(address):
    parts = address.split(".")
    if len(parts) != 4:
        return False
    for item in parts:
        try:
          i = int(item)
        except ValueError:
          return False
        if not 0 <= i <= 255:
            return False
    return True

def is_ipv6(address):
   has_empty = False
   num_part = 0
   last = None
   for i in address.lower():
     if i in '0123456789abcdef': 
       pass
     elif i == ':':
       if last == ':':
         if has_empty:
           return False
         has_empty = True
     else: 
       return False
     last = i
   return True

def toNum (ip):
  ip = [int(i) for i in ip.split('.')]
  return ip[3] + ip[2] * 256 + ip[1] * 256**2 + ip[0] * 256**3 

def toIP (num):
  return num >> 24, (num >> 16)&0xFF, (num >> 8)&0xFF, num&0xFF

def print_header(file, time, contact, primary, person, f):
  print('; File: db.' + file, file=f)
  print('$TTL 300', file=f)
  print('{file}. IN SOA {primary}. {contact}. ({time} 14400 3600 1209600 300)'.format(file=file, time=time, contact=contact, primary=primary), file=f)
  print('@ IN TXT ver={0},person={1},rev={2},ts={3}'.format(version, person, revision, UTC_DT), file=f)


def print_glue(ns, addr, f):
    if is_ipv6(addr):
      print('{0} IN AAAA {1}'.format(ns, addr), file=f)
    elif is_ipv4(addr):
      print('{0} IN A {1}'.format(ns, addr), file=f)

def print_ns(ns, addr, f):
      if is_ipv6(ns):
        print('{0} IN NS {1} ; Invalid Record'.format(addr, dummy(ns)), file=f)
        print_glue(dummy(ns), ns, f)
          
      elif is_ipv4(ns):
        print('{0} IN NS {1} ; Invalid Record'.format(addr, dummy(ns)), file=f)
        print_glue(dummy(ns), ns, f)

      else:
        print('{0} IN NS {1}'.format(addr, ns), file=f)

def dummy(addr):
  return 'DUMMY'+hashlib.sha1(addr.encode()).hexdigest()[:8]


# Build the reverse dns with subnet delegation.

# Build the v6 reverse dns
DNS = {} 
INET6 = {}

with open('inet6.txt') as f:
  def expand_ipv6(addr, length):
    if "::" in addr:
        addr = addr.replace('::', ':' * (9 - addr.count(':')))
    if addr.count(':') != 7 or length % 4 != 0:
        return False
    return ''.join((i.zfill(4) for i in addr.split(":")))[0 : int(length/4)]

  r = csv.reader(f)
  for l in r:
    inet = tuple([i for i in expand_ipv6(l[0], int(l[1]))])
    file='.'.join(inet[:2][::-1]) + '.ip6.arpa'

    d = l[2]
    if is_ipv4(d) or is_ipv6(d):
      pass
    else:
      d = d+'.'

    if file not in INET6:
      INET6[file] = {}

    if inet not in INET6[file]:
      INET6[file][inet] = set()

    INET6[file][inet].add(d)

for file, inet in INET6.items():
  with open('db.'+file,'w') as f:
    print_header(file, UTC_DT, contact, primary, person, f)
    for k, v in inet.items():
      for ns in v:
        print_ns(ns, '.'.join(k[::-1]) + '.ip6.arpa.', f)


