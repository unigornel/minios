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
#include "textflag.h"
"""

GODECL_HEADER = """\
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

package {package}

import "unsafe"
"""

def goasm(input, cmd, edit_syscall, error):
    print(GOASM_HEADER.format(cmd=cmd))

    syscalls, ok = read_syscalls(input, cmd, edit_syscall, error)
    for filename, lineno, s in syscalls:
        try:
            print()
            print(s.to_goasm())
        except MksysException as exc:
            error(filename, lineno, str(exc))
            ok = False

    return ok

def godecl(package, input, cmd, edit_syscall, error):
    print(GODECL_HEADER.format(cmd=cmd, package=package))

    syscalls, ok = read_syscalls(input, cmd, edit_syscall, error)
    for filename, lineno, s in syscalls:
        try:
            decl = s.to_go_declaration()
            if decl:
                print(decl)
        except MksysException as exc:
            error(filename, lineno, str(exc))
            ok = False

    return ok

def read_syscalls(input, cmd, edit_syscall, error):
    import fileinput
    syscalls = []
    state = None
    did_error = False
    for line in input:
        line = line.strip()
        state = process(error, line, state)
        if not state:
            did_error = True
            continue

        state_t, state_s = state
        if state_t == process.STATE_DONE:
            edit_syscall(state_s)
            syscalls.append((fileinput.filename(), fileinput.lineno(), state_s))

    return (syscalls, not did_error)

def process(error, line, state):
    import fileinput

    state, syscall = state if state is not None else (process.STATE_NEW, None)

    try:
        if state == process.STATE_MANUAL:
            is_ret = syscall.add_line_to_body(line)
            return (process.STATE_DONE if is_ret else process.STATE_MANUAL, syscall)

        syscall = Syscall.from_line(line)
        if syscall:
            return (process.STATE_DONE, syscall)

        syscall = ManualSyscall.from_line(line)
        if syscall:
            return (process.STATE_MANUAL, syscall)

    except MksysException as exc:
        error(fileinput.filename(), fileinput.lineno(), str(exc))
        return None

    return (process.STATE_NEW, None)

process.STATE_NEW = 0
process.STATE_MANUAL = 1
process.STATE_DONE = 2

class MksysException(Exception):
    pass

class Syscall(object):
    class InvalidSyscallLineException(MksysException):
        def __init__(self):
            msg = 'Invalid syscall specification'
            super(Syscall.InvalidSyscallLineException, self).__init__(msg)

    class TooManyInputArguments(MksysException):
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
        m = re.match('(\w+)\s+([\w\.]+)\((.*)\)\s*([\w\.]*)$', m)
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
    class InvalidDeclarationException(MksysException):
        def __init__(self):
            msg = 'Invalid manual syscall declaration'
            super(ManualSyscall.InvalidDeclarationException, self).__init__(msg)

    class InvalidLineException(MksysException):
        def __init__(self):
            msg = 'Invalid line in manual syscall declaration'
            super(ManualSyscall.InvalidLineException, self).__init__(msg)

    def __init__(self, cname, goname, stacksize):
        self.cname = cname
        self.goname = goname
        self.stacksize = stacksize
        self.body = []

    def __repr__(self):
        t = (self.cname, self.goname, self.stacksize)
        return 'ManualSyscall(cname=%s, goname=%s, stacksize=%d)' % t

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

    def to_go_declaration(self):
        return None

class Argument(object):
    SIZES = {
        'int32' : 4,
        'uint32' : 4,
        'int64' : 8,
        'uint64': 8,
        'uintptr' : 8,
        'unsafe.Pointer' : 8,
    }
    class UnknownTypeException(MksysException):
        def __init__(self, type):
            self.type = type
            msg = 'Unknown argument type: {0}'.format(type)
            super(Argument.UnknownTypeException, self).__init__(msg)

    class InvalidArgumentStringException(MksysException):
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
    parser.add_argument('--godecl', action='store_true', help='output Go function declarations instead of Go assembly')
    parser.add_argument('--package', type=str, help='replace package in syscall declarations')
    parser.add_argument('--prefix', type=str, help='prefix function names')
    parser.add_argument('--convert-pointers', action='store_true', help='convert pointers to unsafe.Pointer')
    args = parser.parse_args(sys.argv[1:])

    if args.godecl and not args.package:
        print('error: --godecl: you must specify the package using --package', file=sys.stderr)
        sys.exit(1)

    def e(file, line, msg):
        print('error: {0}:{1}: {2}'.format(file, line, msg), file=sys.stderr)

    def r(syscall):
        # Replace goname
        package, func = syscall.goname.rsplit('.', 1)
        if args.package:
            package = args.package
        if args.prefix:
            func = args.prefix + func[:1].upper() + func[1:]
        syscall.goname = package + '.' + func

        # Replace pointers
        if args.convert_pointers:
            for arg in getattr(syscall, 'input', []):
                if arg.pointer:
                    arg.type = 'unsafe.Pointer'
                    arg.pointer = False
            if hasattr(syscall, 'output') and syscall.output:
                if Argument('ret', syscall.output).pointer:
                    syscall.output = 'unsafe.Pointer'

    input = fileinput.input(args.files)

    if args.godecl:
        f = lambda: godecl(args.package, input, cmd=cmd, edit_syscall=r, error=e)
    else:
        f = lambda: goasm(input, cmd=cmd, edit_syscall=r, error=e)
    if not f():
        sys.exit(1)
