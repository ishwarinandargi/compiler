    # ==========================================================================
    #   STORYLANG COMPILER - TARGET CODE GENERATOR (x86 ASSEMBLY)               
    #   Target: x86 / IA-32 (GNU / MinGW GCC Compatible)                        
    # ==========================================================================
    # --------------------------------------------------------------------------
    #   DATA SEGMENT: String Literals, Action Messages & Program Constants      
    # --------------------------------------------------------------------------
    .section .rdata,"dr"
.LC_char_hero_1:
    .asciz  "wanderer"
.LC_act_enter_2:
    .asciz  ">>> [ACTION EXECUTION] ACTION: enter(hero)"
.LC_act_wait_3:
    .asciz  ">>> [ACTION EXECUTION] ACTION: wait(hero)"
    # --------------------------------------------------------------------------
    #   TEXT SEGMENT: Executable Machine Instructions                           
    # --------------------------------------------------------------------------
    .text
    # Declaration: Character 'hero' of role 'wanderer'
    # Declaration: State 'door' with values {open | locked}
    # ----------------------------------------------------------------------
    # Function Definition: main
    # ----------------------------------------------------------------------
    .globl  _main
    .globl  main
    .def    _main; .scl 2; .type 32; .endef
_main:
main:
    pushl   %ebp                    # Save old base pointer
    movl    %esp, %ebp              # Establish new stack frame
    subl    $16, %esp               # Allocate 16 bytes on stack
    movl    $0, -8(%ebp)            # 3AC: door = open
    movl    -8(%ebp), %eax          # 3AC: t0 = door == open
    cmpl    $0, %eax                # Compare operands (door vs open)
    sete    %al                     # Set lower byte based on condition code
    movzbl  %al, %eax               # Zero-extend %al into %eax
    movl    %eax, -12(%ebp)         # Store boolean result to t0
    movl    -12(%ebp), %eax         # 3AC: if_false t0 goto L1
    cmpl    $0, %eax                # Test condition boolean value
    je      L1                      # Jump if false (zero)
    # Parameter: hero
    # 3AC: action enter (1 args)
    pushl   $.LC_act_enter_2        # Push action message address
    call    _puts                   # Trigger runtime action notification via puts
    addl    $4, %esp                # Clean up stack
    jmp     L2                      # 3AC: goto L2
L1:
    # Parameter: hero
    # 3AC: action wait (1 args)
    pushl   $.LC_act_wait_3         # Push action message address
    call    _puts                   # Trigger runtime action notification via puts
    addl    $4, %esp                # Clean up stack
L2:
    movl    $0, %eax                # Return 0 exit code
    movl    %ebp, %esp              # Deallocate stack frame
    popl    %ebp                    # Restore base pointer
    ret                             # Return to caller
