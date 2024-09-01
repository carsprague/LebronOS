    #include "keyboard.h"
    #include "lib.h"
    #include "i8259.h"
    #include "types.h"
    #include "system_call.h"
    #include "paging.h"
    #include "system_call_asm.h"

    #define MAX_CHARS_PER_LINE      80
    #define FD_FOR_KEYBOARD      23

int hello_end_flag = 0;



/* terminal_write
 * print either from the file or keyboard
 * Inputs: nbytes is how many chars to write, buf it the buffer it outputs, fd is not used
 * Outputs: error return -1 else 0
 * Side Effects: write to the screen
 */
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes){
    cli();
    int i;
    // int flag_for_dot;
    ternimal_t* terminal_shown_struct_pointer;
    // commenting this out because i'm virtualizing terminal and i want to see if it breaks it
    if(current_terminal == terminal_shown){
		// set the kernel vidmap virtual address B8 to actual B8
		init_pte_entry(first_page_table, VID_MEM_PTT_IDX, ACTIVE, SUPERVISOR, READ_WRITE, (void*)VID_MEM_PAGE_ADDR);
	}else{
		// set the kernel virtual address B8 to B9, BA, BB accordingly
        // 0x1000 is a 4KB page
		init_pte_entry(first_page_table, VID_MEM_PTT_IDX, ACTIVE, SUPERVISOR, READ_WRITE, (void*)(VID_MEM_PAGE_ADDR+((current_terminal+1)*0x1000)));
	}
    flush_tlb();

    if(strncmp((int8_t*)buf,(int8_t*)"391OS> ", sizeof(buf) ) == 0 && terminal_array[current_terminal].cursor_xpos != 0){
        putc('\n');
    }

    if(buf ==NULL){
        return -1;
    }

    terminal_shown_struct_pointer = &terminal_array[terminal_shown];

    // new numchar will be terminal specific
    int * terminal_numchar = &(terminal_shown_struct_pointer->numchar_for_this_terminal);
    if(fd == FD_FOR_KEYBOARD){
        putc_for_keyboard(((char*)buf)[(*terminal_numchar)]);
    }
    else{
       for(i = 0; i < nbytes; i++){
            if(((char*)buf)[i] != '\0'){
                putc(((char*)buf)[i]);
            }
        }     
    }

    sti();
    return nbytes;
}
/* terminal_read
 * copies what is in keybaord buffer into the buf passed in
 * Inputs: nbytes is how many chars to write, buf it the buffer it outputs, fd is not used
 * Outputs: how many bytes we read, -1 of buf is null
 * Side Effects: copies keyboard buffer into buf, also resets numchar and clears keyboard buffer
 */
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes){
    int i;
    // terminal shown is the one keyboard wants to write to
    ternimal_t* terminal_shown_struct_pointer = &terminal_array[current_terminal];

    // new keyboard buffer will be terminal_shown_struct_pointer->terminal_buffer
    // i am just trying to make the expression easier to read
    char* terminal_shown_buf = terminal_shown_struct_pointer->terminal_buffer;

    // new numchar will be terminal specific
    int * terminal_numchar = &(terminal_shown_struct_pointer->numchar_for_this_terminal);
    terminal_array[current_terminal].enter_flag =0;
    terminal_array[current_terminal].over_80_flag = 0;
    if(buf ==NULL){
        return -1;
    }
    while(terminal_array[current_terminal].enter_flag == 0){ // only run the code after this if enter is pressed
        continue;
    }
    //terminal_switch_in_progress = 0;
    int read;
if(nbytes < (*terminal_numchar)){ // read nbytes even if its not everything
    for(i = 0; i < nbytes; i++){
        if(terminal_shown_buf[i] != '\0'){
            ((char*)buf)[i] = terminal_shown_buf[i];
        }
    }
    read = nbytes;
}
if(nbytes >= (*terminal_numchar)){
    for(i = 0; i < (*terminal_numchar); i++){ // read everything in the buffer and end once we do
        if(terminal_shown_buf[i] != '\0'){
            ((char*)buf)[i] = terminal_shown_buf[i];
        }
    }
    read = (*terminal_numchar);
}

    (*terminal_numchar) =0;
    memset(terminal_shown_buf,'\0',sizeof(terminal_shown_buf)); // reset buffer since enter flag has been active
    return read;
}
/* terminal_open
 * opens the terminal
 * Inputs: it is the file passed in
 * Outputs: 0
 * Side Effects: nothing
 */
int32_t terminal_open(const uint8_t* filename, int32_t fd){
    return 0;
}
/* terminal_close
 * closes terminal
 * Inputs: file direct passed in
 * Outputs: 0
 * Side Effects: nothing
 */
int32_t terminal_close(int32_t fd){
    return 0;
}
