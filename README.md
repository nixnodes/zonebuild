# zonebuild

* Generate DNS zones from dn42 registry files

## Installation
* Prepare:

  ```sh
  apt-get update
  apt-get install bind9
  cd /etc/bind
  ```
* Clone:

  ```sh
  git clone git://github.com/nixnodes/zonebuild.git
  ```
  
* Compile and install zbuild:

 ```sh
 cd zonebuild
 make clean
 ./configure
 sudo make install
 ```
 
* Configure:
  
  `vi scripts/config.user`

  -
 
  ```
  REGISTRY_BASE_PATH=/etc/bind
  REGISTRY_PATH=${REGISTRY_BASE_PATH}/net.dn42.registry/data
  OUT_PATH=/etc/bind
  ```

  * Data is written in `$OUT_PATH/<tier[0-2]|res|ipv6>/`
  * Registry repo will be created in `$REGISTRY_BASE_PATH`
  
  -
  
  Default settings are stored in`scripts/config`, all changes should be put in `scripts/config.user`

## Run:

Execute with absolute path:

* Build reverse and forward tier1 and tier2 zones, self update before running:

  `/etc/bind/zonebuild/scripts/run.sh arpa zone -update`
  
  * include "$REGISTRY_BASE_PATH/tier(1|2)/named.conf";
  
* Build root zones (<a-z>.root-servers.dn42):
  
  `/etc/bind/zonebuild/scripts/run.sh root -update` 

  * include "$REGISTRY_BASE_PATH/tier0/named.conf";

* Build resolver files:
  
  `/etc/bind/zonebuild/scripts/run.sh res -update` 

  * include "$REGISTRY_BASE_PATH/res/named.conf";