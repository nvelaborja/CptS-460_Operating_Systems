// casanova.c for EXAM 1 CASANOVA PROCESS
// Nathan VelaBorja - 11392441

// Very similar to fork, but here we aren't creating a new process, just copying the image and moving to another segment
int khop(int segment)
{
  printf("Enter khop, segment=%x\n", segment);

  copyImage (running->uss, segment, 32 * 1024);   // Copy 32K words

  running->uss = segment;                 // Update childs uss to it's new segment
  running->usp = running->usp;            // Same as parent's usp

  // Change uDS, uES, uCS, and AX in child's ustack
  put_word(segment, segment, running->usp);             // uDS = segment
  put_word(segment, segment, running->usp + 2);         // uES = segment
  put_word(0,       segment, running->usp + 2 * 8);     // uAX = 0
  put_word(segment, segment, running->usp + 2 * 10);    // uCS = segment
}