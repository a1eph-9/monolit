# Monolit

Monolit is a simple, secure and useful commandline tool used for storing personal information like usernames and passwords.  
Useful for people who want a extremely lightweight and versatile info storage utility that can be used without a GUI.
It uses fully encrypted databases that can be used from anywhere without fear of security risks.
It currently only has support for linux systems.  

## Features
* Create, edit and securely erase MLDB databases
* Customisable random password generation
* Securely store personal information offline
* Keyfiles for added security

## How to install
__Dependencies:__
* make
* gcc
* shred
* libsodium
* libtomcrypt

__How to build:__
```
git clone https://github.com/a1eph-9/monolit.git
cd monolit
sudo make
```

After compiling to use the tool run the 'monolit' command
