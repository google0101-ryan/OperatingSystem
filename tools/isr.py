from __future__ import print_function

num_isr = 256
pushes_error = [8, 10, 11, 12, 13, 14, 17]

print('''
extern isr_common
''')

print('; Interrupt service routines')
for i in range(num_isr):
	print('''isr{0}:
	cli
	{1}
	push {0}
	jmp isr_common'''.format(i, 'push 0' if i not in pushes_error else 'nop'))

print('')
print('''
; Vector table

section .data
global isr_table
isr_table:''')

for i in range(num_isr):
	print('  dq isr{}'.format(i))