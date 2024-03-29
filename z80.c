#include "stdafx.h"
/*=====================================================================
  z80.c -> Main File related to the Z80 emulation code.

  Please read documentation files to know how this works :)

  Thanks go to Marat Fayzullin (read z80.h for more info), Ra�l Gomez
  (check his great R80 Spectrum emulator!), Philip Kendall (some code
  of this emulator, such as the flags lookup tabled are from his fuse
  Spectrum emulator) and more people I forget to name here ...

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

 Copyright (c) 2000 Santiago Romero Iglesias.
 Email: sromero@escomposlinux.org
 ======================================================================*/

#ifdef _DEBUG_
#include <mss.h>
#endif

#include "tables.h"
#include <stdio.h>

#include "z80.h"
#include "graphics.h"
#include "monofnt.h"
#include "menu.h"
#include "main.h"
#include "sound.h"
#include "snaps.h"
#include "mem.h"
#include "v_alleg.h"
#include "macros.c"

/* RAM variable, debug toggle variable, pressed key and
   row variables for keyboard emulation                   */
//extern byte *RAM;
extern int debug, main_tecla, scanl;
extern char *msg_log;
extern int fila[5][5];
//extern char *tapfile;
//extern FILE *tapfile;
extern char *tfont;

//#include "macros.c"           // now in aspec.h

extern int TSTATES_PER_LINE, TOP_BORDER_LINES, BOTTOM_BORDER_LINES, SCANLINES;


// previously in macros.c
#ifndef _DISASM_

/* Whether a half carry occured or not can be determined by looking at
   the 3rd bit of the two arguments and the result; these are hashed
   into this table in the form r12, where r is the 3rd bit of the
   result, 1 is the 3rd bit of the 1st argument and 2 is the
   third bit of the 2nd argument; the tables differ for add and subtract
   operations */
byte halfcarry_add_table[] = { 0, FLAG_H, FLAG_H, FLAG_H, 0, 0, 0, FLAG_H };
byte halfcarry_sub_table[] = { 0, 0, FLAG_H, 0, FLAG_H, 0, FLAG_H, FLAG_H };

/* Similarly, overflow can be determined by looking at the 7th bits; again
   the hash into this table is r12 */
byte overflow_add_table[] = { 0, 0, 0, FLAG_V, FLAG_V, 0, 0, 0 };
byte overflow_sub_table[] = { 0, FLAG_V, 0, 0, 0, 0, FLAG_V, 0 };

/* Some more tables; initialised in z80_init_tables() */
byte sz53_table[0x100];		/* The S, Z, 5 and 3 bits of the temp value */
byte parity_table[0x100];	/* The parity of the temp value */
byte sz53p_table[0x100];	/* OR the above two tables together */
/*------------------------------------------------------------------*/

// Contributed by Metalbrain to implement OUTI, etc.
byte ioblock_inc1_table[64];
byte ioblock_dec1_table[64];
byte ioblock_2_table[0x100];

/*--- Memory Write on the A address on no bank machines -------------*/
void Z80WriteMem (word where, word A, Z80Regs * regs)
{
//  if (where >= 16384)
//    regs->RAM[where] = A;
	writemem(where, (byte)A);
}
/*
int Z80ReadMem(word where)
{   
	return readmem(where);
}
*/
#endif // #ifndef _DISASM_

/*====================================================================
  void Z80Reset( Z80Regs *regs, int cycles )

  This function simulates a z80 reset by setting the registers
  to the values they are supposed to take on a real z80 reset.
  You must pass it the Z80 register structure and the number
  of cycles required to check for interrupts and do special
  hardware checking/updating.
 ===================================================================*/
void
Z80Reset (Z80Regs * regs, int int_cycles)
{
  /* reset PC and the rest of main registers: */
  regs->PC.W = regs->R.W = 0x0000;

  regs->AF.W = regs->BC.W = regs->DE.W = regs->HL.W =
    regs->AFs.W = regs->BCs.W = regs->DEs.W = regs->HLs.W =
    regs->IX.W = regs->IY.W = 0x0000;

  /* Make the stack point to $F000 */
  regs->SP.W = 0xF000;

  /* reset variables to their default values */
  regs->I = 0x00;
  regs->IFF1 = regs->IFF2 = regs->IM = regs->halted = 0x00;
  regs->ICount = regs->IPeriod = int_cycles;

  regs->IRequest = INT_NOINT;
  regs->we_are_on_ddfd = regs->dobreak = regs->BorderColor = 0;

//#ifdef _DEBUG_
  regs->DecodingErrors = 1;
//#endif

}

