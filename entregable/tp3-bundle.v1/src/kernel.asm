; ** por compatibilidad se omiten tildes **
; ==============================================================================
; TRABAJO PRACTICO 3 - System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
; ==============================================================================

%include "print.mac"
%include "seg_print.mac"
%include "colors.mac"
%include "defines.mac"


global start
extern GDT_DESC
extern IDT_DESC
extern idt_init
extern gdt_init

extern pic_reset
extern pic_enable

extern mmu_init
extern mmu_initKernelDir
extern mmu_initTaskDir

extern tss_init

extern sched_init
extern game_init

;; Saltear seccion de datos
jmp start

;;
;; Seccion de datos.
;; -------------------------------------------------------------------------- ;;
start_rm_msg db     'Iniciando kernel en Modo Real'
start_rm_len equ    $ - start_rm_msg

start_pm_msg db     'Iniciando kernel en Modo Protegido'
start_pm_len equ    $ - start_pm_msg
screen_cln_msg db     '   xX Usuario de Windows el que lee Xx    xX Usuario de Windows el que lee Xx   '
screen_cln_len equ    $ - screen_cln_msg

single_char db '@'
single_char_len equ $ - single_char

box_msg TIMES 38 db '@'
box_len equ $ - box_msg

group_msg db     ' 072/18 | 195/18 | 364/18 '
group_msg_len equ    $ - group_msg

;;
;; Seccion de código.
;; -------------------------------------------------------------------------- ;;

;; Punto de entrada del kernel.
BITS 16
start:
    ; Deshabilitar interrupciones
    cli

    ; Cambiar modo de video a 80 X 50
    ; ax = 1003h -> para poder tener 16 colores de background
    mov ax, 1003h
    int 10h ; set mode 03h
    xor bx, bx
    mov ax, 1112h
    int 10h ; load 8x8 font

    ; Imprimir mensaje de bienvenida
    print_text_rm start_rm_msg, start_rm_len, 0x07, 0, 0
    

    ; Habilitar A20
    call A20_enable

    ; Cargar la GDT
    lgdt [GDT_DESC]
    ; Setear el bit PE del registro CR0
    
    ; Saltar a modo protegido
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp (GDT_IDX_CODE_0 << 3):modo_protegido
modo_protegido:
BITS 32

    ; Establecer selectores de segmentos

    mov ax, GDT_IDX_DATA_0 << 3
    mov ss, ax

    mov ds, ax
    mov gs, ax
    mov fs, ax

    ; Establecer la base de la pila
    
    mov esp, KERNEL_STACK_BASE

    ; Imprimir mensaje de bienvenida
    print_text_pm start_pm_msg, start_pm_len, 0x07, SCREEN_W / 2 - start_rm_len / 2, SCREEN_H / 2

    ; Inicializar pantalla
    ; Screen Size : 80 x 50 
    mov ax, GDT_IDX_VIDEO << 3
    mov fs, eax ; Usamos el selector de video

    call limpiar_pantalla
    call draw_screen

    mov eax, GDT_IDX_DATA_0 << 3
    mov fs, eax

    ; Inicializar el manejador de memoria
    call mmu_init

    ; Inicializar el directorio de paginas
    call mmu_initKernelDir

    ; Cargar directorio de paginas
    mov eax, KERNEL_PAGE_DIR    ; No hace falta shiftear porque está
                                ; alineada a 4K y no usamos los 
                                ; atributos PCD y PWT (bits 4 y 3)
    mov cr3, eax


    ; Habilitar paginacion
    mov eax, cr0
    or eax, (1 << 31)
    mov cr0, eax

    ; Item 5. d --------------
    ; push 0
    ; call mmu_initTaskDir
    ; mov cr3, eax
    ; ------------------------

    ; Imprimir libretas de integrantes
    call print_group

    ; Inicializar la gdt
    ; Inicializar tss
    call tss_init
    call gdt_init

    ; Inicializar el scheduler
    call sched_init

    ; Inicializar la IDT
    call idt_init
    ; Cargar IDT
    lidt [IDT_DESC]

    ; Configurar controlador de interrupciones
    call pic_reset
    call pic_enable

    ; Cargar tarea inicial
    mov ax, GDT_IDX_TSS_INIT << 3 ; Cargamos TR con la tarea inicial
    ltr ax

    ; Habilitar interrupciones
    sti

    ; Inicializar juego
    call game_init

    ; Saltar a la primera tarea: Idle
    jmp (GDT_IDX_TSS_IDLE << 3):0

    ; Ciclar infinitamente (por si algo sale mal...)
    mov eax, 0xFFFF
    mov ebx, 0xFFFF
    mov ecx, 0xFFFF
    mov edx, 0xFFFF
    jmp $

