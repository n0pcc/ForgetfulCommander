Forgetful Commander
===================

# Author
n0p

# Description
All typos in the flag are on purpose (brute-force prevention...)!


# Challenge Text
```
And you lost a key again. This time it's the key to your missiles command 
station. However, the command station got a silent password override. Even 
though your memory isn't that good, you don't remember the password either, 
your technical skills are! You've already dumped the binary, which checks the 
password. Now you just have to reverse it!
```

# Setup
packer.py builds everything. Requires clang.

# Solution
Simple packer. Main program checks the argument against some XOR stuff. 
Depending on the trap flag, the argument is checked against the fail string.

# Point Value
dynamic

# Flag
```
flag{Just_type__Please__and_the_missles_will_be_launched.}
```
