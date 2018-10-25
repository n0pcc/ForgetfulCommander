#!/usr/bin/env python2

import lief
import os
import subprocess

subprocess.call(["clang", "-s", "-m32", "-nostdlib", "-nodefaultlibs", "-fPIC", "-Wl,-shared", "entry.c", "-oentry_tmp"])
subprocess.call(["clang", "-s", "-m32", "-pie", "forgetful_commander.c", "-oforgetful_commander_tmp"])

entry = lief.parse("entry_tmp")
forgetful_commander = lief.parse("forgetful_commander_tmp")

segment_added = forgetful_commander.add(entry.segments[1])

forgetful_commander.header.entrypoint = segment_added.physical_address

# Encrypt exec segment
content = forgetful_commander.segments[3].content

clr = [0, 0, 0, 0]
flux = [0x78, 0x75, 0x6c, 0x46]
for i in xrange(0, len(content), 4):
	tmp = content[i:i+4]
	content[i] ^= flux[0] ^ clr[0]
	content[i+1] ^= flux[1] ^ clr[1]
	content[i+2] ^= flux[2] ^ clr[2]
	content[i+3] ^= flux[3] ^ clr[3]
	flux.append(flux.pop(0))
	clr = tmp

forgetful_commander.segments[3].content = content
forgetful_commander.segments[3].add(lief.ELF.SEGMENT_FLAGS.W)

# Encrypt ro segment
content = forgetful_commander.segments[4].content

clr = [0, 0, 0, 0]
flux = [0x78, 0x75, 0x6c, 0x46]
for i in xrange(0, len(content), 4):
	tmp = content[i:i+4]
	content[i] ^= flux[0] ^ clr[0]
	content[i+1] ^= flux[1] ^ clr[1]
	content[i+2] ^= flux[2] ^ clr[2]
	content[i+3] ^= flux[3] ^ clr[3]
	flux.append(flux.pop(0))
	clr = tmp

forgetful_commander.segments[4].content = content
forgetful_commander.segments[4].add(lief.ELF.SEGMENT_FLAGS.W)

forgetful_commander.write("public/forgetful_commander")

os.remove("entry_tmp")
os.remove("forgetful_commander_tmp")
