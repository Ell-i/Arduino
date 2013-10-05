#! /usr/bin/env python
#
# Copyright (c) 2013 Antti Louko.
#

import sys
import getopt
import os
import serial
import select
import time
import operator
import struct
import intelhex

def usage(utyp, *msg):
    sys.stderr.write('''Usage: %s [-hbrwAN] /dev/ttyUSB0
    -b rate       serial speed in bits/s
    -r filename   Read flash to ihex file
    -w filename   Write file to flash
    -A 0x........ Start address (for reading, write honours ihex file)
    -N npages     Number of KiBs to read
\n''' % os.path.split(sys.argv[0])[1])
    if msg:
        sys.stderr.write('Error: %s\n' % `msg`)
    sys.exit(1)

def i2nbytes(i,nbytes):
    return struct.pack('>Q',i)[-nbytes:]

class Global:
    def __init__(self):
        self.filename = None
        self.speed = 57600
        self.op = None
        self.addr = 0x08000000
        self.npages = 1
        self.connect = False

        return

    def read_n(self,ser,toread=1):
        nread = 0
        a = []
        while nread < toread:
            rin = [ser.fd]
            iwtd, owtd, ewtd = select.select(rin,[],[],2.0)
            nn = 0
            if ser.fd in iwtd:
                nn = nn+1
                n = min(toread-nread,ser.inWaiting())
                try:
                    d = ser.read(n)
                except OSError:
                    break
                if d:
                    a.append(d)
                    nread += len(d)
            if not nn:
                break
        return ''.join(a)

    ACK = '\x79'
    NACK = '\x1f'

    CMD_GET = 0x00
    CMD_GETVERSION = 0x01
    CMD_GETID = 0x02
    CMD_READMEM = 0x11
    CMD_GO = 0x21
    CMD_WRITEMEM = 0x31
    CMD_ERASE = 0x43
    CMD_EERASE = 0x44
    CMD_WRPROTECT = 0x63
    CMD_WRUNPROTECT = 0x73
    CMD_RDPROTECT = 0x82
    CMD_RDUNPROTECT = 0x92

    def cksum(self,data,xtra=0xff):
        return xtra ^ reduce(operator.xor,[ord(i) for i in data])

    def sendcksum(self,ser,bytes,xtra=0xff,v=False):
        dsend = '%s%c' % (bytes,self.cksum(bytes,xtra))
        if v: self.ferr.write('sendcksum %s\n' % (repr(dsend),))
        ser.write(dsend)
        return

    def check_ack(self,ser,rr=[None]):
        ack = self.read_n(ser,1)
        if ack == self.NACK:
            r = 'NACK'
        elif ack != self.ACK:
            r = 'Not ACK'
        else:
            r = None
        if r:
            rr[0] = r
            return False
        else:
            return True

    def cmd_get(self,ser):
        rr = [None]
        self.sendcksum(ser,chr(self.CMD_GET))
        if not self.check_ack(ser,rr):
            return False,rr[0]

        nbytes = self.read_n(ser,1)
        nbytes = ord(nbytes)+1
        d = self.read_n(ser,nbytes)

        if not self.check_ack(ser,rr):
            return False,rr[0],d

        return True,d

    def cmd_get_version(self,ser):
        rr = [None]
        self.sendcksum(ser,chr(self.CMD_GETVERSION))
        if not self.check_ack(ser,rr):
            return False,rr[0]

        d = self.read_n(ser,3)

        if not self.check_ack(ser,rr):
            return False,rr[0],d

        return True,d

    def cmd_get_id(self,ser):
        rr = [None]
        self.sendcksum(ser,chr(self.CMD_GETID))
        if not self.check_ack(ser,rr):
            return False,rr[0]

        nbytes = self.read_n(ser,1)
        nbytes = ord(nbytes)+1
        d = self.read_n(ser,nbytes)

        if not self.check_ack(ser,rr):
            return False,rr[0],d

        return True,d

    def cmd_read_mem(self,ser,addr,dsize):
        rr = [None]
        self.sendcksum(ser,chr(self.CMD_READMEM))
        if not self.check_ack(ser,rr):
            return False,rr[0],1

        self.sendcksum(ser,i2nbytes(addr,4),0x00)
        if not self.check_ack(ser,rr):
            return False,rr[0],2

        self.sendcksum(ser,i2nbytes(dsize-1,1),0xff)
        if not self.check_ack(ser,rr):
            return False,rr[0],3

        d = self.read_n(ser,dsize)

        return True,d

    def cmd_go(self,ser,addr):
        rr = [None]
        self.sendcksum(ser,chr(self.CMD_GO))
        if not self.check_ack(ser,rr):
            return False,rr[0],1

        self.sendcksum(ser,i2nbytes(addr,4),0x00)

        if not self.check_ack(ser,rr):
            return False,rr[0],2

        if not self.check_ack(ser,rr):
            return False,rr[0],3

        return True,

    def cmd_write_mem(self,ser,addr,data):
        rr = [None]
        self.sendcksum(ser,chr(self.CMD_WRITEMEM))
        if not self.check_ack(ser,rr):
            return False,rr[0],1

        self.sendcksum(ser,i2nbytes(addr,4),0x00)
        if not self.check_ack(ser,rr):
            return False,rr[0],2

        data2 = chr(len(data)-1) + data
        self.sendcksum(ser,data2,0x00)
        if not self.check_ack(ser,rr):
            return False,rr[0],3

        return True,len(data)

    def cmd_erase(self,ser,pages):
        rr = [None]
        self.sendcksum(ser,chr(self.CMD_ERASE))
        if not self.check_ack(ser,rr):
            return False,rr[0],1

        if pages == 255:
            self.sendcksum(ser,chr(0xff))
            if not self.check_ack(ser,rr):
                return False,rr[0],2
            return True,
        else:
            pages = ''.join([chr(i) for i in pages])
            data2 = chr(len(pages)-1) + pages
            self.sendcksum(ser,data2)
            if not self.check_ack(ser,rr):
                return False,rr[0],3
            return True,

    def cmd_extended_erase(self,ser,pages):
        rr = [None]
        self.sendcksum(ser,chr(self.CMD_EERASE))
        if not self.check_ack(ser,rr):
            return False,rr[0],1

        if (type(pages) in (type(1),type(1l)) and
            pages >= 0xfff0 and pages <= 0xffff):
            data = struct.pack('>H',pages)
            self.sendcksum(ser,data,0x00)
            if not self.check_ack(ser,rr):
                return False,rr[0],3
            return True,
        else:
            pages = list(pages)
            pages.insert(0,len(pages)-1)
            data = [struct.pack('>H',i) for i in pages]
            data2 = ''.join(data)
            self.sendcksum(ser,data2,0x00)
            if not self.check_ack(ser,rr):
                return False,rr[0],3
            return True,

    def cmd_write_protect(self,ser,pages):
        rr = [None]
        self.sendcksum(ser,chr(self.CMD_WRPROTECT))
        if not self.check_ack(ser,rr):
            return False,rr[0],1

        pages = ''.join([chr(i) for i in pages])
        data2 = chr(len(pages)-1) + pages
        self.sendcksum(ser,data2)
        if not self.check_ack(ser,rr):
            return False,rr[0],3
        return True,

    def cmd_write_unprotect(self,ser):
        rr = [None]
        self.sendcksum(ser,chr(self.CMD_WRUNPROTECT))
        if not self.check_ack(ser,rr):
            return False,rr[0],1

        if not self.check_ack(ser,rr):
            return False,rr[0],3
        return True,

    def serial_reset(self,ser,dtr):
        ser.setDTR(dtr)
        ser.setRTS(1)
        time.sleep(0.05);
        ser.setRTS(0)
        time.sleep(0.05);

    def doit(self,args):
        fin = sys.stdin
        fout = sys.stdout
        self.fout = fout
        self.ferr = sys.stderr

        if not args:
            usage(1, )

        port = args.pop(0)

        ser = serial.Serial(port, self.speed, parity=serial.PARITY_EVEN)

        ser.close()
        ser.open()

        self.serial_reset(ser,1);

        ser.flushInput()

        ser.write('%c' % (0x7f,))

        d = self.read_n(ser,1)

        r = self.cmd_get(ser)
        cmds = ' '.join(['%02x' % (ord(c),) for c in r[1]])
        fout.write('Get: %s\n' % (cmds,))

        r = self.cmd_get_version(ser)
        fout.write('Get version: %s\n' % (r,))

        r = self.cmd_get_id(ser)
        fout.write('Get id: %s\n' % (r,))

        a0 = self.addr
        a1 = a0 + self.npages*1024

        if self.op == 'r':
            ff = file(self.filename,'wb')
            i1 = intelhex.IntelHex()
            for a in xrange(a0,a1,256):
                r = self.cmd_read_mem(ser,a,256)
                d = r[1]
                fout.write('Read mem: %08x  %3d\n' % (a,len(d)))
                i1[a:a+len(d)] = [ord(c) for c in d]
            i1.tofile(ff,'hex')
            ff.close()

        elif self.op == 'w':
            ff = file(self.filename,'rb')

            i1 = intelhex.IntelHex()
            i1.loadfile(ff,'hex')

            i2 = intelhex.IntelHex(i1)

            r = self.cmd_extended_erase(ser,0xffff)
            if not r[0]:
                fout.write('Erase failed %s\n' % (r[1:],))
                return 1

            while i2.minaddr() < i2.maxaddr():
                a1 = i2.minaddr()
                a2 = a1+0x100
                flashdata = i2[a1:a2].tobinstr()
                del i2[a1:a2]
                fout.write('%08x %4d %s\n' % (a1,len(flashdata),flashdata.encode('hex')[:32]))
                r = self.cmd_write_mem(ser,a1,flashdata)
                if not r[0]:
                    fout.write('Write failed %s\n' % (r[1:],))
                    return 1
                r = self.cmd_read_mem(ser,a1,len(flashdata))
                if not r[0]:
                    fout.write('Read back failed %s\n' % (r[1:],))
                    return 1
                dback = r[1]
                if dback != flashdata:
                    fout.write('Verify failed\n')
                    return 1

        self.serial_reset(ser,0)        
        ser.parity = serial.PARITY_NONE

        if self.connect:
            while True:
                rin = [ser.fd, fin]
                iwtd, owtd, ewtd = select.select(rin,[],[],0)
                if ser.fd in iwtd:
                    fout.write(ser.read(ser.inWaiting()))
                if fin in iwtd:
                    ser.write(fin.readline())

        return

def main(argv):
    gp = Global()
    try:
        opts, args = getopt.getopt(argv[1:],
                                   'b:r:w:ue:vn:g:s:fhcA:N:z',
                                   ['verbose',
                                    'addr=',
                                    'npages=',
                                    'read=',
                                    'write=',
                                    ])
    except getopt.error, msg:
        usage(1, msg)

    for opt, arg in opts:
        if opt in ('-h', '--help'):
            usage(0)
        elif opt in ('-b', '--speed'):
            gp.speed = int(arg)
        elif opt in ('-r', '--read'):
            gp.filename = arg
            gp.op = 'r'
        elif opt in ('-w', '--write'):
            gp.filename = arg
            gp.op = 'w'
        elif opt in ('-A', '--addr'):
            gp.addr = int(arg,0)
        elif opt in ('-N', '--npages'):
            gp.npages = int(arg,0)
        elif opt in ('-c'):
            gp.connect = True;
    
    gp.doit(args)

if __name__ == '__main__':
    main(sys.argv)