/*====================================================================
  word Z80Run( Z80Regs *regs, int numopcodes )

  This function does the whole Z80 simulation. It consists on a
  for(;;) loop (as stated on Marat's Fayzullin HOWTO -How to
  Write a Computer Emulator-) which fetchs the next opcode,
  interprets it (using a switch statement) and then it's
  executed in the right CASE: of that switch. I've put the different
  case statements into C files included here with #include to
  make this more readable (and programming easier! :).

  This function will change regs->ICount register and will execute
  an interrupt when it reaches 0 (or <0). You can then do anything
  related to your machine emulation here, using the Z80Hardware()
  function. This function must be filled by yourself: put there
  the code related to the emulated machine hardware, such as
  screen redrawing, sound playing and so on. This functions can
  return an special value to make Z80Run stop the emulation (and
  return to the caller): that's INT_QUIT. If there is time to
  execute an interrupt, please return INT_IRQ or INT_NMI. Return
  INT_NOINT if there is no time for an interrupt :) .

  Z80Execute() will change PC and all the z80 registers acording
  to the executed opcode, and those values will be returned when
  a INT_QUIT is received.

  Pass as numcycles the number of clock cycle you want to execute
  z80 opcodes for or < 0 (negative) to execute "infinite" opcodes.
 ===================================================================*/
word
Z80Run (Z80Regs * regs, int numcycles)
{
  /* opcode and temp variables */
  register byte opcode;
  eword tmpreg, ops, mread, tmpreg2;
  unsigned long tempdword;
  register int loop;
  unsigned short tempword;

  /* emulate <numcycles> cycles */
  loop = (regs->ICount - numcycles);

  /* this is the emulation main loop */
  while (regs->ICount > loop)
    {
#ifdef DEBUG
      /* test if we have reached the trap address */
      if (regs->PC.W == regs->TrapAddress && regs->dobreak != 0)
	return (regs->PC.W);
#endif

      if (regs->halted == 1)
	{
	  r_PC--;
	  AddCycles (4);
	}

      /* read the opcode from memory (pointed by PC) */
      opcode = Z80ReadMem (regs->PC.W);
      regs->PC.W++;

      /* increment the R register and decode the instruction */
      AddR (1);
      switch (opcode)
	{
#ifndef CPP_COMPILATION
#include "opcodes.c"
#else
#include "opcodes.cpp"
#endif
	case PREFIX_CB:
	  AddR (1);
#ifndef CPP_COMPILATION
#include "op_cb.c"
#else
#include "op_cb.cpp"
#endif
	  break;
	case PREFIX_ED:
	  AddR (1);
#ifndef CPP_COMPILATION
#include "op_ed.c"
#else
#include "op_ed.cpp"
#endif
	  break;
	case PREFIX_DD:
	case PREFIX_FD:
	  AddR (1);
	  if (opcode == PREFIX_DD)
	    {
#define REGISTER regs->IX
	      regs->we_are_on_ddfd = WE_ARE_ON_DD;
#ifndef CPP_COMPILATION
#include "op_dd_fd.c"
#else
#include "op_dd_fd.cpp"
#endif

#undef  REGISTER
	    }
	  else
	    {
#define REGISTER regs->IY
	      regs->we_are_on_ddfd = WE_ARE_ON_FD;
#ifndef CPP_COMPILATION
#include "op_dd_fd.c"
#else
#include "op_dd_fd.cpp"
#endif
#undef  REGISTER
	    }
	  regs->we_are_on_ddfd = 0;
	  break;
	}

      /* patch ROM loading routine */
      // address contributed by Ignacio Burgue�o :)
//     if( r_PC == 0x0569 )
      if (r_PC >= 0x0556 && r_PC <= 0x056c)
	Z80Patch (regs);

      /* check if it's time to do other hardware emulation */
      if (regs->ICount <= 0) {
//	if (regs->petint==1) {
	  regs->petint=0;
/*	  tmpreg.W = Z80Hardware (regs); */ //Z80Hardware alwais return INT_NOINT
	  regs->ICount += regs->IPeriod;
	  loop = regs->ICount + loop;

	  /* check if we must exit the emulation or there is an INT */
/*	  if (tmpreg.W == INT_QUIT)
	    return (regs->PC.W);
	  if (tmpreg.W != INT_NOINT) */   
	    Z80Interrupt (regs, tmpreg.W);
	}
  }

  return (regs->PC.W);
}



/*====================================================================
  void Z80Interrupt( Z80Regs *regs, word ivec )
 ===================================================================*/
