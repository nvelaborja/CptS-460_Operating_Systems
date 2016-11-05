int copyImage(u16 pseg, u16 cseg, u16 size)
{
    u16 i;
    for (i=0; i<size; i++)
        put_word(get_word(pseg, 2*i), cseg, 2*i);
}

int fork()
{
    int pid; u16 segment;
    PROC *p = kfork(0);                        // kfork() a child, do not load image file
    if (p==0) return -1;                       // kfork failed
    segment = (p->pid+1)*0x1000;               // child segment
    copyImage(running->uss, segment, 32*1024); // copy 32K words
    p->uss = segment;                          // child’s own segment
    p->usp = running->usp;                     // same as parent's usp
    //*** change uDS, uES, uCS, AX in child's ustack ****
    put_word(segment, segment, p->usp);        // uDS=segment
    put_word(segment, segment, p->usp+2);      // uES=segment
    put_word(0, segment, p->usp+2*8);          // uax=0
    put_word(segment, segment, p->usp+2*10);   // uCS=segment
    return p->pid;
}

int exec(char* filename)
{
    int i, segment = 0x1000*(running->pid + 1), len, str_offset;
    char buf[32], filepath[64], file[32], *t, *s;
    /* buf is copy of filename, filepath is /bin/u*, file is the actual file name
     * *t and *s are just pointers to the char arrays
     */

    i =0;
    while (i < 32) // size of name is limited to 32 bits
    {
        buf[i] = get_word(running->uss, filename);
        i++; filename++;
    }

//    t = file;
//    s = buf;
//    while (*s)   // get just the first token => file name
//    {
//        if (*s == ' ') // add a null character at the end of file name
//        {
//            *t = 0;
//            break;
//        }
//        *t++ = *s++;       // the filename
//    }

    /* gets the first token => name of umode program file */
    for (i = 0; buf[i] != ' '; i++)
    {
        file[i] = buf[i];
    }
    file[i] = 0;
    //printf("FILE = %s\n", file);

    strcpy(filepath, "/bin/");
    strcat(filepath, file);

    //printf("FILENAME: %s BUF: %s\n", filename, buf);
    if (!load(filepath, segment))   // load filename to segment
        return -1;                  // if load failed, return -1 to Umode

    /* After loading filename, we set up the ustack of the new Umode image */

    /* First we pad the command line with an extra byte to make the total length
     * even, it necessary
     */
    len = strlen(buf) + 1; // the 1 is for the null character // strlen() returns after hits ws
    if (len % 2 != 0)
        len++;

    /* Next, we put the entire string into the high end of ustack
     */
    //printf("LENGTH: %d\n", len);
    for (i = 0; i < len; i++)
    {
        //printf("buf[%d]: ", i); putc(buf[i]);
        put_byte(buf[i], segment, -len + i);
    }

    /* and let s point at the string in ustack */
    str_offset = -len - 2;
    put_word(-len, segment, str_offset);

    /* re-initialize process ustack for it return to VA=0 */
    for (i = 1; i < 12; i++) // throught 12 because dont want o overwrite retPC and 1 because dont 0 out Uds
        put_word(0, segment, -2*i + str_offset);

    running->usp = -24 + str_offset;                   // new usp = -2 * 12
    running->uss = segment;               // set at segment

    /*  -1   -2 -3  -4 -5 -6 -7 -8 -9 -10 -11 -12 */
    /* flag uCS uPC ax bx cx dx bp si  di uES uDs */
    put_word(segment, segment, -2*12+ str_offset);  // saved uDS=segment
    put_word(segment, segment, -2*11+ str_offset);  // saved uES=segment
    put_word(segment, segment, -2*2+ str_offset);   // uCS=segment; uPC=0
    put_word(0x0200,  segment, -2*1+ str_offset);   // Umode flag=0x200
}