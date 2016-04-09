#!/usr/bin/env python3
import sys

AMD64_PARAM_REGISTERS = ['DI', 'SI', 'DX', 'CX', 'R8', 'R9']
AMD64_FUNCTION_REGISTER = 'AX'
AMD64_RETURN_REGISTER = 'AX'

GOASM_HEADER = """\
// Copyright 2009 The Go Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
//
// System calls and other sys.stuff for AMD64, Unigornel.
//
// THIS FILE IS GENERATED DO NOT EDIT.
//
// {cmd}
//

#include "go_asm.h"
#include "go_tls.h"
#include "textflag.h"
"""

def main(input, cmd, goname_replace, error):
    print(GOASM_HEADER.format(cmd=cmd))

    state = None
    did_error = False
    for linenumber, line in enumerate(input):
        line = line.strip()
        state = process(error, goname_replace, line, state)
        if not state:
            did_error = True

    return not did_error

def process(error, goname_replace, line, state):
    import fileinput

    STATE_NEW = 0
    STATE_MANUAL = 1

    state, syscall = state if state is not None else (STATE_NEW, None)

    try:
        if state == STATE_MANUAL:
            is_ret = syscall.add_line_to_body(line)
            if is_ret:
                print("\n" + syscall.to_goasm())
            return (STATE_NEW if is_ret else STATE_MANUAL, syscall)

        syscall = Syscall.from_line(line)
        if syscall:
            syscall.goname = goname_replace(syscall.goname)
            print("\n" + syscall.to_goasm())
            return (STATE_NEW, syscall)

        syscall = ManualSyscall.from_line(line)
        if syscall:
            syscall.goname = goname_replace(syscall.goname)
            return (STATE_MANUAL, syscall)

    except (ManualSyscall.InvalidDeclarationException,
            ManualSyscall.InvalidLineException,
            Syscall.InvalidSyscallLineException,
            Syscall.TooManyInputArguments,
            Argument.InvalidArgumentStringException,
            Argument.UnknownTypeException) as exc:
        error(fileinput.filename(), fileinput.lineno(), str(exc))
        return None

    return (STATE_NEW, None)

class Syscall(object):
    class InvalidSyscallLineException(Exception):
        def __init__(self):
            msg = 'Invalid syscall specification'
            super(Syscall.InvalidSyscallLineException, self).__init__(msg)

    class TooManyInputArguments(Exception):
        def __init__(self):
            msg = 'Too many input arguments'
            super(Syscall.TooManyInputArguments, self).__init__(msg)

    def __init__(self, cname, goname, input, output):
        self.cname = cname
        self.goname = goname
        self.input = input
        self.output = output

    def __repr__(self):
        t = (self.cname, self.goname, self.input, self.output)
        t = tuple(map(repr, t))
        return 'Syscall(cname=%s, goname=%s, input=%s, output=%s)' % t

    @property
    def goasm_name(self):
        return self.goname.replace('.', '\u00B7')

    @property
    def go_func_name(self):
        return self.goname.split('.')[-1]

    @classmethod
    def from_line(cls, line):
        import re
        m = re.match('^//sys\s+(.*)$', line)
        if not m:
            return None

        m = m.group(1).strip()
        m = re.match('(\w+)\s+([\w\.]+)\((.*)\)\s*(\w*)$', m)
        if not m:
            raise cls.InvalidSyscallLineException()

        cname = m.group(1)
        goname = m.group(2)
        if m.group(3) == "":
            input = []
        else:
            input = list(map(Argument.from_input_string, m.group(3).split(',')))
        output = m.group(4) if m.group(4) != "" else None

        return cls(cname, goname, input, output)

    def to_goasm(self):
        def mov_arg(stack, arg, reg):
            if arg.size() == 4: mov, offset = ('L', stack.popl())
            else:               mov, offset = ('Q', stack.popq())
            t = [mov, arg.name, offset, reg]
            return "MOV{0}\t{1}+{2}(FP), {3}".format(*t)

        if len(self.input) > len(AMD64_PARAM_REGISTERS):
            raise TooManyInputArguments()

        stack = GoStack()
        t = "// " + self.to_go_declaration() + "\n"
        t += "TEXT {0}(SB),NOSPLIT,$0\n".format(self.goasm_name)
        for i, arg in enumerate(self.input):
            t += "\t" + mov_arg(stack, arg, AMD64_PARAM_REGISTERS[i]) + "\n"
        t += "\tLEAQ\t{0}(SB), {1}\n".format(self.cname, AMD64_FUNCTION_REGISTER)
        t += "\tCALL\t{0}\n".format(AMD64_FUNCTION_REGISTER)
        if self.output:
            arg = Argument('ret', self.output)
            mov = 'L' if self.output == 4 else 'Q'
            t += "\tMOV{0}\t{1}, ret+{2}(FP)\n".format(mov, AMD64_RETURN_REGISTER, stack.popq())
        t += "\tRET"
        return t

    def to_go_declaration(self):
        if self.output:
            o = " " + self.output
        else:
            o = ""

        i = ', '.join(map(str, self.input))
        return "func {0}({1}){2}".format(self.go_func_name, i, o)

