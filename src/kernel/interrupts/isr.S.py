#!/usr/bin/env python2
# -*- coding: utf-8 -*-

num_isr = 256
pushes_error = [8, 10, 11, 12, 13, 14, 17]

print '''
.intel_syntax noprefix
.extern isr_common
'''


print '// Interrupt Service Routines'
for i in range(num_isr):
    print '''isr{0}:
    cli
    {1}
    push {0}
    jmp isr_common
    '''.format(i,
        'push 0' if i not in pushes_error else 'nop')

print ''
print '''
// Vector table

.section .data
.global isr_table
isr_table:'''

for i in range(num_isr):
    print '  .quad isr{}'.format(i)