void
Z80Interrupt (Z80Regs * regs, word ivec)
{
  word intaddress;

  /* unhalt the computer */
  if (regs->halted == 1)
    regs->halted = 0;

  if (regs->IFF1)
    {
      PUSH (PC);
      regs->IFF1 = 0;
      switch (regs->IM)
	{
	case 0:
	  r_PC = 0x0038;
	  AddCycles (12);
	  break;
	case 1:
	  r_PC = 0x0038;
	  AddCycles (13);
	  break;
	case 2:
	  intaddress = (((regs->I & 0xFF) << 8) | 0xFF);
	  regs->PC.B.l = Z80ReadMem (intaddress);
	  regs->PC.B.h = Z80ReadMem (intaddress + 1);
	  AddCycles (19);
	  break;
	}

    }

}


/*====================================================================
  word  Z80Hardware(register Z80Regs *regs)

  Do here your emulated machine hardware emulation. Read Z80Execute()
  to know about how to quit emulation and generate interrupts.
 ===================================================================*/
word
Z80Hardware (register Z80Regs * regs)
{
  if (debug != 1 /*&& scanl >= 224 */ )
    {
      ;
    }
  return (INT_IRQ);
}


/*====================================================================
  void Z80Patch( register Z80Regs *regs )

  Write here your patches to some z80 opcodes that are quite related
  to the emulated machines (i.e. maybe accessing to the I/O ports
  and so on), such as ED_FE opcode:

     case ED_FE:     Z80Patch(regs);
                     break;

  This allows "BIOS" patching (cassette loading, keyboard ...).
 ===================================================================*/
void
Z80Patch (register Z80Regs * regs)
{
// QUE ALGUIEN ME EXPLIQUE por que hay dos tapfile ???
	extern FILE *tapfile;  
	extern tipo_emuopt emuopt;
  	if (emuopt.tapefile[0] != 0)
    {
	  //	AS_printf("Z80patch:%x\n",tapfile);
      LoadTAP (regs, tapfile);
      POP (PC);
    }

}


/*====================================================================
  byte Z80Debug( register Z80Regs *regs )

  This function is written for debugging purposes (it's supposed to
  be a debugger by itself!). It will debug a single opcode, given
  by the current PC address.

  Return DEBUG_OK to state success and DEBUG_QUIT to quit emulation.
 ===================================================================*/
byte
Z80Debug (register Z80Regs * regs)
{
  return (DEBUG_QUIT);
}



/*====================================================================
  byte Z80MemRead( register word address )

  This function reads from the given memory address. It is not inlined,
  and it's written for debugging purposes.
 ===================================================================*/
byte
Z80MemRead (register word address, Z80Regs * regs)
{
  return (Z80ReadMem (address));
}


/*====================================================================
  void Z80MemWrite( register word address, register byte value )

  This function writes on memory the given value. It is not inlined,
  ands it's written for debugging purposes.
 ===================================================================*/
void
Z80MemWrite (register word address, register byte value, Z80Regs * regs)
{
  Z80WriteMem (address, value, regs);
}


/*====================================================================
  byte Z80InPort(register Z80Regs *regs, eregister word port )

  This function reads from the given I/O port. It is not inlined,
  and it's written for debugging purposes.
 ===================================================================*/