class ManualSyscall(Syscall):
    class InvalidDeclarationException(Exception):
        def __init__(self):
            msg = 'Invalid manual syscall declaration'
            super(ManualSyscall.InvalidDeclarationException, self).__init__(msg)

    class InvalidLineException(Exception):
        def __init__(self):
            msg = 'Invalid line in manual syscall declaration'
            super(ManualSyscall.InvalidLineException, self).__init__(msg)

    def __init__(self, cname, goname, stacksize):
        self.cname = cname
        self.goname = goname
        self.stacksize = stacksize
        self.body = []

    def add_line_to_body(self, line):
        import re
        if re.match('//\s*RET\s*$', line):
            self.body.append("\tRET")
            return True
        m = re.match('//\s*(\w+)\s+(.*)$', line)
        if not m:
            raise self.InvalidLineException()
        line = "\t{0}\t{1}".format(m.group(1), m.group(2))
        self.body.append(line)
        return False

    @classmethod
    def from_line(cls, line):
        import re
        m = re.match('//sysasm\s+(.*)$', line)
        if not m:
            return None

        m = m.group(1).strip()
        m = re.match('(\w+)\s+([\w\.]+)\s+(\d+)$', m)
        if not m:
            raise cls.InvalidDeclarationException()

        cname = m.group(1)
        goname = m.group(2)
        stack = int(m.group(3))
        return cls(cname, goname, stack)

    def to_goasm(self):
        t = "TEXT {0}(SB),NOSPLIT,${1}\n".format(self.goasm_name, self.stacksize)
        t += "\n".join(self.body)
        return t.strip()

class Argument(object):
    SIZES = {
        'int32' : 4,
        'uint32' : 4,
        'int64' : 8,
        'uint64': 8,
        'uintptr' : 8,
        'unsafe.Pointer' : 8,
    }
    class UnknownTypeException(Exception):
        def __init__(self, type):
            self.type = type
            msg = 'Unknown argument type: {0}'.format(type)
            super(Argument.UnknownTypeException, self).__init__(msg)

    class InvalidArgumentStringException(Exception):
        def __init__(self, string):
            self.string = string
            msg = 'Invalid argument string: {0}'.format(string)
            super(Argument.InvalidArgumentStringException, self).__init__(msg)

    def __init__(self, name, type, pointer=False):
        self.name = name
        self.type = type
        self.pointer = pointer

    def __repr__(self):
        t = (self.name, self.type, self.pointer)
        t = tuple(map(repr, t))
        return 'Argument(name=%s, type=%s, pointer=%s)' % t

    def __str__(self):
        p = "*" if self.pointer else ""
        return '{0} {1}{2}'.format(self.name, p, self.type)

    def size(self):
        if self.pointer:
            return 8

        try:
            return self.SIZES[self.type]
        except KeyError:
            raise self.UnknownTypeException(self.type)

    @classmethod
    def from_input_string(cls, line):
        import re
        line = line.strip()
        m = re.match('(\w+)\s*([\s\*])([\w\.]+)$', line)
        if not m:
            raise cls.InvalidArgumentStringException(line)
        name = m.group(1)
        pointer = True if m.group(2) == '*' else False
        type = m.group(3)
        return cls(name, type, pointer)

class GoStack(object):
    def __init__(self):
        self.offset = 0

    def popq(self):
        self.offset += (8 - self.offset) % 8
        o = self.offset
        self.offset += 8
        return o

    def popl(self):
        self.offset += (4 - self.offset) % 4
        o = self.offset
        self.offset += 4
        return o

if __name__ == "__main__":
    import fileinput
    from argparse import ArgumentParser

    cmd = ' '.join(['mksys.py'] + sys.argv[1:])

    parser = ArgumentParser(description='Generate Go assembly for calling system calls')
    parser.add_argument('files', metavar='file', type=str, nargs='+', help='annotated files with syscall declarations')
    parser.add_argument('--package', type=str, help='replace package in syscall declarations')
    parser.add_argument('--prefix', type=str, help='prefix function names')
    args = parser.parse_args(sys.argv[1:])

    def e(file, line, msg):
        print('error: {0}:{1}: {2}'.format(file, line, msg), file=sys.stderr)

    def r(goname):
        package, func = goname.rsplit('.', 1)
        if args.package:
            package = args.package
        if args.prefix:
            func = args.prefix + func[:1].upper() + func[1:]
        return package + '.' + func

    input = fileinput.input(args.files)
    if not main(input, cmd=cmd, goname_replace=r, error=e):
        sys.exit(1)