limpiar_pantalla:
    push ebp
    mov ebp, esp

    mov ecx, SCREEN_H - 1 
    .loop:
        seg_print_text_pm screen_cln_msg, screen_cln_len, C_BG_LIGHT_GREY + C_FG_LIGHT_GREY, ecx, 0
        
        dec ecx
        cmp ecx, 0
        jge .loop

    pop ebp
    ret

draw_screen:
    push ebp
    mov ebp, esp

    ; Barras --------------------------------------
    ; mov ecx, BOARD_H/ 2 + 3
    ; .bar_loop:
    ;     seg_print_text_pm single_char, single_char_len, C_BG_RED + C_FG_BLACK, ecx, 0
    ;     seg_print_text_pm single_char, single_char_len, C_BG_BLUE + C_FG_CYAN, ecx, 79

    ;     dec ecx
    ;     cmp ecx, BOARD_H / 2 - 3
    ;     jge .bar_loop
    ; Barras --------------------------------------

    seg_print_text_pm screen_cln_msg, screen_cln_len, C_BG_BLACK + C_FG_BLACK, SCREEN_H - 1, 0

    ; Cuadrados
    ; 8 * 38
    mov ecx, SCREEN_H - 2
    .box_loop:
        seg_print_text_pm single_char,  single_char_len,    C_BG_BLACK + C_FG_BLACK,    ecx, 0  ; Left outline
        seg_print_text_pm box_msg,      box_len,            C_BG_RED + C_FG_BLACK,      ecx, 1  ; Box
        seg_print_text_pm single_char,  single_char_len,    C_BG_BLACK + C_FG_BLACK,    ecx, 39 ; Middle outline
        seg_print_text_pm single_char,  single_char_len,    C_BG_BLACK + C_FG_BLACK,    ecx, 40 ; Middle outline
        seg_print_text_pm box_msg,      box_len,            C_BG_BLUE + C_FG_CYAN,      ecx, 41 ; Box
        seg_print_text_pm single_char,  single_char_len,    C_BG_BLACK + C_FG_BLACK,    ecx, 79 ; Right outline

        dec ecx
        cmp ecx, SCREEN_H - 9
        jge .box_loop

    seg_print_text_pm screen_cln_msg, screen_cln_len, C_BG_BLACK + C_FG_BLACK, SCREEN_H - 10, 0


    pop ebp
    ret

print_group:
    push ebp
    mov ebp, esp

    print_text_pm group_msg, group_msg_len, C_BG_DARK_GREY + C_FG_DARK_GREY, BOARD_H / 2 - 1, SCREEN_W / 2 - group_msg_len / 2
    print_text_pm group_msg, group_msg_len, C_BG_DARK_GREY + C_FG_WHITE, BOARD_H / 2, SCREEN_W / 2 - group_msg_len / 2
    print_text_pm group_msg, group_msg_len, C_BG_DARK_GREY + C_FG_DARK_GREY, BOARD_H / 2 + 1, SCREEN_W / 2 - group_msg_len / 2

    pop ebp
    ret
;; -------------------------------------------------------------------------- ;;

%include "a20.asm"