byte
Z80InPort (register Z80Regs * regs, register word port)
{
//  int cur_tstate, tmp;
  int code = 0xFF;
  byte valor;
  int x, y;
  extern tipo_emuopt emuopt;
  extern tipo_hwopt hwopt;
  extern int v_border;
  int mousex, mousey, mouseb;

  /* El teclado */
  if (!(port & 0x01))
    {
      if (!(port & 0x0100))
	code &= fila[4][1];
      if (!(port & 0x0200))
	code &= fila[3][1];
      if (!(port & 0x0400))
	code &= fila[2][1];
      if (!(port & 0x0800))
	code &= fila[1][1];
      if (!(port & 0x1000))
	code &= fila[1][2];
      if (!(port & 0x2000))
	code &= fila[2][2];
      if (!(port & 0x4000))
	code &= fila[3][2];
      if (!(port & 0x8000))
	code &= fila[4][2];

      /* la gunstick */
      if (!(port & 0x1000) && (emuopt.gunstick & GS_GUNSTICK) &&
	  (emuopt.gunstick & GS_HAYMOUSE))
	{
    mousex = mouse_x();
    mousey = mouse_y();
    mouseb = mouse_b();
    
	  /* disparo de la gunstick con el raton. */
	  if (mouseb & 1)
	    code &= (0xFE);
	  /* Miramos a ver si los atributos son blanco  */
	  if ((mousex > 31) && (mousex < 287) &&
	      (mousey > v_border) && (mousey < (192 + v_border)))
	    {
	      x = (mousex - 32) / 8;
	      y = (mousey - 1 - v_border) / 8;
	      valor = Z80ReadMem (0x5800 + 32 * y + x);
	      if (((valor & 0x07) == 0x07) || ((valor & 0x38) == 0x38))
		code &= 0xFB;
	    }			// mouse x>....
	}			// port & 0x1000

      /*
         issue 2 emulation, thx to Raul Gomez!!!!!
         I should implement this also:
         if( !ear_on && mic_on )
         code &= 0xbf;
         where earon = bit 4 of the last OUT to the 0xFE port
         and   micon = bit 3 of the last OUT to the 0xFE port
       */
      code &= 0xbf;
    }

  /* in the main ula loop I change higest bit of hwopt.port_ff in function of scanline
 * so the port FF is emulate only when need (change betwen 0xFF y 0xEF) */
  if ((port & 0xFF) == 0xFF) {  
      if (hwopt.port_ff == 0xFF)
	     code = 0xFF;
      else {
	     code = rand ();
	     if (code == 0xFF) code = 0x00;
	 }

  }
/*
  {
      cur_tstate=spectrumZ80.IPeriod-spectrumZ80.ICount;

      // top border
      if (cur_tstate<hwopt.TSTATES_PER_LINE*hwopt.TOP_BORDER_LINES) 
         return 0xff;

      // bottom border
      if (spectrumZ80.ICount<hwopt.TSTATES_PER_LINE*hwopt.BOTTOM_BORDER_LINES) 
         return 0xff;

      tmp=cur_tstate%hwopt.TSTATES_PER_LINE;

      // left border
      if (tmp<hwopt.tstate_border_left) 
         return 0xff;

      // right border  
      if (tmp>hwopt.TSTATES_PER_LINE-hwopt.tstate_border_right) 
         return 0xff;	

      return rand()%0xfe;	// don't return ff!
  }
 */
  return (code);
}


/*====================================================================
  void Z80OutPort( register word port, register byte value )

  This function outs a value to a given I/O port. It is not inlined,
  and it's written for debugging purposes.
 ===================================================================*/
void
Z80OutPort (register Z80Regs * regs, register word port, register byte value)
{
  extern tipo_mem mem;
  extern tipo_hwopt hwopt;

/* INVES "Feature" emulation PENDING esto esta mal casi seguro.
 * basicamente cuando se escribe un puerto tambien se lee de la pagina 0 de ram
 * ante el comflicto se hace un and logico entre ellas
 */
  if (hwopt.hw_model== SPECMDL_INVES ) value &= mem.p[0x10000+(port & 0x3FFF)] ;

/* paginacion del 128K */ 
 
switch (hwopt.hw_model){
	case SPECMDL_128K:
	case SPECMDL_PLUS2:
		if ((port & 0x8002)==0x0000) outbankm_128k(value); // 0x7ffd
		break;
	case SPECMDL_PLUS3:
		if ((port & 0xC002)==0x4000) outbankm_p37(value); // 0x7ffd
		if ((port & 0xF002)==0x1000) outbankm_p31(value); // 0x1ffd
		break;
	}
if ((port & 0xC002)==0xC000) ay8912_outFF(value); // 0x7ffd 11xx xxxx xxxx xx0x
if ((port & 0xC002)==0x8000) ay8912_outBF(value); // 0xbffd 10xx xxxx xxxx xx0x

/* change border colour */
  if (!(port & 0x01)) {
      regs->BorderColor = (value & 0x07);
      logSound (value & 0x10);
    }
}



/*====================================================================
   static void Z80FlagTables ( void );

   Creates a look-up table for future flag setting...
   Taken from fuse's sources. Thanks to Philip Kendall.
 ===================================================================*/
void
Z80FlagTables (void)
{
  int i, j, k;
  byte parity;

  for (i = 0; i < 0x100; i++)
    {
      sz53_table[i] = i & (FLAG_3 | FLAG_5 | FLAG_S);
      j = i;
      parity = 0;
      for (k = 0; k < 8; k++)
	{
	  parity ^= j & 1;
	  j >>= 1;
	}
      parity_table[i] = (parity ? 0 : FLAG_P);
      sz53p_table[i] = sz53_table[i] | parity_table[i];
    }

  sz53_table[0] |= FLAG_Z;
  sz53p_table[0] |= FLAG_Z;
}
